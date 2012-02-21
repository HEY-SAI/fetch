/*
* Stage.cpp
*
*  Created on: Apr 19, 2010
*      Author: Nathan Clack <clackn@janelia.hhmi.org>
*/
/*
* Copyright 2010 Howard Hughes Medical Institute.
* All rights reserved.
* Use is subject to Janelia Farm Research Campus Software Copyright 1.1
* license terms (http://license.janelia.org/license/jfrc_copyright_1_1.html).
*/


#include <list>
#include <string>
#include <sstream>

#include "stage.h"
#include "task.h"
#include "tiling.h"
#include "thread.h"

#include "C843_GCS_DLL.H"


namespace fetch  {

  bool operator==(const cfg::device::C843StageController& a, const cfg::device::C843StageController& b) {return equals(&a,&b);}
  bool operator==(const cfg::device::SimulatedStage& a, const cfg::device::SimulatedStage& b)           {return equals(&a,&b);}
  bool operator==(const cfg::device::Stage& a, const cfg::device::Stage& b)                             {return equals(&a,&b);}

  bool operator!=(const cfg::device::C843StageController& a, const cfg::device::C843StageController& b) {return !(a==b);}
  bool operator!=(const cfg::device::SimulatedStage& a, const cfg::device::SimulatedStage& b)           {return !(a==b);}
  bool operator!=(const cfg::device::Stage& a, const cfg::device::Stage& b)                             {return !(a==b);}

namespace device {

     
  //
  // C843Stage
  //
  
#define C843WRN( expr )  {lock_(); (c843_error_handler( handle_, expr, #expr, __FILE__, __LINE__, warning )); unlock_();}
#define C843ERR( expr )  {BOOL v; lock_(); v=(expr); unlock_(); (c843_error_handler( handle_, v, #expr, __FILE__, __LINE__, error   ));}
#define C843JMP( expr )  {lock_(); if(c843_error_handler( handle_, expr, #expr, __FILE__, __LINE__, warning)) {unlock_(); goto Error;} unlock_();}
#define  CHKWRN( expr )  do {if(!(expr)) warning("[WARNING] C843:"ENDL "%s(%d): %s"ENDL "Expression evaluated as false"ENDL "%s"ENDL,__FILE__,__LINE__,#expr);} while(0)
#define  CHKERR( expr )  do {if(!(expr)) warning("[ERROR  ] C843:"ENDL "%s(%d): %s"ENDL "Expression evaluated as false"ENDL "%s"ENDL,__FILE__,__LINE__,#expr); goto Error;} while(0)


  // returns 0 if no error, otherwise 1
  bool c843_error_handler(long handle, BOOL ok, const char* expr, const char* file, const int line, pf_reporter report)
  {
    char buf[1024];
    long e;
    if(handle<0)
    { report(
        "(%s:%d) C843 Error:"ENDL
        "\t%s"ENDL
        "\tInvalid device id (Got %d)."ENDL,
        file,line,expr,handle);
      return true;
    }
    else if(!ok)
    { e = C843_GetError(handle);

      if(C843_TranslateError(e,buf,sizeof(buf)))
      {
        report(
          "(%s:%d) C843 Error %d:"ENDL
          "\t%s"ENDL
          "\t%s"ENDL,
          file,line,e,expr,buf);
      } else
        error(
          "(%s:%d) Problem with C843 (error %d) but error message was too long for buffer."ENDL,
          "\t%s"ENDL
          __FILE__,__LINE__,e,expr);
      return true;
    }
    return false; //this should be something corresponding to "no error"...guessing zero
  }

#pragma warning(push)
#pragma warning(disable:4244) // lots of float <-- double conversions

  /** Writes current position and velocity to a cache file with a time since last log.
      
      The velocity and time can be used to estimate an upper bound to the logged stage position.
      \returns 1 on success, 0 otherwise
  */
  int log_(double r[3], double v[3], double dt)
  { FILE *fp = NULL;
    CHKERR( fp=fopen("stage_position.f64"));
    fwrite(r ,sizeof(double),3,fp);
    fwrite(v ,sizeof(double),3,fp);
    fwrite(dt,sizeof(double),1,fp);
    fclose(fp);
    return 1;
Error:
    return 0;  
  }

  /** Stage position polling/logging thread.
      
      \param [in] A pointer to a C843Stage instance.
      
      The thread should be started after the stage is attached.
      The thread will stop when the position query fails, presumably because the stage's handle
      was closed.
      
      The cached file gets written in the working directory.
      It records a time between position queries,the stage velocity and the current position.
            
      \todo actually start the thread
      \todo use the cached position to initialize the stage position
  */
  static 
  void *poll_and_cache_stage_position(void* self_)
  { C843Stage *self = (C843Stage*)self_;
    double r[3]={0},v[3]={0},dt=0.0;
    TicTocTimer clock = tic();
    while(1)
    { Sleep(10);
      C843JMP(C843_qPOS(self->handle_,"123",r);      
      C843JMP(C843_qVEL(self->handle_,"123",r);
      t = toc(&clock);
      if(!log_(r,v,dt))
      { warning("%s(%d): C843 Failed to write to position log."ENDL,__FILE__,__LINE__);
        goto Error;
      }
    }
Error:
    return NULL;    
  }

  C843Stage::C843Stage( Agent *agent )
   :StageBase<cfg::device::C843StageController>(agent)
   ,handle_(-1)
  {
    InitializeCriticalSection(&c843_lock_);
  }

  C843Stage::C843Stage( Agent *agent, Config *cfg )
   :StageBase<cfg::device::C843StageController>(agent,cfg)
   ,handle_(-1)
  {
    InitializeCriticalSection(&c843_lock_);
  }

  C843Stage::~C843Stage()
  { DeleteCriticalSection(&c843_lock_);
  }
  
  void C843Stage::lock_()
  { EnterCriticalSection(&c843_lock_);
  }
  
  void C843Stage::unlock_()
  { LeaveCriticalSection(&c843_lock_);
  }
  
  /** The attach callback for this device.
      Connects to the controller, and tries to load axes.  Attempts to set the last known stage position, so the stage is 
      ready for absolute moves.
      \return 0 on success, 1 otherwise
      \todo Should do some validation.  Make sure stage name is in database.  I suppose CST does this though.
  */
  unsigned int C843Stage::on_attach()
  { 
    long ready = 0;   
    C843JMP( (handle_=C843_Connect(_config->id()))>=0 );
    
    for(int i=0;i<_config->axis_size();++i)
    { char id[10];
      const char *name;// = _config->axis(i).id().c_str(),
      itoa(_config->axis(i).id(),id,10);
      name = _config->axis(i).stage().c_str();
      C843JMP( C843_CST(handle_,id,name) );  //configure selected axes       
    }
    C843JMP( C843_INI(handle_,""));          //init all configured axes    
    waitForController_();
    
    reference_();
    return 0; // ok
Error:
    return 1; // fail
  }

  // return 0 on success
  unsigned int C843Stage::on_detach()
  {
    int ecode = 0;
    C843JMP( C843_STP(handle_) );  // all stop - HLT is more gentle
Finalize:    
    lock_();
    C843_CloseConnection(handle_); // always succeeds
    unlock_();
    handle_=-1;
    return ecode;
Error:
    ecode = 1;
    goto Finalize;
  }

  int C843Stage::getTravel(StageTravel* out)
  { 
    /* NOTES:
       o  qTMN and q TMX I think
       o  There's a section in the manual on travel range adjustment (section 5.4)
       o  Assume that (xyz) <==> "123"
       */
    double t[3];
    C843JMP( C843_qTMN(handle_,"123",t) );
    
    out->x.min = t[0];
    out->y.min = t[1];
    out->z.min = t[2];    

    C843JMP( C843_qTMX(handle_,"123",t) );
    out->x.max = t[0];
    out->y.max = t[1];
    out->z.max = t[2];   
    return 1;
Error:
    return 0;
  }

  int C843Stage::getVelocity( float *vx, float *vy, float *vz )
  { double t[3];
    C843JMP( C843_qVEL(handle_,"123",t) );
    *vx = t[0];
    *vy = t[1];
    *vz = t[2];
    return 1;
Error:
    return 0;
  }

  int C843Stage::setVelocity( float vx, float vy, float vz )
  { const double t[3] = {vx,vy,vz};
    C843JMP( C843_VEL(handle_,"123",t) );
    return 1; // success
Error:
    return 0;    
  }

  void C843Stage::setVelocityNoWait( float vx, float vy, float vz )
  { setVelocity(vx,vy,vz); // doesn't block
  }

  int C843Stage::getPos( float *x, float *y, float *z )
  { double t[3];
    C843JMP( C843_qPOS(handle_,"123",t) );
    *x = t[0];
    *y = t[1];
    *z = t[2];
    return 1;
Error:
    return 0;
  }

  int C843Stage::getTarget( float *x, float *y, float *z )
  { double t[3];
    C843JMP( C843_qMOV(handle_,"123",t) );
    *x = t[0];
    *y = t[1];
    *z = t[2];
    return 1;
Error:
    return 0;
  }

  static BOOL all(BOOL* bs, int n)
  { while(n--) if(!bs[n]) return 0;
    return 1;
  }         
  static BOOL any(BOOL *bs, int n)
  { BOOL ret = 0;
    while(n--) ret |= bs[n];
    return ret;
  }
  static int same(double *a, double *b, int n)
  { while(n-->0)
      if( fabs(a[n]-b[n])>1e-4 /*mm*/ )
        return 0;
    return 1;
  }
  

/** Moves the stage to the indicated position.  Will not return till finished (or error).
  \todo  better error handling in case I hit limits
  \todo  what is behavior around limits?
*/
  int C843Stage::setPos( float x, float y, float z )
  { double t[3] = {x,y,z};
    BOOL ontarget[] = {0,0,0};
  
    { float vx,vy,vz;
      getVelocity(&vx,&vy,&vz);
      debug("(%s:%d): C843 Velocity %f %f %f"ENDL,__FILE__,__LINE__,vx,vy,vz);
    }
    C843JMP( C843_HLT(handle_,"123") );              // Stop any motion in progress
    C843JMP( C843_MOV(handle_,"123",t) );            // Move!
    waitForMove_();                                  // Block here until not moving (or error)
    
    C843JMP( C843_qONT(handle_,"123",ontarget) );    // Ensure controller is on-target (if there was an error before, will repeat here?)
    if(!all(ontarget,3))
      goto Error;
      
    { double a[3];
      C843JMP( C843_qMOV(handle_,"123",a) );
      if(!same(a,t,3))
      { double b[3]={0.0,0.0,0.0};
        lock_();
        C843_qPOS(handle_,"123",b);
        unlock_();
        warning(
          "%s(%d): C843 Move command was interupted for a new destination."ENDL
          "\t  Original Target: %f %f %f"ENDL
          "\t   Current Target: %f %f %f"ENDL
          "\t Current Position: %f %f %f"ENDL,
          __FILE__,__LINE__,
          t[0],t[1],t[2],
          a[0],a[1],a[2],
          b[0],b[1],b[2]);
        goto Error;
      }
    }
    return 1; // success
Error:
    return 0;
  }
  
  void C843Stage::setPosNoWait( float x, float y, float z )
  { double t[3] = {x,y,z};
  
    { float vx,vy,vz;
      getVelocity(&vx,&vy,&vz);
      debug("(%s:%d): C843 Velocity %f %f %f"ENDL,__FILE__,__LINE__,vx,vy,vz);
    }
    C843JMP( C843_HLT(handle_,"123") );              // Stop any motion in progress
    C843JMP( C843_MOV(handle_,"123",t) );            // Move!
Error:
    return;
  }

  bool C843Stage::isMoving()
  { BOOL b;
    C843JMP( C843_IsMoving(handle_,"",&b) );
    return b;
Error:
    return false; // if there's an error state, it's probably not moving
  }


  bool C843Stage::isOnTarget()
  { BOOL b[] = {0,0,0};
    C843JMP( C843_qONT(handle_,"123",b) );
    if(all(b,3))
      return true;    
Error:
    return false; // if there's an error state, it's probably not on target
  }

  void C843Stage::waitForController_()
  { 
    long ready = 0;
    while(!ready)
    { C843ERR( C843_IsControllerReady(handle_,&ready) );
      Sleep(20); // check ~ 50x/sec
    }
  }    
  
  void C843Stage::waitForMove_()
  { 
    BOOL isMoving = TRUE;
    while(isMoving == TRUE)    
    { C843JMP( C843_IsMoving(handle_,"",&isMoving) );
      Sleep(20); // check ~ 50x/sec
    }    
Error:
    return;
  }
  
  bool C843Stage::setRefMode_(bool ison)
  { BOOL ons[3] = {ison,ison,ison};
    C843JMP(C843_RON(handle_,"123",ons));
    return 1;
Error:
    return 0;    
  }
  
  bool C843Stage::isReferenced(bool *isok/*=NULL*/)
  { BOOL isrefd[3] ={0,0,0};
    if(isok) *isok=1;
    C843JMP(C843_IsReferenceOK(handle_,"123",isrefd));
    return all(iusrefd,3);
Error:
    if(isok) *isok=0;
    return false;
  }
  
  bool C843Stage::reference()
  { bool isok=1;
    // Only reference if necessary
    if(isReferenced(&isok))  return true;
    if(!isok)                goto Error;    
    if(setRefMode_(1))       goto Error;
    C843JMP( C843_FRF(handle_,"123") );  //fast reference
    waitForController_();
    if(isReferenced(&isok) && isok)
      return true;
Error:
    warning("%s(%d)"ENDL "\tReferencing failed for one or more axes."ENDL,__FILE__,__LINE__);
    return false;
  }
  
  bool C843Stage::setKnownReference(float x, float y, float z)
  { double r[3] = {x,y,z};
    if(isReferenced(&isok))  return true;  // ignore if referenced
    if(!isok)                goto Error;
    if(setRefMode_(0))       goto Error;
    C843JMP(C843_POS(handle_,"123",r));
    if(isReferenced(&isok) && isok)
      return true;
Error:
    warning("%s(%d)"ENDL "\setKnownReference failed for one or more axes."ENDL,__FILE__,__LINE__);
    return false;    
  }
#pragma warning(pop) 
     
  //
  // Simulated
  //

  SimulatedStage::SimulatedStage( Agent *agent )
    :StageBase<cfg::device::SimulatedStage>(agent)
    ,x_(0.0f)
    ,y_(0.0f)
    ,z_(0.0f)
    ,vx_(0.0f)
    ,vy_(0.0f)
    ,vz_(0.0f)
  {}

  SimulatedStage::SimulatedStage( Agent *agent, Config *cfg )
    :StageBase<cfg::device::SimulatedStage>(agent,cfg)
    ,x_(0.0f)
    ,y_(0.0f)
    ,z_(0.0f)
    ,vx_(0.0f)
    ,vy_(0.0f)
    ,vz_(0.0f)
  {}
                              
  int SimulatedStage::getTravel( StageTravel* out )
  { 
    Config c = get_config();
    if(c.axis_size()<3)
    {
      memset(out,0,sizeof(StageTravel));
      return 1;
    }

    out->x.min = c.axis(0).min_mm();
    out->x.max = c.axis(0).max_mm();
    out->y.min = c.axis(1).min_mm();
    out->y.max = c.axis(1).max_mm();
    out->z.min = c.axis(2).min_mm();
    out->z.max = c.axis(2).max_mm();
    return 1;
  }

  int SimulatedStage::getVelocity( float *vx, float *vy, float *vz )
  { *vx=vx_;
    *vy=vy_;
    *vz=vz_;
    return 1;
  }

  int SimulatedStage::setVelocity( float vx, float vy, float vz )
  { vx_=vx;vy_=vy;vz_=vz;
    return 1;
  }

  int SimulatedStage::getPos( float *x, float *y, float *z )
  { *x=x_;
    *y=y_;
    *z=z_;
    return 1;
  }

  int SimulatedStage::setPos( float x, float y, float z )
  { x_=x;y_=y;z_=z;
    return 1;
  }

  //
  // Polymorphic Stage
  //

  Stage::Stage( Agent *agent )
    :StageBase<cfg::device::Stage>(agent)
    ,_c843(NULL)
    ,_simulated(NULL)
    ,_idevice(NULL)
    ,_istage(NULL)
    ,_tiling(NULL)
    ,_fov(NULL)
  {
      setKind(_config->kind());
  }

  Stage::Stage( Agent *agent, Config *cfg )
    :StageBase<cfg::device::Stage>(agent,cfg)
    ,_c843(NULL)
    ,_simulated(NULL)
    ,_idevice(NULL)
    ,_istage(NULL)
    ,_tiling(NULL)
    ,_fov(NULL)
  {
    setKind(_config->kind());
  }   

  void Stage::setKind( Config::StageType kind )
  {
    switch(kind)
    {    
    case cfg::device::Stage_StageType_C843:
      if(!_c843)
        _c843 = new C843Stage(_agent,_config->mutable_c843());
      _idevice  = _c843;
      _istage   = _c843;
      break;
    case cfg::device::Stage_StageType_Simulated:
      if(!_simulated)
        _simulated = new SimulatedStage(_agent,_config->mutable_simulated());
      _idevice = _simulated;
      _istage  = _simulated;
      break;
    default:
      error("Unrecognized kind() for SimulatedStage.  Got: %u\r\n",(unsigned)kind);
    }
  }

  void Stage::setFOV(FieldOfViewGeometry *fov)
  {
    if(!(_fov && _lastfov==(*fov)))
    {
      _fov=fov; 
      _lastfov=*fov; 
      _createTiling(); 
    } 
    _notifyFOVGeometryChanged();
  }

#define _IN(L,X,H) ( ((L)<=(X)) && ((X)<=(H)) )
#define INAXIS(NAME) _IN(t.NAME.min,NAME,t.NAME.max)
  unsigned int Stage::isPosValid( float x, float  y, float z)
  { 
    StageTravel t; 
    getTravel(&t);     
    return INAXIS(x)&&INAXIS(y)&&INAXIS(z); 
  }
#undef _IN
#undef INAXIS

  void Stage::_set_config( Config IN *cfg )
  {
    setKind(cfg->kind());
    Guarded_Assert(_c843||_simulated); // at least one device was instanced
    if(_c843)      _c843->_set_config(cfg->mutable_c843());
    if(_simulated) _simulated->_set_config(cfg->mutable_simulated());
    _config = cfg;
    
    setVelocity(_config->default_velocity_mm_per_sec());
  }

  void Stage::_set_config( const Config &cfg )
  {                        
      cfg::device::Stage_StageType kind = cfg.kind();
      _config->set_kind(kind);
      setKind(kind);
      switch(kind)
      {    
      case cfg::device::Stage_StageType_C843:
        _c843->_set_config(cfg.c843());
        break;
      case cfg::device::Stage_StageType_Simulated:    
        _simulated->_set_config(cfg.simulated());
        break;
      default:
        error("Unrecognized kind() for SimulatedStage.  Got: %u\r\n",(unsigned)kind);
      }
      _config->CopyFrom(cfg);
  }

  float myroundf(float x) {return floorf(x+0.5f);}
  
  Vector3z Stage::getPosInLattice()
  { StageTiling::TTransform l2s(_tiling->latticeToStageTransform());
    Vector3f r(getTarget()*1000.0); // convert to um
    return Vector3z((l2s.inverse() * r).unaryExpr(std::ptr_fun<float,float>(myroundf)).cast<size_t>());    
  }

  // Only use when attached but disarmed.
  void Stage::_createTiling()
  { device::StageTravel travel;    
    if(!getTravel(&travel))
      return;       
    if(_fov)                                                      // A FOV object is stored by the microscope and in the tiling.  The microscope's should be the reference
    {                                                             // not sure why I made this two distinct objects...
      FieldOfViewGeometry fov = *_fov;                            // this should always point to the microscope's FOV object (not the tiling's)
      _destroyTiling();                                           // this call will invalidate the _fov pointer :(  bad design [??? 2011-11 this comment seems questionable]
      _tiling = new StageTiling(travel, fov, _config->tilemode());
      //_fov = &_tiling->fov_;                                    // commented out 2011-11 - not sure why I had this there in the first place...
      { TListeners::iterator i;
        for(i=_listeners.begin();i!=_listeners.end();++i)
          _tiling->addListener(*i);
      }
      _notifyTilingChanged();
    }
  }

  void Stage::_destroyTiling()
  { if(_tiling)
    { StageTiling *t = _tiling;
      _tiling = NULL;
      _notifyTilingChanged();
      delete t;
    }
  }

  void Stage::addListener( StageListener *listener )
  { _listeners.insert(listener);
    if(_tiling) _tiling->addListener(listener);
  }

  void Stage::delListener( StageListener *listener )
  { _listeners.erase(listener);
    if(_tiling) _tiling->delListener(listener);
  }

  void Stage::_notifyTilingChanged()
  { TListeners::iterator i;
    for(i=_listeners.begin();i!=_listeners.end();++i)
      (*i)->tiling_changed(_tiling);  
  }

  void Stage::_notifyMoved()
  { TListeners::iterator i;
    for(i=_listeners.begin();i!=_listeners.end();++i)
      (*i)->moved();
  }
  
  void Stage::_notiveVelocityChanged()
  { TListeners::iterator i;
    for(i=_listeners.begin();i!=_listeners.end();++i)
      (*i)->velocityChanged();
  }

  void Stage::_notifyFOVGeometryChanged()
  {
    TListeners::iterator i;
    for(i=_listeners.begin();i!=_listeners.end();++i)
      (*i)->fov_changed(_fov);
  }

  unsigned int Stage::on_attach()
  {
    unsigned int eflag = _idevice->on_attach();
    if(eflag==0) _createTiling();
    return eflag;
  }

  unsigned int Stage::on_detach()
  {
    unsigned int eflag = _idevice->on_detach();
    if((eflag==0)&&_tiling)
      _destroyTiling();
    return eflag;
  }

  int  Stage::setPos(float  x,float  y,float  z)
  {
    int out = _istage->setPos(x,y,z);
    _config->mutable_last_target_mm()->set_x(x);
    _config->mutable_last_target_mm()->set_y(y);
    _config->mutable_last_target_mm()->set_z(z);
    _notifyMoved(); 
    return out;
  }

  void Stage::setPosNoWait(float  x,float  y,float  z)
  { 
#if 0                                                 
    Config c = get_config();                         // This block performs a "safe" commit
    c.mutable_last_target_mm()->set_x(x);            // of the last position, but will try to
    c.mutable_last_target_mm()->set_y(y);            // stop/restart runnign tasks.
    c.mutable_last_target_mm()->set_z(z);            //
    set_config_nowait(c);                            //
#else                                                //
    // directly set config                           //
    _config->mutable_last_target_mm()->set_x(x);     // This is not thread-safe.  Multiple threads
    _config->mutable_last_target_mm()->set_y(y);     // calling this at the same time might write 
    _config->mutable_last_target_mm()->set_z(z);     // inconsistent values, but that's not really a risk here.
#endif                                               // Better to avoid unneccesarily stopping a running task.

    _istage->setPosNoWait(x,y,z); 
    _notifyMoved();
  }
}} // end anmespace fetch::device

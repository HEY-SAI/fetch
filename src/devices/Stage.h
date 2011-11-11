#pragma once
#include <stdint.h>
#include "object.h"
#include "stage.pb.h" 
#include "agent.h"
#include <list>
#include <set>
#include "devices/FieldOfViewGeometry.h"

#include <Eigen/Core>
using namespace Eigen;


#define TODO_ERR   error("%s(%d) TODO"ENDL,__FILE__,__LINE__)
#define TODO_WRN warning("%s(%d) TODO"ENDL,__FILE__,__LINE__)

namespace fetch {

  bool operator==(const cfg::device::C843StageController& a, const cfg::device::C843StageController& b);
  bool operator==(const cfg::device::SimulatedStage& a, const cfg::device::SimulatedStage& b);
  bool operator==(const cfg::device::Stage& a, const cfg::device::Stage& b);

  bool operator!=(const cfg::device::C843StageController& a, const cfg::device::C843StageController& b);
  bool operator!=(const cfg::device::SimulatedStage& a, const cfg::device::SimulatedStage& b);
  bool operator!=(const cfg::device::Stage& a, const cfg::device::Stage& b);

namespace device {

  class StageTiling;
  typedef Matrix<size_t,1,3>           Vector3z;

  struct StageAxisTravel  { float min,max; };
  struct StageTravel      { StageAxisTravel x,y,z; };




  //////////////////////////////////////////////////////////////////////
  //  Stage  ///////////////////////////////////////////////////////////
  //////////////////////////////////////////////////////////////////////
  struct TilePos
  { float x,y,z;
  };
  
  class IStage
  {
    public:      
      virtual int  getTravel         ( StageTravel* out)                = 0;
      virtual int  getVelocity       ( float *vx, float *vy, float *vz) = 0;
      virtual int  setVelocity       ( float vx, float vy, float vz)    = 0;
      inline  int  setVelocity       ( const Vector3f &r)                   {return setVelocity(r[0],r[1],r[2]);}
      inline  int  setVelocity       ( float v )                            {return setVelocity(v,v,v); }
      virtual void setVelocityNoWait ( float vx, float vy, float vz)    = 0;
      inline  void setVelocityNoWait ( const Vector3f &r)                   {setVelocityNoWait(r[0],r[1],r[2]);}
      inline  void setVelocityNoWait ( float v )                            {setVelocityNoWait(v,v,v); }
      virtual int  getPos            ( float *x, float *y, float *z)    = 0;
      virtual int  getTarget         ( float *x, float *y, float *z)    = 0;
      inline  Vector3f getPos        ()                                     {float x,y,z; getPos(&x,&y,&z); return Vector3f(x,y,z);}
      virtual int  setPos            ( float  x, float  y, float  z)    = 0;
      virtual int  setPos            ( const Vector3f &r)                   {return setPos(r[0],r[1],r[2]);}
      virtual int  setPos            ( const TilePos &r)                    {return setPos(r.x,r.y,r.z);}
      virtual void setPosNoWait      ( float  x, float  y, float  z)    = 0;
      inline  void setPosNoWait      ( const Vector3f &r)                   {setPosNoWait(r[0],r[1],r[2]);}      
      virtual bool isMoving          () = 0;
      virtual bool isOnTarget        () = 0;

      //Move Relative
  };

  template<class T>
    class StageBase:public IStage, public IConfigurableDevice<T>
  {
    public:
      StageBase(Agent *agent) : IConfigurableDevice<T>(agent) {}
      StageBase(Agent *agent, Config *cfg) :IConfigurableDevice<T>(agent,cfg) {}
  };

  class C843Stage:public StageBase<cfg::device::C843StageController>
  {
    public:
      C843Stage(Agent *agent);
      C843Stage(Agent *agent, Config *cfg);
      virtual ~C843Stage();

      virtual unsigned int on_attach();
      virtual unsigned int on_detach();

      virtual int  getTravel         ( StageTravel* out);
      virtual int  getVelocity       ( float *vx, float *vy, float *vz);
      virtual int  setVelocity       ( float vx, float vy, float vz);
      virtual void setVelocityNoWait ( float vx, float vy, float vz);
      virtual int  getPos            ( float *x, float *y, float *z);
      virtual int  getTarget         ( float *x, float *y, float *z);
      virtual int  setPos            ( float  x, float  y, float  z);
      virtual void setPosNoWait      ( float  x, float  y, float  z);
      virtual bool isMoving          ();
      virtual bool isOnTarget        ();
    private:
     int                handle_;
     CRITICAL_SECTION   c843_lock_;
     
     void lock_();
     void unlock_();
     void waitForController_();
     void waitForMove_();
     void reference_();
  };

  class SimulatedStage:public StageBase<cfg::device::SimulatedStage>
  {   float x_,y_,z_,vx_,vy_,vz_;
    public:
      SimulatedStage(Agent *agent);
      SimulatedStage(Agent *agent, Config *cfg);  

      virtual unsigned int on_attach() {/**TODO**/return 0;}
      virtual unsigned int on_detach() {/**TODO**/return 0;}

      virtual int  getTravel         ( StageTravel* out);
      virtual int  getVelocity       ( float *vx, float *vy, float *vz);
      virtual int  setVelocity       ( float vx, float vy, float vz);
      virtual void setVelocityNoWait ( float vx, float vy, float vz)       {setVelocity(vx,vy,vz);}
      virtual int  getPos            ( float *x, float *y, float *z);
      virtual int  getTarget         ( float *x, float *y, float *z)       {return getPos(x,y,z);}
      virtual int  setPos            ( float  x, float  y, float  z);
      virtual void setPosNoWait      ( float  x, float  y, float  z)       {setPos(x,y,z);}
      virtual bool isMoving          () {return 0;}
      virtual bool isOnTarget        () {return 1;}
  };

  class StageListener;
  class Stage:public StageBase<cfg::device::Stage>
  {
  public:
    typedef std::list<TilePos> TilePosList;

  private:
    typedef std::set<StageListener*>  TListeners;

    C843Stage           *_c843;
    SimulatedStage      *_simulated;
    IDevice             *_idevice;
    IStage              *_istage;
    StageTiling         *_tiling;
    TListeners           _listeners;
    FieldOfViewGeometry *_fov;
    FieldOfViewGeometry  _lastfov;
    
    public:
      Stage(Agent *agent);
      Stage(Agent *agent, Config *cfg);
      
      void setKind(Config::StageType kind);
      void setFOV(FieldOfViewGeometry *fov);

      virtual unsigned int on_attach();
      virtual unsigned int on_detach();
      void _set_config( Config IN *cfg );
      void _set_config( const Config &cfg );

      virtual int  getTravel         ( StageTravel* out)                    {return _istage->getTravel(out);}
      virtual int  getVelocity       ( float *vx, float *vy, float *vz)     {return _istage->getVelocity(vx,vy,vz);}
      virtual int  setVelocity       ( float vx, float vy, float vz)        {return _istage->setVelocity(vx,vy,vz);}      
      virtual int  setVelocity       ( float v )                            {return setVelocity(v,v,v);}
      virtual int  setVelocity       ( const cfg::device::Point3d &v)       {return setVelocity(v.x(),v.y(),v.z());}
      virtual void setVelocityNoWait ( float vx, float vy, float vz)        {_istage->setVelocityNoWait(vx,vy,vz);}
      virtual int  getPos            ( float *x, float *y, float *z)        {return _istage->getPos(x,y,z);}      
      inline  Vector3f getPos        ()                                     {float x,y,z; getPos(&x,&y,&z); return Vector3f(x,y,z);}
      virtual int  getTarget         ( float *x, float *y, float *z)        {return _istage->getTarget(x,y,z);}
      inline  Vector3f getTarget     ()                                     {float x,y,z; getTarget(&x,&y,&z); return Vector3f(x,y,z);}
      virtual int  setPos            ( float  x, float  y, float  z);
      virtual void setPosNoWait      ( float  x, float  y, float  z);
      virtual int  setPos            ( const Vector3f &r)                   {return setPos(r[0],r[1],r[2]);}
      virtual int  setPos            ( const TilePos &r)                    {return setPos(r.x,r.y,r.z);}
      virtual int  setPos            ( const TilePosList::iterator &cursor) {return setPos(*cursor);}
      virtual bool isMoving          ()                                     {return _istage->isMoving();}
      virtual bool isOnTarget        ()                                     {return _istage->isOnTarget();};
     
      unsigned int isPosValid        ( float  x, float  y, float  z);
      
      Vector3z getPosInLattice();
      void     getLastTarget         ( float *x, float *y, float *z)        { cfg::device::Point3d r=_config->last_target_mm(); *x=r.x();*y=r.y();*z=r.z(); }
  
              void addListener(StageListener *listener);
              void delListener(StageListener *listener);
      inline  StageTiling* tiling()                                         {return _tiling;}
  protected:
      void    _createTiling();       //only call when disarmed
      void    _destroyTiling();      //only call when disarmed
      void    _notifyTilingChanged();
      void    _notifyMoved();
      void    _notiveVelocityChanged();
      void    _notifyFOVGeometryChanged();
  };

  //////////////////////////////////////////////////////////////////////
  //  StageListener ////////////////////////////////////////////////////
  //////////////////////////////////////////////////////////////////////
  //
  // Allows other objects to respond to stage events
  // o  Defines a set of callbacks that will be called for certain events.
  // o  By default, callbacks do nothing so derived listeners only need
  //    to overload the callbacks they want to listen to.
  //
  // When implementing remember that these are usually called from 
  // a thread running the acquisition process.
  // 
  // o  don't block!
  // 
  // NOTES:
  // maybe I should use an APC to launch the callback from another thread 
  // so that the acquisition loop is guaranteed not to block
  // 
  // Not worrying about this right now since stage motion doesn't occur
  // during super time sensitive parts of the loop.  Still though...
  // 
  // Actually, Qt supports both non-blocking and blocking queued connections
  // (I'm primarily worried about signal/slot communication with the GUI
  // frontend here).  Using the QueuedConncection, which is non-blocking,
  // means the callback shouldn't be blocked.  I can leave the blocking
  // responsibility up to the callback.

  class StageListener
  {
  public:
    virtual void tiling_changed(StageTiling *tiling) {}                      // a new tiling was created.
    virtual void tile_done(size_t index, const Vector3f& pos,uint32_t sts) {}// the specified tile was marked as done                                                      
    virtual void tile_next(size_t index, const Vector3f& pos) {}             // the next tile was requested (stage not necessarily moved yet)

    virtual void fov_changed(const FieldOfViewGeometry *fov) {}
    virtual void moved() {}
    virtual void velocityChanged() {}
  };

  // end namespace fetch::Device
}}


#pragma once
#include "object.h"
#include "stage.pb.h" 
#include "agent.h"
#include <list>
#include <set>

#include <Eigen/Core>
using namespace Eigen;

namespace fetch {
namespace device {

  class StageTiling;

  struct StageAxisTravel  { float min,max; };
  struct StageTravel      { StageAxisTravel x,y,z; };


  //////////////////////////////////////////////////////////////////////
  //  Stage  ///////////////////////////////////////////////////////////
  //////////////////////////////////////////////////////////////////////

  class IStage
  {
    public:      
      virtual void getTravel         ( StageTravel* out)                = 0;
      virtual void getVelocity       ( float *vx, float *vy, float *vz) = 0;
      virtual int  setVelocity       ( float vx, float vy, float vz)    = 0;
      virtual void setVelocityNoWait ( float vx, float vy, float vz)    = 0;
      virtual void getPos            ( float *x, float *y, float *z)    = 0;
      virtual int  setPos            ( float  x, float  y, float  z)    = 0;
      virtual void setPosNoWait      ( float  x, float  y, float  z)    = 0;

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

      virtual unsigned int on_attach() {/**TODO**/ return 0;}
      virtual unsigned int on_detach() {/**TODO**/ return 0;}

      virtual void getTravel         ( StageTravel* out)                   {/**TODO**/}
      virtual void getVelocity       ( float *vx, float *vy, float *vz);
      virtual int  setVelocity       ( float vx, float vy, float vz);
      virtual void setVelocityNoWait ( float vx, float vy, float vz)       {Config c = get_config(); /**TODO**/ Guarded_Assert_WinErr(set_config_nowait(c));}
      virtual void getPos            ( float *x, float *y, float *z);
      virtual int  setPos            ( float  x, float  y, float  z);
      virtual void setPosNoWait      ( float  x, float  y, float  z)       {Config c = get_config(); /**TODO**/ Guarded_Assert_WinErr(set_config_nowait(c));}
  };

  class SimulatedStage:public StageBase<cfg::device::SimulatedStage>
  {   float x_,y_,z_,vx_,vy_,vz_;
    public:
      SimulatedStage(Agent *agent);
      SimulatedStage(Agent *agent, Config *cfg);  

      virtual unsigned int on_attach() {/**TODO**/ return 0;}
      virtual unsigned int on_detach() {/**TODO**/ return 0;}

      virtual void getTravel         ( StageTravel* out);
      virtual void getVelocity       ( float *vx, float *vy, float *vz);
      virtual int  setVelocity       ( float vx, float vy, float vz);
      virtual void setVelocityNoWait ( float vx, float vy, float vz)       {setVelocity(vx,vy,vz);}
      virtual void getPos            ( float *x, float *y, float *z);
      virtual int  setPos            ( float  x, float  y, float  z);
      virtual void setPosNoWait      ( float  x, float  y, float  z)       {setPos(x,y,z);}
  };

  struct TilePos
  { float x,y,z;
  };

  struct StageListener;
  class Stage:public StageBase<cfg::device::Stage>
  {
  public:
    typedef std::list<TilePos> TilePosList;

  private:
    typedef std::set<StageListener*>  TListeners;

    C843Stage          *_c843;
    SimulatedStage     *_simulated;
    IDevice            *_idevice;
    IStage             *_istage;
    StageTiling        *_tiling;
    TListeners          _listeners;
    
    public:
      Stage(Agent *agent);
      Stage(Agent *agent, Config *cfg);

      void setKind(Config::StageType kind);

      virtual unsigned int on_attach();
      virtual unsigned int on_detach();
      void _set_config( Config IN *cfg );
      void _set_config( const Config &cfg );

      virtual void getTravel         ( StageTravel* out)                    {_istage->getTravel(out);}
      virtual void getVelocity       ( float *vx, float *vy, float *vz)     {_istage->getVelocity(vx,vy,vz);}
      virtual int  setVelocity       ( float vx, float vy, float vz)        {return _istage->setVelocity(vx,vy,vz);}
      virtual void setVelocityNoWait ( float vx, float vy, float vz)        {_istage->setVelocityNoWait(vx,vy,vz);}
      virtual void getPos            ( float *x, float *y, float *z)        {_istage->getPos(x,y,z);}
      virtual int  setPos            ( float  x, float  y, float  z)        {int out = _istage->setPos(x,y,z); _notifyMoved(); return out;}
      virtual int  setPos            ( const Vector3f &r)                   {return setPos(r[0],r[1],r[2]);}
      virtual int  setPos            ( const TilePos &r)                    {return setPos(r.x,r.y,r.z);}
      virtual int  setPos            ( const TilePosList::iterator &cursor) {return setPos(*cursor);}
      virtual void setPosNoWait      ( float  x, float  y, float  z)        {_istage->setPosNoWait(x,y,z);}
  
              void addListener(StageListener *listener);
              void delListener(StageListener *listener);
      inline  StageTiling* tiling()                                         {return _tiling;}
  protected:
      void    _createTiling();       //only call when disarmed
      void    _notifyTilingChanged();
      void    _notifyMoved();
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

    virtual void moved() {}
  };

  // end namespace fetch::Device
}}


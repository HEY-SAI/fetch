/*
* LinearScanMirror.cpp
*
*  Created on: Apr 20, 2010
*      Author: Nathan Clack <clackn@janelia.hhmi.org
*/
/*
* Copyright 2010 Howard Hughes Medical Institute.
* All rights reserved.
* Use is subject to Janelia Farm Research Campus Software Copyright 1.1
* license terms (http://license.janelia.org/license/jfrc_copyright_1_1.html).
*/

#include "stdafx.h"
#include "LinearScanMirror.h"

namespace fetch {
  namespace device {

    //
    // NIDAQLinearScanMirror
    //

    NIDAQLinearScanMirror::NIDAQLinearScanMirror(Agent *agent)
      :LSMBase<Config>(agent)
      ,daq(agent,"NIDAQLinearScanMirror")
    {}

    NIDAQLinearScanMirror::NIDAQLinearScanMirror(Agent *agent,Config *cfg)
      :LSMBase<Config>(agent,cfg)
      ,daq(agent,"NIDAQLinearScanMirror")
    {}

    void NIDAQLinearScanMirror::computeSawtooth( float64 *data, int n )
    {      
      double N = n;
      float64 A = _config->vpp();
      for(int i=0;i<n;++i)
        data[i] = A*((i/N)-0.5); // linear ramp from -A/2 to A/2
      data[n-1] = data[0];  // at end of wave, head back to the starting position
    }

    //
    // SimulatedLinearScanMirror
    //

    SimulatedLinearScanMirror::SimulatedLinearScanMirror( Agent *agent )
      :LSMBase<Config>(agent)
      ,_chan(agent,"LSM")
    {}

    SimulatedLinearScanMirror::SimulatedLinearScanMirror( Agent *agent, Config *cfg )
      :LSMBase<Config>(agent,cfg)
      ,_chan(agent,"LSM")
    {}

    void SimulatedLinearScanMirror::computeSawtooth( float64 *data, int n )
    {
      memset(data,0,sizeof(float64)*n);
    }

    //
    // LinearScanMirror
    //

    LinearScanMirror::LinearScanMirror( Agent *agent )
      :LSMBase<cfg::device::LinearScanMirror>(agent)
      ,_nidaq(NULL)
      ,_simulated(NULL)
      ,_idevice(NULL)
      ,_ilsm(NULL)
    {
       setKind(_config->kind());
    }

    LinearScanMirror::LinearScanMirror( Agent *agent, Config *cfg )
      :LSMBase<cfg::device::LinearScanMirror>(agent,cfg)
      ,_nidaq(NULL)
      ,_simulated(NULL)
      ,_idevice(NULL)
      ,_ilsm(NULL)
    {
      setKind(_config->kind());
    }

    LinearScanMirror::~LinearScanMirror()
    {
      if(_nidaq){delete _nidaq; _nidaq=NULL;}
      if(_simulated){delete _simulated; _simulated=NULL;}
    }

    void LinearScanMirror::setKind( Config::LinearScanMirrorType kind )
    {
      switch(kind)
      {    
      case cfg::device::LinearScanMirror_LinearScanMirrorType_NIDAQ:
        if(!_nidaq)
          _nidaq = new NIDAQLinearScanMirror(_agent,_config->mutable_nidaq());
        _idevice  = _nidaq;
        _ilsm = _nidaq;
        break;
      case cfg::device::LinearScanMirror_LinearScanMirrorType_Simulated:
        if(!_simulated)
          _simulated = new SimulatedLinearScanMirror(_agent);
        _idevice  = _simulated;
        _ilsm = _simulated;
        break;
      default:
        error("Unrecognized kind() for LinearScanMirror.  Got: %u\r\n",(unsigned)kind);
      }
    }

    void LinearScanMirror::_set_config( Config IN *cfg )
    {
      setKind(cfg->kind());
      Guarded_Assert(_nidaq||_simulated); // at least one device was instanced
      if(_nidaq)     _nidaq->_set_config(cfg->mutable_nidaq());
      if(_simulated) _simulated->_set_config(cfg->mutable_simulated());
      _config = cfg;
    }

    void LinearScanMirror::_set_config( const Config &cfg )
    {
      cfg::device::LinearScanMirror_LinearScanMirrorType kind = cfg.kind();
      _config->set_kind(kind);
      setKind(kind);
      switch(kind)
      {    
      case cfg::device::LinearScanMirror_LinearScanMirrorType_NIDAQ:
        _nidaq->_set_config(cfg.nidaq());
        break;
      case cfg::device::LinearScanMirror_LinearScanMirrorType_Simulated:    
        _simulated->_set_config(cfg.simulated());
        break;
      default:
        error("Unrecognized kind() for LinearScanMirror.  Got: %u\r\n",(unsigned)kind);
      }
    }

    void LinearScanMirror::computeSawtooth( float64 *data, int n )
    {
      _ilsm->computeSawtooth(data,n);
    }

    //end namespace fetch::device
  }
}
/*
* Pockels.cpp
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

#include "stdafx.h"
#include "Pockels.h"
#include "../task.h"

#define DAQWRN( expr )        (Guarded_DAQmx( (expr), #expr, warning))
#define DAQERR( expr )        (Guarded_DAQmx( (expr), #expr, error  ))
#define DAQJMP( expr )        goto_if_fail( 0==DAQWRN(expr), Error)

namespace fetch  {
namespace device {

  //
  // NIDAQPockels
  //

  NIDAQPockels::NIDAQPockels(Agent *agent)
    :PockelsBase<cfg::device::NIDAQPockels>(agent)
    ,daq(agent,"Pockels")
  {}    

  NIDAQPockels::NIDAQPockels(Agent *agent, Config *cfg )
    :PockelsBase<cfg::device::NIDAQPockels>(agent,cfg)
    ,daq(agent,"NIDAQPockels")
  {}    

  NIDAQPockels::~NIDAQPockels()
  {}

  int
    NIDAQPockels::isValidOpenVolts(f64 volts)
  { return ( volts >= _config->v_lim_min() ) && (volts <= _config->v_lim_max() );
  }

  int
    NIDAQPockels::setOpenVolts(f64 volts)
  { 
    int sts = 0; //fail
    transaction_lock();
    if(sts=isValidOpenVolts(volts))
    { _config->set_v_open(volts);
    update();        
    }
    else
      warning("NIDAQPockels: attempted to set v_open to an out of bounds value.\r\n");
    transaction_unlock();
    return sts;
  }

  int NIDAQPockels::setOpenVoltsNoWait(f64 volts)
  { 
    int sts = 0; //fail
    Config cfg = get_config();
    if(sts=isValidOpenVolts(volts))
    { 
      cfg.set_v_open(volts);
      set_config_nowait(cfg);       
    }
    else
      warning("NIDAQPockels: attempted to set v_open to an out of bounds value.\r\n");
    return sts;
  }

  void NIDAQPockels::computeVerticalBlankWaveform( float64 *data, int n )
  {
    float64 max = _config->v_open(),
            min = _config->v_closed();
    for(int i=0;i<n;++i)
      data[i] = max;           // step to max during y scan
    data[n-1] = min;      // step to zero at end of scan
  }

  //
  // SimulatedPockels
  //

  SimulatedPockels::SimulatedPockels( Agent *agent )
    :PockelsBase<cfg::device::SimulatedPockels>(agent)
    ,_chan(agent,"Pockels")
  {}

  SimulatedPockels::SimulatedPockels( Agent *agent, Config *cfg )
    :PockelsBase<cfg::device::SimulatedPockels>(agent,cfg)
    ,_chan(agent,"Pockels")
  {}

  int SimulatedPockels::isValidOpenVolts( f64 volts )
  {
    return 1;
  }

  int SimulatedPockels::setOpenVolts( f64 volts )
  {
    Config c = get_config();
    c.set_val(volts);
    set_config(c);
    return 1;
  }

  int SimulatedPockels::setOpenVoltsNoWait( f64 volts )
  {
    Config c = get_config();
    c.set_val(volts);
    return set_config_nowait(c);    
  }

  void SimulatedPockels::computeVerticalBlankWaveform( float64 *data, int n )
  {
    memset(data,0,sizeof(float64)*n);
  }

  //
  // Pockels
  //

  Pockels::Pockels( Agent *agent )
    :PockelsBase<cfg::device::Pockels>(agent)
    ,_nidaq(NULL)
    ,_simulated(NULL)
    ,_idevice(NULL)
    ,_ipockels(NULL)
  {
    setKind(_config->kind());
  }

  Pockels::Pockels( Agent *agent, Config *cfg )
    :PockelsBase<cfg::device::Pockels>(agent,cfg)
    ,_nidaq(NULL)
    ,_simulated(NULL)
    ,_idevice(NULL)
    ,_ipockels(NULL)
  {
    setKind(cfg->kind());
  }

  Pockels::~Pockels()
  {
    if(_nidaq)     { delete _nidaq;     _nidaq=NULL; }
    if(_simulated) { delete _simulated; _simulated=NULL; }
  }

  void Pockels::setKind( Config::PockelsType kind )
  {
    switch(kind)
    {    
    case cfg::device::Pockels_PockelsType_NIDAQ:
      if(!_nidaq)
        _nidaq = new NIDAQPockels(_agent,_config->mutable_nidaq());
      _idevice  = _nidaq;
      _ipockels = _nidaq;
      break;
    case cfg::device::Pockels_PockelsType_Simulated:    
      if(!_simulated)
        _simulated = new SimulatedPockels(_agent);
      _idevice  = _simulated;
      _ipockels = _simulated;
      break;
    default:
      error("Unrecognized kind() for Pockels.  Got: %u\r\n",(unsigned)kind);
    }
  }

  void Pockels::_set_config( Config IN *cfg )
  {
    _nidaq->_set_config(cfg->mutable_nidaq());
    _simulated->_set_config(cfg->mutable_simulated());;
    _config = cfg;
    setKind(cfg->kind());
  }

  void Pockels::_set_config( const Config &cfg )
  {
    cfg::device::Pockels_PockelsType kind = cfg.kind();
    _config->set_kind(kind);
    setKind(kind);
    switch(kind)
    {    
    case cfg::device::Pockels_PockelsType_NIDAQ:
      _nidaq->_set_config(cfg.nidaq());
      break;
    case cfg::device::Pockels_PockelsType_Simulated:    
      _simulated->_set_config(cfg.simulated());
      break;
    default:
      error("Unrecognized kind() for Pockels.  Got: %u\r\n",(unsigned)kind);
    }
  }

  int Pockels::isValidOpenVolts( f64 volts )
  {
    Guarded_Assert(_ipockels);
    return _ipockels->isValidOpenVolts(volts);
  }

  int Pockels::setOpenVolts( f64 volts )
  {
    Guarded_Assert(_ipockels);
    return _ipockels->isValidOpenVolts(volts);
  }

  int Pockels::setOpenVoltsNoWait( f64 volts )
  {
    Guarded_Assert(_ipockels);
    return _ipockels->setOpenVoltsNoWait(volts);
  }

  unsigned int Pockels::attach()
  {
    Guarded_Assert(_idevice);
    return _idevice->attach();
  }

  unsigned int Pockels::detach()
  {
    Guarded_Assert(_idevice);
    return _idevice->detach();
  }

} //end device namespace
}   //end fetch  namespace

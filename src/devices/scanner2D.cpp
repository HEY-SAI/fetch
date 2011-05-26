/*
 * Scanner2D.cpp
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
#include "common.h"
#include "scanner2D.h"
#include "frame.h"

#define DIGWRN( expr )  (niscope_chk( vi, expr, #expr, warning ))
#define DIGERR( expr )  (niscope_chk( vi, expr, #expr, error   ))
#define DIGJMP( expr )  goto_if_fail(VI_SUCCESS == niscope_chk( vi, expr, #expr, warning ), Error)
#define DAQWRN( expr )  (Guarded_DAQmx( (expr), #expr, warning))
#define DAQERR( expr )  (Guarded_DAQmx( (expr), #expr, error  ))
#define DAQJMP( expr )  goto_if_fail( 0==DAQWRN(expr), Error)

namespace fetch
{

  namespace device
  {
  

    Scanner2D::Scanner2D( Agent *agent )
      :IConfigurableDevice<Config>(agent)
      ,_digitizer(agent)
      ,_daq(agent)
      ,_shutter(agent)
      ,_LSM(agent)
      ,_pockels(agent)
    {
      _set_config(_config);
      __common_setup();
    }

    Scanner2D::Scanner2D( Agent *agent, Config *cfg )
      :IConfigurableDevice<Config>(agent,cfg)
      ,_digitizer(agent,cfg->mutable_digitizer())
      ,_daq(agent,cfg->mutable_daq())
      ,_shutter(agent,cfg->mutable_shutter())
      ,_LSM(agent,cfg->mutable_linear_scan_mirror())
      ,_pockels(agent,cfg->mutable_pockels())

    {
      __common_setup();
    }

    unsigned int Scanner2D::on_attach()
    {
      unsigned int eflag = 0; //success = 0, error = 1
      return_val_if(eflag |= _shutter.on_attach()  ,eflag);
      return_val_if(eflag |= _digitizer.on_attach(),eflag);
      return_val_if(eflag |= _daq.on_attach()      ,eflag);
      return_val_if(eflag |= _LSM.on_attach()      ,eflag);
      return_val_if(eflag |= _pockels.on_attach()  ,eflag);

      if(!_out)
      { 
        // Guess for how the output will be sized for allocation.
        FrmFmt fmt(
          (u16)_digitizer.record_size(_config->frequency_hz(),_config->line_duty_cycle()),
          (u16)_config->nscans(),
          (u8)_digitizer.nchan(),
          id_u16);
        _alloc_qs_easy(&_out,1,4,fmt.size_bytes());
      }

      return eflag;
    }

    unsigned int Scanner2D::on_detach()
    {      
      unsigned int eflag = 0; //success = 0, error = 1
      return_val_if(eflag |= _shutter.on_detach()  ,eflag);
      return_val_if(eflag |= _digitizer.on_detach(),eflag);
      return_val_if(eflag |= _daq.on_detach()      ,eflag);
      return_val_if(eflag |= _LSM.on_detach()      ,eflag);
      return_val_if(eflag |= _pockels.on_detach()  ,eflag);
      return eflag;
    }

    void Scanner2D::_set_config( Config IN *cfg )
    {
      // Changing where the Config data is referenced
      // Propagate the change to sub-devices.
      _digitizer._set_config(cfg->mutable_digitizer());
      _daq._set_config(cfg->mutable_daq());
      _shutter._set_config(cfg->mutable_shutter());
      _LSM._set_config(cfg->mutable_linear_scan_mirror());
      _pockels._set_config(cfg->mutable_pockels());
      _config = cfg;
    }

    void Scanner2D::_set_config( const Config& cfg )
    {
      // Copies the input config.
      // polymorphic devices need to get notified in case their "kind" changed
      *_config = cfg;
      _digitizer.setKind(_config->digitizer().kind());
      _daq.setKind(_config->daq().kind());
      _shutter.setKind(_config->shutter().kind());
      _LSM.setKind(_config->linear_scan_mirror().kind());
      _pockels.setKind(_config->pockels().kind());
    }

    void Scanner2D::onConfigTask()
    {
      float64 nscans       = _config->nscans(),
              scan_freq_Hz = _config->frequency_hz();
      IDAQChannel *chans[] = {
        _LSM.physicalChannel(),
        _pockels.physicalChannel()
      };
      _daq.setupCLK(nscans,scan_freq_Hz);
      _daq.setupAO(nscans,scan_freq_Hz);
      _daq.setupAOChannels(nscans,scan_freq_Hz,-10,10,chans,2);

      _shutter.Shut();

      _digitizer.setup((int)nscans,scan_freq_Hz,_config->line_duty_cycle());
    }

    void Scanner2D::generateAO()
    { 
      int N = _daq.samplesPerRecordAO();
      transaction_lock();
      vector_f64_request(_ao_workspace,2*N-1/*max index*/);
      f64 *m = _ao_workspace->contents,
          *p = m+N;
      _LSM.computeSawtooth(m,N);
      _pockels.computeVerticalBlankWaveform(p,N);
      transaction_unlock();
    }

    void Scanner2D::__common_setup()
    {
      _ao_workspace = vector_f64_alloc(_daq.samplesPerRecordAO()*2);
    }

    void Scanner2D::writeAO()
    {
      _daq.writeAO(_ao_workspace->contents);
    }

  }
}

/*
 * Scanner3D.cpp
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
#include "StdAfx.h"
#include "Scanner3D.h"
#include "Scanner2D.h"

#define DAQWRN( expr )        (Guarded_DAQmx( (expr), #expr, warning))
#define DAQERR( expr )        (Guarded_DAQmx( (expr), #expr, error  ))
#define DAQJMP( expr )        goto_if_fail( 0==DAQWRN(expr), Error)

namespace fetch
{ namespace device
  {
    Scanner3D::Scanner3D(void)
    {
    }

    Scanner3D::~Scanner3D(void)
    { if (this->detach()>0)
          warning("Could not cleanly detach Scanner3D. (addr = 0x%p)\r\n",this);
    }
    
    unsigned int
    Scanner3D::
    attach(void) 
    { return this->Scanner2D::attach(); // Returns 0 on success, 1 otherwise
    }
    
    unsigned int
    Scanner3D::
    detach(void)
    { return this->Scanner2D::detach(); // Returns 0 on success, 1 otherwise
    }
    
    void
    Scanner3D::
    _generate_ao_waveforms(f64 z_um)
    { 
      int N = this->Scanner2D::config.nsamples;
      f64 *m,*p, *z;
      lock();
      vector_f64_request(ao_workspace, 3*N - 1 /*max index*/);
      m = ao_workspace->contents; // first the mirror data
      z = m + N;                  // then the zpiezo  data
      p = z + N;                  // then the pockels data
      _compute_linear_scan_mirror_waveform__sawtooth( &this->LinearScanMirror::config, m, N);
      _compute_zpiezo_waveform_step(                  &this->ZPiezo::config    , z_um, z, N);
      _compute_pockels_vertical_blanking_waveform(    &this->Pockels::config         , p, N);
      unlock();
    }

    static void
    _setup_ao_chan(TaskHandle cur_task,
                   double     freq,
                   device::Scanner2D::Config        *cfg,
                   device::LinearScanMirror::Config *lsm_cfg,
                   device::Pockels::Config          *pock_cfg,
                   device::ZPiezo::Config           *zpiezo_cfg)
    {
      char aochan[ POCKELS_MAX_CHAN_STRING
                  +LINEAR_SCAN_MIRROR__MAX_CHAN_STRING
                  +ZPIEZO_MIRROR__MAX_CHAN_STRING
                  +1];
      f64 vmin,vmax;

      // concatenate the channel names
      memset(aochan, 0, sizeof(aochan));
      strcat(aochan, lsm_cfg->channel);
      strcat(aochan, ",");
      strcat(aochan, zpiezo_cfg->channel);
      strcat(aochan, ",");
      strcat(aochan, pock_cfg->ao_chan);

      vmin = MIN( lsm_cfg->v_lim_min, pock_cfg->v_lim_min );
      vmin = MIN( vmin, zpiezo_cfg->v_lim_min );
      vmax = MAX( lsm_cfg->v_lim_max, pock_cfg->v_lim_max );
      vmax = MAX( vmax, zpiezo_cfg->v_lim_max );
      { f64 v[4];
        // NI DAQ's typically have multiple voltage ranges capable of achieving different precisions.
        // The 6259 has 2 ranges.
        DAQERR(DAQmxGetDevAOVoltageRngs("Dev1",v,4));        // FIXME: HACK - need to get device name
        vmin = MAX(vmin,v[2]);        
        vmax = MIN(vmax,v[3]);        
      }

      DAQERR( DAQmxCreateAOVoltageChan(cur_task,
              aochan,                                  //eg: "/Dev1/ao0,/Dev1/ao2,/Dev1/ao1"
              "vert-mirror-out,zpiezo-out,pockels-out",//name to assign to channel
              vmin,                                    //Volts eg: -10.0
              vmax,                                    //Volts eg:  10.0
              DAQmx_Val_Volts,                         //Units
              NULL));                                  //Custom scale (none)

      DAQERR( DAQmxCfgAnlgEdgeStartTrig(cur_task,
              cfg->trigger,
              DAQmx_Val_Rising,
              0.0));

      DAQERR( DAQmxCfgSampClkTiming(cur_task,
              cfg->clock,          // "Ctr1InternalOutput",
              freq,
              DAQmx_Val_Rising,
              DAQmx_Val_ContSamps, // use continuous output so that counter stays in control
              cfg->nsamples));
    }

    void
    Scanner3D::_config_daq()
    { TaskHandle             cur_task = 0;

      ViInt32   N          = Scanner2D::config.nsamples;
      float64   frame_time = Scanner2D::config.nscans / Scanner2D::config.frequency_Hz;  //  512 records / (7920 records/sec)
      float64   freq       = N/frame_time;                         // 4096 samples / 64 ms = 63 kS/s

      //
      // VERTICAL
      //

      // set up ao task - vertical
      cur_task = this->ao;

      // Setup AO channels
      DAQERR( DAQmxClearTask(this->ao) );                   // Once a DAQ task is started, it needs to be cleared before restarting
      DAQERR( DAQmxCreateTask( "scanner3d-ao", &this->ao)); //
      _setup_ao_chan(this->ao,
                     freq,
                     &this->Scanner2D::config,
                     &this->LinearScanMirror::config,
                     &this->Pockels::config,
                     &this->ZPiezo::config);
      this->_generate_ao_waveforms();

      // set up counter for sample clock
      // - A finite pulse sequence is generated by a pair of onboard counters.
      //   In testing, it appears that after the device is reset, initializing
      //   the counter task doesn't work quite right.  First, I have to start the
      //   task with the paired counter once.  Then, I can set things up normally.
      //   After initializing with the paired counter once, things work fine until
      //   the device (or computer) is reset.  My guess is this is a fault of the
      //   board or driver software.
      // - below, we just cycle the counters when config gets called.  This ensures
      //   everything configures correctly the first time, even after a device
      //   reset or cold start.

      // The "fake" initialization
      DAQERR( DAQmxClearTask(this->clk) );                  // Once a DAQ task is started, it needs to be cleared before restarting
      DAQERR( DAQmxCreateTask("scanner3d-clk",&this->clk)); //
      cur_task = this->clk;
      DAQERR( DAQmxCreateCOPulseChanFreq       ( cur_task,
                                                 this->Scanner2D::config.ctr_alt,     // "Dev1/ctr0"
                                                 "sample-clock",
                                                 DAQmx_Val_Hz,
                                                 DAQmx_Val_Low,
                                                 0.0,
                                                 freq,
                                                 0.5 ));
      DAQERR( DAQmxStartTask(cur_task) );

      // The "real" initialization
      DAQERR( DAQmxClearTask(this->clk) );                  // Once a DAQ task is started, it needs to be cleared before restarting
      DAQERR( DAQmxCreateTask("scanner3d-clk",&this->clk)); //
      cur_task = this->clk;
      DAQERR( DAQmxCreateCOPulseChanFreq       ( cur_task,
                                                 this->Scanner2D::config.ctr,     // "Dev1/ctr1"
                                                 "sample-clock",
                                                 DAQmx_Val_Hz,
                                                 DAQmx_Val_Low,
                                                 0.0,
                                                 freq,
                                                 0.5 ));

      DAQERR( DAQmxCfgImplicitTiming           ( cur_task, DAQmx_Val_FiniteSamps, N ));
      DAQERR( DAQmxCfgDigEdgeStartTrig         ( cur_task, "AnalogComparisonEvent", DAQmx_Val_Rising ));
      DAQERR( DAQmxSetArmStartTrigType         ( cur_task, DAQmx_Val_DigEdge ));
      DAQERR( DAQmxSetDigEdgeArmStartTrigSrc   ( cur_task, this->Scanner2D::config.armstart ));
      DAQERR( DAQmxSetDigEdgeArmStartTrigEdge  ( cur_task, DAQmx_Val_Rising ));

      // Set up the shutter control
      this->Shutter::Bind();

      return;
    }

    /*
     * Compute ZPiezo Waveform Step:
     * ----------------------------
     *
     *           0    N
     *           |    |____________  ,- z_um + z_step
     *               /
     *              /
     *             /
     *            /
     *  _________/                   ,- z_um
     *
     *  Notice that N-1 is equiv. to z_step - z_step/N
     */
    void
    Scanner3D::_compute_zpiezo_waveform_step( ZPiezo::Config *cfg, f64 z_um, f64 *data, f64 N )
    { int i=(int)N;
      f64 A = cfg->um_step * cfg->um2v,
              off = z_um * cfg->um2v;
      while(i--)
        data[i] = A*(i/(N-1))+off; // linear ramp from off to off+A
    }

  }
}

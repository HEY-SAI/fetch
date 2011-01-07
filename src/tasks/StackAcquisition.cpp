/*
 * StackAcquisition.cpp
 *
 *  Created on: May 10, 2010
 *      Author: Nathan Clack <clackn@janelia.hhmi.org>
 */
/*
 * Copyright 2010 Howard Hughes Medical Institute.
 * All rights reserved.
 * Use is subject to Janelia Farm Research Campus Software Copyright 1.1
 * license terms (http://license.janelia.org/license/jfrc_copyright_1_1.html).
 */
#include "common.h"
#include "StackAcquisition.h"
#include "Video.h"
#include "frame.h"
#include "devices\digitizer.h"
#include "devices\Microscope.h"

#if 0
#define DBG(...) debug(__VA_ARGS__);
#else
#define DBG(...)
#endif

#define DIGWRN( expr )  (niscope_chk( vi, expr, #expr, warning ))
#define DIGERR( expr )  (niscope_chk( vi, expr, #expr, error   ))
#define DIGJMP( expr )  goto_if_fail(VI_SUCCESS == niscope_chk( vi, expr, #expr, warning ), Error)
#define DAQWRN( expr )  (Guarded_DAQmx( (expr), #expr, warning))
#define DAQERR( expr )  (Guarded_DAQmx( (expr), #expr, error  ))
#define DAQJMP( expr )  goto_if_fail( 0==DAQWRN(expr), Error)

#define CHKERR( expr )  {if(expr) {error("Expression indicated failure:\r\n\t%s\r\n",#expr);}} 0 //( (expr), #expr, error  ))
#define CHKJMP( expr )  goto_if((expr),Error)

#if 1
#define SCANNER_DEBUG_FAIL_WHEN_FULL
#else
#define SCANNER_DEBUG_SPIN_WHEN_FULL
#endif

namespace fetch
{

  namespace task
  {

    //
    // StackAcquisition -  microscope task
    //

    namespace microscope {

      //Upcasting
      unsigned int StackAcquisition::config(IDevice *d) {return config(dynamic_cast<device::Microscope*>(d));}
      unsigned int StackAcquisition::run   (IDevice *d) {return run   (dynamic_cast<device::Microscope*>(d));}

      unsigned int StackAcquisition::config(device::Microscope *d)
      { 
        static task::scanner::ScanStack<i16> grabstack;
        std::string filename;

        Guarded_Assert(d);

        //Assemble pipeline here
        IDevice *cur;
        cur = d->configPipeline();

        //d->next_filename();
        filename = d->next_filename();
        Guarded_Assert( d->disk.close()==0 );
        IDevice::connect(&d->disk,0,cur,0);
        d->file_series.ensurePathExists();
        Guarded_Assert( d->disk.open(filename,"w")==0);

        d->__scan_agent.arm_nowait(&grabstack,&d->scanner,INFINITE);

        return 1; //success
      }

      static int _handle_wait_for_result(DWORD result, const char *msg)
      {
          return_val_if( result == WAIT_OBJECT_0  , 0 );
          return_val_if( result == WAIT_OBJECT_0+1, 1 );
          Guarded_Assert_WinErr( result != WAIT_FAILED );
          if(result == WAIT_ABANDONED_0)
              warning("StackAcquisition: Wait 0 abandoned\r\n\t%s\r\n", msg);
          if(result == WAIT_ABANDONED_0+1)
              warning("StackAcquisition: Wait 1 abandoned\r\n\t%s\r\n", msg);

          if(result == WAIT_TIMEOUT)
              warning("StackAcquisition: Wait timeout\r\n\t%s\r\n", msg);

          Guarded_Assert_WinErr( result != WAIT_FAILED );

          return -1;
      }

      unsigned int StackAcquisition::run(device::Microscope *dc)
      { 
        unsigned int eflag = 0; // success
        
        Guarded_Assert(dc->__scan_agent.is_runnable());
        Guarded_Assert(dc->__io_agent.is_running());

        eflag |= dc->__scan_agent.run() != 1;

        { HANDLE hs[] = {dc->__scan_agent._thread,          
                         dc->__self_agent._notify_stop};
          DWORD res;
          std::string filename;
          int   t;

          // wait for scan to complete (or cancel)
          res = WaitForMultipleObjects(2,hs,FALSE,INFINITE);
          t = _handle_wait_for_result(res,"StackAcquisition::run - Wait for scanner to finish.");
          switch(t)
          { case 0:       // in this case, the scanner thread stopped.  Nothing left to do.
              eflag |= 0; // success
              //break; 
            case 1:       // in this case, the stop event triggered and must be propagated.
              eflag |= dc->__scan_agent.stop(SCANNER2D_DEFAULT_TIMEOUT) != 1;
              break;
            default:      // in this case, there was a timeout or abandoned wait
              eflag |= 1; //failure              
          }
          
          // Increment file
          eflag |= dc->disk.close();
          filename = dc->next_filename();          
          dc->file_series.ensurePathExists();
          dc->connect(&dc->disk,0,dc->pipelineEnd(),0);
          eflag |= dc->disk.open(filename,"w");
          
        }
        return eflag;
      }

    }  // namespace microscope

    //
    // ScanStack - scanner task
    //

    namespace scanner
    {

      template class ScanStack<i8 >;
      template class ScanStack<i16>;

      // upcasts
      template<class TPixel> unsigned int ScanStack<TPixel>::config (IDevice *d) {return config(dynamic_cast<device::Scanner3D*>(d));}
      template<class TPixel> unsigned int ScanStack<TPixel>::update (IDevice *d) {return update(dynamic_cast<device::Scanner3D*>(d));}

      template<class TPixel> unsigned int ScanStack<TPixel>::run    (IDevice *d) 
      {
        device::Scanner3D *s = dynamic_cast<device::Scanner3D*>(d);
        device::Digitizer::Config digcfg = s->_scanner2d._digitizer.get_config();
        switch(digcfg.kind())
        {
        case cfg::device::Digitizer_DigitizerType_NIScope:
          return run_niscope(s);
          break;
        case cfg::device::Digitizer_DigitizerType_Alazar:
          return run_alazar(s);
          break;
        case cfg::device::Digitizer_DigitizerType_Simulated:
          return run_simulated(s);
          break;
        default:
          warning("ScanStack<>::run() - Got invalid kind() for Digitizer.get_config\r\n");          
        }
        return 0; //failure
      }

      template<class TPixel>
        unsigned int
        ScanStack<TPixel>::
        config(device::Scanner3D *d)
        {
          d->onConfig();
          debug("Scanner3D configured for StackAcquisition<%s>\r\n", TypeStr<TPixel> ());
          return 1; //success
        }

      template<class TPixel>
        unsigned int
        ScanStack<TPixel>::
        update(device::Scanner3D *scanner)
        { 
          scanner->generateAO();          
          return 1;
        }

      template<class TPixel>
        unsigned int
        ScanStack<TPixel>::run_niscope(device::Scanner3D *d)
        {
          asynq *qdata = d->_out->contents[0], *qwfm = d->_out->contents[1];
          Frame *frm = NULL;
          Frame_With_Interleaved_Lines ref;
          struct niScope_wfmInfo *wfm = NULL;
          ViInt32 nwfm;
          ViInt32 width;
          int i = 0, status = 1; // status == 0 implies success, error otherwise
          size_t nbytes, nbytes_info;
          f64 z_um,ummax,ummin,umstep;

          TicTocTimer outer_clock = tic(), inner_clock = tic();
          double dt_in = 0.0, dt_out = 0.0;

          device::NIScopeDigitizer *dig = d->_scanner2d._digitizer._niscope;
          device::NIScopeDigitizer::Config digcfg = dig->get_config();

          ViSession vi = dig->_vi;
          ViChar *chan = const_cast<ViChar*>(digcfg.chan_names().c_str());                   
          
          ref = _describe_actual_frame_niscope<TPixel>(
            dig,                                 // NISCope Digitizer
            d->_scanner2d.get_config().nscans(), // number of scans
            &width,                              // width in pixels (queried from board)
            &nwfm);                              // this should be the number of scans (queried from board)
          nbytes = ref.size_bytes();
          nbytes_info = nwfm * sizeof(struct niScope_wfmInfo);
          //
          Asynq_Resize_Buffers(qdata, nbytes);
          Asynq_Resize_Buffers(qwfm, nbytes_info);
          frm = (Frame*) Asynq_Token_Buffer_Alloc(qdata);
          wfm = (struct niScope_wfmInfo*) Asynq_Token_Buffer_Alloc(qwfm);
          nbytes = qdata->q->buffer_size_bytes;
          nbytes_info = qwfm->q->buffer_size_bytes;
          //
          ref.format(frm);

          d->_zpiezo.getScanRange(&ummin,&ummax,&umstep);
          
          d->generateAORampZ((float)ummin); //d->_generate_ao_waveforms__z_ramp_step(ummin);
          d->writeAO();
          d->_scanner2d._shutter.Open();
          CHKJMP(d->_scanner2d._daq.startAO());          
          for(z_um=ummin+umstep;z_um<ummax && !d->_agent->is_stopping();z_um+=umstep)
          { 
            CHKJMP(d->_scanner2d._daq.startCLK());            
            DIGJMP(niScope_InitiateAcquisition(vi));

            dt_out = toc(&outer_clock);            
            toc(&inner_clock);
#if 1
            DIGJMP(Fetch<TPixel> (vi, chan, SCANNER_STACKACQ_TASK_FETCH_TIMEOUT,//10.0, //(-1=infinite) (0.0=immediate) // seconds
                                  width,
                                  (TPixel*) frm->data,
                                  wfm));
#endif

            // Push the acquired data down the output pipes
            DBG("Task: StackAcquisition<%s>: pushing wfm\r\n", TypeStr<TPixel> ());
            Asynq_Push(qwfm, (void**) &wfm, nbytes_info, 0);
            DBG("Task: StackAcquisition<%s>: pushing frame\r\n", TypeStr<TPixel> ());
#ifdef SCANNER_DEBUG_FAIL_WHEN_FULL                     //"fail fast"
            if( !Asynq_Push_Try( qdata,(void**) &frm,nbytes ))
#elif defined( SCANNER_DEBUG_SPIN_WHEN_FULL )           //"fail proof" - overwrites when full
            if( !Asynq_Push( qdata,(void**) &frm, nbytes, FALSE ))
#else
            error("Choose a push behavior by compiling with the appropriate define.\r\n");
#endif
            {
              warning("Scanner output frame queue overflowed.\r\n\tAborting stack acquisition task.\r\n");
              goto Error;
            }
            ref.format(frm);
            dt_in = toc(&inner_clock);
            toc(&outer_clock);
            CHKJMP(d->_scanner2d._daq.waitForDone(SCANNER2D_DEFAULT_TIMEOUT));
            d->_scanner2d._daq.stopCLK();            
            debug("Generating AO for z = %f\r\n.",z_um);
            d->generateAORampZ((float)z_um);
            d->writeAO();
            ++i;
          };

          status = 0;
          DBG("Scanner - Stack Acquisition task completed normally.\r\n");
Finalize: 
          d->_scanner2d._shutter.Shut();          
          free(frm);
          free(wfm);
          niscope_debug_print_status(vi);  
          CHKERR(d->_scanner2d._daq.stopAO());
          CHKERR(d->_scanner2d._daq.stopCLK());
          DIGERR(niScope_Abort(vi));
          return status;
Error: 
          warning("Error occurred during ScanStack<%s> task.\r\n",TypeStr<TPixel>());
          goto Finalize;
        }

        template<class TPixel>
        unsigned int fetch::task::scanner::ScanStack<TPixel>::run_simulated( device::Scanner3D *d )
        {
          warning("Implement me!\r\n");
          return 1;
        }

        template<class TPixel>
        unsigned int fetch::task::scanner::ScanStack<TPixel>::run_alazar( device::Scanner3D *d )
        {
          warning("Implement me!\r\n");
          return 1;
        }
    }
  }
}

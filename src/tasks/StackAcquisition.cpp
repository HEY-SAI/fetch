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

#define DIGWRN( expr )  (niscope_chk( vi, expr, #expr, __FILE__, __LINE__, warning ))
#define DIGERR( expr )  (niscope_chk( vi, expr, #expr, __FILE__, __LINE__, error   ))
#define DIGJMP( expr )  goto_if_fail(VI_SUCCESS == niscope_chk( vi, expr, #expr, __FILE__, __LINE__, warning ), Error)
#define DAQWRN( expr )  (Guarded_DAQmx( (expr), #expr, __FILE__, __LINE__, warning))
#define DAQERR( expr )  (Guarded_DAQmx( (expr), #expr, __FILE__, __LINE__, error  ))
#define DAQJMP( expr )  goto_if_fail( 0==DAQWRN(expr), Error)

#define CHKERR( expr )  if(expr) {error("%s(%d)"ENDL"\tExpression indicated failure:"ENDL"\t%s"ENDL,__FILE__,__LINE__,#expr);}
#define CHKJMP( expr )  do{if(expr) {warning("%s(%d)"ENDL"\tExpression evaluated to an error code (non-zero)."ENDL"\t%s"ENDL,__FILE__,__LINE__,#expr); goto Error;}} while(0)

#define LOG(...)     debug(__VA_ARGS__)
#define ECHO(e)      LOG(e)
#define REPORT(e)    LOG("%s(%d) - %s():"ENDL "\t%s"ENDL "\tExpression evaluated as false."ENDL,__FILE__,__LINE__,__FUNCTION__,e)
#define TRY(e)       do{ ECHO(#e); if(!(e)) {REPORT(#e); goto Error;}} while(0)

#if 0
#define SCANNER_DEBUG_FAIL_WHEN_FULL
#else
#define SCANNER_DEBUG_WAIT_WHEN_FULL
#endif


#ifdef SCANNER_DEBUG_FAIL_WHEN_FULL
#define SCANNER_PUSH(...) Chan_Next_Try(__VA_ARGS__)
#elif defined(SCANNER_DEBUG_WAIT_WHEN_FULL)
#define SCANNER_PUSH(...) Chan_Next(__VA_ARGS__)
#else
#error("An overflow behavior for the scanner should be specified");
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
        static task::scanner::ScanStack<u16> grabstack;
        std::string filename;

        Guarded_Assert(d);

        //Assemble pipeline here
        IDevice *cur;
        cur = d->configPipeline();

        CHKJMP( !d->file_series.ensurePathExists() );

        d->file_series.inc();
        filename = d->stack_filename();
        IDevice::connect(&d->disk,0,cur,0);
        Guarded_Assert( d->disk.close()==0 );
        //Guarded_Assert( d->disk.open(filename,"w")==0);

        d->__scan_agent.arm(&grabstack,&d->scanner);                      // why was this arm_nowait?

        return 1; //success
Error:
        return 0; //failure
      }

      static int _handle_wait_for_result(DWORD result, const char *msg)
      {
          return_val_if( result == WAIT_OBJECT_0  , 0 );
          return_val_if( result == WAIT_OBJECT_0+1, 1 );
          Guarded_Assert_WinErr( result != WAIT_FAILED );
          if(result == WAIT_ABANDONED_0)
              warning("%s(%d)"ENDL "StackAcquisition: Wait 0 abandoned"ENDL"\t%s"ENDL, __FILE__,__LINE__,msg);
          if(result == WAIT_ABANDONED_0+1)
              warning("%s(%d)"ENDL "StackAcquisition: Wait 1 abandoned"ENDL"\t%s"ENDL, __FILE__,__LINE__,msg);

          if(result == WAIT_TIMEOUT)
              warning("%s(%d)"ENDL "StackAcquisition: Wait timeout"ENDL"\t%s"ENDL, __FILE__,__LINE__,msg);

          Guarded_Assert_WinErr( result != WAIT_FAILED );

          return -1;
      }

      unsigned int StackAcquisition::run(device::Microscope *dc)
      {
        std::string filename;
        unsigned int eflag = 0; // success

        Guarded_Assert(dc->__scan_agent.is_runnable());
        //Guarded_Assert(dc->__io_agent.is_running());

        dc->transaction_lock();
        filename = dc->stack_filename();
        dc->file_series.ensurePathExists();
        eflag |= dc->disk.open(filename,"w");
        if(eflag)
          return eflag;
        eflag |= dc->runPipeline();
        eflag |= dc->__scan_agent.run() != 1;


        //Chan_Wait_For_Writer_Count(dc->__scan_agent._owner->_out->contents[0],1);

        { HANDLE hs[] = {dc->__scan_agent._thread,
                         dc->__self_agent._notify_stop};
          DWORD res;
          int   t;

          // wait for scan to complete (or cancel)
          dc->transaction_unlock();
          res = WaitForMultipleObjects(2,hs,FALSE,INFINITE);
          dc->transaction_lock();
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
        }

        // Output metadata and Increment file
        eflag |= dc->disk.close();
        dc->write_stack_metadata();
        dc->file_series.inc();
        //dc->connect(&dc->disk,0,dc->pipelineEnd(),0);

        eflag |= dc->stopPipeline(); // wait till the  pipeline stops
        dc->transaction_unlock();
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
          warning("%s(%d)"ENDL "\tScanStack<>::run() - Got invalid kind() for Digitizer.get_config"ENDL,__FILE__,__LINE__);
        }
        return 0; //failure
      }

      template<class TPixel>
        unsigned int
        ScanStack<TPixel>::
        config(device::Scanner3D *d)
        {
          d->onConfigTask();
          debug("%s(%d)"ENDL "\tScanner3D configured for StackAcquisition<%s>"ENDL,__FILE__,__LINE__, TypeStr<TPixel> ());
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
#ifdef HAVE_NISCOPE
          Chan *qdata = Chan_Open(d->_out->contents[0],CHAN_WRITE),
               *qwfm  = Chan_Open(d->_out->contents[1],CHAN_WRITE);
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

          d->onConfigTask();

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
          Chan_Resize(qdata, nbytes);
          Chan_Resize(qwfm, nbytes_info);
          frm = (Frame*) Chan_Token_Buffer_Alloc(qdata);
          wfm = (struct niScope_wfmInfo*) Chan_Token_Buffer_Alloc(qwfm);
          nbytes = Chan_Buffer_Size_Bytes(qdata);
          nbytes_info = Chan_Buffer_Size_Bytes(qwfm);
          //
          ref.format(frm);

          d->_zpiezo.getScanRange(&ummin,&ummax,&umstep);
          //d->_zpiezo.moveTo(ummin); // reset to starting position

          // One dead cycle to reset stack position
          {
            d->generateAOConstZ(ummin);  // Note CONST waveform
            CHKJMP(d->writeAO());
            CHKJMP(d->_scanner2d._daq.startAO());
            CHKJMP(d->_scanner2d._daq.startCLK());
            DIGJMP(niScope_InitiateAcquisition(vi)); // have to do an acquisition (for my triggers), though the data will get thrown away
#if 1
            //debug("%s(%d)"ENDL "\tFetch start -- dummy to set stack position."ENDL,__FILE__,__LINE__);
            DIGJMP(Fetch<TPixel> (vi, chan, SCANNER_STACKACQ_TASK_FETCH_TIMEOUT,//10.0, //(-1=infinite) (0.0=immediate) // seconds
                                  width,
                                  (TPixel*) frm->data,
                                  wfm));
            //debug("\tFetch end -- dummy to set stack position."ENDL);
#endif
            CHKJMP(d->_scanner2d._daq.waitForDone(SCANNER2D_DEFAULT_TIMEOUT));
            d->_scanner2d._daq.stopCLK();
          }
          //The real thing
          d->_scanner2d._shutter.Open();
          // test is for z_um<=ummax, with a precision of umstep/2.
          for(z_um=ummin; ((ummax-z_um)/umstep)>=-0.5f && !d->_agent->is_stopping();z_um+=umstep)
          { debug("%s(%d)"ENDL "\tGenerating AO for z = %f."ENDL,__FILE__,__LINE__,z_um);
            d->generateAORampZ((float)z_um);
            CHKJMP(d->writeAO());
            CHKJMP(d->_scanner2d._daq.startCLK());
            DIGJMP(niScope_InitiateAcquisition(vi));

            dt_out = toc(&outer_clock);
            toc(&inner_clock);
#if 1
            //debug("%s(%d)"ENDL "\tFetch start."ENDL,__FILE__,__LINE__);
            DIGJMP(Fetch<TPixel> (vi, chan, SCANNER_STACKACQ_TASK_FETCH_TIMEOUT,//10.0, //(-1=infinite) (0.0=immediate) // seconds
                                  width,
                                  (TPixel*) frm->data,
                                  wfm));
            //debug("\tFetch end."ENDL);
#endif

            // Push the acquired data down the output pipes
            DBG("%s(%d)"ENDL "\tTask: StackAcquisition<%s>: pushing wfm"ENDL,__FILE__,__LINE__, TypeStr<TPixel> ());
            Chan_Next_Try(qwfm,(void**)&wfm,nbytes_info);
            DBG("%s(%d)"ENDL "\tTask: StackAcquisition<%s>: pushing frame"ENDL,__FILE__,__LINE__, TypeStr<TPixel> ());
            if(CHAN_FAILURE( SCANNER_PUSH(qdata,(void**)&frm,nbytes) ))
            { warning("(%s:%d) Scanner output frame queue overflowed."ENDL"\tAborting stack acquisition task."ENDL,__FILE__,__LINE__);
              goto Error;
            }
            ref.format(frm);
            dt_in = toc(&inner_clock);
            toc(&outer_clock);
            CHKJMP(d->_scanner2d._daq.waitForDone(SCANNER2D_DEFAULT_TIMEOUT));
            d->_scanner2d._daq.stopCLK();
            ++i;
          }
          d->_scanner2d._shutter.Shut();
          status = 0;
          DBG("%s(%d)"ENDL "\tScanner - Stack Acquisition task completed normally."ENDL,__FILE__,__LINE__);
          // One last cycle to reset stack position
          {
            d->generateAOConstZ(ummin);  // Note CONST waveform
            CHKJMP(d->writeAO());
            CHKJMP(d->_scanner2d._daq.startCLK());
            DIGJMP(niScope_InitiateAcquisition(vi)); // have to do an acquisition (for my triggers), though the data will get thrown away
#if 1
            //debug("%s(%d)"ENDL "\tFetch start -- dummy to reset stack position."ENDL,__FILE__,__LINE__);
            DIGJMP(Fetch<TPixel> (vi, chan, SCANNER_STACKACQ_TASK_FETCH_TIMEOUT,//10.0, //(-1=infinite) (0.0=immediate) // seconds
                                  width,
                                  (TPixel*) frm->data,
                                  wfm));
            //debug("\tFetch end -- dummy to reset stack position."ENDL);
#endif
            CHKJMP(d->_scanner2d._daq.waitForDone(SCANNER2D_DEFAULT_TIMEOUT));
            d->_scanner2d._daq.stopCLK();
          }
Finalize:
          d->_scanner2d._shutter.Shut();

          free(frm);
          free(wfm);
          Chan_Close(qdata);
          Chan_Close(qwfm);
          //niscope_debug_print_status(vi);
          CHKERR(d->_scanner2d._daq.stopAO());
          CHKERR(d->_scanner2d._daq.stopCLK());
          DIGERR(niScope_Abort(vi));
          return status;
Error:
          warning("%s(%d)"ENDL "\tError occurred during ScanStack<%s> task."ENDL,__FILE__,__LINE__,TypeStr<TPixel>());
          d->_scanner2d._daq.stopAO();
          d->_scanner2d._daq.stopCLK();
          goto Finalize;
#else //HAVE_NISCOPE
          return 1; //failure - no niscope
#endif
        }

        template<class TPixel>
        unsigned int fetch::task::scanner::ScanStack<TPixel>::run_simulated( device::Scanner3D *d )
        {
          Chan *qdata = Chan_Open(d->_out->contents[0],CHAN_WRITE);
          Frame *frm   = NULL;
          device::SimulatedDigitizer *dig = d->_scanner2d._digitizer._simulated;
          Frame_With_Interleaved_Planes ref(
            dig->get_config().width(),
            d->_scanner2d.get_config().nscans()*2,
            3,TypeID<TPixel>());
          size_t nbytes;
          int status = 1; // status == 0 implies success, error otherwise
          f64 z_um,ummax,ummin,umstep;

          nbytes = ref.size_bytes();
          Chan_Resize(qdata, nbytes);
          frm = (Frame*)Chan_Token_Buffer_Alloc(qdata);
          ref.format(frm);

          debug("Simulated Stack!"ENDL);
          HERE;
          d->_zpiezo.getScanRange(&ummin,&ummax,&umstep);
          for(z_um=ummin+umstep;z_um<ummax && !d->_agent->is_stopping();z_um+=umstep)
          //for(int d=0;d<100;++d)
          { size_t pitch[4];
            size_t n[3];
            frm->compute_pitches(pitch);
            frm->get_shape(n);

            //Fill frame w random colors.
            { TPixel *c,*e;
              const f32 low = TypeMin<TPixel>(),
                       high = TypeMax<TPixel>(),
                        ptp = high - low,
                        rmx = RAND_MAX;
              c=e=(TPixel*)frm->data;
              e+=pitch[0]/pitch[3];
              for(;c<e;++c)
                *c = (TPixel) ((ptp*rand()/(float)RAND_MAX) + low);
            }

            if(CHAN_FAILURE( SCANNER_PUSH(qdata,(void**)&frm,nbytes) ))
            { warning("Scanner output frame queue overflowed."ENDL"\tAborting acquisition task."ENDL);
              goto Error;
            }
            ref.format(frm);
            DBG("Task: ScanStack<%s>: pushing frame"ENDL,TypeStr<TPixel>());
          }
          HERE;
Finalize:
          Chan_Close(qdata);
          free( frm );
          return status; // status == 0 implies success, error otherwise
Error:
          warning("Error occurred during ScanStack<%s> task."ENDL,TypeStr<TPixel>());
          goto Finalize;
        }


        struct alazar_fetch_thread_ctx_t
        { device::Scanner3D *d;
          int nframes;
          int running;
          int ok;
          Basic_Type_ID tid;
          alazar_fetch_thread_ctx_t(device::Scanner3D *d,int nframes,Basic_Type_ID tid)
            : d(d),nframes(nframes),tid(tid),running(1),ok(1) {}
        };

        DWORD alazar_fetch_thread(void *ctx_)
        { alazar_fetch_thread_ctx_t *ctx=(alazar_fetch_thread_ctx_t*)ctx_;
          device::Scanner3D *d=ctx->d;
          Chan *q = Chan_Open(d->_out->contents[0],CHAN_WRITE);
          device::AlazarDigitizer *dig=d->_scanner2d._digitizer._alazar;
          unsigned w,h,i;
          const int nframes=ctx->nframes;
          dig->get_image_size(&w,&h);
          size_t nbytes;
          Frame *frm   = NULL;
          Frame_With_Interleaved_Planes ref(w,h,dig->nchan(),ctx->tid);
          nbytes = ref.size_bytes();
          Chan_Resize(q, nbytes);
          frm = (Frame*)Chan_Token_Buffer_Alloc(q);
          ref.format(frm);

          TRY(dig->fetch(frm)); // first dead frame to set to zmin
          for(i=0;i<nframes && !d->_agent->is_stopping();++i)
          { TRY(dig->fetch(frm));
            TRY(CHAN_SUCCESS(Chan_Next(q,(void**)&frm,nbytes)));
            ref.format(frm);
          }
          TRY(dig->fetch(frm)); // last dead frame to set back to zmin
Finalize:
          ctx->running=0;
          if(frm) free(frm);
          Chan_Close(q);
          return 0;
Error:
          ctx->ok=0;        // FIXME UNSAFE
          goto Finalize;
        }

        template<class TPixel>
        unsigned int fetch::task::scanner::ScanStack<TPixel>::run_alazar( device::Scanner3D *d )
        { int ecode = 0; // ecode == 0 implies success, error otherwise
          f64 z_um,ummax,ummin,umstep;
          HANDLE fetch_thread=0;

          d->_zpiezo.getScanRange(&ummin,&ummax,&umstep);
          alazar_fetch_thread_ctx_t ctx(d,(ummax-ummin)/umstep,TypeID<TPixel>());
          // NOTES:
          // 1. First dead frame will be captured by the digitizer.
          // 2. Should tell the pockels to stay closed during the first frame
          d->generateAOConstZ(ummin);                                           // first frame is a dead frame to lock to ummin
          d->writeAO();
          TRY(!d->_scanner2d._daq.startCLK());
          TRY(!d->_scanner2d._daq.startAO());
          TRY(d->_scanner2d._digitizer._alazar->start());
          Guarded_Assert_WinErr(fetch_thread=CreateThread(NULL,0,alazar_fetch_thread,&ctx,0,NULL));
          for(z_um=ummin; ((ummax-z_um)/umstep)>=-0.5f && ctx.running;z_um+=umstep)
          { d->generateAORampZ(z_um);
            TRY(!d->writeAO());
          }
          d->generateAOConstZ(ummin);                                           // last frame is a dead frame to lock to ummin
          TRY(!d->writeAO());
          Guarded_Assert_WinErr(WAIT_OBJECT_0==WaitForSingleObject(fetch_thread,INFINITE));
          TRY(ctx.ok);
HERE;
warning("WORK IN PROGRESS");
Finalize:
          d->_scanner2d._digitizer._alazar->stop();
          d->_scanner2d._daq.stopCLK();
          d->_scanner2d._daq.stopAO();
          if(fetch_thread) CloseHandle(fetch_thread);
          return ecode; // ecode == 0 implies success, error otherwise
Error:
          warning("Error occurred during ScanStack<%s> task."ENDL,TypeStr<TPixel>());
          ecode=1;
          goto Finalize;
        }
    }
  }
}

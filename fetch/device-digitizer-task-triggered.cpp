#include "stdafx.h"
#include "device-digitizer-task-triggered.h"
#include "util-niscope.h"
#include "device-digitizer.h"
#include "device.h"
#include "frame.h"
#include "frame-interface-digitizer.h"

#define CheckWarn( expression )  (niscope_chk( Digitizer_Get()->vi, expression, #expression, &warning ))
#define CheckPanic( expression ) (niscope_chk( Digitizer_Get()->vi, expression, #expression, &error   ))
#define ViErrChk( expression )    goto_if( CheckWarn(expression), Error )

#if 0
#define digitizer_task_fetch_forever_debug(...) debug(__VA_ARGS__)
#else
#define digitizer_task_fetch_forever_debug(...)
#endif

#if 1
#define DIGITIZER_DEBUG_FAIL_WHEN_FULL
#else
#define DIGITIZER_DEBUG_SPIN_WHEN_FULL
#endif

typedef ViInt16 TPixel;

//
// TASK - Streaming on all channels
//

Digitizer_Frame_Metadata*
_Digitizer_Task_Triggered_Frame_Metadata( ViInt32 record_length, ViInt32 nwfm )
{ static Digitizer_Frame_Metadata meta;    
  meta.height = 512;
  meta.width  = (u16) (record_length / meta.height);
  meta.nchan  = (u8)  nwfm;
  meta.Bpp    = sizeof(TPixel);
  return &meta;
}

unsigned int
_Digitizer_Task_Triggered_Cfg( Device *d, vector_PASYNQ *in, vector_PASYNQ *out )
{ Digitizer *dig = Digitizer_Get();
  ViSession   vi = dig->vi;
  Digitizer_Config cfg = dig->config;
  ViInt32 i;
  int nchan = 0;
   
  // Vertical
  for(i=0; i<cfg.num_channels; i++)
  { Digitizer_Channel_Config ch = cfg.channels[i];
    CheckPanic(niScope_ConfigureVertical (vi, 
                                          ch.name,     //channelName, 
                                          ch.range,    //verticalRange, 
                                          0.0,         //verticalOffset, 
                                          ch.coupling, //verticalCoupling, 
                                          1.0,         //probeAttenuation, 
                                          ch.enabled));//enabled?
    nchan += ch.enabled;
  }
  // Horizontal
  CheckPanic (niScope_ConfigureHorizontalTiming (vi, 
                                                cfg.sample_rate,       // sample rate (S/s)
                                                cfg.record_length,     // record length (S)
                                                cfg.reference_position,// reference position (% units???)
                                                cfg.num_records,       // number of records to fetch per acquire
                                                NISCOPE_VAL_TRUE));    // enforce real time?
  // Configure software trigger, but never send the trigger.
  // This starts an infinite acquisition, until you call niScope_Abort
  // or niScope_close
  CheckPanic (niScope_ConfigureTriggerSoftware (vi, 
                                                 0.0,   // hold off (s)
                                                 0.0)); // delay    (s)

  { ViInt32 nwfm;
    ViInt32 record_length;
    Frame_Descriptor desc;
    
    CheckPanic( niScope_ActualNumWfms(vi, cfg.acquisition_channels, &nwfm ) );
    CheckPanic( niScope_ActualRecordLength(vi, &record_length) );
    
    // Fill in frame description
    { Digitizer_Frame_Metadata *meta = 
        _Digitizer_Task_Triggered_Frame_Metadata( record_length, nwfm );
      Frame_Descriptor_Change( &desc, FRAME_INTEFACE_DIGITIZER__INTERFACE_ID, meta, sizeof(Digitizer_Frame_Metadata) );
    }
    { size_t nbuf[2] = {DIGITIZER_BUFFER_NUM_FRAMES,
                        DIGITIZER_BUFFER_NUM_FRAMES},
               sz[2] = {Frame_Get_Size_Bytes(&desc),             // frame data
                        nwfm*sizeof(struct niScope_wfmInfo)};    // description of each frame
      // DeviceTask_Free_Outputs( d->task );  // free channels that already exist (FIXME: thrashing)
      // Channels are reference counted so the memory may not be freed till the other side Unrefs.
      if( d->task->out == NULL ) // channels may already be alloced (occurs on a detach->attach cycle)
        DeviceTask_Alloc_Outputs( d->task, 2, nbuf, sz );
    }
  }
  debug("Digitizer configured for Triggered\r\n");
  return 1;
}

unsigned int
_Digitizer_Task_Triggered_Proc( Device *d, vector_PASYNQ *in, vector_PASYNQ *out )
{ Digitizer *dig = ((Digitizer*)d->context);
  ViSession   vi = dig->vi;
  ViChar   *chan = dig->config.acquisition_channels;
  asynq   *qdata = out->contents[0],
          *qwfm  = out->contents[1];
  ViInt32  nelem,
           nwfm,
           ttl = 0,ttl2=0,
           old_state;
  u32      nfetches = 0,
           nframes  = 0,
           every    = 32;  // must be power of two - used for reporting FPS
  TicTocTimer t, delay_clock;
  double delay, maxdelay = 0.0, accdelay = 0.0;
  u32    last_max_fetch = 0;  
  Frame                  *frm  = (Frame*)            Asynq_Token_Buffer_Alloc(qdata);
  struct niScope_wfmInfo *wfm  = (niScope_wfmInfo*)  Asynq_Token_Buffer_Alloc(qwfm);
  Frame_Descriptor       *desc, ref;
  int change_token;
  TPixel                 *buf;
  unsigned int ret = 1;

  Frame_From_Bytes(frm, (void**)&buf, &desc );
    
  CheckPanic( niScope_ActualNumWfms(vi, chan, &nwfm ) );
  CheckPanic( niScope_ActualRecordLength(vi, &nelem) );

  // Fill in frame description
  { Digitizer_Frame_Metadata *meta = 
      _Digitizer_Task_Triggered_Frame_Metadata( nelem, nwfm );
    Frame_Descriptor_Change( desc, FRAME_INTEFACE_DIGITIZER__INTERFACE_ID, meta, sizeof(Digitizer_Frame_Metadata) );
    change_token = desc->change_token;
    ref = *desc;
  }
      
  
  ViErrChk   (niScope_GetAttributeViInt32 (vi, NULL,   // TODO: reset to default when done
                                           NISCOPE_ATTR_FETCH_RELATIVE_TO,         //?TODO: push/pop state for niscope?
                                           &old_state ));    
  ViErrChk   (niScope_SetAttributeViInt32 (vi, NULL,   // TODO: reset to default when done
                                           NISCOPE_ATTR_FETCH_RELATIVE_TO,         //?TODO: push/pop state for niscope?
                                           NISCOPE_VAL_READ_POINTER ));

  // Loop until the stop event is triggered
  debug("Digitizer Triggered - Running -\r\n");
  t = tic();
  
  ViErrChk   (niScope_InitiateAcquisition (vi));
  delay_clock = tic();
  do 
  { ttl2 += ttl;
    ttl = 0;
    do
    { // Fetch the available data without waiting
      delay = toc( &delay_clock );
      delay = toc( &delay_clock );
      maxdelay = MAX(delay, maxdelay);
      ViStatus sts = niScope_FetchBinary16 ( vi, 
                                        chan,          // (acquistion channels)
                                        0.0,           // Immediate
                                        nelem - ttl,   // Remaining space in buffer
                                        buf   + ttl,   // Where to put the data
                                        wfm);          // metadata for fetch
      if( delay > maxdelay )
      { maxdelay = delay;
        last_max_fetch = nfetches;
      }
      accdelay += delay;
      if( sts != VI_SUCCESS )
      { niscope_chk(vi,sts, "niScope_FetchBinary16", warning);
        goto Error;
      }
      ++nfetches;     
      ttl += wfm->actualSamples;  // add the chunk size to the total samples count     
      Asynq_Push( qwfm,(void**) &wfm, 0 );    // Push (swap) the info from the last fetch
    } while(ttl!=nelem);
    
    // Handle the full buffer
    { //double dt;
#ifdef DIGITIZER_DEBUG_FAIL_WHEN_FULL
      if(  !Asynq_Push_Try( qdata,(void**) &frm )) //   Push buffer and reset total samples count
#elif defined( DIGITIZER_DEBUG_SPIN_WHEN_FULL )
      if(  !Asynq_Push( qdata,(void**) &frm, FALSE )) //   Push buffer and reset total samples count
#else
        error("Choose a push behavior for digitizer by compileing with the appropriate define.\r\n");
#endif
      { warning("Digitizer output queue overflowed.\r\n\tAborting acquisition task.\r\n");
        goto Error;
      }
      ++nframes;
      { ViReal64 pts = 0;
        CheckPanic( niScope_GetAttributeViReal64( vi, NULL, NISCOPE_ATTR_BACKLOG, &pts ));
        digitizer_task_fetch_forever_debug("Digitizer Backlog: %4.1f MS\r\n",pts/1024.0/1024.0);
      }  
      Frame_From_Bytes(frm, (void**)&buf, &desc ); //get addresses
      
      memcpy(desc,&ref,sizeof(Frame_Descriptor));
      
      //dt = toc(&t);
      //if( !MOD_UNSIGNED_POW2(nframes+1,every) )
      //  debug("FPS: %3.1f Frame time: %5.4f MS/s: %3.1f  MB/s: %3.1f Q: %3d Digitizer\r\n",
      //        1.0/dt, dt, nelem/dt/1000000.0, nelem*sizeof(TPixel)*nwfm/1000000.0/dt,
      //        qdata->q->head - qdata->q->tail );
    }    
  } while ( WAIT_OBJECT_0 != WaitForSingleObject(d->notify_stop, 0) );
  debug("Digitizer Triggered - Running done -\r\n"
        "Task done: normal exit\r\n");
  ret = 0; //success
Error:
  free( frm );
  free( wfm );
  debug("Digitizer: nfetches: %u nframes: %u\r\n"
        "\tDelay - max: %g (on fetch %d) mean:%g\r\n"
        "\tTotal acquired samples %f MS\r\n",nfetches, nframes,maxdelay, last_max_fetch,accdelay/nfetches,ttl2/1024.0/1024.0);
  { ViReal64 pts = 0;
    CheckPanic( niScope_GetAttributeViReal64( vi, NULL, NISCOPE_ATTR_BACKLOG, &pts ));
    debug("Digitizer Backlog: %4.1f MS\r\n",pts/1024.0/1024.0);
  }
  { ViInt32 mem = 0;
    CheckPanic( niScope_GetAttributeViInt32( vi, NULL, NISCOPE_ATTR_ONBOARD_MEMORY_SIZE, &mem ));
    debug("Digitizer                          Buffer size: %4.1f MB\r\n",mem/1024.0/1024.0);
  }
  { ViInt32 mem = 0;
    CheckPanic( niScope_GetAttributeViInt32( vi, NULL, NISCOPE_ATTR_DATA_TRANSFER_BLOCK_SIZE, &mem ));
    debug("Digitizer             Data Transfer Block size: %4.1f MB\r\n",mem/1024.0/1024.0);
  }
  { ViReal64 mem = 0;
    CheckPanic( niScope_GetAttributeViReal64( vi, NULL, NISCOPE_ATTR_DATA_TRANSFER_MAXIMUM_BANDWIDTH, &mem ));
    debug("Digitizer Data Transfer Maximum Bandwidth size: %4.1f MB\r\n",mem/1024.0/1024.0);
  }
  { ViInt32 mem = 0;
    CheckPanic( niScope_GetAttributeViInt32( vi, NULL, NISCOPE_ATTR_DATA_TRANSFER_PREFERRED_PACKET_SIZE, &mem ));
    debug("Digitizer   Data Transfer Prefered Packet size: %4.1f MB\r\n",mem/1024.0/1024.0);
  }
  { ViReal64 mem = 0;
    CheckPanic( niScope_GetAttributeViReal64( vi, NULL, NISCOPE_ATTR_MAX_REAL_TIME_SAMPLING_RATE, &mem ));
    debug("Digitizer     Data Max real time sampling rate: %4.1f MHz\r\n",mem/1024.0/1024.0);
  }
  { ViReal64 mem = 0;
    CheckPanic( niScope_GetAttributeViReal64( vi, NULL, NISCOPE_ATTR_HORZ_SAMPLE_RATE, &mem ));
    debug("Digitizer                 actual sampling rate: %4.1f MHz\r\n",mem/1024.0/1024.0);
  }
  { ViReal64 mem = 0;
    CheckPanic( niScope_GetAttributeViReal64( vi, NULL, NISCOPE_ATTR_DEVICE_TEMPERATURE, &mem ));
    debug("Digitizer                          Temperature: %4.1f C\r\n",mem);
  }
  CheckPanic( niScope_Abort(vi) );
  return ret;
}

DeviceTask*
Digitizer_Create_Task_Triggered(void)
{ return DeviceTask_Alloc(_Digitizer_Task_Triggered_Cfg,
                          _Digitizer_Task_Triggered_Proc);
}


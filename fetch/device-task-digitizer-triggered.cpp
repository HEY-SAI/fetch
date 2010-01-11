#include "stdafx.h"
#include "device-task-digitizer-triggered.h"
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
void
_Digitizer_Task_Triggered_Set_Default_Parameters(void)
{ Digitizer_Config cfg =  DIGITIZER_CONFIG_DEFAULT;
  double resonant_frequency_Hz = 7920.0;
  *cfg = (Digitizer_Config) DIGITIZER_CONFIG_DEFAULT;
  cfg.sample_rate          = 60000000;
  cfg.record_length        = (ViInt32) cfg.sample_rate / resonant_frequency_Hz;
  cfg.num_records          = 512; // number of scans
  cfg.acquisition_channels = "0,1";
  cfg.channels[0].range    = 2.0;  // Volts peak-to-peak  
  cfg.channels[1].range    = 20.0; // Volts peak-to-peak
  cfg.channels[0].enabled  = VI_TRUE;
  cfg.channels[1].enabled  = VI_TRUE;
  
  Digitizer_Get()->config = cfg;
}

Digitizer_Frame_Metadata*
_Digitizer_Task_Triggered_Frame_Metadata( ViInt32 record_length, ViInt32 nrecords, ViInt32 nwfm )
{ static Digitizer_Frame_Metadata meta;    
  meta.height = (u16) nrecords;        // e.g. 512  for 1024 lines with bidirectional scanning
  meta.width  = (u16) record_length;   // e.g. 7575 for 60MS/s sampling using a 7920 Hz resonant scanner.
  meta.nchan  = (u8)  (nwfm/nrecords); // e.g. 3    for 3 channels
  meta.Bpp    = sizeof(TPixel);        // e.g. 2    for 16 bit samples
  return &meta;
}

unsigned int
_Digitizer_Task_Triggered_Cfg( Device *d, vector_PASYNQ *in, vector_PASYNQ *out )
{ Digitizer *dig = Digitizer_Get();
  ViSession   vi = dig->vi;
  Digitizer_Config cfg = dig->config;
  ViInt32 i;
  int nchan = 0;
  
  _Digitizer_Task_Triggered_Set_Default_Parameters();
   
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
  // Configure reference trigger
  CheckPanic (niScope_ConfigureTriggerEdge( vi, 
                                            "0",                  // trigger source
                                            0.0,                  // trigger voltage
                                            NISCOPE_VAL_POSITIVE, // slope
                                            NISCOPE_VAL_DC,       // coupling
                                            0.0,                  // holdoff - seconds
                                            0.0 ));               // delay
  
  // Set the start trigger to PFI0 (which will accept the frame sync pulse).
  CheckPanic( niScope_SetAttributeViString( vi,                          // session
                                            "",                          // channel list not applicable
                                            NISCOPE_ATTR_ACQ_ARM_SOURCE, // start trigger
                                            NISCOPE_VAL_PFI_1 ));        // PFI1 is the pfi line on the back of the card
                                            

  { ViInt32 nwfm;
    ViInt32 record_length;
    Frame_Descriptor desc;
    
    CheckPanic( niScope_ActualNumWfms(vi, cfg.acquisition_channels, &nwfm ) );
    CheckPanic( niScope_ActualRecordLength(vi, &record_length) );
    
    // Fill in frame description
    { Digitizer_Frame_Metadata *meta = 
        _Digitizer_Task_Triggered_Frame_Metadata( record_length, nwfm );
      Frame_Descriptor_Change( &desc, FRAME_INTERFACE_DIGITIZER_INTERLEAVED_PLANES__INTERFACE_ID, meta, sizeof(Digitizer_Frame_Metadata) );
    }
    { size_t nbuf[2] = {DIGITIZER_BUFFER_NUM_FRAMES,
                        DIGITIZER_BUFFER_NUM_FRAMES},
               sz[2] = {Frame_Get_Size_Bytes(&desc),             // frame data
                        nwfm*sizeof(struct niScope_wfmInfo)};    // description of each frame
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
           nwfm;
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
    Frame_Descriptor_Change( desc, FRAME_INTERFACE_DIGITIZER_INTERLEAVED_PLANES__INTERFACE_ID, meta, sizeof(Digitizer_Frame_Metadata) );
    change_token = desc->change_token;                                                                                   // TODO: figure out an interface for this pattern
    ref = *desc;
  }
      
  // Loop until the stop event is triggered
  debug("Digitizer Triggered - Running -\r\n");
  
  do    // ...while stop event has not been set
  { ttl2 += ttl;
    ttl = 0;
    
    ViErrChk   (niScope_InitiateAcquisition (vi));
    { ViStatus sts = niScope_FetchBinary16 (vi,
                                            chan,          // (acquistion channels)
                                            0.0,           // Immediate
                                            nelem - ttl,   // Remaining space in buffer
                                            buf   + ttl,   // Where to put the data
                                            wfm);          // metadata for fetch
      if( sts != VI_SUCCESS )
      { niscope_chk(vi,sts, "niScope_FetchBinary16", warning);
        goto Error;
      }
    }
    ttl += wfm->actualSamples;
    Asynq_Push( qwfm,(void**) &wfm, 0 );    // Push (swap) the info from the last fetch
        
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
      Frame_From_Bytes(frm, (void**)&buf, &desc ); //get address of new frame buffer and descriptor      
      memcpy(desc,&ref,sizeof(Frame_Descriptor));     
    }    
  } while ( WAIT_OBJECT_0 != WaitForSingleObject(d->notify_stop, 0) );
  debug("Digitizer Triggered - Running done -\r\n"
        "Task done: normal exit\r\n");
  ret = 0; //success
Error:
  free( frm );
  free( wfm );
  niscope_debug_print_status(vi);
  CheckPanic( niScope_Abort(vi) );
  return ret;
}

DeviceTask*
Digitizer_Create_Task_Triggered(void)
{ return DeviceTask_Alloc(_Digitizer_Task_Triggered_Cfg,
                          _Digitizer_Task_Triggered_Proc);
}


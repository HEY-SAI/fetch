#include "stdafx.h"
#include "device.h"
#include "device-digitizer.h"
#include "frame.h"
#include "types.h"

#define WORKER_DEFAULT_NUM_BUFFERS 4
#define WORKER_DEFAULT_TIMEOUT     INFINITE

void
_free_context(Device *d)
{ if(d && (d->context) )
  { free( d->context );
    d->context = NULL;
  }
}

//----------------------------------------------------------------------------
//
//  Averager
//  - input stream must be f32 pixels
//  - one channel in, one channel out
//

#define AVERAGER_DEFAULT_NUM_BUFFERS 8

struct _averager_context
{ int ntimes;
};

unsigned int
_Averager_f32_Cfg( Device *d, vector_PASYNQ *in, vector_PASYNQ *out )
{ struct _averager_context *ctx = (struct _averager_context*) d->context;
  size_t qsize, bufsize;
  Guarded_Assert( d->task->out == NULL );
  qsize = WORKER_DEFAULT_NUM_BUFFERS;
  bufsize  = in->contents[ 0 ]->q->buffer_size_bytes;            // Size of output buffers is determined
  DeviceTask_Alloc_Outputs( d->task, 1, &qsize, &bufsize );      //   by input channel
  return 1; // success
}

unsigned int
_Averager_f32_Proc( Device *d, vector_PASYNQ *in, vector_PASYNQ *out )
{ struct _caster_context *ctx = (struct _caster_context*) d->context;

  asynq *qsrc = in->contents[ 0 ],
        *qdst = out->contents[ 0 ];

  f32  *buf = (f32*) Asynq_Token_Buffer_Alloc(qsrc),
       *acc = (f32*) Asynq_Token_Buffer_Alloc(qdst);
  size_t nbytes = qdst->q->buffer_size_bytes,
         nelem  = nbytes / sizeof(f32);

  int count=0,
      every = ((struct _averager_context*) d->context)->ntimes;
  _free_context(d); // at this point we're done with the context which is basically just used to pass parameters

  memset( acc, 0, nbytes ); 
  do
  { while( Asynq_Pop(qsrc, (void**)&buf) )
    { f32 *src_cur = buf + nelem,
          *acc_cur = acc + nelem;
      while( src_cur >= buf )            // accumulate
        *(--acc_cur) += *(--src_cur);
      if( count % every == 0 )           // emit and reset every so often
      { acc_cur = acc + nelem;
        while( acc_cur-- > acc )         //   average
          *acc_cur /= (float)count;
        goto_if_fail(                    //   push - wait till successful
          Asynq_Push_Timed( qdst, (void**)&acc, WORKER_DEFAULT_TIMEOUT ),
          OutputQueueTimeoutError);
        memset( acc, 0, nbytes );        //   clear the recieved buffer
      }
    }
  } while ( WAIT_OBJECT_0 != WaitForSingleObject(d->notify_stop, 0) );
  Asynq_Token_Buffer_Free(buf);
  Asynq_Token_Buffer_Free(acc);
  return 0;
OutputQueueTimeoutError:
  warning("Pushing to output queue timedout\r\n.");
  Asynq_Token_Buffer_Free(buf);
  Asynq_Token_Buffer_Free(acc);
  return 1; // failure
}

DeviceTask*
Worker_Create_Task_Averager_f32(Device *d, unsigned int ntimes)
{ // 1. setup context
  d->context = Guarded_Malloc( sizeof(struct _averager_context), "Worker_Create_Task_Averager" );
  ((struct _averager_context*)d->context)->ntimes = ntimes; // Boy, do I feel awkward.
  
  // 2. make task
  return DeviceTask_Alloc(_Averager_f32_Cfg,
                          _Averager_f32_Proc);          
}


//----------------------------------------------------------------------------
//
//  Caster - casts pixel type from one kind to another
//  - input stream must be i16 pixels
//  - one channel in, one channel out
//

struct _caster_context
{ Basic_Type_ID source_type,
                dest_type;
};

unsigned int
_Caster_Cfg( Device *d, vector_PASYNQ *in, vector_PASYNQ *out )
{ Guarded_Assert( d->task->out == NULL );
  { struct _caster_context *ctx = (struct _caster_context*) d->context;
    size_t qsize     = WORKER_DEFAULT_NUM_BUFFERS,
           Bpp_in    = g_type_attributes[ ctx->source_type ].bytes,
           Bpp_out   = g_type_attributes[ ctx->dest_type   ].bytes,
           bytes_in  = in->contents[ 0 ]->q->buffer_size_bytes, // Size of output buffers is determined by input
           bytes_out = bytes_in * Bpp_out / Bpp_in;
    DeviceTask_Alloc_Outputs( d->task, 1, &qsize, &bytes_out );
  }
  return 1; // success
}

#define _CASTER_LOOP(TI,TO) {\
  do\
  { while( Asynq_Pop(qsrc, &src) )\
    { TI *src_cur = (TI*)src + nelem;\
      TO *dst_cur = (TO*)dst + nelem;\
      while( src_cur >= src )\
        *(--dst_cur) = (TO) *(--src_cur);\
      goto_if_fail(\
        Asynq_Push_Timed( qdst, &dst, WORKER_DEFAULT_TIMEOUT ),\
        OutputQueueTimeoutError);\
    }\
  } while ( WAIT_OBJECT_0 != WaitForSingleObject(d->notify_stop, 0) );\
  } break

unsigned int
_Caster_Proc( Device *d, vector_PASYNQ *in, vector_PASYNQ *out )
{ struct _caster_context *ctx = (struct _caster_context*) d->context;

  asynq *qsrc = in->contents[0],
        *qdst = out->contents[0];

  size_t nbytes_in = qdst->q->buffer_size_bytes,
         Bpp_in    = g_type_attributes[ ctx->source_type ].bytes,
         nelem     = nbytes_in / Bpp_in;
         
  void  *src =  Asynq_Token_Buffer_Alloc(qsrc),
        *dst =  Asynq_Token_Buffer_Alloc(qdst);

  switch( ctx->source_type )
  { case id_u8:
    { switch( ctx->dest_type )
      { case id_u8:               _CASTER_LOOP(u8 ,u8 );
        case id_u16:              _CASTER_LOOP(u8 ,u16);
        case id_u32:              _CASTER_LOOP(u8 ,u32);
        case id_u64:              _CASTER_LOOP(u8 ,u64);
        case id_i8:               _CASTER_LOOP(u8 ,i8 );
        case id_i16:              _CASTER_LOOP(u8 ,i16);
        case id_i32:              _CASTER_LOOP(u8 ,i32);
        case id_i64:              _CASTER_LOOP(u8 ,i64);
        case id_f32:              _CASTER_LOOP(u8 ,f32);
        case id_f64:              _CASTER_LOOP(u8 ,f64);
        default:
          goto UnknownTypeError;
      }
      break;
    }
    case id_u16:
    { switch( ctx->dest_type )
      { case id_u8:               _CASTER_LOOP(u16,u8 );
        case id_u16:              _CASTER_LOOP(u16,u16);
        case id_u32:              _CASTER_LOOP(u16,u32);
        case id_u64:              _CASTER_LOOP(u16,u64);
        case id_i8:               _CASTER_LOOP(u16,i8 );
        case id_i16:              _CASTER_LOOP(u16,i16);
        case id_i32:              _CASTER_LOOP(u16,i32);
        case id_i64:              _CASTER_LOOP(u16,i64);
        case id_f32:              _CASTER_LOOP(u16,f32);
        case id_f64:              _CASTER_LOOP(u16,f64);
        default:
          goto UnknownTypeError;
      }
      break;
    }
    case id_u32:
    { switch( ctx->dest_type )
      { case id_u8:               _CASTER_LOOP(u32,u8 );
        case id_u16:              _CASTER_LOOP(u32,u16);
        case id_u32:              _CASTER_LOOP(u32,u32);
        case id_u64:              _CASTER_LOOP(u32,u64);
        case id_i8:               _CASTER_LOOP(u32,i8 );
        case id_i16:              _CASTER_LOOP(u32,i16);
        case id_i32:              _CASTER_LOOP(u32,i32);
        case id_i64:              _CASTER_LOOP(u32,i64);
        case id_f32:              _CASTER_LOOP(u32,f32);
        case id_f64:              _CASTER_LOOP(u32,f64);
        default:
          goto UnknownTypeError;
      }
      break;
    }
    case id_u64:
    { switch( ctx->dest_type )
      { case id_u8:               _CASTER_LOOP(u64,u8 );
        case id_u16:              _CASTER_LOOP(u64,u16);
        case id_u32:              _CASTER_LOOP(u64,u32);
        case id_u64:              _CASTER_LOOP(u64,u64);
        case id_i8:               _CASTER_LOOP(u64,i8 );
        case id_i16:              _CASTER_LOOP(u64,i16);
        case id_i32:              _CASTER_LOOP(u64,i32);
        case id_i64:              _CASTER_LOOP(u64,i64);
        case id_f32:              _CASTER_LOOP(u64,f32);
        case id_f64:              _CASTER_LOOP(u64,f64);
        default:
          goto UnknownTypeError;
      }
      break;
    }
    case id_i8:
    { switch( ctx->dest_type )
      { case id_u8:               _CASTER_LOOP(i8 ,u8 );
        case id_u16:              _CASTER_LOOP(i8 ,u16);
        case id_u32:              _CASTER_LOOP(i8 ,u32);
        case id_u64:              _CASTER_LOOP(i8 ,u64);
        case id_i8:               _CASTER_LOOP(i8 ,i8 );
        case id_i16:              _CASTER_LOOP(i8 ,i16);
        case id_i32:              _CASTER_LOOP(i8 ,i32);
        case id_i64:              _CASTER_LOOP(i8 ,i64);
        case id_f32:              _CASTER_LOOP(i8 ,f32);
        case id_f64:              _CASTER_LOOP(i8 ,f64);
        default:
          goto UnknownTypeError;
      }
      break;
    }
    case id_i16:
    { switch( ctx->dest_type )
      { case id_u8:               _CASTER_LOOP(i16,u8 );
        case id_u16:              _CASTER_LOOP(i16,u16);
        case id_u32:              _CASTER_LOOP(i16,u32);
        case id_u64:              _CASTER_LOOP(i16,u64);
        case id_i8:               _CASTER_LOOP(i16,i8 );
        case id_i16:              _CASTER_LOOP(i16,i16);
        case id_i32:              _CASTER_LOOP(i16,i32);
        case id_i64:              _CASTER_LOOP(i16,i64);
        case id_f32:              _CASTER_LOOP(i16,f32);
        case id_f64:              _CASTER_LOOP(i16,f64);
        default:
          goto UnknownTypeError;
      }
      break;
    }
    case id_i32:
    { switch( ctx->dest_type )
      { case id_u8:               _CASTER_LOOP(i32,u8 ); break;
        case id_u16:              _CASTER_LOOP(i32,u16); break;
        case id_u32:              _CASTER_LOOP(i32,u32); break;
        case id_u64:              _CASTER_LOOP(i32,u64); break;
        case id_i8:               _CASTER_LOOP(i32,i8 ); break;
        case id_i16:              _CASTER_LOOP(i32,i16); break;
        case id_i32:              _CASTER_LOOP(i32,i32); break;
        case id_i64:              _CASTER_LOOP(i32,i64); break;
        case id_f32:              _CASTER_LOOP(i32,f32); break;
        case id_f64:              _CASTER_LOOP(i32,f64); break;
        default:
          goto UnknownTypeError;
      }
      break;
    }
    case id_i64:
    { switch( ctx->dest_type )
      { case id_u8:               _CASTER_LOOP(i64,u8 ); break;
        case id_u16:              _CASTER_LOOP(i64,u16); break;
        case id_u32:              _CASTER_LOOP(i64,u32); break;
        case id_u64:              _CASTER_LOOP(i64,u64); break;
        case id_i8:               _CASTER_LOOP(i64,i8 ); break;
        case id_i16:              _CASTER_LOOP(i64,i16); break;
        case id_i32:              _CASTER_LOOP(i64,i32); break;
        case id_i64:              _CASTER_LOOP(i64,i64); break;
        case id_f32:              _CASTER_LOOP(i64,f32); break;
        case id_f64:              _CASTER_LOOP(i64,f64); break;
        default:
          goto UnknownTypeError;
      }
      break;
    }
    case id_f32:
    { switch( ctx->dest_type )
      { case id_u8:               _CASTER_LOOP(f32,u8 ); break;
        case id_u16:              _CASTER_LOOP(f32,u16); break;
        case id_u32:              _CASTER_LOOP(f32,u32); break;
        case id_u64:              _CASTER_LOOP(f32,u64); break;
        case id_i8:               _CASTER_LOOP(f32,i8 ); break;
        case id_i16:              _CASTER_LOOP(f32,i16); break;
        case id_i32:              _CASTER_LOOP(f32,i32); break;
        case id_i64:              _CASTER_LOOP(f32,i64); break;
        case id_f32:              _CASTER_LOOP(f32,f32); break;
        case id_f64:              _CASTER_LOOP(f32,f64); break;
        default:
          goto UnknownTypeError;
      }
      break;
    }
    case id_f64:
    { switch( ctx->dest_type )
      { case id_u8:               _CASTER_LOOP(f64,u8 ); break;
        case id_u16:              _CASTER_LOOP(f64,u16); break;
        case id_u32:              _CASTER_LOOP(f64,u32); break;
        case id_u64:              _CASTER_LOOP(f64,u64); break;
        case id_i8:               _CASTER_LOOP(f64,i8 ); break;
        case id_i16:              _CASTER_LOOP(f64,i16); break;
        case id_i32:              _CASTER_LOOP(f64,i32); break;
        case id_i64:              _CASTER_LOOP(f64,i64); break;
        case id_f32:              _CASTER_LOOP(f64,f32); break;
        case id_f64:              _CASTER_LOOP(f64,f64); break;
        default:
          goto UnknownTypeError;
      }
      break;
    }
    default:
      goto UnknownTypeError;
  }

  _free_context(d);
  Asynq_Token_Buffer_Free(src);
  Asynq_Token_Buffer_Free(dst);
  return 0;
OutputQueueTimeoutError:
  warning("Pushing to output queue timed out.\r\n");
  _free_context(d);
  Asynq_Token_Buffer_Free(src);
  Asynq_Token_Buffer_Free(dst);
  return 1; // failure
UnknownTypeError:
  warning("Could not recognize type id.\r\n");
  _free_context(d);
  return 1; // failure
}

DeviceTask*
Worker_Create_Task_Caster(Device *d, Basic_Type_ID source_type, Basic_Type_ID dest_type)
{ // 1. setup context
  struct _caster_context *ctx = (struct _caster_context *) Guarded_Malloc( sizeof(struct _caster_context), "Worker_Create_Task_Averager" );
  ctx->source_type = source_type;
  ctx->dest_type = dest_type;
  d->context = (void*) ctx;
  
  // 2. make task
  return DeviceTask_Alloc(_Caster_Cfg,
                          _Caster_Proc);
}

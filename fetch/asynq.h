#pragma once

#include "stdafx.h"
#include "ring-fifo.h"

/*
 AsynQ
 -----

 This is a threadsafe asynchronous queue.  It can be used from multiple threads
 without explicit locking.

 The implimentation wraps a FIFO queue implimented on a circular store of
 pointers that address distinct, pre-allocated buffers.  

 There are three basic operations: pop, push and peek. Push and pop use a swap
 operation that exchanges the pointer to a token buffer with the pointer to the
 head/tail.  On a push, the input buffer has been filled with data to enqueue.
 On a pop, the input buffer is just a place holder; the contained data may be
 deleted or overwritten.

 The only operation that initiates a blocking copy of an enqueued buffer is
 peek.  Peek copies data from the tail buffer to an input buffer.
 
 Peeking and popping block when the queue is empty.  On termination, if no data
 is being pushed on, waiting consumers will become deadlocked.  There are two
 mechanisms provided for dealing with this:

   1.  Flush waiting consumers by triggering the notify_data event until there
       are no more waiting consumers.  Pops and peeks will return indicating
       failure because they double check the queue state after they're notified.

   2.  Use *_Timed functions. These will indicate failure via the return value.
   
 The one to use depends on the situation.  Flushing all waiting consumers is
 really designed to be performed on shutdown after waiting for threads to
 terminate (with a timeout).  The flush will ensure that consumer threads can
 unblock and exit normally, giving them a chance to release resources.       

 *_Slaved_* functions
 ....................
 
 These functions are for accessing multiple queue's in lock-step.  This is 
 useful when different types of data are pushed on to different queue's at
 the same source.
 
 These functions operate by first locking and waiting on the master queue
 (to have data for example).  Once the master is no longer waiting, it is 
 assumed the other queues have been updated.  The push/pop/peek is performed
 using the same index as the master queue (that is, they use the master's head
 and tail index).
 
 This seems a bit unsafe since the slaves might not have been updated.
 The choice of master may be important here.

 Notes to self:
 ..............
 - This seems very similar in intention to the SwapChain in DirectX10.
 
*/

#include "stdafx.h"
#include "ring-fifo.h"

typedef struct _asynq
{ RingFIFO *q;  

  u32 ref_count;
  u32 waiting_producers;
  u32 waiting_consumers;
  
  CRITICAL_SECTION   lock;            // mutex
  HANDLE             notify_space;    // triggered when queue is not full (has available space)
  HANDLE             notify_data;     // triggered when queue is not empty (has available data).
  HANDLE             notify_abort;    // triggered to abort any pending operations (e.g. for shutdown).
} asynq;

asynq *Asynq_Alloc   ( size_t buffer_count, size_t buffer_size_bytes );
asynq *Asynq_Ref     ( asynq *self );
int    Asynq_Unref   ( asynq *self );

unsigned int Asynq_Push       ( asynq *self, void **pbuf, int expand_on_full );
unsigned int Asynq_Push_Copy  ( asynq *self, void  *buf,  int expand_on_full );
unsigned int Asynq_Push_Try   ( asynq *self, void **pbuf );
unsigned int Asynq_Push_Timed ( asynq *self, void **pbuf, DWORD timeout_ms );

unsigned int Asynq_Pop          ( asynq *self, void **pbuf );
unsigned int Asynq_Pop_Try      ( asynq *self, void **pbuf );
unsigned int Asynq_Pop_Copy_Try ( asynq *self, void  *buf  );
unsigned int Asynq_Pop_Timed    ( asynq *self, void **pbuf, DWORD timeout_ms );

unsigned int Asynq_Peek       ( asynq *self, void  *buf );
unsigned int Asynq_Peek_Try   ( asynq *self, void  *buf );
unsigned int Asynq_Peek_Timed ( asynq *self, void  *buf, DWORD timeout_ms );

unsigned int Asynq_Slaved_Peek       ( asynq **qs, unsigned int nelem, unsigned int imaster, void **bufs );
unsigned int Asynq_Slaved_Peek_Timed ( asynq **qs, unsigned int nelem, unsigned int imaster, void **bufs, DWORD timeout_ms );


       void  Asynq_Flush_Waiting_Consumers( asynq *self );

inline void  Asynq_Lock    ( asynq *self );
inline void  Asynq_Unlock  ( asynq *self );

int Asynq_Is_Full( asynq *self );
int Asynq_Is_Empty( asynq *self );

void*  Asynq_Token_Buffer_Alloc( asynq *self );
void*  Asynq_Token_Buffer_Alloc_And_Copy ( asynq *self, void *src );
void   Asynq_Token_Buffer_Free ( void *buf );

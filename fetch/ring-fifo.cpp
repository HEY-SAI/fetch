#include "stdafx.h"
#include "ring-fifo.h"

#define DEBUG_RINGFIFO_ALLOC
#define DEBUG_RINGFIFO_EXPAND

#if 1
#define DEBUG_RING_FIFO_PUSH
#endif

TYPE_VECTOR_DEFINE(PVOID);

RingFIFO*
RingFIFO_Alloc(size_t buffer_count, size_t buffer_size_bytes )
{ RingFIFO *self = (RingFIFO *)Guarded_Malloc( sizeof(RingFIFO), "RingFIFO_Alloc" );
  
  Guarded_Assert( IS_POW2( buffer_count ) );

  self->head = 0;
  self->tail = 0;
  self->buffer_size_bytes = buffer_size_bytes;

  self->ring = vector_PVOID_alloc( buffer_count );
  { vector_PVOID *r = self->ring;
    PVOID *cur = r->contents + r->nelem,
          *beg = r->contents;
    while( cur-- > beg )
      *cur = Guarded_Malloc( buffer_size_bytes, "RingFIFO_Alloc: Allocating buffers" );
  }

#ifdef DEBUG_RINGFIFO_ALLOC
  Guarded_Assert( RingFIFO_Is_Empty(self) );
  Guarded_Assert(!RingFIFO_Is_Full (self) );
#endif
  return self;
}

void 
RingFIFO_Free( RingFIFO *self )
{ return_if_fail(self);
  if( self->ring )
  { vector_PVOID *r = self->ring;
    PVOID *cur = r->contents + r->nelem,
          *beg = r->contents;
    while( cur-- > beg )
      free(*cur);
    vector_PVOID_free( r );
    self->ring = NULL;    
  }
  free(self);	
}

void 
RingFIFO_Expand( RingFIFO *self ) 
{ vector_PVOID *r = self->ring;
  size_t old  = r->nelem,   // size of ring buffer _before_ realloc
         n,                 // delta in size (number of added elements)
         head = MOD_UNSIGNED_POW2( self->head, old ),
         tail = MOD_UNSIGNED_POW2( self->tail, old ),
         buffer_size_bytes = self->buffer_size_bytes;

  vector_PVOID_request_pow2( r, old+1 ); // size to next pow2  
  n = r->nelem - old; // the number of slots added
    
  { PVOID *buf = r->contents,
          *beg = buf,     // (will be) beginning of interval requiring new malloced data
          *cur = buf + n; // (will be) end       "  "        "         "   "        "
    
    // Need to gaurantee that the new interval is outside of the 
    //    tail -> head interval (active data runs from tail to head).
    if( head < tail ) 
    { // Move data to end
      size_t nelem = old-tail;  // size of terminal interval
      beg += tail;              // source
      cur += tail;              // dest
      if( n > nelem ) memcpy ( cur, beg, nelem * sizeof(PVOID) ); // no overlap - this should be the common case
      else            memmove( cur, beg, nelem * sizeof(PVOID) ); // some overlap
      // adjust indices
      self->head = head + r->nelem; // want to maintain head-tail == # queue items
      self->tail = tail;
    } else // tail < head - no need to move data
    { //adjust indices
      self->head += tail - self->tail;
      self->tail = tail;
      
      //setup interval for mallocing new data
      beg += old;
      cur += old;
    }
    while( cur-- > beg )
      *cur = Guarded_Malloc( buffer_size_bytes, 
                             "RingFIFO_Expand: Allocating new buffers" );
  }
}

static inline size_t
_swap( RingFIFO *self, void **pbuf, size_t idx)
{ vector_PVOID *r = self->ring;                  // taking the mod on read (here) saves some ops                     
  idx = MOD_UNSIGNED_POW2( idx, r->nelem );      //   relative to on write since that involve writing to a reference.
  { void **cur = r->contents + idx,              // in pop/push idx could overflow, but that 
          *t   = *cur;                           //   would be ok since r->nelem is a divisor of
    *cur  = *pbuf;                               //   2^sizeof(size_t)
    *pbuf = t;                                   // Note that the ...Is_Empty and ...Is_Full 
  }                                              //   constructs still work with the possibility of
  return idx;                                    //   an overflow
}

extern inline 
unsigned int
RingFIFO_Pop( RingFIFO *self, void **pbuf)
{ return_val_if( RingFIFO_Is_Empty(self), 1);
  _swap( self, pbuf, self->tail++ );
  return 0;
}

extern inline 
unsigned int
RingFIFO_Peek( RingFIFO *self, void *buf)
{ return_val_if( RingFIFO_Is_Empty(self), 1);
  { vector_PVOID *r = self->ring;
    memcpy( buf, 
            r->contents[MOD_UNSIGNED_POW2(self->tail, r->nelem)],
            self->buffer_size_bytes );
  }
  return 0;
}

extern inline 
unsigned int
RingFIFO_Push_Try( RingFIFO *self, void **pbuf)
{ if( RingFIFO_Is_Full(self) )
    return 1;
  _swap( self, pbuf, self->head++ );
  return 0;
}

unsigned int
RingFIFO_Push( RingFIFO *self, void **pbuf, int expand_on_full)
{ unsigned int retval = 0;
  return_val_if( RingFIFO_Push_Try(self, pbuf)==0, 0 );
  // Handle when full
  if( expand_on_full )      // Expand
  { RingFIFO_Expand(self);
#ifdef DEBUG_RING_FIFO_PUSH
    Guarded_Assert( !RingFIFO_Is_Full(self) );  // FIXME: Once this is tested it can be removed
    Guarded_Assert( !RingFIFO_Is_Empty(self) ); // FIXME: Once this is tested it can be removed
#endif    
  } else {                  // Overwrite    
    self->tail++;    
  }
  _swap( self, pbuf, self->head++ );
  return !expand_on_full;
}

void*
RingFIFO_Alloc_Token_Buffer( RingFIFO *self )
{ return Guarded_Malloc( self->buffer_size_bytes, 
                         "RingFIFO_Alloc_Token_Buffer" );
}

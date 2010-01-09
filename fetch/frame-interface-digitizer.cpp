#pragma once
#include "stdafx.h"
#include "frame.h"
#include "frame-interface-digitizer.h"

#if 0
#define DEBUG_FRAME_INTERFACE_DIGITIZER_INTERLEAVED_PLANES
#endif

//
// Defaults
// These functions will probably be resused for different frame formats
//

size_t
frame_interface_digitizer__default__get_nchannels (Frame_Descriptor* fd)
{ Digitizer_Frame_Metadata *meta = (Digitizer_Frame_Metadata*) fd->metadata;
  return meta->nchan;
}

size_t
frame_interface_digitizer__default__get_nbytes (Frame_Descriptor* fd)
{ DEBUG_FRAME_INTERFACE_DIGITIZER_INTERLEAVED_PLANES__CHECK_DESCRIPTOR;
  Digitizer_Frame_Metadata *meta = (Digitizer_Frame_Metadata*) fd->metadata;
  return meta->width * meta->height * meta->Bpp;
}

void
frame_interface_digitizer__default__copy_channel( Frame_Descriptor* fd, void *dst, void *src, size_t ichan )
{ DEBUG_FRAME_INTERFACE_DIGITIZER_INTERLEAVED_PLANES__CHECK_DESCRIPTOR;
  Digitizer_Frame_Metadata        *meta = (Digitizer_Frame_Metadata*) fd->metadata;  
  size_t                    chan_stride = meta->width * meta->height * meta->Bpp;
  memcpy( dst, (u8*)src + chan_stride * ichan , chan_stride );
  return;
}

void
frame_interface_digitizer__default__get_dimensions ( Frame_Descriptor* fd, vector_size_t *vdim )
{ DEBUG_FRAME_INTERFACE_DIGITIZER_INTERLEAVED_PLANES__CHECK_DESCRIPTOR;
  Digitizer_Frame_Metadata        *meta = (Digitizer_Frame_Metadata*) fd->metadata;
  vector_size_t_request( vdim, 2 );
  vdim->count       = 2;
  vdim->contents[0] = meta->width;
  vdim->contents[1] = meta->height;
}

//
// Interleaved planes
// Each channel is in it's own contiguous memory block
//
#ifdef DEBUG_FRAME_INTERFACE_DIGITIZER_INTERLEAVED_PLANES
#define DEBUG_FRAME_INTERFACE_DIGITIZER_INTERLEAVED_PLANES__CHECK_DESCRIPTOR\
  { Guarded_Assert( fd->interface_id    == FRAME_INTERFACE_DIGITIZER_INTERLEAVED_PLANES__INTERFACE_ID );\
    Guarded_Assert( fd->metadata_nbytes == FRAME_INTERFACE_DIGITIZER_INTERLEAVED_PLANES__METADATA_BYTES;\
  }
#else
  #define DEBUG_FRAME_INTERFACE_DIGITIZER_INTERLEAVED_PLANES__CHECK_DESCRIPTOR
#endif

void
frame_interface_digitizer_interleaved_planes__copy_channel( Frame_Descriptor* fd, void *dst, void *src, size_t ichan )
{ DEBUG_FRAME_INTERFACE_DIGITIZER_INTERLEAVED_PLANES__CHECK_DESCRIPTOR;
  Digitizer_Frame_Metadata        *meta = (Digitizer_Frame_Metadata*) fd->metadata;  
  size_t                    chan_stride = meta->width * meta->height * meta->Bpp;
  memcpy( dst, (u8*)src + chan_stride * ichan , chan_stride );
  return;
}

//
// Interleaved lines
// Within a line, each channel is contiguous in memory.
//

#ifdef DEBUG_FRAME_INTERFACE_DIGITIZER_INTERLEAVED_LINES
#define DEBUG_FRAME_INTERFACE_DIGITIZER_INTERLEAVED_LINES__CHECK_DESCRIPTOR\
  { Guarded_Assert( fd->interface_id    == FRAME_INTERFACE_DIGITIZER_INTERLEAVED_LINES__INTERFACE_ID );\
    Guarded_Assert( fd->metadata_nbytes == FRAME_INTERFACE_DIGITIZER_INTERLEAVED_LINES__METADATA_BYTES;\
  }
#else
  #define DEBUG_FRAME_INTERFACE_DIGITIZER_INTERLEAVED_LINES__CHECK_DESCRIPTOR
#endif

void
frame_interface_digitizer_interleaved_lines__copy_channel( Frame_Descriptor* fd, void *dst, void *src, size_t ichan )
{ DEBUG_FRAME_INTERFACE_DIGITIZER_INTERLEAVED_LINES__CHECK_DESCRIPTOR;
  Digitizer_Frame_Metadata        *meta = (Digitizer_Frame_Metadata*) fd->metadata;  
  size_t                         nlines = meta->height,
                            line_nbytes = meta->width * meta->Bpp,
                           plane_nbytes = nlines * line_nbytes,
                                 stride = line_nbytes * meta->nchan;
  u8 *src_cur = (u8*)src + stride * nlines,
     *dst_cur = (u8*)dst + plane_nbytes;
  while( src_cur > src )
  { src_cur -= stride;
    dst_cur -= line_nbytes;
    memcpy( dst_cur, src_cur, plane_nbytes );
  }
  return;
}
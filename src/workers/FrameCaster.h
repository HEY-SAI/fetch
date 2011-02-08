/*
 * FrameCast.h
 *
 *  Created on: Apr 22, 2010
 *      Author: Nathan Clack <clackn@janelia.hhmi.org>
 */
/*
 * Copyright 2010 Howard Hughes Medical Institute.
 * All rights reserved.
 * Use is subject to Janelia Farm Research Campus Software Copyright 1.1
 * license terms (http://license.janelia.org/license/jfrc_copyright_1_1.html).
 */
/*
 * FrameCast
 * =========
 *
 * WorkTask that transforms the pixel type of Frames.
 *
 * Usage
 * -----
 * Scanner2D source;
 * WorkAgent<FrameCast<f32>> step1(source,0,NULL);
 */
#pragma once

#include "WorkTask.h"
#include "util/util-image.h"
#include "frame.h"
#include "types.h"

namespace fetch
{

  namespace task
  {
    
    template<typename Tdst>
    class FrameCast : public fetch::task::OneToOneWorkTask<Frame>
    {
    public:
        unsigned int reshape(IDevice *d, Frame *dst);
        unsigned int work(IDevice *dc, Frame *fdst, Frame *fsrc);
        
        virtual void alloc_output_queues(IDevice *dc);
    };

    

    /*
     * Implementation
     */    
    template<typename Tdst>
    inline unsigned int
      FrameCast<Tdst>::
      reshape(IDevice *idc, Frame *fdst)
    { 
      fdst->rtti = TypeID<Tdst> ();
      fdst->Bpp  = g_type_attributes[TypeID<Tdst> ()].bytes;

      return 1; // success
    }


    template<typename Tdst>
    inline unsigned int
    FrameCast<Tdst>::
    work(IDevice *idc, Frame *fdst, Frame *fsrc)
    { 
      void  *s;
      Tdst  *d;
      size_t dst_pitch[4], src_pitch[4], n[3];
      fdst->compute_pitches(dst_pitch);
      fsrc->compute_pitches(src_pitch);
      fsrc->get_shape(n);
      
      //debug("In FrameCast::work.\r\n");      
      //fsrc->dump("FrameCast-src.%s",TypeStrFromID(fsrc->rtti));
      
      s = fsrc->data;
      d = (Tdst*)fdst->data;
      switch (fsrc->rtti) {
        case id_u8 : imCastCopy<Tdst,u8 >(d,dst_pitch,(u8* )s,src_pitch,n); break;
        case id_u16: imCastCopy<Tdst,u16>(d,dst_pitch,(u16*)s,src_pitch,n); break;
        case id_u32: imCastCopy<Tdst,u32>(d,dst_pitch,(u32*)s,src_pitch,n); break;
        case id_u64: imCastCopy<Tdst,u64>(d,dst_pitch,(u64*)s,src_pitch,n); break;
        case id_i8 : imCastCopy<Tdst,i8 >(d,dst_pitch,(i8* )s,src_pitch,n); break;
        case id_i16: imCastCopy<Tdst,i16>(d,dst_pitch,(i16*)s,src_pitch,n); break;
        case id_i32: imCastCopy<Tdst,i32>(d,dst_pitch,(i32*)s,src_pitch,n); break;
        case id_i64: imCastCopy<Tdst,i64>(d,dst_pitch,(i64*)s,src_pitch,n); break;
        case id_f32: imCastCopy<Tdst,f32>(d,dst_pitch,(f32*)s,src_pitch,n); break;
        case id_f64: imCastCopy<Tdst,f64>(d,dst_pitch,(f64*)s,src_pitch,n); break;
        default:
          warning("Could not interpret source type.\r\n");
          return 0; // failure
          break;
      }
      
      //fdst->dump("FrameCast-dst.%s",TypeStrFromID(fdst->rtti));
      
      return 1; // success
    }
    
    template<typename Tdst>
    void
    FrameCast<Tdst>::
    alloc_output_queues(IDevice *dc)
    { // Allocates an output queue on out[0] that has matching storage to in[0].      
      dc->_alloc_qs_easy(&dc->_out,
                            1,                                               // number of output channels to allocate
                            Chan_Buffer_Count(dc->_in->contents[0]),         // copy number of output buffers from input queue
                            Chan_Buffer_Size_Bytes(dc->_in->contents[0])*sizeof(Tdst)); // copy buffer size from input queue - prepare for worst case
    }
  } 
      
  namespace worker
  { 
    typedef WorkAgent<task::FrameCast<u8 >> FrameCastAgent_u8;
    typedef WorkAgent<task::FrameCast<u16>> FrameCastAgent_u16;
    typedef WorkAgent<task::FrameCast<u32>> FrameCastAgent_u32;
    typedef WorkAgent<task::FrameCast<u64>> FrameCastAgent_u64;
    typedef WorkAgent<task::FrameCast<i8 >> FrameCastAgent_i8;
    typedef WorkAgent<task::FrameCast<i16>> FrameCastAgent_i16;
    typedef WorkAgent<task::FrameCast<i32>> FrameCastAgent_i32;
    typedef WorkAgent<task::FrameCast<i64>> FrameCastAgent_i64;
    typedef WorkAgent<task::FrameCast<f32>> FrameCastAgent_f32;
    typedef WorkAgent<task::FrameCast<f64>> FrameCastAgent_f64;
  }
}




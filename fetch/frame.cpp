#include "stdafx.h"
#include "frame.h"

#define DEBUG_FRAME_DESCRIPTOR_TO_FILE

//----------------------------------------------------------------------------
// PROTOCOL TABLE
//----------------------------------------------------------------------------

#include "frame-interface-digitizer.h"

Frame_Interface g_interfaces[] = {
  { frame_interface_digitizer_get_nchannels,   // 0 - frame-interface-digitizer
    frame_interface_digitizer_get_nbytes,
    frame_interface_digitizer_get_channel,
    frame_interface_digitizer_get_dimensions
  },
};


//----------------------------------------------------------------------------
// Frame_Descriptor_Get_Interface
//----------------------------------------------------------------------------
Frame_Interface*
Frame_Descriptor_Get_Interface( Frame_Descriptor *self )
{ int n = sizeof(g_interfaces)/sizeof(Frame_Interface);
  Guarded_Assert( self->interface_id < n );
  
  return g_interfaces + self->interface_id;
}

//----------------------------------------------------------------------------
// Frame_Descriptor_Initialize
//----------------------------------------------------------------------------
void
Frame_Descriptor_Change( Frame_Descriptor *self, u8 interface_id, void *metadata, size_t nbytes )
{ self->is_change = 1;
  self->interface_id = interface_id;
  
  Guarded_Assert( nbytes < FRAME_DESCRIPTOR_MAX_METADATA_BYTES );
  memcpy( self->metadata, metadata, nbytes );
  self->metadata_nbytes = nbytes;
}

//----------------------------------------------------------------------------
// Frame_Descriptor_To_File
//----------------------------------------------------------------------------
void
_frame_descriptor_write( FILE *fp, Frame_Descriptor *self, u64 count )
{ Guarded_Assert(1== fwrite( (void*) &self->interface_id,    sizeof( self->interface_id ), 1, fp ) );
  Guarded_Assert(1== fwrite( (void*) &count,                 sizeof( u64 ),                1, fp ) );
  Guarded_Assert(1== fwrite( (void*) &self->metadata_nbytes, sizeof( u32 ),                1, fp ) );
  Guarded_Assert(1== fwrite( (void*) &self->metadata,        self->metadata_nbytes,        1, fp ) );
}

void
Frame_Descriptor_To_File( FILE *fp, Frame_Descriptor *self )
{ size_t count = 1;
  size_t item_bytes = sizeof( self->interface_id ) 
                    + sizeof( count )
                    + self->metadata_nbytes;
  if( self->is_change )                 // add new record
  { _frame_descriptor_write(fp, self, 1 );
  } else
  { Frame_Descriptor last;              // update last record (read and overwrite)
    Guarded_Assert(0== fseek( fp, -(long)item_bytes, SEEK_CUR ) );    
    Guarded_Assert(    Frame_Descriptor_From_File_Read_Next( fp, &last, &count) );
#ifdef DEBUG_FRAME_DESCRIPTOR_TO_FILE
    Guarded_Assert( last.interface_id == self->interface_id );
#endif
    Guarded_Assert(0== fseek( fp, -(long)item_bytes, SEEK_CUR ) );
    _frame_descriptor_write(fp, &last, count+1 );
  }
}

//----------------------------------------------------------------------------
// Frame_Descriptor_From_File_Read_Next
//----------------------------------------------------------------------------
u8
Frame_Descriptor_From_File_Read_Next( FILE *fp, Frame_Descriptor *dst, size_t *repeat_count )
{ if( 1!= fread( &dst->interface_id,    sizeof( dst->interface_id ), 1, fp ) ) goto err;
  if( 1!= fread( repeat_count,          sizeof( u64 ),               1, fp ) ) goto err;
  if( 1!= fread( &dst->metadata_nbytes, sizeof( u32 ),               1, fp ) ) goto err;
  if( 1!= fread( &dst->metadata,        dst->metadata_nbytes,        1, fp ) ) goto err;
  return 1; // success
err:
  Guarded_Assert( feof(fp) );  
  return 0; // failed to read (eof)   
}
// Video Frame Resource
// --------------------
//
// This object encapsulates an image resource bound to the video display hardware.
//
// This particular implimentation is for a directx10 texture.  Composition and display
// are driven through a ID3D10Effect (a shader).
//

#pragma once
#include "stdafx.h"
#include <d3d10.h>
#include <d3dx10.h>
#include "frame.h"
#include "types.h"

typedef struct _t_video_frame_resource
{ ID3D10ShaderResourceView           *srv;
  ID3D10Texture3D                    *tex;
  ID3D10EffectShaderResourceVariable *shader_variable;
  vector_u8                          *buf;
  Basic_Type_ID                       type;
  UINT                                nchan;
  size_t                              nlines;
  size_t                              stride;                // line pitch in bytes
} Video_Frame_Resource;

#define EMPTY_VIDEO_FRAME_RESOURCE {NULL,NULL,NULL,NULL,0,0,0}

Video_Frame_Resource *Video_Frame_Resource_Alloc  ( void);
void                  Video_Frame_Resource_Free   ( Video_Frame_Resource *self );

void                  Video_Frame_Resource_Attach ( Video_Frame_Resource *self,
                                                    Basic_Type_ID         type,
                                                    UINT                 width,    // width  of frame in samples
                                                    UINT                height,    // height of frame in samples
                                                    UINT                 nchan,    // the number of channels.  Each channel gets its own colormap.
                                                    ID3D10Effect       *effect,    // bind to this effect
                                                    const char           *name );  // bind to this variable in the effect
void                  Video_Frame_Resource_Detach ( Video_Frame_Resource *self );
void                  Video_Frame_Resource_Commit ( Video_Frame_Resource *self );

void                  Video_Frame_From_Frame_Descriptor ( Video_Frame_Resource *self, void *buf,
                                                          Frame_Descriptor *desc );
void                  Video_Frame_From_Raw              ( Video_Frame_Resource *self, void *buf, 
                                                          size_t Bpp, size_t width, size_t height, size_t nchan );
                                                          
void                  Video_Frame_Autolevel( Video_Frame_Resource *self, int ichan, float thresh, float *min, float *max ); // Computes min and max for colormapping using last stored frame.

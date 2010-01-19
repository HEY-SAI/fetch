#pragma once

#include "stdafx.h"
#include <d3d10.h>
#include <d3dx10.h>

typedef struct _t_colormap_resource
{ ID3D10ShaderResourceView           *resource_view;
  ID3D10Texture2D                    *texture;
  ID3D10EffectShaderResourceVariable *resource_variable;
  UINT                                nchan;
  size_t                              stride;
} Colormap_Resource;

#define EMPTY_COLORMAP_RESOURCE {NULL,NULL,NULL,0,0}

Colormap_Resource *Colormap_Resource_Alloc  (void);
void               Colormap_Resource_Free   (Colormap_Resource *cmap);

void               Colormap_Resource_Attach (Colormap_Resource *cmap,    // self
                                             UINT               nelem,   // width of colormaps in samples
                                             UINT               nchan,   // the number of channels.  Each channel gets its own colormap.
                                             ID3D10Effect      *effect,  // bind to this effect
                                             const char        *name);   // bind to this variable in the effect
void               Colormap_Resource_Detach (Colormap_Resource *cmap);
void               Colormap_Resouce_Fill    (Colormap_Resource *cmap, UINT ichan, f32 *bytes, size_t nbytes);

// Single Channel Colormap functions
// ---------------------------------
// Each generates a colormap for a single channel and binds it to the colormap resource.
// These are not optimized for speed.
//
// <min> and <max> determine where a linear map crosses 0 and 1.
// That is: 0.0 = slope * <min> + intercept and
//          1.0 = slope * <max> + intercept.
// <min> and <max> are normalized to the number of samples in the colormap.
// i.e. The beginning of the colormap is 0.0 and then end 1.0.
// 
void               Colormap_Black         ( Colormap_Resource *cmap, UINT ichan);

void               Colormap_Gray          ( Colormap_Resource *cmap, UINT ichan, float min, float max );
void               Colormap_Inverse_Gray  ( Colormap_Resource *cmap, UINT ichan, float min, float max );

void               Colormap_Red           ( Colormap_Resource *cmap, UINT ichan, float min, float max );
void               Colormap_Blue          ( Colormap_Resource *cmap, UINT ichan, float min, float max );
void               Colormap_Green         ( Colormap_Resource *cmap, UINT ichan, float min, float max );
void               Colormap_Alpha         ( Colormap_Resource *cmap, UINT ichan, float min, float max );

void               Colormap_HSV_Hue       ( Colormap_Resource *cmap, UINT ichan, float S, float V, float A, float min, float max );
void               Colormap_HSV_Hue_Value ( Colormap_Resource *cmap, UINT ichan, float S, float A         , float min, float max );
void               Colormap_HSV_Saturation( Colormap_Resource *cmap, UINT ichan, float H, float V, float A, float min, float max );
void               Colormap_HSV_Value     ( Colormap_Resource *cmap, UINT ichan, float H, float S, float A, float min, float max );
void               Colormap_HSV_Alpha     ( Colormap_Resource *cmap, UINT ichan, float H, float S, float V, float min, float max );

// Multichannel Colormap functions
// -------------------------------
void               Colormap_Autosetup     ( Colormap_Resource *cmap, float *min, float *max );
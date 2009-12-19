#include "stdafx.h"
#include "render-colormap.h"

#define CLAMP(v,low,high) ((v)<(low))?(low):(((v)>(high))?(high):(v))

Colormap_Resource*
Colormap_Resource_Alloc (void)
{ return (Colormap_Resource*) Guarded_Calloc( sizeof(Colormap_Resource), 1, "Colormap_Resource_Alloc");
}

void
Colormap_Resource_Free(Colormap_Resource *cmap)
{ if(cmap)
  { //ensure detached
    if(cmap->texture) // then the others got attached too
      Colormap_Resource_Detach(cmap);
    free(cmap);
  }
}

inline void 
_create_texture(ID3D10Device *device, ID3D10Texture1D **pptex, UINT width )
{ D3D10_TEXTURE1D_DESC desc;
  ZeroMemory( &desc, sizeof(desc) );  
  desc.Width                         = width;
  desc.MipLevels = desc.ArraySize    = 1;
  desc.Format                        = DXGI_FORMAT_R32G32B32A32_FLOAT;  
  desc.Usage                         = D3D10_USAGE_DYNAMIC;
  desc.CPUAccessFlags                = D3D10_CPU_ACCESS_WRITE;
  desc.BindFlags                     = D3D10_BIND_SHADER_RESOURCE;
  
  Guarded_Assert(SUCCEEDED(
    device->CreateTexture1D( &desc, NULL, pptex ) ));
}

inline void
_bind_texture_to_shader_variable(ID3D10Device *device, Colormap_Resource *cmap )
{ D3D10_SHADER_RESOURCE_VIEW_DESC srvDesc;
  D3D10_RESOURCE_DIMENSION        type;
  D3D10_TEXTURE1D_DESC            desc;
  ID3D10Texture1D                *tex = cmap->texture;
  
  tex->GetDesc( &desc );
  tex->GetType( &type );
  Guarded_Assert( type == D3D10_RESOURCE_DIMENSION_TEXTURE1D );

  srvDesc.Format                    = desc.Format;
  srvDesc.ViewDimension             = D3D10_SRV_DIMENSION_TEXTURE1D;
  srvDesc.Texture1D.MipLevels       = desc.MipLevels;
  srvDesc.Texture1D.MostDetailedMip = desc.MipLevels - 1;

  Guarded_Assert(SUCCEEDED( 
    device->CreateShaderResourceView( tex, 
                                     &srvDesc, 
                                     &cmap->resource_view ) ));                                               
  Guarded_Assert(SUCCEEDED(
    cmap->resource_variable->SetResource( cmap->resource_view ) ));
}

void
Colormap_Resource_Attach (Colormap_Resource *cmap, UINT width, ID3D10Effect *effect, const char* name)
{ ID3D10Device *device = NULL;
  Guarded_Assert( effect->GetDevice(&device) );
  
  cmap->resource_variable = effect->GetVariableByName(name)->AsShaderResource();
  _create_texture( device, &cmap->texture, width );
  _bind_texture_to_shader_variable( device, cmap );
  device->Release();
}

void
Colormap_Resource_Detach (Colormap_Resource *cmap)
{ cmap->resource_view->Release();
  cmap->texture->Release();
  cmap->resource_view = NULL;
  cmap->texture       = NULL;
}

void
Colormap_Resource_Fill (Colormap_Resource *cmap, f32 *bytes, size_t nbytes)
{ void *data;
  ID3D10Texture1D       *dst = cmap->texture;
  Guarded_Assert( !FAILED( 
      dst->Map( D3D10CalcSubresource(0, 0, 1), D3D10_MAP_WRITE_DISCARD, 0, &data ) ));  
  memcpy(data, bytes, nbytes);
  dst->Unmap( D3D10CalcSubresource(0, 0, 1) );        
}

f32 *_linear_colormap_get_params( float x0, float x1, float sign, size_t nrows, float *slope, float *intercept, size_t *nbytes )
// Sizes static colormap buffer and computes slope and intercept
{ float m   =  sign/(x1-x0),
        b   = -m*x0;
  static f32 *rgba = NULL;
  static size_t n  = 0, sz;
  
  if(!rgba)               // alloc
  { sz = nrows*4*sizeof(f32);
    rgba = (f32*) Guarded_Malloc(sz , "_linear_colormap_params" );
  } else if( n != nrows ) // realloc
  { sz = nrows*4*sizeof(f32);
    Guarded_Realloc( (void**) &rgba, sz , "_linear_colormap_params" );
  }
  *nbytes = sz;
  *slope = m;
  *intercept = b;
  return rgba;
}

void
Colormap_Gray( Colormap_Resource *cmap, float min, float max, size_t nrows )
{ size_t nbytes;
  float m,b,
       *rgba = _linear_colormap_get_params( min, max, 1.0f, nrows, &m, &b, &nbytes );
  while(nrows--)
  { f32 *row = rgba + nrows;
    f32    v = m*nrows + b;    
    row[0] = row[1] = row[2] = CLAMP(v,0.0f,1.0f);
    row[3] = 1.0f;
  }      
  Colormap_Resource_Fill( cmap, rgba, nbytes );      
}

void Colormap_Inverse_Gray( Colormap_Resource *cmap, float min, float max, size_t nrows )
{ size_t nbytes;
  float m,b,
       *rgba = _linear_colormap_get_params( min, max, -1.0f, nrows, &m, &b, &nbytes );
  while(nrows--)
  { f32 *row = rgba + nrows;
    f32    v = m*nrows + b;
    row[0] = row[1] = row[2] = CLAMP(v,0.0f,1.0f);
    row[3] = 1.0f;
  }      
  Colormap_Resource_Fill( cmap, rgba, nbytes );
}

inline void 
_colormap_single_channel( Colormap_Resource *cmap, float min, float max, size_t nrows, int channel )
{ size_t nbytes;
  float m,b,
       *rgba = _linear_colormap_get_params( min, max, -1.0f, nrows, &m, &b, &nbytes );
  memset(rgba,0,nbytes);
  while(nrows--)
  { f32 *row = rgba + nrows;
    f32    v = m*nrows + b;    
    row[channel] = CLAMP(v,0.0f,1.0f);
    row[3] = 1.0f;
  }
  Colormap_Resource_Fill( cmap, rgba, nbytes );
}

void
Colormap_Red ( Colormap_Resource *cmap, float min, float max, size_t nrows )
{ _colormap_single_channel( cmap, min, max, nrows, 0 );
}

void
Colormap_Blue ( Colormap_Resource *cmap, float min, float max, size_t nrows )
{ _colormap_single_channel( cmap, min, max, nrows, 1 );
}

void
Colormap_Green ( Colormap_Resource *cmap, float min, float max, size_t nrows )
{ _colormap_single_channel( cmap, min, max, nrows, 2 );
}

void
Colormap_Alpha ( Colormap_Resource *cmap, float min, float max, size_t nrows )
{ _colormap_single_channel( cmap, min, max, nrows, 3 );
}

inline 
void _convert_single_hsva_to_rgba( f32 *hsva, f32* rgba)
{ f32 H = 6.0f* hsva[0], V = hsva[2],
      S = hsva[1]      , A = hsva[3];
  f32 f,p,ih;
                         // v  t/q  p
  static int idx[][3] = { { 0 , 1 , 2 }, // 0 - t
                          { 1 , 0 , 2 }, // 1 - q
                          { 1 , 2 , 0 }, // 2 - t
                          { 2 , 1 , 0 }, // 3 - q
                          { 2 , 0 , 1 }, // 4 - t
                          { 1 , 2 , 1 }};// 5 - q
  static f32 q,t,
            *qt[] = {&q, &t};
  
  ih = floorf(H);
  f  = H - ih;
  p  = V * (1.0f -       S);
  q  = V * (1.0f -    f *S);
  t  = V * (1.0f - (1-f)*S);
  
  { unsigned int i = (unsigned int) ih;
    rgba[ idx[i][0] ] = V;
    rgba[ idx[i][1] ] = *qt[i&1];
    rgba[ idx[i][2] ] = p;
    rgba[ 3         ] = hsva[3];
  }
}

inline 
void _convert_array_hsva_to_rgba( f32 *hsva, f32* rgba, size_t nrows )
// all hsva and rbga values are in [0,1]
{ typedef struct {f32 h; f32 s; f32 v; f32 a;} T;
  T   *sb = (T*)hsva,
      *s  = sb + nrows,
      *db = (T*)rgba,
      *d  = db + nrows;
  while( s-- > sb )
    _convert_single_hsva_to_rgba((f32*)s,(f32*)--d);
}

inline void
_set_channel_to_constant( f32 *rgba, size_t nrows, int channel, f32 v )
{ f32 *beg = rgba, *cur = rgba + 4*nrows;
  while((cur-=4) > beg)
    cur[channel] = v;
}

inline void
_set_channel_linear_ramp( f32 *rgba, size_t nrows, int channel, f32 m, f32 b )
{ while(nrows--)
  { f32 v =  m * nrows + b; 
    rgba[channel+4*nrows] = CLAMP(v,0.0f,1.0f);
  }
} 

void Colormap_HSV_Hue( Colormap_Resource *cmap, float S, float V, float A, float min, float max, size_t nrows )
{ size_t nbytes;
  float m,b,
       *hsva = _linear_colormap_get_params( min, max, 1.0f, nrows, &m, &b, &nbytes );
  _set_channel_linear_ramp( hsva, nrows, 0, m, b );
  _set_channel_to_constant( hsva, nrows, 1, S );
  _set_channel_to_constant( hsva, nrows, 2, V );
  _set_channel_to_constant( hsva, nrows, 3, A );
  
  _convert_array_hsva_to_rgba(hsva,hsva,nrows);
  Colormap_Resource_Fill( cmap, hsva, nbytes );
}

void Colormap_HSV_Saturation( Colormap_Resource *cmap, float H, float V, float A, float min, float max, size_t nrows )
{ size_t nbytes;
  float m,b,
       *hsva = _linear_colormap_get_params( min, max, 1.0f, nrows, &m, &b, &nbytes );  
  _set_channel_to_constant( hsva, nrows, 0, H );
  _set_channel_linear_ramp( hsva, nrows, 1, m, b );
  _set_channel_to_constant( hsva, nrows, 2, V );
  _set_channel_to_constant( hsva, nrows, 3, A );
  
  _convert_array_hsva_to_rgba(hsva,hsva,nrows);
  Colormap_Resource_Fill( cmap, hsva, nbytes );
}

void Colormap_HSV_Value( Colormap_Resource *cmap, float H, float S, float A, float min, float max, size_t nrows )
{ size_t nbytes;
  float m,b,
       *hsva = _linear_colormap_get_params( min, max, 1.0f, nrows, &m, &b, &nbytes );
  _set_channel_to_constant( hsva, nrows, 0, H );
  _set_channel_to_constant( hsva, nrows, 1, S );
  _set_channel_linear_ramp( hsva, nrows, 2, m, b );
  _set_channel_to_constant( hsva, nrows, 3, A );
  
  _convert_array_hsva_to_rgba(hsva,hsva,nrows);
  Colormap_Resource_Fill( cmap, hsva, nbytes );
}

void Colormap_HSV_Alpha( Colormap_Resource *cmap, float H, float S, float V, float min, float max, size_t nrows )
{ size_t nbytes;
  float m,b,
       *hsva = _linear_colormap_get_params( min, max, 1.0f, nrows, &m, &b, &nbytes );
  _set_channel_to_constant( hsva, nrows, 0, H );
  _set_channel_to_constant( hsva, nrows, 1, S );
  _set_channel_to_constant( hsva, nrows, 2, V );
  _set_channel_linear_ramp( hsva, nrows, 3, m, b );
  
  _convert_array_hsva_to_rgba(hsva,hsva,nrows);
  Colormap_Resource_Fill( cmap, hsva, nbytes );
}
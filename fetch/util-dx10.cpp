#include "stdafx.h"
#include <d3d10.h>
#include <d3dx10.h>

void
dx10_effect_variable_set_f32( ID3D10Effect *effect, char *name, f32 val )
{ Guarded_Assert( SUCCEEDED(
    effect->GetVariableByName(name)->AsScalar()->SetFloat(val)
  ));
}

f32
dx10_effect_variable_get_f32( ID3D10Effect *effect, char *name )
{ f32 val;
  Guarded_Assert( SUCCEEDED(
    effect->GetVariableByName(name)->AsScalar()->GetFloat(&val)
  ));
  return val;
}

void
dx10_tex2d_dump( char *filename, ID3D10Texture2D *tex )
{ D3D10_MAPPED_TEXTURE2D data;
  D3D10_TEXTURE2D_DESC desc;
  size_t nbytes, bpp;  
  
  memset(&desc,0,sizeof( D3D10_TEXTURE2D_DESC ));
  tex->GetDesc( &desc );  
  
  Guarded_Assert(SUCCEEDED( 
      tex->Map( D3D10CalcSubresource(0, 0, 1),
                D3D10_MAP_READ, // map type
                0,              // map flags
                &data ) ));
  nbytes = data.RowPitch * desc.Height;
  
  { FILE *fp = fopen(filename, "wb");
    fwrite( data.pData, 1, nbytes, fp );
    fclose(fp);
  }
  
  tex->Unmap( D3D10CalcSubresource(0, 0, 1) );
}
//--------------------------------------------------------------------------------------
// File: Tutorial07.cpp
//
// This application demonstrates texturing
//
// Copyright (c) Microsoft Corporation. All rights reserved.
//--------------------------------------------------------------------------------------
#include "stdafx.h"
#include <d3d10.h>
#include <d3dx10.h>
#include "window-video.h"
#include <stdlib.h> // for rand - for testing - remove when done

#define VIDEO_WINDOW_TEXTURE_RESOURCE_NAME "tx"
#define VIDEO_WINDOW_PATH_TO_SHADER        "../fetch/shader.fx"
#define VIDEO_WINDOW_SHADER_TECHNIQUE_NAME "Render"

//--------------------------------------------------------------------------------------
// Structures
//--------------------------------------------------------------------------------------
struct SimpleVertex
{
    D3DXVECTOR3 Pos;
    D3DXVECTOR2 Tex;
};


//--------------------------------------------------------------------------------------
// Global Variables
//--------------------------------------------------------------------------------------
HWND                                g_hWnd = NULL;
D3D10_DRIVER_TYPE                   g_driverType = D3D10_DRIVER_TYPE_NULL;
ID3D10Device*                       g_pd3dDevice = NULL;
IDXGISwapChain*                     g_pSwapChain = NULL;
ID3D10RenderTargetView*             g_pRenderTargetView = NULL;
ID3D10Effect*                       g_pEffect = NULL;
ID3D10EffectTechnique*              g_pTechnique = NULL;
ID3D10InputLayout*                  g_pVertexLayout = NULL;
ID3D10Buffer*                       g_pVertexBuffer = NULL;
ID3D10Buffer*                       g_pIndexBuffer = NULL;
ID3D10ShaderResourceView*           g_pTextureRV = NULL;
ID3D10Texture2D*                    g_pStagingTexture = NULL;
ID3D10Texture2D*                    g_pActiveTexture = NULL;
ID3D10EffectShaderResourceVariable* g_pDiffuseVariable = NULL;


//struct t_video_display
//{ 
//  HWND                                hwnd;
//  D3D10_DRIVER_TYPE                   driver_type;
//  ID3D10Device*                       device;
//  IDXGISwapChain*                     swap_chain;
//  ID3D10RenderTargetView*             render_target_view;
//  ID3D10Effect*                       effect;
//  ID3D10EffectTechnique*              technique = NULL;
//  ID3D10InputLayout*                  vertex_layout = NULL;
//  ID3D10Buffer*                       g_pVertexBuffer = NULL;
//  ID3D10Buffer*                       g_pIndexBuffer = NULL;
//  ID3D10ShaderResourceView*           g_pTextureRV = NULL;
//  ID3D10Texture2D*                    g_pStagingTexture = NULL;
//  ID3D10Texture2D*                    g_pActiveTexture = NULL;
//  ID3D10EffectMatrixVariable*         g_pWorldVariable = NULL;
//  ID3D10EffectMatrixVariable*         g_pViewVariable = NULL;
//  ID3D10EffectMatrixVariable*         g_pProjectionVariable = NULL;
//  ID3D10EffectVectorVariable*         g_pMeshColorVariable = NULL;
//  ID3D10EffectShaderResourceVariable* g_pDiffuseVariable = NULL;
//  D3DXMATRIX                          g_World;
//  D3DXMATRIX                          g_View;
//  D3DXMATRIX                          g_Projection;
//  D3DXVECTOR4                         g_vMeshColor( 0.7f, 0.7f, 0.7f, 1.0f );
//};

//--------------------------------------------------------------------------------------
// Forward declarations
//--------------------------------------------------------------------------------------
HRESULT             _InitWindow( HINSTANCE hInstance, int nCmdShow );
HRESULT             _InitDevice();
void                _CleanupDevice();

//--------------------------------------------------------------------------------------
// Initializes everything.
//--------------------------------------------------------------------------------------
HWND Video_Window_Attach( HINSTANCE hInstance, int nCmdShow)
{ if( FAILED( _InitWindow( hInstance, nCmdShow ) ) )
    return NULL;

  if( FAILED( _InitDevice() ) )
  { _CleanupDevice();
    return NULL;
  }
  return g_hWnd;
}

//--------------------------------------------------------------------------------------
// Uninitializes everything.
//--------------------------------------------------------------------------------------
HRESULT Video_Window_Release()
{ _CleanupDevice();
  return S_OK;
}

//--------------------------------------------------------------------------------------
// Register class and create window
//--------------------------------------------------------------------------------------
HRESULT _InitWindow( HINSTANCE hInstance, int nCmdShow )
{
    // Register class
    WNDCLASSEX wcex;
    wcex.cbSize = sizeof( WNDCLASSEX );
    wcex.style = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc = Video_WndProc;
    wcex.cbClsExtra = 0;
    wcex.cbWndExtra = 0;
    wcex.hInstance = hInstance;
    wcex.hIcon = LoadIcon( hInstance, ( LPCTSTR )IDI_FETCH );
    wcex.hCursor = LoadCursor( NULL, IDC_ARROW );
    wcex.hbrBackground = ( HBRUSH )( COLOR_WINDOW + 1 );
    wcex.lpszMenuName = NULL;
    wcex.lpszClassName = "TutorialWindowClass";
    wcex.hIconSm = LoadIcon( wcex.hInstance, ( LPCTSTR )IDI_FETCH );
    if( !RegisterClassEx( &wcex ) )
        return E_FAIL;

    // Create window    
    RECT rc = { 0, 0, 640, 480 };
    AdjustWindowRect( &rc, WS_OVERLAPPEDWINDOW, FALSE );
    g_hWnd = CreateWindow( "TutorialWindowClass", 
                            "Direct3D 10 Tutorial 7",
                            WS_OVERLAPPEDWINDOW,
                            CW_USEDEFAULT,
                            CW_USEDEFAULT,
                            rc.right - rc.left,
                            rc.bottom - rc.top,
                            NULL,
                            NULL,
                            hInstance,
                            NULL );
    if( !g_hWnd )
        return E_FAIL;

    ShowWindow( g_hWnd, nCmdShow );

    return S_OK;
}


//--------------------------------------------------------------------------------------
// Create Direct3D device and swap chain
//--------------------------------------------------------------------------------------
HRESULT _InitDevice()
{
    HRESULT hr = S_OK;

    RECT rc;
    GetClientRect( g_hWnd, &rc );
    UINT width = rc.right - rc.left;
    UINT height = rc.bottom - rc.top;

    UINT createDeviceFlags = 0;
#ifdef _DEBUG
    createDeviceFlags |= D3D10_CREATE_DEVICE_DEBUG;
#endif

    D3D10_DRIVER_TYPE driverTypes[] =
    {
        D3D10_DRIVER_TYPE_HARDWARE,
        D3D10_DRIVER_TYPE_REFERENCE,
    };
    UINT numDriverTypes = sizeof( driverTypes ) / sizeof( driverTypes[0] );

    DXGI_SWAP_CHAIN_DESC sd;
    ZeroMemory( &sd, sizeof( sd ) );
    sd.BufferCount = 1;
    sd.BufferDesc.Width = width;
    sd.BufferDesc.Height = height;
    sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    sd.BufferDesc.RefreshRate.Numerator = 60;
    sd.BufferDesc.RefreshRate.Denominator = 1;
    sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    sd.OutputWindow = g_hWnd;
    sd.SampleDesc.Count = 1;
    sd.SampleDesc.Quality = 0;
    sd.Windowed = TRUE;

    for( UINT driverTypeIndex = 0; driverTypeIndex < numDriverTypes; driverTypeIndex++ )
    {
        g_driverType = driverTypes[driverTypeIndex];
        hr = D3D10CreateDeviceAndSwapChain( NULL, g_driverType, NULL, createDeviceFlags,
                                            D3D10_SDK_VERSION, &sd, &g_pSwapChain, &g_pd3dDevice );
        if( SUCCEEDED( hr ) )
            break;
    }
    if( FAILED( hr ) )
        return hr;

    // Create a render target view
    ID3D10Texture2D* pBuffer;
    hr = g_pSwapChain->GetBuffer( 0, __uuidof( ID3D10Texture2D ), ( LPVOID* )&pBuffer );
    if( FAILED( hr ) )
        return hr;

    hr = g_pd3dDevice->CreateRenderTargetView( pBuffer, NULL, &g_pRenderTargetView );
    pBuffer->Release();
    if( FAILED( hr ) )
        return hr;

    g_pd3dDevice->OMSetRenderTargets( 1, &g_pRenderTargetView, NULL );

    // Setup the viewport
    D3D10_VIEWPORT vp;
    vp.Width = width;
    vp.Height = height;
    vp.MinDepth = 0.0f;
    vp.MaxDepth = 1.0f;
    vp.TopLeftX = 0;
    vp.TopLeftY = 0;
    g_pd3dDevice->RSSetViewports( 1, &vp );

    // Create the effect
    DWORD dwShaderFlags = D3D10_SHADER_ENABLE_STRICTNESS;
#if defined( DEBUG ) || defined( _DEBUG )
    // Set the D3D10_SHADER_DEBUG flag to embed debug information in the shaders.
    // Setting this flag improves the shader debugging experience, but still allows 
    // the shaders to be optimized and to run exactly the way they will run in 
    // the release configuration of this program.
    dwShaderFlags |= D3D10_SHADER_DEBUG;
    #endif
    hr = D3DX10CreateEffectFromFile( VIDEO_WINDOW_PATH_TO_SHADER,
                                     NULL,
                                     NULL,
                                     "fx_4_0",
                                     dwShaderFlags,
                                     0,
                                     g_pd3dDevice,
                                     NULL,
                                     NULL,
                                     &g_pEffect,
                                     NULL,
                                     NULL );
    if( FAILED( hr ) )
    {   warning( "Could not compile the FX file.  Perhaps it could not be located.");
        return hr;
    }

    // Obtain the technique
    g_pTechnique = g_pEffect->GetTechniqueByName( VIDEO_WINDOW_SHADER_TECHNIQUE_NAME );

    // Obtain the variables
    g_pDiffuseVariable = g_pEffect->GetVariableByName( VIDEO_WINDOW_TEXTURE_RESOURCE_NAME )
                                  ->AsShaderResource();

    // Define the input layout
    D3D10_INPUT_ELEMENT_DESC layout[] =
    {
        { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D10_INPUT_PER_VERTEX_DATA, 0 },
        { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 12, D3D10_INPUT_PER_VERTEX_DATA, 0 },
    };
    UINT numElements = sizeof( layout ) / sizeof( layout[0] );

    // Create the input layout
    { D3D10_PASS_DESC PassDesc;
      g_pTechnique->GetPassByIndex( 0 )->GetDesc( &PassDesc );
      hr = g_pd3dDevice->CreateInputLayout( layout, numElements, PassDesc.pIAInputSignature,
                                            PassDesc.IAInputSignatureSize, &g_pVertexLayout );
      if( FAILED( hr ) )
          return hr;

      // Set the input layout
      g_pd3dDevice->IASetInputLayout( g_pVertexLayout );      
    }

    // Create vertex buffer
    SimpleVertex vertices[] =
    {   { D3DXVECTOR3( -1.0f, -1.0f, 1.0f ), D3DXVECTOR2( 0.0f, 0.0f ) },
        { D3DXVECTOR3( 1.0f, -1.0f, 1.0f ), D3DXVECTOR2( 1.0f, 0.0f ) },
        { D3DXVECTOR3( 1.0f, 1.0f, 1.0f ), D3DXVECTOR2( 1.0f, 1.0f ) },
        { D3DXVECTOR3( -1.0f, 1.0f, 1.0f ), D3DXVECTOR2( 0.0f, 1.0f ) },
    };

    D3D10_BUFFER_DESC bd;
    bd.Usage = D3D10_USAGE_DEFAULT;
    bd.ByteWidth = sizeof(vertices); // sizeof( SimpleVertex );
    bd.BindFlags = D3D10_BIND_VERTEX_BUFFER;
    bd.CPUAccessFlags = 0;
    bd.MiscFlags = 0;
    D3D10_SUBRESOURCE_DATA InitData;
    InitData.pSysMem = vertices;
    hr = g_pd3dDevice->CreateBuffer( &bd, &InitData, &g_pVertexBuffer );
    if( FAILED( hr ) )
        return hr;

    // Set vertex buffer
    UINT stride = sizeof( SimpleVertex );
    UINT offset = 0;
    g_pd3dDevice->IASetVertexBuffers( 0, 1, &g_pVertexBuffer, &stride, &offset );

    // Create index buffer
    // Create vertex buffer
    DWORD indices[] =
    { 3,2,0,1
    };
    bd.Usage = D3D10_USAGE_DEFAULT;
    bd.ByteWidth = sizeof(indices); //sizeof( DWORD ) * 36;
    bd.BindFlags = D3D10_BIND_INDEX_BUFFER;
    bd.CPUAccessFlags = 0;
    bd.MiscFlags = 0;
    InitData.pSysMem = indices;
    hr = g_pd3dDevice->CreateBuffer( &bd, &InitData, &g_pIndexBuffer );
    if( FAILED( hr ) )
        return hr;

    // Set index buffer
    g_pd3dDevice->IASetIndexBuffer( g_pIndexBuffer, DXGI_FORMAT_R32_UINT, 0 );

    // Set primitive topology
    g_pd3dDevice->IASetPrimitiveTopology( D3D10_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP );
    
    // Load the Texture ... from memory!
    // ...see D3D10_USAGE_STAGING
    // ...MAP_WRITE with DO_NOT_WAIT to avoid stalls
    // ...http://www.docstoc.com/docs/3670084/DirectX-10-Performance-Tips
     
    { g_pActiveTexture = NULL;
    
      size_t src_nbytes = 1024*1024;          // create the source buffer
      u8 *src = (u8*)calloc(src_nbytes,1);
      int i,j;
      for(i=0;i<1024;i++)
        for(j=0;j<1024;j++)
          src[j + 1024 * i] = (u8) rand();           
      
      // Based on: ms-help://MS.VSCC.v90/MS.VSIPCC.v90/MS.Windows_Graphics.August.2009.1033/Windows_Graphics/d3d10_graphics_programming_guide_resources_creating_textures.htm#Creating_Empty_Textures
      {
        D3D10_TEXTURE2D_DESC desc;
        ZeroMemory( &desc, sizeof(desc) );
        desc.Width                         = 1024;
        desc.Height                        = 1024;
        desc.MipLevels = desc.ArraySize    = 1;
        desc.Format                        = DXGI_FORMAT_R8_UNORM;
        desc.SampleDesc.Count              = 1;
        desc.Usage                         = D3D10_USAGE_DEFAULT;
        desc.BindFlags                     = D3D10_BIND_SHADER_RESOURCE;
                
        Guarded_Assert( !FAILED( g_pd3dDevice->CreateTexture2D( &desc, NULL, &g_pActiveTexture ) ));
        
        desc.Usage                         = D3D10_USAGE_DYNAMIC;
        desc.CPUAccessFlags                = D3D10_CPU_ACCESS_WRITE;        
        
        Guarded_Assert( !FAILED( g_pd3dDevice->CreateTexture2D( &desc, NULL, &g_pStagingTexture ) ));
      }

      { D3D10_MAPPED_TEXTURE2D mappedTex;
        Guarded_Assert( !FAILED( 
            g_pStagingTexture->Map( D3D10CalcSubresource(0, 0, 1), D3D10_MAP_WRITE_DISCARD, 0, &mappedTex ) ));

        u8* pTexels = (u8*)mappedTex.pData;
        memcpy(pTexels, src, src_nbytes);

        g_pStagingTexture->Unmap( D3D10CalcSubresource(0, 0, 1) );
        free(src);        
      }
      g_pd3dDevice->CopyResource( g_pActiveTexture, g_pStagingTexture );
      
      // Create the resource view and bind the texture
      { D3D10_SHADER_RESOURCE_VIEW_DESC srvDesc;
        D3D10_RESOURCE_DIMENSION type;
        g_pActiveTexture->GetType( &type );
        Guarded_Assert( type == D3D10_RESOURCE_DIMENSION_TEXTURE2D );

        D3D10_TEXTURE2D_DESC desc;
        g_pActiveTexture->GetDesc( &desc );
		
        srvDesc.Format                    = desc.Format;
        srvDesc.ViewDimension             = D3D10_SRV_DIMENSION_TEXTURE2D;
        srvDesc.Texture2D.MipLevels       = desc.MipLevels;
        srvDesc.Texture2D.MostDetailedMip = desc.MipLevels - 1;

        g_pTextureRV = NULL;
        Guarded_Assert(!FAILED( g_pd3dDevice->CreateShaderResourceView( g_pActiveTexture, &srvDesc, &g_pTextureRV ) ));
      }
    }                                          
    if( FAILED( hr ) )
        return hr;

    g_pDiffuseVariable->SetResource( g_pTextureRV );

    return S_OK;
}


//--------------------------------------------------------------------------------------
// Clean up the objects we've created
//--------------------------------------------------------------------------------------
void _CleanupDevice()
{
    if( g_pd3dDevice ) g_pd3dDevice->ClearState();

    if( g_pVertexBuffer ) g_pVertexBuffer->Release();
    if( g_pIndexBuffer ) g_pIndexBuffer->Release();
    if( g_pVertexLayout ) g_pVertexLayout->Release();
    if( g_pTextureRV ) g_pTextureRV->Release();
    if( g_pActiveTexture ) g_pActiveTexture->Release();
    if( g_pStagingTexture ) g_pStagingTexture->Release();
    if( g_pEffect ) g_pEffect->Release();
    if( g_pRenderTargetView ) g_pRenderTargetView->Release();
    if( g_pSwapChain ) g_pSwapChain->Release();
    if( g_pd3dDevice ) g_pd3dDevice->Release();
}


//--------------------------------------------------------------------------------------
// Called every time the application receives a message
//--------------------------------------------------------------------------------------
LRESULT CALLBACK Video_WndProc( HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam )
{   int wmId, wmEvent;
    PAINTSTRUCT ps;
    HDC hdc;

    switch( message )
    {
        case WM_PAINT:
            hdc = BeginPaint( hWnd, &ps );
            EndPaint( hWnd, &ps );
            break;

        case WM_DESTROY:
            PostQuitMessage( 0 );
            break;
            
        case WM_COMMAND:
		        wmId    = LOWORD(wParam);
		        wmEvent = HIWORD(wParam);
		        // Parse the menu selections:
		        switch (wmId)
		        {
		        case ID_COMMAND_VIDEODISPLAY:
              { HWND h = g_hWnd;
                if( !IsWindowVisible(h) || IsIconic(h) )
                  ShowWindow(   h, SW_SHOWNORMAL );
                else
                  ShowWindow(   h, SW_HIDE );
                UpdateWindow( h );
              }
              break;
            case IDM_EXIT:
			        DestroyWindow(g_hWnd);
			        break;
		        default:
			        return DefWindowProc(hWnd, message, wParam, lParam);
		        }
		        break;

        default:
            return DefWindowProc( hWnd, message, wParam, lParam );
    }

    return 0;
}


//--------------------------------------------------------------------------------------
// Render a frame
//--------------------------------------------------------------------------------------
void Video_Window_Render_One_Frame()
{   TicTocTimer clock = tic();
    static u8* src = NULL;
    size_t src_nbytes = 1024*1024;
    
    // create the source buffer
    if(!src)
      src = (u8*)calloc(src_nbytes,1);
      
    { int i,j;
      int low = 1024 * 0.1,
         high = 1024 * 0.9;
      for(i=low;i<high;i++)
        for(j=low;j<high;j++)
          src[j + 1024 * i] = (u8) rand();
    }
    
    { D3D10_MAPPED_TEXTURE2D mappedTex;
      Guarded_Assert( !FAILED( 
          g_pStagingTexture->Map( D3D10CalcSubresource(0, 0, 1), D3D10_MAP_WRITE_DISCARD, 0, &mappedTex ) ));

      u8* pTexels = (u8*)mappedTex.pData;
      memcpy(pTexels, src, src_nbytes);

      g_pStagingTexture->Unmap( D3D10CalcSubresource(0, 0, 1) );        
    }
    g_pd3dDevice->CopyResource( g_pActiveTexture, g_pStagingTexture );

    //
    // Clear the back buffer
    //
    float ClearColor[4] = { 0.0f, 0.125f, 0.3f, 1.0f }; // red, green, blue, alpha
    g_pd3dDevice->ClearRenderTargetView( g_pRenderTargetView, ClearColor );

    //
    // Update variables that change once per frame
    //
    //g_pWorldVariable->SetMatrix( ( float* )&g_World );
    //g_pMeshColorVariable->SetFloatVector( ( float* )g_vMeshColor );

    //
    // Render
    //
    D3D10_TECHNIQUE_DESC techDesc;
    g_pTechnique->GetDesc( &techDesc );
    for( UINT p = 0; p < techDesc.Passes; ++p )
    {
        g_pTechnique->GetPassByIndex( p )->Apply( 0 );
        g_pd3dDevice->DrawIndexed( 4, 0, 0 );
    }

    //
    // Present our back buffer to our front buffer
    //
    g_pSwapChain->Present( 0, 0 );
    double dt = toc(&clock);
    debug("FPS: %f\r\n",1.0/dt);
}
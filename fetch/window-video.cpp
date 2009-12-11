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
HINSTANCE                           g_hInst = NULL;
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
ID3D10EffectMatrixVariable*         g_pWorldVariable = NULL;
ID3D10EffectMatrixVariable*         g_pViewVariable = NULL;
ID3D10EffectMatrixVariable*         g_pProjectionVariable = NULL;
ID3D10EffectVectorVariable*         g_pMeshColorVariable = NULL;
ID3D10EffectShaderResourceVariable* g_pDiffuseVariable = NULL;
D3DXMATRIX                          g_World;
D3DXMATRIX                          g_View;
D3DXMATRIX                          g_Projection;
D3DXVECTOR4                         g_vMeshColor( 0.7f, 0.7f, 0.7f, 1.0f );


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
    g_hInst = hInstance;
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
    hr = D3DX10CreateEffectFromFile( "../fetch/Tutorial07.fx",
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
    {
        MessageBox( NULL,
                    "The FX file cannot be located.  Please run this executable from the directory that contains the FX file.", "Error", MB_OK );
        return hr;
    }

    // Obtain the technique
    g_pTechnique = g_pEffect->GetTechniqueByName( "Render" );

    // Obtain the variables
    g_pWorldVariable = g_pEffect->GetVariableByName( "World" )->AsMatrix();
    g_pViewVariable = g_pEffect->GetVariableByName( "View" )->AsMatrix();
    g_pProjectionVariable = g_pEffect->GetVariableByName( "Projection" )->AsMatrix();
    g_pMeshColorVariable = g_pEffect->GetVariableByName( "vMeshColor" )->AsVector();
    g_pDiffuseVariable = g_pEffect->GetVariableByName( "txDiffuse" )->AsShaderResource();

    // Define the input layout
    D3D10_INPUT_ELEMENT_DESC layout[] =
    {
        { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D10_INPUT_PER_VERTEX_DATA, 0 },
        { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 12, D3D10_INPUT_PER_VERTEX_DATA, 0 },
    };
    UINT numElements = sizeof( layout ) / sizeof( layout[0] );

    // Create the input layout
    D3D10_PASS_DESC PassDesc;
    g_pTechnique->GetPassByIndex( 0 )->GetDesc( &PassDesc );
    hr = g_pd3dDevice->CreateInputLayout( layout, numElements, PassDesc.pIAInputSignature,
                                          PassDesc.IAInputSignatureSize, &g_pVertexLayout );
    if( FAILED( hr ) )
        return hr;

    // Set the input layout
    g_pd3dDevice->IASetInputLayout( g_pVertexLayout );

    // Create vertex buffer
    SimpleVertex vertices[] =
    {
        { D3DXVECTOR3( -1.0f, 1.0f, -1.0f ), D3DXVECTOR2( 0.0f, 0.0f ) },
        { D3DXVECTOR3( 1.0f, 1.0f, -1.0f ), D3DXVECTOR2( 1.0f, 0.0f ) },
        { D3DXVECTOR3( 1.0f, 1.0f, 1.0f ), D3DXVECTOR2( 1.0f, 1.0f ) },
        { D3DXVECTOR3( -1.0f, 1.0f, 1.0f ), D3DXVECTOR2( 0.0f, 1.0f ) },

        { D3DXVECTOR3( -1.0f, -1.0f, -1.0f ), D3DXVECTOR2( 0.0f, 0.0f ) },
        { D3DXVECTOR3( 1.0f, -1.0f, -1.0f ), D3DXVECTOR2( 1.0f, 0.0f ) },
        { D3DXVECTOR3( 1.0f, -1.0f, 1.0f ), D3DXVECTOR2( 1.0f, 1.0f ) },
        { D3DXVECTOR3( -1.0f, -1.0f, 1.0f ), D3DXVECTOR2( 0.0f, 1.0f ) },

        { D3DXVECTOR3( -1.0f, -1.0f, 1.0f ), D3DXVECTOR2( 0.0f, 0.0f ) },
        { D3DXVECTOR3( -1.0f, -1.0f, -1.0f ), D3DXVECTOR2( 1.0f, 0.0f ) },
        { D3DXVECTOR3( -1.0f, 1.0f, -1.0f ), D3DXVECTOR2( 1.0f, 1.0f ) },
        { D3DXVECTOR3( -1.0f, 1.0f, 1.0f ), D3DXVECTOR2( 0.0f, 1.0f ) },

        { D3DXVECTOR3( 1.0f, -1.0f, 1.0f ), D3DXVECTOR2( 0.0f, 0.0f ) },
        { D3DXVECTOR3( 1.0f, -1.0f, -1.0f ), D3DXVECTOR2( 1.0f, 0.0f ) },
        { D3DXVECTOR3( 1.0f, 1.0f, -1.0f ), D3DXVECTOR2( 1.0f, 1.0f ) },
        { D3DXVECTOR3( 1.0f, 1.0f, 1.0f ), D3DXVECTOR2( 0.0f, 1.0f ) },

        { D3DXVECTOR3( -1.0f, -1.0f, -1.0f ), D3DXVECTOR2( 0.0f, 0.0f ) },
        { D3DXVECTOR3( 1.0f, -1.0f, -1.0f ), D3DXVECTOR2( 1.0f, 0.0f ) },
        { D3DXVECTOR3( 1.0f, 1.0f, -1.0f ), D3DXVECTOR2( 1.0f, 1.0f ) },
        { D3DXVECTOR3( -1.0f, 1.0f, -1.0f ), D3DXVECTOR2( 0.0f, 1.0f ) },

        { D3DXVECTOR3( -1.0f, -1.0f, 1.0f ), D3DXVECTOR2( 0.0f, 0.0f ) },
        { D3DXVECTOR3( 1.0f, -1.0f, 1.0f ), D3DXVECTOR2( 1.0f, 0.0f ) },
        { D3DXVECTOR3( 1.0f, 1.0f, 1.0f ), D3DXVECTOR2( 1.0f, 1.0f ) },
        { D3DXVECTOR3( -1.0f, 1.0f, 1.0f ), D3DXVECTOR2( 0.0f, 1.0f ) },
    };

    D3D10_BUFFER_DESC bd;
    bd.Usage = D3D10_USAGE_DEFAULT;
    bd.ByteWidth = sizeof( SimpleVertex ) * 24;
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
    {
        3,1,0,
        2,1,3,

        6,4,5,
        7,4,6,

        11,9,8,
        10,9,11,

        14,12,13,
        15,12,14,

        19,17,16,
        18,17,19,

        22,20,21,
        23,20,22
    };
    bd.Usage = D3D10_USAGE_DEFAULT;
    bd.ByteWidth = sizeof( DWORD ) * 36;
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
    g_pd3dDevice->IASetPrimitiveTopology( D3D10_PRIMITIVE_TOPOLOGY_TRIANGLELIST );

    // Load the Texture ... from a file
    hr = D3DX10CreateShaderResourceViewFromFile( g_pd3dDevice, 
                                                 "seafloor.dds",
                                                 NULL, 
                                                 NULL, 
                                                 &g_pTextureRV, 
                                                 NULL );
    // Load the Texture ... from memory!
    // ...see D3D10_USAGE_STAGING
    // ...MAP_WRITE with DO_NOT_WAIT to avoid stalls
    // ...http://www.docstoc.com/docs/3670084/DirectX-10-Performance-Tips
     
    //{ D3DX10_IMAGE_LOAD_INFO load_info;
    //  D3DX10_IMAGE_INFO      src_info;
    //  
    //  D3DX10GetImageInfoFromMemory( src, src_nelem, NULL, &src_info, NULL );
    //  
    //  load_info.Width;
    //  load_info.Height;
    //  load_info.Depth;
    //  load_info.FirstMipLevel  = 0;                   // Use highest resolution as first
    //  load_info.MipLevels      = D3DX10_DEFAULT;      // Make full chain of mipmaps
    //  load_info.Usage          = D3D10_USAGE_DYNAMIC; // Allow CPU to update (see ID3D10Device::UpdateSubresource and  ID3D10Device::CopyResource or ID3D10Device::CopySubresourceRegion)
    //  load_info.BindFlags      = D3D10_BIND_SHADER_RESOURCE; // Bind to a shader stage
    //  load_info.CpuAccessFlags = D3D10_CPU_ACCESS_WRITE;     // Allow CPU to update
    //  load_info.MiscFlags      = D3D10_RESOURCE_MISC_GENERATE_MIPS;
    //  load_info.Format         = D3DX10_DEFAULT;
    //  load_info.Filter         = D3DX10_FILTER_LINEAR // Linear interpolation (4-nn)
    //                           | D3DX10_FILTER_MIRROR;// Mirror boundary conditions (as opposed to wrap)
    //  load_info.MipFilter      = D3DX10_FILTER_LINEAR;// Linear interpolation (4-nn)
    //  load_info.pSrcInfo       = &src_info;
    //                                                  
    //  hr = D3DX10CreateTextureFromMemory( g_pd3dDevice,
    //                                      src,
    //                                      src_nbytes,
    //                                      loadinfo,
    //                                      NULL,
    //                                      &g_pTextureRV,
    //                                      NULL );
    //}                                          
    if( FAILED( hr ) )
        return hr;

    // Initialize the world matrices
    D3DXMatrixIdentity( &g_World );

    // Initialize the view matrix
    D3DXVECTOR3 Eye( 0.0f, 3.0f, -6.0f );
    D3DXVECTOR3 At( 0.0f, 1.0f, 0.0f );
    D3DXVECTOR3 Up( 0.0f, 1.0f, 0.0f );
    D3DXMatrixLookAtLH( &g_View, &Eye, &At, &Up );

    // Initialize the projection matrix
    D3DXMatrixPerspectiveFovLH( &g_Projection, ( float )D3DX_PI * 0.25f, width / ( FLOAT )height, 0.1f, 100.0f );

    // Update Variables that never change
    g_pViewVariable->SetMatrix( ( float* )&g_View );
    g_pProjectionVariable->SetMatrix( ( float* )&g_Projection );
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
{
    // Update our time
    static float t = 0.0f;
    if( g_driverType == D3D10_DRIVER_TYPE_REFERENCE )
    {
        t += ( float )D3DX_PI * 0.0125f;
    }
    else
    {
        static DWORD dwTimeStart = 0;
        DWORD dwTimeCur = GetTickCount();
        if( dwTimeStart == 0 )
            dwTimeStart = dwTimeCur;
        t = ( dwTimeCur - dwTimeStart ) / 1000.0f;
    }

    // Rotate cube around the origin
    D3DXMatrixRotationY( &g_World, t );

    // Modify the color
    g_vMeshColor.x = ( sinf( t * 1.0f ) + 1.0f ) * 0.5f;
    g_vMeshColor.y = ( cosf( t * 3.0f ) + 1.0f ) * 0.5f;
    g_vMeshColor.z = ( sinf( t * 5.0f ) + 1.0f ) * 0.5f;

    //
    // Clear the back buffer
    //
    float ClearColor[4] = { 0.0f, 0.125f, 0.3f, 1.0f }; // red, green, blue, alpha
    g_pd3dDevice->ClearRenderTargetView( g_pRenderTargetView, ClearColor );

    //
    // Update variables that change once per frame
    //
    g_pWorldVariable->SetMatrix( ( float* )&g_World );
    g_pMeshColorVariable->SetFloatVector( ( float* )g_vMeshColor );

    //
    // Render the cube
    //
    D3D10_TECHNIQUE_DESC techDesc;
    g_pTechnique->GetDesc( &techDesc );
    for( UINT p = 0; p < techDesc.Passes; ++p )
    {
        g_pTechnique->GetPassByIndex( p )->Apply( 0 );
        g_pd3dDevice->DrawIndexed( 36, 0, 0 );
    }

    //
    // Present our back buffer to our front buffer
    //
    g_pSwapChain->Present( 0, 0 );
}
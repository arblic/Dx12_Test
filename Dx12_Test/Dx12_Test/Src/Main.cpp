#include "stdafx.h"


//namespace
//{
// 頂点レイアウト
class cVertex {
public:
	float	x, y, z, w;
	float	u0, v0, u1, v1;
	float	r, g, b, a;
	void	setPosition	( float fX, float fY, float fZ, float fW )		{ x=fX; y=fY; z=fZ; w=fW; }
	void	setTexcoord	( float fU0, float fV0 )					{ u0=fU0; v0=fV0; u1=0.0f; v1=0.0f; }
	void	setColor	( float fR, float fG, float fB, float fA )		{ r=fR; g=fG; b=fB; a=fA; }
};

struct cBuf {
	DirectX::XMMATRIX	World;
	float				Color[4];
};

static LPCWSTR	kWindowTitle = L"DX12 Sample";
static const int	kWindowWidth = 640;
static const int	kWindowHeight = 360;

HWND	g_hWnd;

D3D12::cDevice				m_Device;

ID3D12RootSignature*		g_pRootSignature;
ID3D12PipelineState*		g_pPipelineState;

D3D12::cShader				m_Simple;
D3D12::cShader				m_SimpleEx;

D3D12::cVertexBuffer		m_VertexBuffer;
D3D12::cIndexBuffer			m_IndexBuffer;

D3D12::cTexture				m_Texture;

D3D12::cConstantBuffer		m_CBViewProj;
D3D12::cConstantBuffer		m_CBWorld[ 100 ];

double	g_SumTime	= 0.0;
int		g_SumCount	= 0;

bool	g_EveryFrameCreateView	= false;

//}	// namespace

// Window Proc
LRESULT CALLBACK WindowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	// Handle destroy/shutdown messages.
	switch (message)
	{
	case WM_DESTROY:
		PostQuitMessage(0);
		return 0;
	case WM_KEYDOWN:
		switch( wParam ) {
		case VK_SPACE:
			g_EveryFrameCreateView	= !g_EveryFrameCreateView;

			if( g_EveryFrameCreateView ) {
				OutputDebugStringW( L"=== 毎回ビュー生成 ===\n" );
			} else {
				OutputDebugStringW( L"=== ビュー生成なし ===\n" );
			}
			g_SumTime	= 0.0;
			g_SumCount	= 0;

			break;
		}
		return 0;
	}

	// Handle any messages the switch statement didn't.
	return DefWindowProc(hWnd, message, wParam, lParam);
}

// Windowの初期化
void InitWindow(HINSTANCE hInstance, int nCmdShow)
{
	// Initialize the window class.
	WNDCLASSEX windowClass = { 0 };
	windowClass.cbSize = sizeof(WNDCLASSEX);
	windowClass.style = CS_HREDRAW | CS_VREDRAW;
	windowClass.lpfnWndProc = WindowProc;
	windowClass.hInstance = hInstance;
	windowClass.hCursor = LoadCursor(NULL, IDC_ARROW);
	windowClass.lpszClassName = L"WindowClass1";
	RegisterClassEx(&windowClass);

	RECT windowRect = { 0, 0, kWindowWidth, kWindowHeight };
	AdjustWindowRect(&windowRect, WS_OVERLAPPEDWINDOW, FALSE);

	// Create the window and store a handle to it.
	g_hWnd = CreateWindowEx(NULL,
		L"WindowClass1",
		kWindowTitle,
		WS_OVERLAPPEDWINDOW,
		300,
		300,
		windowRect.right - windowRect.left,
		windowRect.bottom - windowRect.top,
		NULL,		// We have no parent window, NULL.
		NULL,		// We aren't using menus, NULL.
		hInstance,
		NULL);		// We aren't using multiple windows, NULL.

	ShowWindow(g_hWnd, nCmdShow);
}

void SetDescriptor( D3D12::cDevice & rDevice, int index )
{
	D3D12_CPU_DESCRIPTOR_HANDLE		Handle;
	D3D12_CONSTANT_BUFFER_VIEW_DESC	cbvDesc		= {};
	D3D12_SHADER_RESOURCE_VIEW_DESC	srvDesc		= {};
	D3D12_SAMPLER_DESC				SamplerDesc;

	m_CBViewProj.getViewDesc( cbvDesc );
	Handle	= rDevice.GetCpuHandle( D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, 3 * index + 0 );
	rDevice.m_pDevice->CreateConstantBufferView(&cbvDesc, Handle);

	m_CBWorld[ index ].getViewDesc( cbvDesc );
	Handle	= rDevice.GetCpuHandle( D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, 3 * index + 1 );
	rDevice.m_pDevice->CreateConstantBufferView(&cbvDesc, Handle);

	m_Texture.getViewDesc( srvDesc );
	Handle	= rDevice.GetCpuHandle( D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, 3 * index + 2 );
	rDevice.m_pDevice->CreateShaderResourceView( m_Texture.m_pTexture, &srvDesc, Handle);



	SamplerDesc.Filter				= D3D12_FILTER_MIN_MAG_MIP_POINT;
	SamplerDesc.AddressU			= D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	SamplerDesc.AddressV			= D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	SamplerDesc.AddressW			= D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	SamplerDesc.MipLODBias			= 0.0f;
	SamplerDesc.MaxAnisotropy		= 1;
	SamplerDesc.ComparisonFunc		= D3D12_COMPARISON_FUNC_NEVER;
	SamplerDesc.BorderColor[0]		= 1.0f;
	SamplerDesc.BorderColor[1]		= 1.0f;
	SamplerDesc.BorderColor[2]		= 1.0f;
	SamplerDesc.BorderColor[3]		= 1.0f;
	SamplerDesc.MinLOD				= -D3D12_FLOAT32_MAX;
	SamplerDesc.MaxLOD				= D3D12_FLOAT32_MAX;

	Handle	= rDevice.GetCpuHandle( D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER, index );
	rDevice.m_pDevice->CreateSampler( &SamplerDesc, Handle);
}

// アセットの初期化
void InitAssets()
{
	HRESULT hr;

	// シェーダバイナリを読み込む
	assert( m_Simple.create( L"C:/MyProject/Dx12_Test/Dx12_Test/Dx12_Test/bin", L"Simple2D" ) );
	assert( m_SimpleEx.create( L"C:/MyProject/Dx12_Test/Dx12_Test/Dx12_Test/bin", L"SimpleEx" ) );

	// ルートシグネチャを作成する
	{
		// バインドする定数バッファやシェーダリソースのバインド情報設定
		// DescriptorHeapのここからここまでをこのインデックスにバインド、って感じ？
		D3D12_DESCRIPTOR_RANGE		CbvRanges[3], SamplerRange[1];
		D3D12_ROOT_PARAMETER		rootParameters[2];
		D3D12_STATIC_SAMPLER_DESC	staticSampler[1];

		CbvRanges[0].RangeType									= D3D12_DESCRIPTOR_RANGE_TYPE_CBV;
		CbvRanges[0].NumDescriptors								= 1;
		CbvRanges[0].BaseShaderRegister							= 0;
		CbvRanges[0].RegisterSpace								= 0;
		CbvRanges[0].OffsetInDescriptorsFromTableStart			= D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

		CbvRanges[1].RangeType									= D3D12_DESCRIPTOR_RANGE_TYPE_CBV;
		CbvRanges[1].NumDescriptors								= 1;
		CbvRanges[1].BaseShaderRegister							= 1;
		CbvRanges[1].RegisterSpace								= 0;
		CbvRanges[1].OffsetInDescriptorsFromTableStart			= D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

		CbvRanges[2].RangeType									= D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
		CbvRanges[2].NumDescriptors								= 1;
		CbvRanges[2].BaseShaderRegister							= 0;
		CbvRanges[2].RegisterSpace								= 0;
		CbvRanges[2].OffsetInDescriptorsFromTableStart			= D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

		rootParameters[0].ParameterType							= D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
		rootParameters[0].DescriptorTable.NumDescriptorRanges	= _countof(CbvRanges);
		rootParameters[0].DescriptorTable.pDescriptorRanges		= CbvRanges;
		rootParameters[0].ShaderVisibility						= D3D12_SHADER_VISIBILITY_ALL;

		SamplerRange[0].RangeType								= D3D12_DESCRIPTOR_RANGE_TYPE_SAMPLER;
		SamplerRange[0].NumDescriptors							= 1;
		SamplerRange[0].BaseShaderRegister						= 0;
		SamplerRange[0].RegisterSpace							= 0;
		SamplerRange[0].OffsetInDescriptorsFromTableStart		= D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

		rootParameters[1].ParameterType							= D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
		rootParameters[1].DescriptorTable.NumDescriptorRanges	= _countof(SamplerRange);
		rootParameters[1].DescriptorTable.pDescriptorRanges		= SamplerRange;
		rootParameters[1].ShaderVisibility						= D3D12_SHADER_VISIBILITY_ALL;

		staticSampler[0].Filter				= D3D12_FILTER_MIN_MAG_MIP_POINT;
		staticSampler[0].AddressU			= D3D12_TEXTURE_ADDRESS_MODE_WRAP;
		staticSampler[0].AddressV			= D3D12_TEXTURE_ADDRESS_MODE_WRAP;
		staticSampler[0].AddressW			= D3D12_TEXTURE_ADDRESS_MODE_WRAP;
		staticSampler[0].MipLODBias			= 0.0f;
		staticSampler[0].MaxAnisotropy		= 1;
		staticSampler[0].ComparisonFunc		= D3D12_COMPARISON_FUNC_NEVER;
		staticSampler[0].BorderColor		= D3D12_STATIC_BORDER_COLOR_TRANSPARENT_BLACK;
		staticSampler[0].MinLOD				= -D3D12_FLOAT32_MAX;
		staticSampler[0].MaxLOD				= D3D12_FLOAT32_MAX;
		staticSampler[0].ShaderRegister		= 0;
		staticSampler[0].RegisterSpace		= 0;
		staticSampler[0].ShaderVisibility	= D3D12_SHADER_VISIBILITY_ALL;

		D3D12_ROOT_SIGNATURE_DESC desc;
		desc.NumParameters		= _countof(rootParameters);
		desc.pParameters		= rootParameters;
//		desc.NumStaticSamplers	= _countof(staticSampler);
//		desc.pStaticSamplers	= staticSampler;
		desc.NumStaticSamplers	= 0;
		desc.pStaticSamplers	= nullptr;
		desc.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;

		ID3DBlob* pSignature;
		ID3DBlob* pError;
		hr = D3D12SerializeRootSignature(&desc, D3D_ROOT_SIGNATURE_VERSION_1, &pSignature, &pError);
		if( FAILED( hr ) ) {
			OutputDebugStringA( (LPCSTR)pError->GetBufferPointer() );
			assert( 0 );
		}

		hr = m_Device.m_pDevice->CreateRootSignature(0, pSignature->GetBufferPointer(), pSignature->GetBufferSize(), IID_PPV_ARGS(&g_pRootSignature));
		assert(SUCCEEDED(hr));
	}

	// PSOを作成
	{
		D3D12_INPUT_ELEMENT_DESC elementDescs[] = {
			{ "POSITION", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0,  0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
			{ "TEXCOORD", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 16, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
			{ "COLOR",    0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 32, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		};

		D3D12_RASTERIZER_DESC rasterDesc = {};
		rasterDesc.FillMode = D3D12_FILL_MODE_SOLID;
		rasterDesc.CullMode = D3D12_CULL_MODE_FRONT;
		rasterDesc.FrontCounterClockwise = false;
		rasterDesc.DepthBias = D3D12_DEFAULT_DEPTH_BIAS;
		rasterDesc.DepthBiasClamp = D3D12_DEFAULT_DEPTH_BIAS_CLAMP;
		rasterDesc.SlopeScaledDepthBias = D3D12_DEFAULT_SLOPE_SCALED_DEPTH_BIAS;
		rasterDesc.DepthClipEnable = true;
		rasterDesc.MultisampleEnable = false;
		rasterDesc.AntialiasedLineEnable = false;
		rasterDesc.ForcedSampleCount = 0;
		rasterDesc.ConservativeRaster = D3D12_CONSERVATIVE_RASTERIZATION_MODE_OFF;

		D3D12_BLEND_DESC blendDesc = {};
		blendDesc.AlphaToCoverageEnable = false;
		blendDesc.IndependentBlendEnable = false;
		blendDesc.RenderTarget[0].BlendEnable = false;
		blendDesc.RenderTarget[0].LogicOpEnable = false;
		blendDesc.RenderTarget[0].SrcBlend = D3D12_BLEND_ONE;
		blendDesc.RenderTarget[0].DestBlend = D3D12_BLEND_ZERO;
		blendDesc.RenderTarget[0].BlendOp = D3D12_BLEND_OP_ADD;
		blendDesc.RenderTarget[0].SrcBlendAlpha = D3D12_BLEND_ONE;
		blendDesc.RenderTarget[0].DestBlendAlpha = D3D12_BLEND_ZERO;
		blendDesc.RenderTarget[0].BlendOpAlpha = D3D12_BLEND_OP_ADD;
		blendDesc.RenderTarget[0].LogicOp = D3D12_LOGIC_OP_NOOP;
		blendDesc.RenderTarget[0].RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;

		D3D12_GRAPHICS_PIPELINE_STATE_DESC desc = {};
		desc.InputLayout = { elementDescs, _countof(elementDescs) };
		desc.pRootSignature = g_pRootSignature;
#if 1
		desc.VS = m_Simple.getVS();
		desc.PS = m_Simple.getPS();
#else
		desc.VS = m_Shader.getVS();
		desc.PS = m_Shader.getPS();
#endif
		desc.RasterizerState = rasterDesc;
		desc.BlendState = blendDesc;
		desc.DepthStencilState.DepthEnable = FALSE;
		desc.DepthStencilState.StencilEnable = FALSE;
		desc.SampleMask = UINT_MAX;
		desc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
		desc.NumRenderTargets = 1;
		desc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
		desc.SampleDesc.Count = 1;

		hr = m_Device.m_pDevice->CreateGraphicsPipelineState(&desc, IID_PPV_ARGS(&g_pPipelineState));
		assert(SUCCEEDED(hr));
	}



	{// 頂点バッファ
		cVertex	pVertex[ 24 ];
		int		Index	= 0;
		float	Pos		= 0.5f;
		pVertex[ Index ].setPosition( -Pos,  Pos,  Pos, 1.0f );	pVertex[ Index ].setTexcoord( 0.0f, 0.0f );	pVertex[ Index ].setColor( 1.0f, 1.0f, 1.0f, 1.0f );	++Index;
		pVertex[ Index ].setPosition(  Pos,  Pos,  Pos, 1.0f );	pVertex[ Index ].setTexcoord( 1.0f, 0.0f );	pVertex[ Index ].setColor( 1.0f, 0.0f, 0.0f, 1.0f );	++Index;
		pVertex[ Index ].setPosition( -Pos, -Pos,  Pos, 1.0f );	pVertex[ Index ].setTexcoord( 0.0f, 1.0f );	pVertex[ Index ].setColor( 0.0f, 1.0f, 0.0f, 1.0f );	++Index;
		pVertex[ Index ].setPosition(  Pos, -Pos,  Pos, 1.0f );	pVertex[ Index ].setTexcoord( 1.0f, 1.0f );	pVertex[ Index ].setColor( 0.0f, 0.0f, 1.0f, 1.0f );	++Index;

		pVertex[ Index ].setPosition(  Pos,  Pos,  Pos, 1.0f );	pVertex[ Index ].setTexcoord( 0.0f, 0.0f );	pVertex[ Index ].setColor( 1.0f, 1.0f, 1.0f, 1.0f );	++Index;
		pVertex[ Index ].setPosition(  Pos,  Pos, -Pos, 1.0f );	pVertex[ Index ].setTexcoord( 1.0f, 0.0f );	pVertex[ Index ].setColor( 1.0f, 0.0f, 0.0f, 1.0f );	++Index;
		pVertex[ Index ].setPosition(  Pos, -Pos,  Pos, 1.0f );	pVertex[ Index ].setTexcoord( 0.0f, 1.0f );	pVertex[ Index ].setColor( 0.0f, 1.0f, 0.0f, 1.0f );	++Index;
		pVertex[ Index ].setPosition(  Pos, -Pos, -Pos, 1.0f );	pVertex[ Index ].setTexcoord( 1.0f, 1.0f );	pVertex[ Index ].setColor( 0.0f, 0.0f, 1.0f, 1.0f );	++Index;

		pVertex[ Index ].setPosition(  Pos,  Pos, -Pos, 1.0f );	pVertex[ Index ].setTexcoord( 0.0f, 0.0f );	pVertex[ Index ].setColor( 1.0f, 1.0f, 1.0f, 1.0f );	++Index;
		pVertex[ Index ].setPosition( -Pos,  Pos, -Pos, 1.0f );	pVertex[ Index ].setTexcoord( 1.0f, 0.0f );	pVertex[ Index ].setColor( 1.0f, 0.0f, 0.0f, 1.0f );	++Index;
		pVertex[ Index ].setPosition(  Pos, -Pos, -Pos, 1.0f );	pVertex[ Index ].setTexcoord( 0.0f, 1.0f );	pVertex[ Index ].setColor( 0.0f, 1.0f, 0.0f, 1.0f );	++Index;
		pVertex[ Index ].setPosition( -Pos, -Pos, -Pos, 1.0f );	pVertex[ Index ].setTexcoord( 1.0f, 1.0f );	pVertex[ Index ].setColor( 0.0f, 0.0f, 1.0f, 1.0f );	++Index;

		pVertex[ Index ].setPosition( -Pos,  Pos, -Pos, 1.0f );	pVertex[ Index ].setTexcoord( 0.0f, 0.0f );	pVertex[ Index ].setColor( 1.0f, 1.0f, 1.0f, 1.0f );	++Index;
		pVertex[ Index ].setPosition( -Pos,  Pos,  Pos, 1.0f );	pVertex[ Index ].setTexcoord( 1.0f, 0.0f );	pVertex[ Index ].setColor( 1.0f, 0.0f, 0.0f, 1.0f );	++Index;
		pVertex[ Index ].setPosition( -Pos, -Pos, -Pos, 1.0f );	pVertex[ Index ].setTexcoord( 0.0f, 1.0f );	pVertex[ Index ].setColor( 0.0f, 1.0f, 0.0f, 1.0f );	++Index;
		pVertex[ Index ].setPosition( -Pos, -Pos,  Pos, 1.0f );	pVertex[ Index ].setTexcoord( 1.0f, 1.0f );	pVertex[ Index ].setColor( 0.0f, 0.0f, 1.0f, 1.0f );	++Index;

		pVertex[ Index ].setPosition( -Pos,  Pos,  Pos, 1.0f );	pVertex[ Index ].setTexcoord( 0.0f, 0.0f );	pVertex[ Index ].setColor( 1.0f, 1.0f, 1.0f, 1.0f );	++Index;
		pVertex[ Index ].setPosition( -Pos,  Pos, -Pos, 1.0f );	pVertex[ Index ].setTexcoord( 1.0f, 0.0f );	pVertex[ Index ].setColor( 1.0f, 0.0f, 0.0f, 1.0f );	++Index;
		pVertex[ Index ].setPosition(  Pos,  Pos,  Pos, 1.0f );	pVertex[ Index ].setTexcoord( 0.0f, 1.0f );	pVertex[ Index ].setColor( 0.0f, 1.0f, 0.0f, 1.0f );	++Index;
		pVertex[ Index ].setPosition(  Pos,  Pos, -Pos, 1.0f );	pVertex[ Index ].setTexcoord( 1.0f, 1.0f );	pVertex[ Index ].setColor( 0.0f, 0.0f, 1.0f, 1.0f );	++Index;

		pVertex[ Index ].setPosition( -Pos, -Pos,  Pos, 1.0f );	pVertex[ Index ].setTexcoord( 0.0f, 0.0f );	pVertex[ Index ].setColor( 1.0f, 1.0f, 1.0f, 1.0f );	++Index;
		pVertex[ Index ].setPosition(  Pos, -Pos,  Pos, 1.0f );	pVertex[ Index ].setTexcoord( 1.0f, 0.0f );	pVertex[ Index ].setColor( 1.0f, 0.0f, 0.0f, 1.0f );	++Index;
		pVertex[ Index ].setPosition( -Pos, -Pos, -Pos, 1.0f );	pVertex[ Index ].setTexcoord( 0.0f, 1.0f );	pVertex[ Index ].setColor( 0.0f, 1.0f, 0.0f, 1.0f );	++Index;
		pVertex[ Index ].setPosition(  Pos, -Pos, -Pos, 1.0f );	pVertex[ Index ].setTexcoord( 1.0f, 1.0f );	pVertex[ Index ].setColor( 0.0f, 0.0f, 1.0f, 1.0f );	++Index;

		m_VertexBuffer.create( &m_Device, pVertex, sizeof(pVertex), sizeof(cVertex) );
	}

	{// インデクスバッファ
		UINT16	pIndex[ 36 ] = {0,  1,  2,  2,  1,  3,
								4,  5,  6,  6,  5,  7,
								8,  9, 10, 10,  9, 11,
								12, 13, 14, 14, 13, 15,
								16, 17, 18, 18, 17, 19,
								20, 21, 22, 22, 21, 23, };

		m_IndexBuffer.create( &m_Device, pIndex, sizeof(pIndex) );
	}

	// 定数バッファを作成する
	{
		// 定数バッファ
		DirectX::XMMATRIX		ViewMtx, ProjMtx, VPMtx;
		DirectX::XMVECTORF32	Pos;
		DirectX::XMVECTORF32	At;
		DirectX::XMVECTORF32	Up;

		Pos.f[0]=0.0f;Pos.f[1]=3.0f;Pos.f[2]=-15.0f;Pos.f[3]=1.0f;
		At.f[0]=0.0f;At.f[1]=0.0f;At.f[2]=0.0f;At.f[3]=1.0f;
		Up.f[0]=0.0f;Up.f[1]=1.0f;Up.f[2]=0.0f;Up.f[3]=1.0f;

		ViewMtx	= DirectX::XMMatrixLookAtLH( Pos, At, Up );
		ProjMtx	= DirectX::XMMatrixPerspectiveFovLH( DirectX::XMConvertToRadians( 50.0f ), (float)kWindowWidth/(float)kWindowHeight, 1.0f, 100.0f );
		VPMtx	= DirectX::XMMatrixMultiply( ViewMtx, ProjMtx );
		VPMtx	= DirectX::XMMatrixTranspose( VPMtx );

		m_CBViewProj.create( &m_Device );
		m_CBViewProj.setValue( &VPMtx, sizeof(VPMtx) );

		for( int i=0; i<_countof(m_CBWorld); ++i ) {
			m_CBWorld[ i ].create( &m_Device );
		}
	}

	// テクスチャ
	{
		m_Texture.create( &m_Device, nullptr );
	}

	SetDescriptor( m_Device, 0 );
}

// アセットを破棄する
void DestroyAssets()
{
	m_Texture.destroy();

	for( int i=0; i<_countof(m_CBWorld); ++i ) {
		m_CBWorld[ i ].destroy();
	}
	m_CBViewProj.destroy();

	m_IndexBuffer.destroy();
	m_VertexBuffer.destroy();

	g_pPipelineState->Release();

	m_SimpleEx.destroy();
	m_Simple.destroy();

	g_pRootSignature->Release();
}

// シーンのアップデート
void UpdateScene()
{
	static float sAngle = 0.0f;
	sAngle += 1.0f;
	float PosX	= -6.0f;
	float PosY	= 6.0f;
	float GapX	= 1.8f;
	float GapY	= 1.2f;

	float MulX, MulY;

	DirectX::XMVECTORF32	AxisY;
	DirectX::XMMATRIX		Rot, Trans, World;
	AxisY.f[0]=0.0f;AxisY.f[1]=1.0f;AxisY.f[2]=0.0f;AxisY.f[3]=0.0f;


	Rot	= DirectX::XMMatrixRotationAxis( AxisY, DirectX::XMConvertToRadians(sAngle) );

	for( int i=0; i<_countof(m_CBWorld); ++i ) {

		MulX	= (float)(i%10);
		MulY	= (float)(i/10);

		Trans	= DirectX::XMMatrixTranslation( PosX + GapX * MulX, PosY - GapY * MulY, 0.0f );
		World	= DirectX::XMMatrixTranspose( DirectX::XMMatrixMultiply( Rot, Trans ) );

		m_CBWorld[ i ].setValue( &World, sizeof(World) );
	}
}

// 描画
void DrawScene( D3D12::cDevice & rDevice )
{
	rDevice.m_pCommandList->SetPipelineState(g_pPipelineState);
	rDevice.m_pCommandList->SetGraphicsRootSignature(g_pRootSignature);

	rDevice.setVertexBuffer( &m_VertexBuffer );
	rDevice.setIndexBuffer( &m_IndexBuffer );

	rDevice.m_pCommandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);




	ID3D12DescriptorHeap *	ppHeaps[] = {	rDevice.GetDescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV),
											rDevice.GetDescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER), };
	rDevice.m_pCommandList->SetDescriptorHeaps( _countof(ppHeaps), ppHeaps );


	for( int i=0; i<_countof(m_CBWorld); ++i ) {

		if( !g_EveryFrameCreateView ) {
			SetDescriptor( m_Device, i );
		}

		rDevice.m_pCommandList->SetGraphicsRootDescriptorTable(0, m_Device.GetGpuHandle( D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, i*3 ) );
		rDevice.m_pCommandList->SetGraphicsRootDescriptorTable(1, m_Device.GetGpuHandle( D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER, i ) );
		rDevice.m_pCommandList->DrawIndexedInstanced( 36, 1, 0, 0, 0 );
	}

}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE, LPSTR, int nCmdShow)
{
	DWORD	Begin;
	wchar_t Str[ 256 ]	= {0};



	InitWindow(hInstance, nCmdShow);

	if( FAILED( m_Device.create( g_hWnd, kWindowWidth, kWindowHeight ) ) ) {
		return -1;
	}
	InitAssets();


	// メインループ
	MSG msg = { 0 };
	while (true)
	{
		// Process any messages in the queue.
		if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);

			if (msg.message == WM_QUIT)
				break;
		}

		// アップデート
		UpdateScene();


		Begin = GetTickCount();

		m_Device.BeginDraw();
		DrawScene( m_Device );
		m_Device.EndDraw();


		if( 100 <= g_SumCount ) {

			swprintf_s( Str, L"%f[ms]\n", g_SumTime * 0.01 );
			OutputDebugStringW( Str );

			g_SumTime	= 0.0;
			g_SumCount	= 0;

		} else {
			g_SumTime	+= (double)( GetTickCount() - Begin );
			++g_SumCount;
		}

		m_Device.Present();
		m_Device.WaitDrawDone();
	}

	m_Device.WaitDrawDone();
	DestroyAssets();
	m_Device.destroy();

	return static_cast<char>(msg.wParam);
}

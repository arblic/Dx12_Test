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


static LPCWSTR	kWindowTitle = L"DX12 Sample";
static const int	kWindowWidth = 640;
static const int	kWindowHeight = 360;

HWND	g_hWnd;

D3D12::cDevice				m_Device;

ID3D12RootSignature*		g_pRootSignature;
ID3D12PipelineState*		g_pPipelineState;

D3D12::cShader				m_Shader;
D3D12::cShader				m_Simple;

D3D12::cVertexBuffer		m_VertexBuffer;
D3D12::cIndexBuffer			m_IndexBuffer;

D3D12::cTexture				m_Texture;

ID3D12DescriptorHeap*		g_pCbvHeap;

D3D12::cConstantBuffer		m_CBViewProj;
D3D12::cConstantBuffer		m_CBWorld[ 100 ];

//void *						g_pConstBufferData;
DirectX::XMMATRIX			m_ViewProjMtx;
DirectX::XMMATRIX *			m_pWorldMtx;

int		g_frameIndex;
UINT	g_rtvDescriptorSize;
UINT64	g_fenceValue;

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
	D3D12_CONSTANT_BUFFER_VIEW_DESC	cbvDesc		= {};
	D3D12_SHADER_RESOURCE_VIEW_DESC	srvDesc		= {};
	D3D12_CPU_DESCRIPTOR_HANDLE		CpuHandle	= g_pCbvHeap->GetCPUDescriptorHandleForHeapStart();
	D3D12_GPU_DESCRIPTOR_HANDLE		GpuHandle	= g_pCbvHeap->GetGPUDescriptorHandleForHeapStart();
	UINT							GapSize		= rDevice.m_pDevice->GetDescriptorHandleIncrementSize( D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV );
	size_t							DefPtr;

	DefPtr		= CpuHandle.ptr + ( GapSize * 3 * index );

	m_CBViewProj.getViewDesc( cbvDesc );
	CpuHandle.ptr	= DefPtr + GapSize * 0;
	rDevice.m_pDevice->CreateConstantBufferView(&cbvDesc, CpuHandle);

	m_CBWorld[ index ].getViewDesc( cbvDesc );
	CpuHandle.ptr	= DefPtr + GapSize * 1;
	rDevice.m_pDevice->CreateConstantBufferView(&cbvDesc, CpuHandle);

	m_Texture.getViewDesc( srvDesc );
	CpuHandle.ptr	= DefPtr + GapSize * 2;
	rDevice.m_pDevice->CreateShaderResourceView( m_Texture.m_pTexture, &srvDesc, CpuHandle);
}

// アセットの初期化
void InitAssets()
{
	HRESULT hr;

	// ルートシグネチャを作成する
	{
		// バインドする定数バッファやシェーダリソースのバインド情報設定
		// DescriptorHeapのここからここまでをこのインデックスにバインド、って感じ？
		D3D12_DESCRIPTOR_RANGE ranges[3];
		D3D12_ROOT_PARAMETER rootParameters[1];
		D3D12_STATIC_SAMPLER_DESC staticSampler[1];

		ranges[0].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_CBV;									// このDescriptorRangeは定数バッファ
		ranges[0].NumDescriptors = 1;															// Descriptorは1つ
		ranges[0].BaseShaderRegister = 0;														// シェーダ側の開始インデックスは0番
		ranges[0].RegisterSpace = 0;															// TODO: SM5.1からのspaceだけど、どういうものかよくわからない
		ranges[0].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;		// TODO: とりあえず-1を入れておけばOK？

		ranges[1].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_CBV;									// このDescriptorRangeは定数バッファ
		ranges[1].NumDescriptors = 1;															// Descriptorは1つ
		ranges[1].BaseShaderRegister = 1;														// シェーダ側の開始インデックスは0番
		ranges[1].RegisterSpace = 0;															// TODO: SM5.1からのspaceだけど、どういうものかよくわからない
		ranges[1].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;		// TODO: とりあえず-1を入れておけばOK？

		ranges[2].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;									// このDescriptorRangeは定数バッファ
		ranges[2].NumDescriptors = 1;															// Descriptorは1つ
		ranges[2].BaseShaderRegister = 0;														// シェーダ側の開始インデックスは0番
		ranges[2].RegisterSpace = 0;															// TODO: SM5.1からのspaceだけど、どういうものかよくわからない
		ranges[2].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;		// TODO: とりあえず-1を入れておけばOK？

		//ranges[3].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SAMPLER;									// このDescriptorRangeは定数バッファ
		//ranges[3].NumDescriptors = 1;															// Descriptorは1つ
		//ranges[3].BaseShaderRegister = 0;														// シェーダ側の開始インデックスは0番
		//ranges[3].RegisterSpace = 0;															// TODO: SM5.1からのspaceだけど、どういうものかよくわからない
		//ranges[3].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;		// TODO: とりあえず-1を入れておけばOK？

		rootParameters[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;			// このパラメータはDescriptorTableとして使用する
		rootParameters[0].DescriptorTable.NumDescriptorRanges = _countof(ranges);								// DescriptorRangeの数は1つ
		rootParameters[0].DescriptorTable.pDescriptorRanges = ranges;							// DescriptorRangeの先頭アドレス
		rootParameters[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;					// このパラメータは頂点シェーダからのみ見える
																								// D3D12_SHADER_VISIBILITY_ALL にすればすべてのシェーダからアクセス可能

		staticSampler[ 0 ].Filter			= D3D12_FILTER_MIN_MAG_MIP_POINT;
		staticSampler[ 0 ].AddressU			= D3D12_TEXTURE_ADDRESS_MODE_WRAP;
		staticSampler[ 0 ].AddressV			= D3D12_TEXTURE_ADDRESS_MODE_WRAP;
		staticSampler[ 0 ].AddressW			= D3D12_TEXTURE_ADDRESS_MODE_WRAP;
		staticSampler[ 0 ].MipLODBias		= 0.0f;
		staticSampler[ 0 ].MaxAnisotropy	= 1;
		staticSampler[ 0 ].ComparisonFunc	= D3D12_COMPARISON_FUNC_NEVER;
		staticSampler[ 0 ].BorderColor		= D3D12_STATIC_BORDER_COLOR_TRANSPARENT_BLACK;
		staticSampler[ 0 ].MinLOD			= -D3D12_FLOAT32_MAX;
		staticSampler[ 0 ].MaxLOD			= D3D12_FLOAT32_MAX;
		staticSampler[ 0 ].ShaderRegister	= 0;
		staticSampler[ 0 ].RegisterSpace	= 0;
		staticSampler[ 0 ].ShaderVisibility	= D3D12_SHADER_VISIBILITY_ALL;

		D3D12_ROOT_SIGNATURE_DESC desc;
		desc.NumParameters = _countof(rootParameters);
		desc.pParameters = rootParameters;
		desc.NumStaticSamplers = 1;
		desc.pStaticSamplers = staticSampler;
		desc.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;

		ID3DBlob* pSignature;
		hr = D3D12SerializeRootSignature(&desc, D3D_ROOT_SIGNATURE_VERSION_1, &pSignature, nullptr);
		assert(SUCCEEDED(hr));

		hr = m_Device.m_pDevice->CreateRootSignature(0, pSignature->GetBufferPointer(), pSignature->GetBufferSize(), IID_PPV_ARGS(&g_pRootSignature));
		assert(SUCCEEDED(hr));
	}

	// シェーダバイナリを読み込む
	assert( m_Shader.create( L"C:/MyProject/Dx12_Test/Dx12_Test/bin", L"Sample" ) );
	assert( m_Simple.create( L"C:/MyProject/Dx12_Test/Dx12_Test/bin", L"Simple2D" ) );

	// PSOを作成
	{
		D3D12_INPUT_ELEMENT_DESC elementDescs[] = {
			{ "POSITION", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0,  0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
			{ "TEXCOORD", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 16, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
			{ "COLOR",    0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 32, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		};

		D3D12_RASTERIZER_DESC rasterDesc = {};
		rasterDesc.FillMode = D3D12_FILL_MODE_SOLID;
		rasterDesc.CullMode = D3D12_CULL_MODE_NONE;
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
		pVertex[ Index ].setPosition(  Pos,  Pos,  Pos, 1.0f );	pVertex[ Index ].setTexcoord( 1.0f, 0.0f );	pVertex[ Index ].setColor( 1.0f, 1.0f, 1.0f, 1.0f );	++Index;
		pVertex[ Index ].setPosition( -Pos, -Pos,  Pos, 1.0f );	pVertex[ Index ].setTexcoord( 0.0f, 1.0f );	pVertex[ Index ].setColor( 1.0f, 1.0f, 1.0f, 1.0f );	++Index;
		pVertex[ Index ].setPosition(  Pos, -Pos,  Pos, 1.0f );	pVertex[ Index ].setTexcoord( 1.0f, 1.0f );	pVertex[ Index ].setColor( 1.0f, 1.0f, 1.0f, 1.0f );	++Index;

		pVertex[ Index ].setPosition(  Pos,  Pos,  Pos, 1.0f );	pVertex[ Index ].setTexcoord( 0.0f, 0.0f );	pVertex[ Index ].setColor( 1.0f, 1.0f, 1.0f, 1.0f );	++Index;
		pVertex[ Index ].setPosition(  Pos,  Pos, -Pos, 1.0f );	pVertex[ Index ].setTexcoord( 1.0f, 0.0f );	pVertex[ Index ].setColor( 1.0f, 1.0f, 1.0f, 1.0f );	++Index;
		pVertex[ Index ].setPosition(  Pos, -Pos,  Pos, 1.0f );	pVertex[ Index ].setTexcoord( 0.0f, 1.0f );	pVertex[ Index ].setColor( 1.0f, 1.0f, 1.0f, 1.0f );	++Index;
		pVertex[ Index ].setPosition(  Pos, -Pos, -Pos, 1.0f );	pVertex[ Index ].setTexcoord( 1.0f, 1.0f );	pVertex[ Index ].setColor( 1.0f, 1.0f, 1.0f, 1.0f );	++Index;

		pVertex[ Index ].setPosition(  Pos,  Pos, -Pos, 1.0f );	pVertex[ Index ].setTexcoord( 0.0f, 0.0f );	pVertex[ Index ].setColor( 1.0f, 1.0f, 1.0f, 1.0f );	++Index;
		pVertex[ Index ].setPosition( -Pos,  Pos, -Pos, 1.0f );	pVertex[ Index ].setTexcoord( 1.0f, 0.0f );	pVertex[ Index ].setColor( 1.0f, 1.0f, 1.0f, 1.0f );	++Index;
		pVertex[ Index ].setPosition(  Pos, -Pos, -Pos, 1.0f );	pVertex[ Index ].setTexcoord( 0.0f, 1.0f );	pVertex[ Index ].setColor( 1.0f, 1.0f, 1.0f, 1.0f );	++Index;
		pVertex[ Index ].setPosition( -Pos, -Pos, -Pos, 1.0f );	pVertex[ Index ].setTexcoord( 1.0f, 1.0f );	pVertex[ Index ].setColor( 1.0f, 1.0f, 1.0f, 1.0f );	++Index;

		pVertex[ Index ].setPosition( -Pos,  Pos, -Pos, 1.0f );	pVertex[ Index ].setTexcoord( 0.0f, 0.0f );	pVertex[ Index ].setColor( 1.0f, 1.0f, 1.0f, 1.0f );	++Index;
		pVertex[ Index ].setPosition( -Pos,  Pos,  Pos, 1.0f );	pVertex[ Index ].setTexcoord( 1.0f, 0.0f );	pVertex[ Index ].setColor( 1.0f, 1.0f, 1.0f, 1.0f );	++Index;
		pVertex[ Index ].setPosition( -Pos, -Pos, -Pos, 1.0f );	pVertex[ Index ].setTexcoord( 0.0f, 1.0f );	pVertex[ Index ].setColor( 1.0f, 1.0f, 1.0f, 1.0f );	++Index;
		pVertex[ Index ].setPosition( -Pos, -Pos,  Pos, 1.0f );	pVertex[ Index ].setTexcoord( 1.0f, 1.0f );	pVertex[ Index ].setColor( 1.0f, 1.0f, 1.0f, 1.0f );	++Index;

		pVertex[ Index ].setPosition( -Pos,  Pos,  Pos, 1.0f );	pVertex[ Index ].setTexcoord( 0.0f, 0.0f );	pVertex[ Index ].setColor( 1.0f, 1.0f, 1.0f, 1.0f );	++Index;
		pVertex[ Index ].setPosition( -Pos,  Pos, -Pos, 1.0f );	pVertex[ Index ].setTexcoord( 1.0f, 0.0f );	pVertex[ Index ].setColor( 1.0f, 1.0f, 1.0f, 1.0f );	++Index;
		pVertex[ Index ].setPosition(  Pos,  Pos,  Pos, 1.0f );	pVertex[ Index ].setTexcoord( 0.0f, 1.0f );	pVertex[ Index ].setColor( 1.0f, 1.0f, 1.0f, 1.0f );	++Index;
		pVertex[ Index ].setPosition(  Pos,  Pos, -Pos, 1.0f );	pVertex[ Index ].setTexcoord( 1.0f, 1.0f );	pVertex[ Index ].setColor( 1.0f, 1.0f, 1.0f, 1.0f );	++Index;

		pVertex[ Index ].setPosition( -Pos, -Pos,  Pos, 1.0f );	pVertex[ Index ].setTexcoord( 0.0f, 0.0f );	pVertex[ Index ].setColor( 1.0f, 1.0f, 1.0f, 1.0f );	++Index;
		pVertex[ Index ].setPosition(  Pos, -Pos,  Pos, 1.0f );	pVertex[ Index ].setTexcoord( 1.0f, 0.0f );	pVertex[ Index ].setColor( 1.0f, 1.0f, 1.0f, 1.0f );	++Index;
		pVertex[ Index ].setPosition( -Pos, -Pos, -Pos, 1.0f );	pVertex[ Index ].setTexcoord( 0.0f, 1.0f );	pVertex[ Index ].setColor( 1.0f, 1.0f, 1.0f, 1.0f );	++Index;
		pVertex[ Index ].setPosition(  Pos, -Pos, -Pos, 1.0f );	pVertex[ Index ].setTexcoord( 1.0f, 1.0f );	pVertex[ Index ].setColor( 1.0f, 1.0f, 1.0f, 1.0f );	++Index;

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
		// 定数バッファ用のDescriptorHeapを作成
		{
			D3D12_DESCRIPTOR_HEAP_DESC desc = {};
			desc.NumDescriptors	= 512;
			desc.Type			= D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
			desc.Flags			= D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
			hr	= m_Device.m_pDevice->CreateDescriptorHeap(&desc, IID_PPV_ARGS(&g_pCbvHeap));
			assert(SUCCEEDED(hr));
		}

	}

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

		D3D12_SHADER_RESOURCE_VIEW_DESC desc = {};

		desc.Format	= DXGI_FORMAT_R8G8B8A8_UNORM;
		desc.ViewDimension	= D3D12_SRV_DIMENSION_TEXTURE2D;
		desc.Shader4ComponentMapping		= D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
		desc.Texture2D.MipLevels			= 1;
		desc.Texture2D.MostDetailedMip		= 0;
		desc.Texture2D.PlaneSlice			= 0;
		desc.Texture2D.ResourceMinLODClamp	= 0;

		D3D12_CPU_DESCRIPTOR_HANDLE	CpuHandle	= g_pCbvHeap->GetCPUDescriptorHandleForHeapStart();

		CpuHandle.ptr	+= ( m_Device.m_pDevice->GetDescriptorHandleIncrementSize( D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV ) * 2 );
		m_Device.m_pDevice->CreateShaderResourceView( m_Texture.m_pTexture, &desc, CpuHandle);
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

	g_pCbvHeap->Release();

	m_IndexBuffer.destroy();
	m_VertexBuffer.destroy();

	g_pPipelineState->Release();

	m_Shader.destroy();

	g_pRootSignature->Release();
}

// シーンのアップデート
void UpdateScene()
{
	static float sAngle = 0.0f;
	sAngle += 1.0f;
	float PosX	= -6.0f;
	float PosY	= 5.0f;
	float Gap	= 1.5f;

	float MulX, MulY;

	DirectX::XMVECTORF32	AxisY;
	DirectX::XMMATRIX		Rot, Trans, World;
	AxisY.f[0]=0.0f;AxisY.f[1]=1.0f;AxisY.f[2]=0.0f;AxisY.f[3]=0.0f;


	Rot	= DirectX::XMMatrixRotationAxis( AxisY, DirectX::XMConvertToRadians(sAngle) );

	for( int i=0; i<_countof(m_CBWorld); ++i ) {

		MulX	= (float)(i%10);
		MulY	= (float)(i/10);

		Trans	= DirectX::XMMatrixTranslation( PosX + Gap * MulX, PosY - Gap * MulY, 0.0f );
		World	= DirectX::XMMatrixTranspose( DirectX::XMMatrixMultiply( Rot, Trans ) );

		m_CBWorld[ i ].setValue( &World, sizeof(World) );
	}
}

// 描画
void DrawScene( D3D12::cDevice & rDevice )
{
	rDevice.m_pCommandList->SetPipelineState(g_pPipelineState);
	rDevice.m_pCommandList->SetGraphicsRootSignature(g_pRootSignature);

	D3D12_CONSTANT_BUFFER_VIEW_DESC	cbvDesc		= {};
	D3D12_SHADER_RESOURCE_VIEW_DESC	srvDesc		= {};
	D3D12_CPU_DESCRIPTOR_HANDLE		CpuHandle	= g_pCbvHeap->GetCPUDescriptorHandleForHeapStart();
	D3D12_GPU_DESCRIPTOR_HANDLE		GpuHandle	= g_pCbvHeap->GetGPUDescriptorHandleForHeapStart();
	UINT							GapSize		= rDevice.m_pDevice->GetDescriptorHandleIncrementSize( D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV );
	size_t							DefCpuPtr	= CpuHandle.ptr;
	size_t							DefGpuPtr	= GpuHandle.ptr;

	//SetDescriptor( m_Device, 0 );

	rDevice.m_pCommandList->SetDescriptorHeaps(1, &g_pCbvHeap);
	rDevice.m_pCommandList->SetGraphicsRootDescriptorTable(0, GpuHandle);

	rDevice.setVertexBuffer( &m_VertexBuffer );
	rDevice.setIndexBuffer( &m_IndexBuffer );

	rDevice.m_pCommandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);







	for( int i=0; i<_countof(m_CBWorld); ++i ) {

		if( !g_EveryFrameCreateView ) {
			SetDescriptor( m_Device, i );
		}

		GpuHandle.ptr	= DefGpuPtr + ( GapSize * 3 * i );

		rDevice.m_pCommandList->SetGraphicsRootDescriptorTable(0, GpuHandle);
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

//-----------------------------------------------------------------------------
//! @file   D3D12Device.cpp
//! @brief	デバイス
//! @author 
//-----------------------------------------------------------------------------
#include "stdafx.h"
#include "D3D12Device.h"

namespace D3D12 {

//-----------------------------------------------------------------------------
//!	ctor
//-----------------------------------------------------------------------------
cDevice::cDevice( void )
{
	m_pDevice				= nullptr;
	m_pCommandQueue			= nullptr;
	m_pCommandAllocator		= nullptr;
	m_pCommandList			= nullptr;
	m_pSwapChain			= nullptr;
	m_pRtvHeap				= nullptr;
	m_pRenderTargets[ 0 ]	= nullptr;
	m_pRenderTargets[ 1 ]	= nullptr;
	m_pFence				= nullptr;
	m_fenceEvent			= INVALID_HANDLE_VALUE;

	m_frameIndex			= 0;
	m_rtvDescriptorSize		= 0;
	m_fenceValue			= 0;
}

//-----------------------------------------------------------------------------
//!	dtor
//-----------------------------------------------------------------------------
cDevice::~cDevice( void )
{
	destroy();
}

//-----------------------------------------------------------------------------
//!	
//-----------------------------------------------------------------------------
HRESULT cDevice::create( HWND hWnd, UINT32 WindowWidth, UINT32 WindowHeight )
{
	HRESULT	hr	= S_OK;

#ifdef _DEBUG
	{	// デバッグバージョンではデバッグレイヤーを有効化する
		ID3D12Debug* debugController;
		if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&debugController))))
		{
			debugController->EnableDebugLayer();
			debugController->Release();
		}
	}
#endif

	// ファクトリを作成
	IDXGIFactory4* factory;
	CreateDXGIFactory1(IID_PPV_ARGS(&factory));

	// デバイスの作成
	hr	= D3D12CreateDevice( nullptr, D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS( &m_pDevice ) );
	if( FAILED( hr ) ) {

		IDXGIAdapter *	pWarp	= NULL;

		hr	= factory->EnumWarpAdapter( IID_PPV_ARGS( &pWarp ) );
		if( S_OK == hr ) {
			hr = D3D12CreateDevice( pWarp, D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS( &m_pDevice ) );
			pWarp->Release();

			if( FAILED( hr ) ) {
				return hr;
			}
		}
	}

//#ifdef _DEBUG
//	{
//		ID3D12DebugDevice * pDebugDevice;
//		if( SUCCEEDED( m_pDevice->QueryInterface( &pDebugDevice ) ) ) {
//			// 生きてるオブジェクトを報告する
//			pDebugDevice->ReportLiveDeviceObjects( D3D12_RLDO_DETAIL|D3D12_RLDO_IGNORE_INTERNAL );
//			pDebugDevice->Release();
//		}
//	}
//#endif

	// コマンドキューを作成する
	// 最初なので直接コマンドキュー
	{
		D3D12_COMMAND_QUEUE_DESC desc = {};
		desc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;		// GPUタイムアウトが有効
		desc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;		// 直接コマンドキュー

		hr = m_pDevice->CreateCommandQueue(&desc, IID_PPV_ARGS(&m_pCommandQueue));
		assert(SUCCEEDED(hr));
	}

	// スワップチェインを作成
	{
		DXGI_SWAP_CHAIN_DESC desc = {};
		desc.BufferCount = 2;			// フレームバッファとバックバッファで2枚
		desc.BufferDesc.Width = WindowWidth;
		desc.BufferDesc.Height = WindowHeight;
		desc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		desc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
		desc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
		desc.OutputWindow = hWnd;
		desc.SampleDesc.Count = 1;
		desc.Windowed = true;

		IDXGISwapChain* pSwap;
		hr = factory->CreateSwapChain(m_pCommandQueue, &desc, &pSwap);
		assert(SUCCEEDED(hr));

		hr = pSwap->QueryInterface(IID_PPV_ARGS(&m_pSwapChain));
		assert(SUCCEEDED(hr));

		m_frameIndex = m_pSwapChain->GetCurrentBackBufferIndex();

		pSwap->Release();
	}

	// スワップチェインをRenderTargetとして使用するためのDescriptorHeapを作成
	{
		D3D12_DESCRIPTOR_HEAP_DESC desc = {};
		desc.NumDescriptors = 2;		// フレームバッファとバックバッファ
		desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;		// RenderTargetView
		desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;	// シェーダからアクセスしないのでNONEでOK

		hr = m_pDevice->CreateDescriptorHeap(&desc, IID_PPV_ARGS(&m_pRtvHeap));
		assert(SUCCEEDED(hr));

		m_rtvDescriptorSize = m_pDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
	}

	// スワップチェインのバッファを先に作成したDescriptorHeapに登録する
	{
		D3D12_CPU_DESCRIPTOR_HANDLE handle = m_pRtvHeap->GetCPUDescriptorHandleForHeapStart();

		for (int i = 0; i < 2; i++)
		{
			hr = m_pSwapChain->GetBuffer(i, IID_PPV_ARGS(&m_pRenderTargets[i]));
			assert(SUCCEEDED(hr));

			m_pDevice->CreateRenderTargetView(m_pRenderTargets[i], nullptr, handle);
			handle.ptr += m_rtvDescriptorSize;
		}
	}

	// コマンドアロケータを作成
	hr = m_pDevice->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&m_pCommandAllocator));
	assert(SUCCEEDED(hr));

	factory->Release();

	// ビューポートとシザーボックスの設定
	m_viewport.Width = static_cast<float>(WindowWidth);
	m_viewport.Height = static_cast<float>(WindowHeight);
	m_viewport.TopLeftX = m_viewport.TopLeftY = 0.0f;
	m_viewport.MinDepth = 0.0f;
	m_viewport.MaxDepth = 1.0f;
	m_scissorRect.left = m_scissorRect.top = 0;
	m_scissorRect.right = WindowWidth;
	m_scissorRect.bottom = WindowHeight;

	// コマンドリストを作成する
	{
		hr = m_pDevice->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, m_pCommandAllocator, nullptr, IID_PPV_ARGS(&m_pCommandList));
		assert(SUCCEEDED(hr));

		// コマンドリストを一旦クローズしておく
		// ループ先頭がコマンドリストクローズ状態として処理されているため？
		m_pCommandList->Close();
	}

	// 同期をとるためのフェンスを作成する
	{
		hr = m_pDevice->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&m_pFence));
		assert(SUCCEEDED(hr));
		m_fenceValue = 1;

		m_fenceEvent = CreateEventEx(nullptr, FALSE, FALSE, EVENT_ALL_ACCESS);
		assert(m_fenceEvent != nullptr);
	}

	return hr;
}

//-----------------------------------------------------------------------------
//!	
//-----------------------------------------------------------------------------
void cDevice::destroy( void )
{
	CloseHandle( m_fenceEvent );
	m_fenceEvent	= INVALID_HANDLE_VALUE;
	SAFE_RELEASE( m_pFence );
	SAFE_RELEASE( m_pCommandList );

	SAFE_RELEASE( m_pCommandAllocator );
	SAFE_RELEASE( m_pRtvHeap );
	for( int i=0; i<2; ++i )	SAFE_RELEASE( m_pRenderTargets[ i ] );
	SAFE_RELEASE( m_pSwapChain );
	SAFE_RELEASE( m_pCommandQueue );
	SAFE_RELEASE( m_pDevice );
}

//-----------------------------------------------------------------------------
//!	
//-----------------------------------------------------------------------------
void cDevice::BeginDraw( void )
{
	HRESULT hr;

	// コマンドアロケータをリセット
	hr = m_pCommandAllocator->Reset();
	assert(SUCCEEDED(hr));

	// コマンドリストをリセット
	hr = m_pCommandList->Reset(m_pCommandAllocator, nullptr);
	assert(SUCCEEDED(hr));

	// バックバッファが描画ターゲットとして使用できるようになるまで待つ
	D3D12_RESOURCE_BARRIER barrier;
	barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;					// バリアはリソースの状態遷移に対して設置
	barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
	barrier.Transition.pResource = m_pRenderTargets[m_frameIndex];			// リソースは描画ターゲット
	barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_PRESENT;			// 遷移前はPresent
	barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;		// 遷移後は描画ターゲット
	barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
	m_pCommandList->ResourceBarrier(1, &barrier);

	// バックバッファを描画ターゲットとして設定する
	D3D12_CPU_DESCRIPTOR_HANDLE handle = m_pRtvHeap->GetCPUDescriptorHandleForHeapStart();
	handle.ptr += (m_frameIndex * m_rtvDescriptorSize);
	m_pCommandList->OMSetRenderTargets(1, &handle, false, nullptr);

	// ビューポートとシザーボックスを設定
	m_pCommandList->RSSetViewports(1, &m_viewport);
	m_pCommandList->RSSetScissorRects(1, &m_scissorRect);

	// バックバッファをクリアする
	const float kClearColor[] = { 0.0f, 0.0f, 0.6f, 1.0f };
	m_pCommandList->ClearRenderTargetView(handle, kClearColor, 0, nullptr);
}

//-----------------------------------------------------------------------------
//!	
//-----------------------------------------------------------------------------
void cDevice::EndDraw( void )
{
	// バックバッファの描画完了を待つためのバリアを設置
	D3D12_RESOURCE_BARRIER barrier;
	barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;					// バリアはリソースの状態遷移に対して設置
	barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
	barrier.Transition.pResource = m_pRenderTargets[m_frameIndex];			// リソースは描画ターゲット
	barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;	// 遷移前は描画ターゲット
	barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_PRESENT;			// 遷移後はPresent
	barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
	m_pCommandList->ResourceBarrier(1, &barrier);

	// コマンドリストをクローズする
	m_pCommandList->Close();
}

//-----------------------------------------------------------------------------
//!	
//-----------------------------------------------------------------------------
void cDevice::Present( void )
{
	// コマンドリストを実行する
	ID3D12CommandList* ppCommandLists[] = { m_pCommandList };
	m_pCommandQueue->ExecuteCommandLists(_countof(ppCommandLists), ppCommandLists);

	// スワップチェインのPresent
	m_pSwapChain->Present(1, 0);
}

//-----------------------------------------------------------------------------
//!	
//-----------------------------------------------------------------------------
void cDevice::WaitDrawDone( void )
{
	// 現在のFence値がコマンド終了後にFenceに書き込まれるようにする
	UINT64 fvalue = m_fenceValue;
	m_pCommandQueue->Signal(m_pFence, fvalue);
	m_fenceValue++;

	// まだコマンドキューが終了していないことを確認する
	// ここまででコマンドキューが終了してしまうとイベントが一切発火されなくなるのでチェックしている
	if (m_pFence->GetCompletedValue() < fvalue)
	{
		// このFenceにおいて、fvalue の値になったらイベントを発火させる
		m_pFence->SetEventOnCompletion(fvalue, m_fenceEvent);
		// イベントが発火するまで待つ
		WaitForSingleObject(m_fenceEvent, INFINITE);
	}

	m_frameIndex = m_pSwapChain->GetCurrentBackBufferIndex();
}

//-----------------------------------------------------------------------------
//!	
//-----------------------------------------------------------------------------
void cDevice::setVertexBuffer( cVertexBuffer * pVertexBuffer )
{
	D3D12_VERTEX_BUFFER_VIEW	vbv;

	vbv.BufferLocation	= pVertexBuffer->m_pBuffer->GetGPUVirtualAddress();
	vbv.SizeInBytes		= pVertexBuffer->m_Size;
	vbv.StrideInBytes	= pVertexBuffer->m_Stride;

	m_pCommandList->IASetVertexBuffers( 0, 1, &vbv );
}

//-----------------------------------------------------------------------------
//!	
//-----------------------------------------------------------------------------
void cDevice::setIndexBuffer( cIndexBuffer * pIndexBuffer )
{
	D3D12_INDEX_BUFFER_VIEW	ibv;

	ibv.BufferLocation	= pIndexBuffer->m_pBuffer->GetGPUVirtualAddress();
	ibv.SizeInBytes		= pIndexBuffer->m_Size;
	ibv.Format			= pIndexBuffer->m_Format;

	m_pCommandList->IASetIndexBuffer( &ibv );
}

};/*namespace D3D12*/

//---< end of file >-----------------------------------------------------------
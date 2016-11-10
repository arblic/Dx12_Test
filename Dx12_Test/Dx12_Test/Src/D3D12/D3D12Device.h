//-----------------------------------------------------------------------------
//! @file   D3D12Device.h
//! @brief	デバイス
//! @author 
//-----------------------------------------------------------------------------
#ifndef __D3D12_DEVICE_H__
#define	__D3D12_DEVICE_H__

#include "D3D12DescriptorHeap.h"

//-----------------------------------------------------------------------------
//!	D3D12
//-----------------------------------------------------------------------------
namespace D3D12 {

class cVertexBuffer;
class cIndexBuffer;
class cDescriptorHeap;

//-----------------------------------------------------------------------------
//! @brief	デバイス
//-----------------------------------------------------------------------------
class cDevice {
public:
	cDevice( void );
	virtual~cDevice( void );

	HRESULT create( HWND hWnd, UINT32 WindowWidth, UINT32 WindowHeight );
	void destroy( void );

	void BeginDraw( void );
	void EndDraw( void );
	void Present( void );
	void WaitDrawDone( void );

	void setVertexBuffer( cVertexBuffer * pVertexBuffer );
	void setIndexBuffer( cIndexBuffer * pIndexBuffer );

	ID3D12DescriptorHeap *		GetDescriptorHeap( D3D12_DESCRIPTOR_HEAP_TYPE eType );
	D3D12_CPU_DESCRIPTOR_HANDLE	GetCpuHandle( D3D12_DESCRIPTOR_HEAP_TYPE eType, UINT index );
	D3D12_GPU_DESCRIPTOR_HANDLE	GetGpuHandle( D3D12_DESCRIPTOR_HEAP_TYPE eType, UINT index );

private:
public:
	ID3D12Device *				m_pDevice;
	D3D12_VIEWPORT				m_viewport;
	D3D12_RECT					m_scissorRect;
	ID3D12CommandQueue *		m_pCommandQueue;
	ID3D12CommandAllocator *	m_pCommandAllocator;
	ID3D12GraphicsCommandList *	m_pCommandList;
	IDXGISwapChain3 *			m_pSwapChain;
	ID3D12Resource *			m_pRenderTargets[2];
	ID3D12Fence *				m_pFence;
	HANDLE						m_fenceEvent;

	int							m_frameIndex;
	UINT64						m_fenceValue;

	cDescriptorHeap				m_DescriptorHeap[ D3D12_DESCRIPTOR_HEAP_TYPE_NUM_TYPES ];
};

};/*namespace D3D12*/

#endif/*__D3D12_DEVICE_H__*/

//---< end of file >-----------------------------------------------------------
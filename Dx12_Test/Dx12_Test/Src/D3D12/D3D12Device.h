//-----------------------------------------------------------------------------
//! @file   D3D12Device.h
//! @brief	デバイス
//! @author 
//-----------------------------------------------------------------------------
#ifndef __D3D12_DEVICE_H__
#define	__D3D12_DEVICE_H__

//-----------------------------------------------------------------------------
//!	D3D12
//-----------------------------------------------------------------------------
namespace D3D12 {

class cVertexBuffer;
class cIndexBuffer;

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

private:
public:
	ID3D12Device *				m_pDevice;
	D3D12_VIEWPORT				m_viewport;
	D3D12_RECT					m_scissorRect;
	ID3D12CommandQueue *		m_pCommandQueue;
	ID3D12CommandAllocator *	m_pCommandAllocator;
	ID3D12GraphicsCommandList *	m_pCommandList;
	IDXGISwapChain3 *			m_pSwapChain;
	ID3D12DescriptorHeap *		m_pRtvHeap;
	ID3D12Resource *			m_pRenderTargets[2];
	ID3D12Fence *				m_pFence;
	HANDLE						m_fenceEvent;

	int							m_frameIndex;
	UINT						m_rtvDescriptorSize;
	UINT64						m_fenceValue;
};

};/*namespace D3D12*/

#endif/*__D3D12_DEVICE_H__*/

//---< end of file >-----------------------------------------------------------
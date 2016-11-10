//-----------------------------------------------------------------------------
//! @file   D3D12DescriptorHeap.h
//! @brief  D3D12 デスクリプタヒープ
//! @author 
//-----------------------------------------------------------------------------
#ifndef __D3D12_DESCRIPTORHEAP_H__
#define	__D3D12_DESCRIPTORHEAP_H__

//-----------------------------------------------------------------------------
//!	D3D12
//-----------------------------------------------------------------------------
namespace D3D12 {

class cDevice;

//-----------------------------------------------------------------------------
//! @brief  D3D12 デスクリプタヒープ
//-----------------------------------------------------------------------------
class cDescriptorHeap {
public:
	cDescriptorHeap( void );
	virtual~cDescriptorHeap( void );

	bool create( cDevice * pDevice, UINT NumDescriptors, D3D12_DESCRIPTOR_HEAP_TYPE eType, bool bShaderVisiblity );
	void destroy( void );

	D3D12_CPU_DESCRIPTOR_HANDLE	GetCpuHandle( UINT index );
	D3D12_GPU_DESCRIPTOR_HANDLE	GetGpuHandle( UINT index );

private:
public:
	ID3D12DescriptorHeap *		m_pDescriptorHeap;
	D3D12_DESCRIPTOR_HEAP_DESC	m_Desc;
	UINT						m_IncrementalSize;
};

};/*namespace D3D12*/

#endif/*__D3D12_DESCRIPTORHEAP_H__*/

//---< end of file >-----------------------------------------------------------
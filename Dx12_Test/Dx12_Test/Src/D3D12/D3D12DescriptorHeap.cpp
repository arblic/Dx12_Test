//-----------------------------------------------------------------------------
//! @file   D3D12DescriptorHeap.cpp
//! @brief  D3D12 デスクリプタヒープ
//! @author 
//-----------------------------------------------------------------------------
#include "stdafx.h"
#include "D3D12DescriptorHeap.h"

namespace D3D12 {

//-----------------------------------------------------------------------------
//!	ctor
//-----------------------------------------------------------------------------
cDescriptorHeap::cDescriptorHeap( void )
{
	m_pDescriptorHeap		= NULL;
	ZeroMemory( &m_Desc, sizeof(m_Desc) );
	m_IncrementalSize		= 0;
}

//-----------------------------------------------------------------------------
//!	dtor
//-----------------------------------------------------------------------------
cDescriptorHeap::~cDescriptorHeap( void )
{
	destroy();
}

//-----------------------------------------------------------------------------
//!	
//-----------------------------------------------------------------------------
bool cDescriptorHeap::create( cDevice * pDevice, UINT NumDescriptors, D3D12_DESCRIPTOR_HEAP_TYPE eType, bool bShaderVisiblity )
{
	HRESULT		hr	= S_OK;

	m_Desc.NumDescriptors	= NumDescriptors;
	m_Desc.Type				= eType;
	m_Desc.Flags			= bShaderVisiblity ? D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE : D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	m_Desc.NodeMask			= 0;

	hr	= pDevice->m_pDevice->CreateDescriptorHeap( &m_Desc, IID_PPV_ARGS(&m_pDescriptorHeap) );
	if( FAILED( hr ) ) {
		return false;
	}

	m_IncrementalSize	= pDevice->m_pDevice->GetDescriptorHandleIncrementSize( eType );

	return true;
}

//-----------------------------------------------------------------------------
//!	
//-----------------------------------------------------------------------------
void cDescriptorHeap::destroy( void )
{
	SAFE_RELEASE( m_pDescriptorHeap );
	ZeroMemory( &m_Desc, sizeof(m_Desc) );
}

//-----------------------------------------------------------------------------
//!	
//-----------------------------------------------------------------------------
D3D12_CPU_DESCRIPTOR_HANDLE	cDescriptorHeap::GetCpuHandle( UINT index )
{
	D3D12_CPU_DESCRIPTOR_HANDLE	Handle = {};

	if( m_pDescriptorHeap ) {
		Handle	= m_pDescriptorHeap->GetCPUDescriptorHandleForHeapStart();
		Handle.ptr	+= m_IncrementalSize * index;
	}

	return Handle;
}

//-----------------------------------------------------------------------------
//!	
//-----------------------------------------------------------------------------
D3D12_GPU_DESCRIPTOR_HANDLE	cDescriptorHeap::GetGpuHandle( UINT index )
{
	D3D12_GPU_DESCRIPTOR_HANDLE	Handle = {};

	if( m_pDescriptorHeap ) {
		Handle	= m_pDescriptorHeap->GetGPUDescriptorHandleForHeapStart();
		Handle.ptr	+= m_IncrementalSize * index;
	}

	return Handle;
}

};/*namespace D3D12*/

//---< end of file >-----------------------------------------------------------
//-----------------------------------------------------------------------------
//! @file   D3D12VertexBuffer.cpp
//! @brief  D3D12 頂点バッファ
//! @author 
//-----------------------------------------------------------------------------
#include "stdafx.h"
#include "D3D12VertexBuffer.h"

namespace D3D12 {

//-----------------------------------------------------------------------------
//!	ctor
//-----------------------------------------------------------------------------
cVertexBuffer::cVertexBuffer( void )
{
	m_pBuffer	= NULL;
	m_Size		= 0;
}

//-----------------------------------------------------------------------------
//!	dtor
//-----------------------------------------------------------------------------
cVertexBuffer::~cVertexBuffer( void )
{
	destroy();
}

//-----------------------------------------------------------------------------
//!	
//-----------------------------------------------------------------------------
bool cVertexBuffer::create( cDevice * pDevice, const void * pData, size_t Size, size_t Stride )
{
	HRESULT	hr	= S_OK;
	UINT8 *	p	= NULL;

	D3D12_HEAP_PROPERTIES prop = {};
	prop.Type = D3D12_HEAP_TYPE_UPLOAD;
	prop.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
	prop.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
	prop.CreationNodeMask = 1;
	prop.VisibleNodeMask = 1;

	D3D12_RESOURCE_DESC desc = {};
	desc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
	desc.Alignment = 0;
	desc.Width = Size;
	desc.Height = 1;
	desc.DepthOrArraySize = 1;
	desc.MipLevels = 1;
	desc.Format = DXGI_FORMAT_UNKNOWN;
	desc.SampleDesc.Count = 1;
	desc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
	desc.Flags = D3D12_RESOURCE_FLAG_NONE;

	hr	= pDevice->m_pDevice->CreateCommittedResource(&prop, D3D12_HEAP_FLAG_NONE, &desc, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&m_pBuffer));
	assert(SUCCEEDED(hr));

	// バッファにコピー
	hr = m_pBuffer->Map(0, nullptr, reinterpret_cast<void **>(&p));
	assert(SUCCEEDED(hr));
	memcpy( p, pData, Size );
	m_pBuffer->Unmap( 0, nullptr );

	m_Size		= (UINT)Size;
	m_Stride	= (UINT)Stride;

	return true;
}

//-----------------------------------------------------------------------------
//!	
//-----------------------------------------------------------------------------
void cVertexBuffer::destroy( void )
{
	SAFE_RELEASE( m_pBuffer );
	m_Size		= 0;
}

};/*namespace D3D12*/

//---< end of file >-----------------------------------------------------------
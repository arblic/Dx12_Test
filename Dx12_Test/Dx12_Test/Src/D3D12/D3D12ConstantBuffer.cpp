//-----------------------------------------------------------------------------
//! @file   D3D12ConstantBuffer.cpp
//! @brief  D3D12 コンスタントバッファ
//! @author 
//-----------------------------------------------------------------------------
#include "stdafx.h"
#include "D3D12ConstantBuffer.h"

namespace D3D12 {

//-----------------------------------------------------------------------------
//!	ctor
//-----------------------------------------------------------------------------
cConstantBuffer::cConstantBuffer( void )
{
	m_pBuffer	= NULL;
	m_Size		= 0;
}

//-----------------------------------------------------------------------------
//!	dtor
//-----------------------------------------------------------------------------
cConstantBuffer::~cConstantBuffer( void )
{
	destroy();
}

//-----------------------------------------------------------------------------
//!	
//-----------------------------------------------------------------------------
bool cConstantBuffer::create( cDevice * pDevice, size_t Size )
{
	HRESULT	hr	= S_OK;
	UINT8 *	p	= NULL;

	D3D12_HEAP_PROPERTIES prop;
	prop.Type = D3D12_HEAP_TYPE_UPLOAD;
	prop.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
	prop.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
	prop.CreationNodeMask = 1;
	prop.VisibleNodeMask = 1;

	D3D12_RESOURCE_DESC desc;

	desc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
	desc.Alignment = 0;
	desc.Width = Size;			// とりあえず256バイトとっておく
								// 定数バッファサイズは256バイトでアラインメントされる必要があるらしい
	desc.Height = 1;
	desc.DepthOrArraySize = 1;
	desc.MipLevels = 1;
	desc.Format = DXGI_FORMAT_UNKNOWN;
	desc.SampleDesc = { 1, 0 };
	desc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
	desc.Flags = D3D12_RESOURCE_FLAG_NONE;

	hr = pDevice->m_pDevice->CreateCommittedResource(&prop, D3D12_HEAP_FLAG_NONE, &desc, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&m_pBuffer) );
	assert(SUCCEEDED(hr));

	m_Size		= (UINT)Size;

	return true;
}

//-----------------------------------------------------------------------------
//!	
//-----------------------------------------------------------------------------
void cConstantBuffer::destroy( void )
{
	SAFE_RELEASE( m_pBuffer );
	m_Size		= 0;
}

//-----------------------------------------------------------------------------
//!	
//-----------------------------------------------------------------------------
void cConstantBuffer::setValue( const void * pData, size_t Size )
{
	HRESULT	hr	= S_OK;
	UINT8 *	p	= NULL;

	hr = m_pBuffer->Map( 0, nullptr, reinterpret_cast<void **>( &p ) );
	if( FAILED( hr ) ) {
		return;
	}
	
	// バッファにコピー
	memcpy( p, pData, Size );

	m_pBuffer->Unmap( 0, nullptr );
}

//-----------------------------------------------------------------------------
//!	
//-----------------------------------------------------------------------------
void cConstantBuffer::getViewDesc( D3D12_CONSTANT_BUFFER_VIEW_DESC & rDesc )
{
	rDesc.BufferLocation	= m_pBuffer->GetGPUVirtualAddress();
	rDesc.SizeInBytes		= m_Size;
}

};/*namespace D3D12*/

//---< end of file >-----------------------------------------------------------
//-----------------------------------------------------------------------------
//! @file   D3D12Texture.cpp
//! @brief  D3D12 テクスチャ
//! @author 
//-----------------------------------------------------------------------------
#include "stdafx.h"
#include "D3D12Texture.h"

namespace D3D12 {

//-----------------------------------------------------------------------------
//!	ctor
//-----------------------------------------------------------------------------
cTexture::cTexture( void )
{
	m_pTexture	= nullptr;
	m_pData		= nullptr;
	m_Size		= 0;
}

//-----------------------------------------------------------------------------
//!	dtor
//-----------------------------------------------------------------------------
cTexture::~cTexture( void )
{
	destroy();
}

//-----------------------------------------------------------------------------
//!	
//-----------------------------------------------------------------------------
bool cTexture::create( cDevice * pDevice, const wchar_t * pFileName )
{
	HRESULT		hr	= S_OK;
	UINT8 *		pData	= nullptr;
	size_t		DataSize	= 256 * 256 * 4;

	D3D12_HEAP_PROPERTIES HeapProps;
	HeapProps.Type = D3D12_HEAP_TYPE_CUSTOM;// D3D12_HEAP_TYPE_DEFAULT;
	HeapProps.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_WRITE_BACK; //D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
	HeapProps.MemoryPoolPreference = D3D12_MEMORY_POOL_L0;// D3D12_MEMORY_POOL_UNKNOWN;
	HeapProps.CreationNodeMask = 1;
	HeapProps.VisibleNodeMask = 1;

	D3D12_RESOURCE_DESC ResourceDesc;
	ResourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	ResourceDesc.Alignment = 0;
	ResourceDesc.Width = static_cast<UINT64>( 256 );
	ResourceDesc.Height = static_cast<UINT>( 256 );
	ResourceDesc.DepthOrArraySize = static_cast<UINT>( 1 );
	ResourceDesc.MipLevels = static_cast<UINT>( 1 );
	ResourceDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	ResourceDesc.SampleDesc.Count = 1;
	ResourceDesc.SampleDesc.Quality = 0;
	ResourceDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
	ResourceDesc.Flags = D3D12_RESOURCE_FLAG_NONE;

	hr	= pDevice->m_pDevice->CreateCommittedResource(	&HeapProps,
														D3D12_HEAP_FLAG_NONE,
														&ResourceDesc,
														D3D12_RESOURCE_STATE_COMMON,
														nullptr,
														MY_IID_PPV_ARGS(&m_pTexture) );
	if( FAILED( hr ) ) {
		return false;
	}

	pData	= new UINT8[ DataSize ];
	memset( pData, 0xFF, DataSize );
	D3D12_BOX box = {0, 0, 0, 256, 256, 1};
	hr = m_pTexture->WriteToSubresource( 0, &box, pData, 4 * 256, 4 * 256 * 256 );

	delete[] pData;

	return true;
}

//-----------------------------------------------------------------------------
//!	
//-----------------------------------------------------------------------------
void cTexture::destroy( void )
{
	if( m_pTexture ) {
		m_pTexture->Release();
	}
	m_pTexture	= nullptr;

	if( m_pData ) {
		delete[] m_pData;
	}
	m_pData	= NULL;
	m_Size	= 0;
}

//-----------------------------------------------------------------------------
//!	
//-----------------------------------------------------------------------------
void cTexture::getViewDesc( D3D12_SHADER_RESOURCE_VIEW_DESC & rDesc )
{
	rDesc.Format						= DXGI_FORMAT_R8G8B8A8_UNORM;
	rDesc.ViewDimension					= D3D12_SRV_DIMENSION_TEXTURE2D;
	rDesc.Shader4ComponentMapping		= D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	rDesc.Texture2D.MipLevels			= 1;
	rDesc.Texture2D.MostDetailedMip		= 0;
	rDesc.Texture2D.PlaneSlice			= 0;
	rDesc.Texture2D.ResourceMinLODClamp	= 0;
}

};/*namespace D3D12*/

//---< end of file >-----------------------------------------------------------
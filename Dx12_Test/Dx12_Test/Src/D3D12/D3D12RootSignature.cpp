//-----------------------------------------------------------------------------
//! @file   D3D12RootSignature.cpp
//! @brief  D3D12 ルートシグネチャ
//! @author 
//-----------------------------------------------------------------------------
#include "stdafx.h"
#include "D3D12RootSignature.h"

namespace D3D12 {

//-----------------------------------------------------------------------------
//!	ctor
//-----------------------------------------------------------------------------
cRootSignature::cRootSignature( void )
{
	m_pRootSignature	= NULL;
	m_pData				= NULL;
}

//-----------------------------------------------------------------------------
//!	dtor
//-----------------------------------------------------------------------------
cRootSignature::~cRootSignature( void )
{
	destroy();
}

//-----------------------------------------------------------------------------
//!	
//-----------------------------------------------------------------------------
bool cRootSignature::create( cDevice * pDevice, char * pSettingJson, size_t Length )
{
	HRESULT	hr	= S_OK;

	//json_t *	pRoot;



	//ID3DBlob* pSignature;
	//ID3DBlob* pError;
	//hr = D3D12SerializeRootSignature(&desc, D3D_ROOT_SIGNATURE_VERSION_1, &pSignature, &pError);
	//if( FAILED( hr ) ) {
	//	OutputDebugStringA( (LPCSTR)pError->GetBufferPointer() );
	//	assert( 0 );
	//}

	//hr = pDevice->m_pDevice->CreateRootSignature(0, pSignature->GetBufferPointer(), pSignature->GetBufferSize(), IID_PPV_ARGS(&m_pRootSignature));

	return true;
}

//-----------------------------------------------------------------------------
//!	
//-----------------------------------------------------------------------------
void cRootSignature::destroy( void )
{
	SAFE_RELEASE( m_pRootSignature );
	SAFE_RELEASE( m_pData );
}

};/*namespace D3D12*/

//---< end of file >-----------------------------------------------------------
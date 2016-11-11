//-----------------------------------------------------------------------------
//! @file   D3D12Shader.cpp
//! @brief  D3D12 シェーダ
//! @author 
//-----------------------------------------------------------------------------
#include "stdafx.h"
#include "D3D12Shader.h"

namespace D3D12 {

//-----------------------------------------------------------------------------
//!	ctor
//-----------------------------------------------------------------------------
cShader::cShader( void )
{
	memset( &m_VS, 0, sizeof(m_VS) );
	memset( &m_PS, 0, sizeof(m_PS) );
	m_pDataVS	= NULL;
	m_SizeVS	= 0;
	m_pDataPS	= NULL;
	m_SizePS	= 0;
}

//-----------------------------------------------------------------------------
//!	dtor
//-----------------------------------------------------------------------------
cShader::~cShader( void )
{
	destroy();
}

//-----------------------------------------------------------------------------
//!	
//-----------------------------------------------------------------------------
bool cShader::create( const wchar_t * pDirectory, const wchar_t * pShaderName )
{
	wchar_t	VS[ MAX_PATH ] = {0};
	wchar_t	PS[ MAX_PATH ] = {0};

	swprintf_s( VS, L"%s/%s.vs", pDirectory, pShaderName );
	swprintf_s( PS, L"%s/%s.ps", pDirectory, pShaderName );

	if( !FileProxy::isExist( VS ) )		return false;
	if( !FileProxy::isExist( PS ) )		return false;

	m_SizeVS	= FileProxy::getSize( VS );
	if( 0 == m_SizeVS )	return false;
	m_pDataVS	= new BYTE[ m_SizeVS ]();
	if( !m_pDataVS )	return false;
	if( !FileProxy::read( VS, m_pDataVS, m_SizeVS ) )	return false;

	m_SizePS	= FileProxy::getSize( PS );
	if( 0 == m_SizePS )	return false;
	m_pDataPS	= new BYTE[ m_SizePS ]();
	if( !m_pDataPS )	return false;
	if( !FileProxy::read( PS, m_pDataPS, m_SizePS ) )	return false;

	m_VS.pShaderBytecode	= m_pDataVS;
	m_VS.BytecodeLength		= m_SizeVS;
	m_PS.pShaderBytecode	= m_pDataPS;
	m_PS.BytecodeLength		= m_SizePS;

	analyze();

	return true;
}

//-----------------------------------------------------------------------------
//!	
//-----------------------------------------------------------------------------
void cShader::destroy( void )
{
	memset( &m_VS, 0, sizeof(m_VS) );
	memset( &m_PS, 0, sizeof(m_PS) );

	if( m_pDataVS )	delete[] m_pDataVS;
	if( m_pDataPS )	delete[] m_pDataPS;

	m_pDataVS	= NULL;
	m_pDataPS	= NULL;
	m_SizeVS	= 0;
	m_SizePS	= 0;
}

//-----------------------------------------------------------------------------
//!	
//-----------------------------------------------------------------------------
void cShader::analyze( void )
{
	ID3D12ShaderReflection *				pRef;
	ID3D12ShaderReflectionConstantBuffer *	pRefCBuf;
	D3D12_SHADER_DESC						ShaderDesc;
	D3D12_SHADER_BUFFER_DESC				ShaderBufDesc;
	D3D12_SHADER_INPUT_BIND_DESC			ShaderInputBindDesc;

	// シェーダリフレクション
	D3DReflect( m_pDataVS, m_SizeVS, IID_PPV_ARGS( &pRef ) );

	pRef->GetDesc( &ShaderDesc );

	for( UINT i=0; i<ShaderDesc.ConstantBuffers; ++i ) {
		pRefCBuf	= pRef->GetConstantBufferByIndex( i );
		if( pRefCBuf ) {
			pRefCBuf->GetDesc( &ShaderBufDesc );
		}
	}

	for( UINT i=0; i<ShaderDesc.BoundResources; ++i ) {
		pRef->GetResourceBindingDesc( i, &ShaderInputBindDesc );
	}

	SAFE_RELEASE( pRef );


	// シェーダリフレクション
	D3DReflect( m_pDataPS, m_SizePS, IID_PPV_ARGS( &pRef ) );

	pRef->GetDesc( &ShaderDesc );

	for( UINT i=0; i<ShaderDesc.ConstantBuffers; ++i ) {
		pRefCBuf	= pRef->GetConstantBufferByIndex( i );
		if( pRefCBuf ) {
			pRefCBuf->GetDesc( &ShaderBufDesc );
		}
	}

	for( UINT i=0; i<ShaderDesc.BoundResources; ++i ) {
		pRef->GetResourceBindingDesc( i, &ShaderInputBindDesc );
	}

	SAFE_RELEASE( pRef );

}

//-----------------------------------------------------------------------------
//!	
//-----------------------------------------------------------------------------
const D3D12_SHADER_BYTECODE & cShader::getVS( void )
{
	return m_VS;
}

//-----------------------------------------------------------------------------
//!	
//-----------------------------------------------------------------------------
const D3D12_SHADER_BYTECODE & cShader::getPS( void )
{
	return m_PS;
}

//-----------------------------------------------------------------------------
//!	
//-----------------------------------------------------------------------------
size_t cShader::getRootSignatureJsonText( char * pTextBuf, size_t Length )
{
	size_t	CopyLength	= 0;

	memset( pTextBuf, 0x00, sizeof(char) * Length );

	if( !pTextBuf ) {
		return m_RootSignatureJsonTextLength;
	} else {
		CopyLength	= min( Length - 1, m_RootSignatureJsonTextLength );

		memcpy_s( pTextBuf, CopyLength, m_pRootSignatureJsonText, m_RootSignatureJsonTextLength );
	}

	return CopyLength;
}

};/*namespace D3D12*/

//---< end of file >-----------------------------------------------------------
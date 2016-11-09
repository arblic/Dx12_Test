//-----------------------------------------------------------------------------
//! @file   D3D12Shader.h
//! @brief  D3D12 シェーダ
//! @author 
//-----------------------------------------------------------------------------
#ifndef __D3D12_SHADER_H__
#define	__D3D12_SHADER_H__

//-----------------------------------------------------------------------------
//!	D3D12
//-----------------------------------------------------------------------------
namespace D3D12 {

//-----------------------------------------------------------------------------
//!	シェーダ
//-----------------------------------------------------------------------------
class cShader {
public:
	cShader( void );
	virtual~cShader( void );

	bool create( const wchar_t * pDirectory, const wchar_t * pShaderName );
	void destroy( void );

	const D3D12_SHADER_BYTECODE &	getVS( void );
	const D3D12_SHADER_BYTECODE &	getPS( void );

private:
	D3D12_SHADER_BYTECODE	m_VS;
	D3D12_SHADER_BYTECODE	m_PS;
	BYTE *					m_pDataVS;
	size_t					m_SizeVS;
	BYTE *					m_pDataPS;
	size_t					m_SizePS;
};

};/*namespace D3D12*/

#endif/*__D3D12_SHADER_H__*/

//---< end of file >-----------------------------------------------------------
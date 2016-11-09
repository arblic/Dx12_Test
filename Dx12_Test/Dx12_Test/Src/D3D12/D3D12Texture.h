//-----------------------------------------------------------------------------
//! @file   D3D12Texture.h
//! @brief  D3D12 テクスチャ
//! @author 
//-----------------------------------------------------------------------------
#ifndef __D3D12_TEXTURE_H__
#define	__D3D12_TEXTURE_H__

//-----------------------------------------------------------------------------
//!	D3D12
//-----------------------------------------------------------------------------
namespace D3D12 {

//-----------------------------------------------------------------------------
//! @brief  D3D12 テクスチャ
//-----------------------------------------------------------------------------
class cTexture {
public:
	cTexture( void );
	virtual~cTexture( void );

	bool create( cDevice * pDevice, const wchar_t * pFileName );
	void destroy( void );

	void getViewDesc( D3D12_SHADER_RESOURCE_VIEW_DESC & rDesc );

private:
public:
	ID3D12Resource *		m_pTexture;
	BYTE *					m_pData;
	size_t					m_Size;
};

};/*namespace D3D12*/

#endif/*__D3D12_TEXTURE_H__*/

//---< end of file >-----------------------------------------------------------
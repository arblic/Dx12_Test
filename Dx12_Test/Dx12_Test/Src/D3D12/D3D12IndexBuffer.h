//-----------------------------------------------------------------------------
//! @file   D3D12IndexBuffer.h
//! @brief  D3D12 インデクスバッファ
//! @author 
//-----------------------------------------------------------------------------
#ifndef __D3D12_INDEXBUFFER_H__
#define	__D3D12_INDEXBUFFER_H__

//-----------------------------------------------------------------------------
//!	D3D12
//-----------------------------------------------------------------------------
namespace D3D12 {

//-----------------------------------------------------------------------------
//!	インデクスバッファ
//-----------------------------------------------------------------------------
class cIndexBuffer {
public:
	cIndexBuffer( void );
	virtual~cIndexBuffer( void );

	bool create( cDevice * pDevice, const void * pData, size_t Size, bool b32bit=false );
	void destroy( void );

private:
public:
	ID3D12Resource *		m_pBuffer;
	UINT					m_Size;
	DXGI_FORMAT				m_Format;
};

};/*namespace D3D12*/

#endif/*__D3D12_INDEXBUFFER_H__*/

//---< end of file >-----------------------------------------------------------
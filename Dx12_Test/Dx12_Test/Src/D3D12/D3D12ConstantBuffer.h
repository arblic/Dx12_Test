//-----------------------------------------------------------------------------
//! @file   D3D12ConstantBuffer.h
//! @brief  D3D12 コンスタントバッファ
//! @author 
//-----------------------------------------------------------------------------
#ifndef __D3D12_CONSTANTBUFFER_H__
#define	__D3D12_CONSTANTBUFFER_H__

//-----------------------------------------------------------------------------
//!	D3D12
//-----------------------------------------------------------------------------
namespace D3D12 {

//-----------------------------------------------------------------------------
//! @brief  D3D12 コンスタントバッファ
//-----------------------------------------------------------------------------
class cConstantBuffer {
public:
	cConstantBuffer( void );
	virtual~cConstantBuffer( void );

	bool create( cDevice * pDevice, size_t Size=256 );
	void destroy( void );

	void setValue( const void * pData, size_t Size );
	void getViewDesc( D3D12_CONSTANT_BUFFER_VIEW_DESC & rDesc );

private:
public:
	ID3D12Resource *		m_pBuffer;
	UINT					m_Size;
};

};/*namespace D3D12*/

#endif/*__D3D12_CONSTANTBUFFER_H__*/

//---< end of file >-----------------------------------------------------------
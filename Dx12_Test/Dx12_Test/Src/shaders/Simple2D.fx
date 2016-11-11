//------------------------------------------------------------------------------
//!	
//------------------------------------------------------------------------------

//!
struct VSInput {
	float4	position	: POSITION;
	float4	uv			: TEXCOORD;
	float4	color		: COLOR;
};

//!
struct VSOutput {
	float4	position	: SV_POSITION;
	float4	uv			: TEXCOORD;
	float4	color		: COLOR;
};

//!
cbuffer CBufViewProj : register( b0 ) {
	matrix	VPMtx;
};

//!
cbuffer CBufWorld : register( b1 ) {
	matrix	WorldMtx;
};

//
Texture2D		g_Texture_00		: register( t0 );
SamplerState	g_Texture_00Sampler	: register( s0 );

//!
VSOutput VS( VSInput In )
{
	VSOutput Out;

	Out.position	= mul( In.position, WorldMtx );
	Out.position	= mul( Out.position, VPMtx );
	Out.uv			= In.uv;
	Out.color		= In.color;

	return Out;
}

//!
float4 PS( VSOutput In ) : SV_TARGET0
{
	float4	TexColor	= g_Texture_00.Sample( g_Texture_00Sampler, In.uv.xy );

	return In.color * TexColor;
}

//---< end of file >------------------------------------------------------------
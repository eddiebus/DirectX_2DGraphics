cbuffer VConstant : register(b0)
{
	matrix WorldMatrix;
	matrix ViewMatrix;
	matrix ProjectionMartix;
};

struct VS_INPUT
{
	float4 position : POSITION;
	float4 colour : COLOR;
	float2 TexCoord: TEXCOORD0;
};

struct PS_INPUT
{
	float4 position : SV_Position;
	float4 colour : COLOR;
	float2 TexCoord: TEXCOORD0;
};

PS_INPUT vsMain(VS_INPUT input)
{
	PS_INPUT output;
	input.position.w = 1.0f;
	output.colour = input.colour;
	output.position = input.position;
	output.position = mul(output.position, WorldMatrix);
	output.position = mul(output.position, ViewMatrix);
	output.position = mul(output.position, ProjectionMartix);
	output.TexCoord = input.TexCoord;

	return output;
};
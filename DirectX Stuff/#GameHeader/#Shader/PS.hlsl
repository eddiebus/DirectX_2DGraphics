SamplerState SampleState;
Texture2D Texture : register (t0);
Texture2D AlphaMask: register(t1);

cbuffer PConstant : register(b0)
{
	float2 Texturing;
	float2 AlphaMasking;
	float4 ColourBalance;
};

struct PS_INPUT
{
	float4 position : SV_Position;
	float4 colour : COLOR;
	float2 TexCoord: TEXCOORD0;
};


bool CheckColourEqual(float4 Colour1, float4 Colour2)
{
	int Count = 0;
	Count += Colour1.r == Colour2.r;
	Count += Colour1.g == Colour2.g;
	Count += Colour1.b == Colour2.b;
	Count += Colour1.a == Colour2.a;
	if (Count == 4) //All Channels Match | Colours are equal
	{
		return true;
	}
	else
	{
		return false;
	}
}

float4 main(PS_INPUT input) : SV_Target
{
	float4 OutputColour = input.colour;
	if (Texturing.x)
	{
		OutputColour = Texture.Sample(SampleState,input.TexCoord);
	}

	if (AlphaMasking.x)
	{
		OutputColour.a = AlphaMask.Sample(SampleState,input.TexCoord);
	}

	OutputColour *= ColourBalance;
	return OutputColour;
};
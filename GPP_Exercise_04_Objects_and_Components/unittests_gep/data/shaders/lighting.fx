Texture2D diffuse;
SamplerState samLinear
{
    Filter = MIN_MAG_MIP_LINEAR;
    AddressU = Wrap;
    AddressV = Wrap;
};

cbuffer cbNeverChanges
{
    matrix View;
};

cbuffer cbChangeOnResize
{
    matrix Projection;
};

cbuffer cbChangesEveryFrame
{
    matrix Model;
};

struct VS_INPUT
{
    float3 Pos : POSITION;
	float3 Normal : NORMAL;
	float2 Tex : TEXCOORD;
};

struct PS_INPUT
{
    float4 Pos : SV_POSITION;
	float3 Normal : TEXCOORD0;
	float2 Tex : TEXCOORD1;
};


//--------------------------------------------------------------------------------------
// Vertex Shader
//--------------------------------------------------------------------------------------
PS_INPUT VS( VS_INPUT input )
{
    PS_INPUT output = (PS_INPUT)0;
    output.Pos = mul( float4(input.Pos, 1.0f), Model );
    output.Pos = mul( output.Pos, View );
    output.Pos = mul( output.Pos, Projection );
	
	output.Normal = mul( input.Normal, Model );
	output.Tex = input.Tex;
    
    return output;
}


//--------------------------------------------------------------------------------------
// Pixel Shader
//--------------------------------------------------------------------------------------
float4 PS( PS_INPUT input) : SV_Target
{
	float3 n = normalize(input.Normal);
	float3 lightDir = normalize(float3(2, 2, 3));

    return float4(diffuse.Sample( samLinear, input.Tex ).rgb, 1.0f) * (saturate( dot( n, lightDir) ) + 0.3);
}


//--------------------------------------------------------------------------------------
RasterizerState rs { CullMode = Front; };

BlendState bs
{ 
    BlendEnable[0] = False;
};

technique10 Render
{
    pass P0
    {
		SetRasterizerState(rs);
		SetBlendState(bs, float4( 0.0f, 0.0f, 0.0f, 0.0f ), 0xFFFFFFFF);
        SetVertexShader( CompileShader( vs_4_0, VS() ) );
        SetGeometryShader( NULL );
        SetPixelShader( CompileShader( ps_4_0, PS() ) );
    }
}


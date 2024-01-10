#include "./shaders/common.hlsl"

struct VS_INPUT {
    float3  position    :   POSITION;
    float2  texcoord    :   TEXCOORD;
    float3  normal      :   NORMAL;
};

struct VS_OUTPUT {
    float4 position : SV_POSITION;
    float2 texcoord : TEXCOORD0;
    float3 normal : NORMAL;
};

struct GS_INPUT {
    float4 position : POSITION;
    float2 texcoord : TEXCOORD0;
    float3 normal : NORMAL;
};

struct PS_INPUT {
    float4 position : POSITION;
    float2 texcoord : TEXCOORD0;
    float3 normal : NORMAL;
};

VS_OUTPUT vs_main(VS_INPUT i) 
{
    VS_OUTPUT o;

    o.position = mul(g_projection_matrix, mul(g_view_matrix, float4(i.position, 1.0)));
    o.texcoord = i.texcoord;
    o.normal = i.normal;
    return o;
}

[maxvertexcount(3)]
void gs_main(triangle GS_INPUT input[3], inout TriangleStream<PS_INPUT> triStream)
{
    for (int i = 0; i < 3; ++i)
    {
        PS_INPUT output;
        output.position = input[i].position;
        output.texcoord = input[i].texcoord;
        output.normal = input[i].normal;

        triStream.Append(output);
    }
}

float4 ps_main(PS_INPUT i) : SV_TARGET
{  
    float mask = fmod(floor(5.0 * i.texcoord.x),2.0);
    mask += fmod(floor(5.0 * i.texcoord.y),2.0);
    mask = 0.5 + 0.1 * mask;
    float3 col = lerp((0.5 + 0.5 * i.normal),float3(1.0,0.0,1.0),0.7);
    return float4(mask * col, 1.0);
}

#include "./shaders/common.hlsl"
#include "./shaders/noise.hlsl"


Buffer<float> vertices : register(t0);

struct VS_INPUT {
    uint vertexid : SV_VertexID;
};

struct VS_OUTPUT {
    uint vertexid : TEXCOORD0;
};

struct PS_INPUT {
    float4 position  : SV_POSITION;
    float3 color : TEXCOORD0;
};

VS_OUTPUT vs_main(VS_INPUT i) 
{
    VS_OUTPUT o;
    o.vertexid = i.vertexid;
    return o;
}

[maxvertexcount(3)]
void gs_main(point VS_OUTPUT input[1], inout TriangleStream<PS_INPUT> triStream)
{
    float3 color = float3(Hash(input[0].vertexid),Hash(1.167*input[0].vertexid+2.991),Hash(2.1*input[0].vertexid+0.991));
    for (int i = 0; i < 3; i++)
    {
        PS_INPUT output;

        float3 world_pos = float3(vertices[8*(3*input[0].vertexid+i)+0],vertices[8*(3*input[0].vertexid+i)+1],vertices[8*(3*input[0].vertexid+i)+2]);
        output.position = mul(g_projection_matrix,mul(g_view_matrix,float4(world_pos,1.0)));
        output.color = color;
        triStream.Append(output);
    }

    triStream.RestartStrip();
}

float4 ps_main(PS_INPUT i) : SV_TARGET
{  
    return float4(i.color,1.0);
}

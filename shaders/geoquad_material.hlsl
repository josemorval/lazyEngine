
#define GPU_PARTICLE
#include "./shaders/common.hlsl"

struct VS_INPUT
{
    uint instanceid : SV_InstanceID;
    uint vertexID : SV_VertexId;
};

struct GS_INPUT
{
    uint vertexID : VERTEXID;
};

struct PS_INPUT
{
    float4 position : SV_POSITION;
    float time : TEXCOORD0;
};


GS_INPUT vs_main(VS_INPUT input)
{
    GS_INPUT output = (GS_INPUT)0;
    output.vertexID = input.vertexID + input.instanceid;
    return output;
}

[maxvertexcount(4)]
void gs_main(point GS_INPUT input[1], inout TriangleStream<PS_INPUT> OutputStream) {

    float size = particle_buffer[input[0].vertexID].time/particle_buffer[input[0].vertexID].lifetime;
    size  = 0.1*smoothstep(0.0,1.0,size)*smoothstep(1.0,0.0,size);
    float squareSize = size;
    float4 vertices[4];
    vertices[0] = float4(-squareSize, -squareSize, 0.0, 1.0);
    vertices[1] = float4(-squareSize, squareSize, 0.0, 1.0);
    vertices[2] = float4(squareSize, -squareSize, 0.0, 1.0);
    vertices[3] = float4(squareSize, squareSize, 0.0, 1.0);

    float4x4 _m =  mul(g_projection_matrix, g_view_matrix);
    float3 right = mul(float4(1.0,0.0,0.0,0.0),g_view_matrix).xyz;
    float3 up = mul(float4(0.0,1.0,0.0,0.0),g_view_matrix).xyz;

    for (int i = 0; i < 4; ++i) {
        PS_INPUT output;
        float3 pos = vertices[i].x * right + vertices[i].y*up + particle_buffer[input[0].vertexID].position;
        vertices[i] = mul(_m, float4(pos,1.0));
        output.position = vertices[i];
        output.time = particle_buffer[input[0].vertexID].time/particle_buffer[input[0].vertexID].lifetime ; 
        OutputStream.Append(output);
    }

}


float4 ps_main(PS_INPUT input) : SV_TARGET
{   
    float3 col = lerp(float3(1.0,1.0,1.0),float3(0.0,0.0,0.0),input.time);
    return float4(col, 1.0f);
}
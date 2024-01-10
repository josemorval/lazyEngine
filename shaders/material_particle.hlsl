#include "./shaders/common.hlsl"

struct VS_INPUT {
    float3  position    :   POSITION;
    float2  texcoord    :   TEXCOORD;
    float3  normal      :   NORMAL;
    uint    instanceid  :   SV_InstanceID;
};

struct VS_OUTPUT {
    float4 position : SV_POSITION;
    float2 texcoord : TEXCOORD0;
    float3 normal : NORMAL;
};

VS_OUTPUT vs_main(VS_INPUT i) 
{
    VS_OUTPUT o;

    float3 pos = particle_buffer[i.instanceid].position;
    float angle = 3.1415 + sin(0.4*g_global_time);

    float3 right = float3(g_view_matrix[0][0],g_view_matrix[0][1],g_view_matrix[0][2]);
    float3 up = float3(g_view_matrix[1][0],g_view_matrix[1][1],g_view_matrix[1][2]);
    
    float3 pos_center = pos;
    pos += 2.0 * particle_buffer[i.instanceid].size * right * i.position.x;
    pos += 2.0 * particle_buffer[i.instanceid].size * up * i.position.y;
    
    o.position = mul(g_projection_matrix,mul(g_view_matrix,float4(pos,1.0)));
    o.texcoord = i.texcoord;
    o.normal = pos-pos;
    return o;
}

float4 ps_main(VS_OUTPUT i) : SV_TARGET
{
    float mask = step(abs(length(i.texcoord-0.5)-0.45),0.05);
    return float4(1.0,1.0,1.0,mask);
}
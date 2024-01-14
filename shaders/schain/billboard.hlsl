#define GPU_PARTICLE
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
    float3 world_position: TEXCOORD1;
};

VS_OUTPUT vs_main(VS_INPUT i) 
{
    VS_OUTPUT o;

    float3 right = float3(g_view_matrix[0][0],g_view_matrix[0][1],g_view_matrix[0][2]);
    float3 up = float3(g_view_matrix[1][0],g_view_matrix[1][1],g_view_matrix[1][2]);
    
    i.position *= 2.0*particle_buffer[ i.instanceid ].size;
    float3 local_pos = i.position.x * right + i.position.y * up;

    o.world_position = local_pos + particle_buffer[ i.instanceid ].position;
    o.position = mul(g_projection_matrix,mul(g_view_matrix,float4(o.world_position,1.0)));    
    o.texcoord = i.texcoord;
    return o;
}

float4 ps_main(VS_OUTPUT i) : SV_TARGET
{
    if(abs(length(i.texcoord.xy-0.5)-0.4)>0.05) discard;
    float3 color = float3(1.0,1.0,1.0);    
    return float4(color,1.0);
}
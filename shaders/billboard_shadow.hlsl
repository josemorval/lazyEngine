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

    float3 right = float3(g_view_matrix[0][0],g_view_matrix[0][1],g_view_matrix[0][2]);
    float3 up = float3(g_view_matrix[1][0],g_view_matrix[1][1],g_view_matrix[1][2]);
    
    float4 local_pos = float4(i.position.x * right + i.position.y * up,1.0);

    float4 world_pos = mul(translation_matrix[i.instanceid],mul(rotation_matrix[i.instanceid],mul(scale_matrix[i.instanceid],local_pos)));
    o.position = mul(g_light_projection_matrix,mul(g_light_view_matrix,world_pos));
    o.texcoord = i.texcoord;
    o.normal = i.normal;
    return o;
}

float4 ps_main(VS_OUTPUT i) : SV_TARGET
{
    if(length(i.texcoord.xy-0.5)>0.5) discard;
    return float4(0.0,0.0,0.0,0.0);
}
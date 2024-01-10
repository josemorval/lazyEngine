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

    float4 local_pos = float4(i.position,1.0);
    float4 world_pos = mul(translation_matrix[i.instanceid],mul(rotation_matrix[i.instanceid],mul(scale_matrix[i.instanceid],local_pos)));
    o.position = mul(g_light_projection_matrix,mul(g_light_view_matrix,world_pos));
    o.texcoord = i.texcoord;
    o.normal = i.normal;
    return o;
}

float4 ps_main(VS_OUTPUT i) : SV_TARGET
{
    return float4(0.0,0.0,0.0,0.0);
}
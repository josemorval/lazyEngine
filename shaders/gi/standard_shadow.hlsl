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
    float3 world_normal : TEXCOORD2;
    float3 local_position : TEXCOORD3;
};

VS_OUTPUT vs_main(VS_INPUT i) 
{
    VS_OUTPUT o;

    float4 local_pos = float4(i.position,1.0);
    o.world_position = mul(translation_matrix[i.instanceid],mul(rotation_matrix[i.instanceid],mul(scale_matrix[i.instanceid],float4(local_pos.xyz,1.0))));
    o.position = mul(g_light_projection_matrix,mul(g_light_view_matrix,float4(o.world_position,1.0)));
    
    o.world_normal = normalize(mul(rotation_matrix[i.instanceid],float4(i.normal,0.0)).xyz);
    o.texcoord = i.texcoord;
    o.local_position = mul(scale_matrix[i.instanceid],float4(i.position,1.0)).xyz;

    return o;
}

float4 ps_main(VS_OUTPUT i) : SV_TARGET
{
    return float4(0.0,0.0,0.0,0.0);
}
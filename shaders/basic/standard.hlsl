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
    o.position = mul(g_projection_matrix,mul(g_view_matrix,float4(o.world_position,1.0)));
    
    o.world_normal = mul(rotation_matrix[i.instanceid],float4(i.normal,0.0)).xyz;
    o.texcoord = i.texcoord;
    o.local_position = mul(scale_matrix[i.instanceid],float4(i.position,1.0)).xyz;

    return o;
}

float4 ps_main(VS_OUTPUT i) : SV_TARGET
{
    float3 light_dir = -mul(float4(0,0,1,0),g_light_inv_view_matrix).xyz;
    float light_mask = dot(light_dir,i.world_normal);

    float scale = 5.0;
    float3 lp = i.local_position*1.001;
    float mask = step(frac(scale*lp.x+0.5*(floor(2.0*scale*lp.y)%2.0)+0.5*(floor(2.0*scale*lp.z)%2.0)),0.5);

    float3 color = float3(1.0,1.0,1.0);
    color *= (0.6 + 0.2*mask) * light_mask;
    color *= float3(1.0,0.95+0.0001*i.position.x,0.9-0.0001*i.position.y);
    return float4(color,1.0);
}
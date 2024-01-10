#include "./shaders/common.hlsl"

struct VS_INPUT {
    float3  position    :   POSITION;
};

struct VS_OUTPUT {
    float3 world_position: TEXCOORD0;
    float4 position : SV_POSITION;
};

VS_OUTPUT vs_main(VS_INPUT i) 
{
    VS_OUTPUT o;

    o.world_position = 10.0*float3(i.position.x,0.0,i.position.y);

    o.position = mul(g_projection_matrix,
                    mul(g_view_matrix,
                        float4(o.world_position,1.0)
                    )
                );
 
    o.world_position = i.position.xyz;
    return o;
}

float4 ps_main(VS_OUTPUT i) : SV_TARGET
{
    float2 uv = i.world_position.xy;

    float mask = 0.0;
    mask = max(mask,step(0.97,sin(300.0*uv.x)));
    mask = max(mask,step(0.97,sin(300.0*uv.y)));
    mask *= smoothstep(0.3,0.0,length(i.world_position.xy));
    return float4(1.0,1.0,1.0,mask);
}
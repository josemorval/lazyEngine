#include "./shaders/common.hlsl"

struct VS_INPUT {
    float3  position    :   POSITION;
    float2  texcoord    :   TEXCOORD;
    float3  normal      :   NORMAL;
    uint    instanceid  :   SV_InstanceID;
};

Texture2D<float4> texture0 : register(t1);

struct VS_OUTPUT {
    float4 position : SV_POSITION;
    float2 texcoord : TEXCOORD0;
};

VS_OUTPUT vs_main(VS_INPUT i) 
{
    VS_OUTPUT o;

    o.position = float4(i.position.xy,0.0,0.5);
    o.texcoord = i.position.xy + 0.5;
    return o;
}

float4 ps_main(VS_OUTPUT i) : SV_TARGET
{
    float4 col = texture0.SampleLevel(point_wrap_sampler,float2(i.texcoord.x,1.-i.texcoord.y),0);
    return col;
}
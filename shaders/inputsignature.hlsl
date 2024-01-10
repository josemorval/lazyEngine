struct VS_INPUT {
    float3 position : POSITION;
    float2 texcoord : TEXCOORD;
    float3 normal : NORMAL;

};

float4 main(VS_INPUT input) : SV_POSITION
{
    return float4(0.0, 0.0, 0.0, 0.0);
}
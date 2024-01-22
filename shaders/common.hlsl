#define MAX_PARTICLE_ARRAY_SIZE 50

SamplerState linear_wrap_sampler
{
    Filter = MIN_MAG_MIP_LINEAR;
    AddressU = WRAP;
    AddressV = WRAP;
    AddressW = WRAP;
};

SamplerState point_wrap_sampler
{
    Filter = MIN_MAG_MIP_POINT;
    AddressU = WRAP;
    AddressV = WRAP;
    AddressW = WRAP;
};

SamplerState linear_clamp_sampler
{
    Filter = MIN_MAG_MIP_LINEAR;
    AddressU = CLAMP;
    AddressV = CLAMP;
    AddressW = CLAMP;
};

SamplerState point_clamp_sampler
{
    Filter = MIN_MAG_MIP_POINT;
    AddressU = CLAMP;
    AddressV = CLAMP;
    AddressW = CLAMP;
};

cbuffer constants : register(b0)
{
    float4x4 g_view_matrix;
    float4x4 g_inv_view_matrix;
    float4x4 g_projection_matrix;
    float4x4 g_light_view_matrix;
    float4x4 g_light_inv_view_matrix;
    float4x4 g_light_projection_matrix;
    float g_global_time;
    float g_delta_time;
};

cbuffer transform_constants : register(b1)
{
    float4x4 translation_matrix[64];
    float4x4 rotation_matrix[64];
    float4x4 scale_matrix[64];
};

cbuffer custom_data_constants : register(b2)
{
    float4 custom_data0;
    float4 custom_data1;
    float4 custom_data2;
}
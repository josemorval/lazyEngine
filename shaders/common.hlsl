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
    float4x4 g_projection_matrix;
    float4x4 g_light_view_matrix;
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
};

struct Particle
{
    float3 position;
    float3 velocity;
    float time;
    float lifetime;
    float size;
};


struct CellGrid
{
    uint particles_in_cell[MAX_PARTICLE_ARRAY_SIZE];
    uint particles_in_cell_max_count;
    uint particles_in_cell_current_count;
};


#ifdef GPU_PARTICLE
    StructuredBuffer<Particle> particle_buffer : register(t2);
    StructuredBuffer<CellGrid> cellgrid_buffer : register(t3);

    #ifdef COMPUTE_SHADER
    RWStructuredBuffer<Particle> rw_particle_buffer : register(u2);
    RWStructuredBuffer<CellGrid> rw_cellgrid_buffer : register(u3);
    #endif
#endif

#ifdef VOXELIZER
    Texture3D<float4> voxel_texture : register(t4);
    RWTexture3D<float4> rw_voxel_texture: register(u4);
#endif

#ifdef SHADOwMAP
    Texture2D<float4> light_shadowmap_texture : register(t1);
#endif

Texture2D main_texture0 : register(t10);
Texture2D main_texture1 : register(t11);
Texture2D main_texture2 : register(t12);
Texture2D main_texture3 : register(t13);
Texture2D main_texture4 : register(t14);


float3 compute_direct_lighting(float3 world_position, float3 world_normal, Texture2D t_texture, SamplerState s_texture)
{
    float4 position_light_space = mul(g_light_view_matrix, float4(world_position, 1.0));
    position_light_space = mul(g_light_projection_matrix, position_light_space);

    float depth = t_texture.Sample(s_texture, 0.5 - 0.5 * float2(-1.0, 1.0) * position_light_space.xy).r;
    float current_depth = position_light_space.z;

    float3 light_forward = -(mul(float4(0.0, 0.0, 1.0, 0.0), g_light_view_matrix)).xyz;

    float sha = 1.0 - smoothstep(-0.0, -0.01, depth - current_depth);
    float3 nor = normalize(world_normal);
    float dif = clamp(dot(nor, normalize(light_forward)), 0.0, 1.0);

    float3 col = float3(1.0,1.0,1.0);
    col *= sha * dif;

    return col;
}


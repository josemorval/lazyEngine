cbuffer voxel_constants : register(b3)
{
    float4 g_voxel_center_size;
    float4 g_voxel_dim;
};

    
RWTexture3D<float4> voxelize_scene_render : register(u1);

Texture2D<float4> light_shadowmap_texture : register(t0);
Texture3D<float4> voxelize_scene_texture : register(t1);


float compute_direct_lighting(float3 world_position, float3 world_normal, Texture2D t_texture, SamplerState s_texture)
{
    float4 position_light_space = mul(g_light_view_matrix, float4(world_position, 1.0));
    position_light_space = mul(g_light_projection_matrix, position_light_space);

    float depth = t_texture.Sample(s_texture, 0.5 - 0.5 * float2(-1.0, 1.0) * position_light_space.xy).r;
    
    float current_depth = position_light_space.z;

    float3 light_forward = -(mul(g_light_inv_view_matrix, float4(0.0, 0.0, 1.0, 0.0))).xyz;

    float sha = smoothstep(0.001, 0.0, current_depth-depth);
    float3 nor = normalize(world_normal);
    float dif = clamp(dot(nor, normalize(light_forward)), 0.0, 1.0);

    return sha * dif;
}

float4 sample_voxel(float3 pos, float diameter)
{
    float MipLevel = log2(max(diameter, g_voxel_center_size.w/g_voxel_dim.x) / (g_voxel_center_size.w/g_voxel_dim.x));
    float3 VoxelPos = (pos - g_voxel_center_size.xyz)/g_voxel_center_size.w + 0.5;
    return voxelize_scene_texture.SampleLevel(point_wrap_sampler, VoxelPos,min(MipLevel,4));
}

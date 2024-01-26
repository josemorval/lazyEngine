#include "./shaders/common.hlsl"
#include "./shaders/gi/gi_common.hlsl"
 

struct VS_INPUT {
    float3  position    :   POSITION;
    float2  texcoord    :   TEXCOORD;
    float3  normal      :   NORMAL;
    uint    instanceid  :   SV_InstanceID;
};

struct VS_OUTPUT {
    float4 position : SV_POSITION;
    float3 color : TEXCOORD;
};

VS_OUTPUT vs_main(VS_INPUT i) 
{
    VS_OUTPUT o;

    float4 local_pos = float4(i.position,1.0);


    uint3 voxel_index = uint3(0,0,0);
    {
        int i_voxel_dim = floor(g_voxel_dim.x);
        int x = i.instanceid % i_voxel_dim;
        int y = ((i.instanceid - x)/i_voxel_dim) % i_voxel_dim;
        int z = (i.instanceid - x - g_voxel_dim * y) / (i_voxel_dim * i_voxel_dim);
        voxel_index = uint3(x,y,z);
    }
    
    float3 world_position_offset = (voxel_index / g_voxel_dim.x - 0.5) * g_voxel_center_size.w + g_voxel_center_size.xyz;
    
    if( voxelize_scene_texture[ voxel_index ].a < 0.5 ) local_pos.xyz *= 0;

    float3 world_position = world_position_offset + local_pos.xyz * g_voxel_center_size.w / g_voxel_dim.x;
    o.position = mul(g_projection_matrix,mul(g_view_matrix,float4(world_position,1.0)));
    o.color = voxelize_scene_texture[ voxel_index ].rgb;
    return o;
}

float4 ps_main(VS_OUTPUT i) : SV_TARGET
{
    float3 color = i.color;
    return float4(color,1.0);
}
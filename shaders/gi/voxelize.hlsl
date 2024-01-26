#include "./shaders/common.hlsl"
#include "./shaders/gi/gi_common.hlsl"

struct VS_INPUT {
    float3  position      : POSITION;
    float2  texcoord      : TEXCOORD;
    float3  normal        : NORMAL;
    uint    instanceid    : SV_InstanceID;
};

struct VS_OUTPUT {
    float4 position       : POSITION;
    float2 texcoord       : TEXCOORD0;
    float3 normal         : NORMAL;
};

struct PS_INPUT {
    float4 position       : SV_POSITION;
    float3 world_position : TEXCOORD0;
    float3 world_normal   : TEXCOORD1;
    float2 texcoord       : TEXCOORD2;
};

VS_OUTPUT vs_main(VS_INPUT i) 
{
    VS_OUTPUT o;

    float4 local_pos = float4(i.position,1.0);
    o.position = mul(translation_matrix[i.instanceid],mul(rotation_matrix[i.instanceid],mul(scale_matrix[i.instanceid],float4(local_pos.xyz,1.0))));
    o.texcoord = i.texcoord;
    o.normal = normalize(mul(rotation_matrix[i.instanceid],float4(i.normal,0.0)).xyz);
    return o;
}

[maxvertexcount(3)]
void gs_main(triangle VS_OUTPUT input[3], inout TriangleStream<PS_INPUT> triStream)
{
    for (int i = 0; i < 3; i++)
    {
        PS_INPUT output;

        float angle = 2. * 3.1419 * float(i)/3.0;
        float2 posxy = 0.7*float2(cos(angle), sin(angle));

        output.position = float4(posxy,1.0,1.0);
        output.world_position = input[i].position;
        output.world_normal = input[i].normal;
        output.texcoord = input[i].texcoord;
        
        triStream.Append(output);
    }

    triStream.RestartStrip();
}

float4 ps_main(PS_INPUT i) : SV_TARGET
{  
    float li = compute_direct_lighting(i.world_position, i.world_normal, light_shadowmap_texture, point_wrap_sampler);
    float3 world_position_centered = i.world_position - g_voxel_center_size.xyz;
    world_position_centered = floor((world_position_centered/g_voxel_center_size.w + 0.5) * g_voxel_dim.x);

    if  (
        world_position_centered.x < g_voxel_dim.x && 
        world_position_centered.y < g_voxel_dim.x && 
        world_position_centered.z < g_voxel_dim.x
        )
    {
        voxelize_scene_render[uint3(world_position_centered.x, world_position_centered.y, world_position_centered.z)] = float4(li*float3(1.0,1.0,1.0), 1.0);
    }
    
    
    return float4(0.0,0.0,0.0,0.0);
}

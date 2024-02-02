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
    
    o.world_normal = normalize(mul(rotation_matrix[i.instanceid],float4(i.normal,0.0)).xyz);
    o.texcoord = i.texcoord;
    o.local_position = mul(scale_matrix[i.instanceid],float4(i.position,1.0)).xyz;

    return o;
}

float4 ps_main(VS_OUTPUT i) : SV_TARGET
{   
    float li = compute_direct_lighting(i.world_position, i.world_normal, light_shadowmap_texture, linear_wrap_sampler);
    //return float4(li*float3(1.0,1.0,1.0),1.0);

    float VoxelCellSize = (g_voxel_center_size.w/g_voxel_dim.x);
    float aperture = 0.1;
    float stepSize = 0.1;

    float3 light_forward = -(mul(g_light_inv_view_matrix, float4(0.0, 0.0, 1.0, 0.0))).xyz;
    float3 normal = i.world_normal;
    float3 pos = i.world_position;
    
    float3 col = float3(1.0,1.0,1.0);
    
    float t = 3.0 * VoxelCellSize;
    pos += 1.0 * normal * VoxelCellSize;
    float occlusion = 0.0;
    float3 lightDir = light_forward;
    float max_t = 10.0;
    float3 currPos = pos + t * lightDir;
    float diameter = 2.0 * t * tan(aperture/ 2.0);
    while(occlusion < 1.0 && t < max_t) {
        float s = sample_voxel(currPos, diameter).a;
        s = 1.0 - pow((1.0 - s), diameter / VoxelCellSize);
        occlusion += (1.0 - occlusion) * s;
        t += stepSize * diameter;
        diameter = 2 * t * tan(aperture/ 2.0);
        currPos = pos + t * lightDir;
    }
    
    float q = (1.0 - min(1.0, occlusion)) * max(dot(lightDir, normal), 0.0);
    return float4(q*float3(1.0,1.0,1.0),1.0);
}
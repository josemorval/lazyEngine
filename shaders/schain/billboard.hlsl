
#include "./shaders/common.hlsl"

struct Particle
{
    float3 position;
    float3 velocity;
    float time;
    float lifetime;
    float size;
};

StructuredBuffer<Particle> particle_buffer : register(t2);

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
};

VS_OUTPUT vs_main(VS_INPUT i) 
{
    VS_OUTPUT o;

    float3 right = float3(g_view_matrix[0][0],g_view_matrix[0][1],g_view_matrix[0][2]);
    float3 up = float3(g_view_matrix[1][0],g_view_matrix[1][1],g_view_matrix[1][2]);
    
    i.position *= 2.0*particle_buffer[ i.instanceid ].size;
    float3 local_pos = i.position.x * right + i.position.y * up;

    o.world_position = local_pos + particle_buffer[ i.instanceid ].position;
    o.position = mul(g_projection_matrix,mul(g_view_matrix,float4(o.world_position,1.0)));    
    o.texcoord = i.texcoord;
    return o;
}

struct PS_OUTPUT
{
    float4 color : SV_TARGET;
    float depth : SV_DEPTH;
};

PS_OUTPUT ps_main(VS_OUTPUT i)
{
    PS_OUTPUT output;
    
    if(length(i.texcoord.xy-0.5)>0.5) discard;

    float3 view_normal = float3(1.0*(i.texcoord.xy-0.5),0.0);
    view_normal.z = -1.0;
    view_normal = normalize(view_normal);

    float3 world_normal = mul(float4(view_normal,0.0),g_view_matrix).xyz;
    world_normal = normalize(world_normal);
    
    output.depth = i.position.z+0.001*view_normal.z;
    output.color = float4(0.5+0.5*world_normal,1.0);
    output.color *= 1.0*smoothstep(0.0,0.3,i.position.z/i.position.w);
    return output;
}
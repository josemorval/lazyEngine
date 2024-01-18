#define COMPUTE_SHADER
#define GPU_PARTICLE
#include "./shaders/common.hlsl"

Particle spawn_particle(int id, int len)
{
    float rand1 = Hash(id.x + 0.121);
    float rand2 = Hash(3.21*id.x + 6.321);
    float rand3 = Hash(1.519*id.x + 2.621);
    float rand4 = Hash(3.119*id.x + 5.113 + g_global_time);
    
    //Initialization
    Particle p;
    {
        p.position = float3(1.0,0.1,1.0)*float3(rand1-0.5,rand2-0.5,rand3-0.5);
        p.position += 1.0*float3(cos(2.0*g_global_time),0.0,sin(2.0*g_global_time));

        float fbm1 = FBM( 3.0*p.position + float3(0.2,0.1,0.412) + 0.1*g_global_time, 5);
        float fbm2 = FBM( 3.0*p.position+ float3(1.2,3.1,1.412) + 0.1*g_global_time, 5);
        float fbm3 = FBM( 3.0*p.position + 0.1*g_global_time, 5);
        
        p.velocity = 1.0*float3(fbm1,fbm2,fbm3);
        p.time = 0.0;
        p.lifetime = 0.5+0.5*rand4;
        p.size = 0.1;
    }

    return p;
}

[numthreads(64, 1, 1)]
void init(uint3 id : SV_DispatchThreadID) 
{
    uint dim, stride;
    rw_particle_buffer.GetDimensions(dim,stride);
    rw_particle_buffer[id.x] = spawn_particle(id.x, dim);
}

[numthreads(64, 1, 1)]
void update(uint3 id : SV_DispatchThreadID) 
{
    uint dim, stride;
    rw_particle_buffer.GetDimensions(dim,stride);

    float dt = g_delta_time;
    float t = g_global_time;

    Particle p;
    p = rw_particle_buffer[id.x];
    p.time += dt;

    if(p.time>p.lifetime)
    {
        p = spawn_particle(id.x,dim);
    }
    else
    {
        float3 force = float3(0.0,0.0,0.0);

        float3 prevpos = p.position - dt * p.velocity;
        p.position = 2.0*p.position - prevpos + dt*dt*force;
        prevpos
         = rw_particle_buffer[id.x].position;
        p.velocity = 0.99999*(p.position-prevpos)/dt;
    }

    rw_particle_buffer[id.x]= p;
}

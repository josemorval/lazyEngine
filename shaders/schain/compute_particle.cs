#include "./shaders/common.hlsl"
#include "./shaders/noise.hlsl"

struct Particle
{
    float3 position;
    float3 velocity;
    float time;
    float lifetime;
    float size;
};

RWStructuredBuffer<Particle> rw_particle_buffer : register(u2);

Particle spawn_particle(int id, int len)
{
    float rand1 = Hash(id.x + 0.121);
    float rand2 = Hash(3.21*id.x + 6.321);
    float rand3 = Hash(1.519*id.x + 2.621);
    float rand4 = Hash(3.119*id.x + 5.113 + g_global_time);
    
    //Initialization
    Particle p;
    {
        float k = 0.2;

        float t = float(id)/(len-1.0);
        t-=0.5;
        t*=k*(len-1);

        //t/=0.05*(len-1);

        p.position = float3(t,0.0,0.0);
        p.velocity = float3(0.0,0.0,0.0);
        p.time = 0.0;
        p.lifetime = 0.0;
        p.size = 0.5*k;
    }

    return p;

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

    if(id.x<dim)
    {
        float dt = 0.016*20/10;
        float t = 20.0*g_global_time;

        Particle p;
        p = rw_particle_buffer[id.x];
        float size = p.size;

        //Main parameters
        float freq = 0.01;
        float3 mbar = float3(cos(freq*t),sin(freq*t),0.0);
        float3 force = float3(0.0,0.0,0.0);

        float F0 = 1.63;
        float a = p.size;
        
        for(int j=0;j<dim;j++)
        {
            uint nindex = j;
            if(nindex>=0 && nindex<dim && j!=id.x)
            {
                float3 nposition = rw_particle_buffer[nindex].position;
                float3 eab = p.position-nposition;
                float rab = length(eab);

                float rab4 = rab/a; rab4*=rab4; rab4*=rab4;
                eab = normalize(eab);
                
                float dp_me = dot(mbar,eab);

                float3 _force = 2.0 * dp_me * mbar;
                _force -= (5.0 * dp_me * dp_me - 1.0) * eab;
                _force *= F0/rab4;

                force += _force;
            }
        }

        p.position = p.position + dt * p.velocity * (1.0-30.5*dt) + dt*dt*force;

        float3 delta = float3(0.0,0.0,0.0);
        float count = 0.0;
        for(int i=0;i<dim;i++)
        {
            uint nindex = i;
            if(nindex>=0 && nindex<dim && i!=id.x)
            {
                float3 nposition = rw_particle_buffer[nindex].position;
                float nsize = rw_particle_buffer[nindex].size;
                float3 eab = p.position-nposition;
                float rab = length(eab);

                if(rab<nsize+p.size)
                {
                    delta += 0.5*(p.size+nsize-rab)*normalize(eab);
                    count += 1.0;
                }
            }
        }


        p.position += delta/(count+1.0);
        p.velocity = (p.position-rw_particle_buffer[id.x].position)/dt;
        
        rw_particle_buffer[id.x]= p;
    }
}

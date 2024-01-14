#define COMPUTE_SHADER
#define GPU_PARTICLE
#include "./shaders/common.hlsl"

GridParticle spawn_particle(int id, int len)
{
    float rand1 = Hash(id.x + 0.121);
    float rand2 = Hash(3.21*id.x + 6.321);
    float rand3 = Hash(1.519*id.x + 2.621);
    float rand4 = Hash(13.119*id.x + 5.113);
    
    //Initialization
    GridParticle p;
    {
        p.position = 1.0*float3(rand1-0.5,0.0,rand3-0.5);
        p.position.y = 0.5;
        p.velocity = float3(0.0,0.0,0.0);
        p.time = -1.0*rand2;
        p.lifetime = 1.0;
        p.size = 0.2;
        p.cellindex = 0;
    }

    return p;
}


static uint N = 30;

bool check_inside_grid(uint3 grid_index)
{
    
    return grid_index.x>=0 && grid_index.x<N &&
           grid_index.y>=0 && grid_index.y<N &&
           grid_index.z>=0 && grid_index.z<N; 
        
}

uint3 get_grid_index_from_linear_index(uint linear_index)
{
    uint x = linear_index%N;
    uint y = (linear_index-x)/N % N;
    uint z = (linear_index-x-N*y)/(N*N);
    return uint3(x,y,z);
}

uint get_linear_index_from_grid_index(uint3 grid_index)
{
    return grid_index.x + N * grid_index.y + N * N * grid_index.z;
}

uint3 get_grid_index_from_position(float3 pos)
{
    float size_box = 10.0;
    pos/=size_box;
    pos += 0.5;
    return uint3(floor(pos*N));
}

[numthreads(64, 1, 1)]
void init_particles(uint3 id : SV_DispatchThreadID) 
{
    uint dim, stride;
    rw_gridparticle_buffer.GetDimensions(dim,stride);

    if(id.x<dim)
    {
        rw_gridparticle_buffer[id.x] = spawn_particle(id.x,dim);
    }
}

[numthreads(64, 1, 1)]
void init_grid(uint3 id : SV_DispatchThreadID) 
{
    uint dim, stride;
    rw_cellgrid_buffer.GetDimensions(dim,stride);

    if(id.x<dim)
    {
        CellGrid cg = rw_cellgrid_buffer[id.x];
        cg.particles_in_cell_current_count = 0;
        rw_cellgrid_buffer[id.x] = cg;
    }

}

[numthreads(64, 1, 1)]
void populate_grid(uint3 id : SV_DispatchThreadID) 
{
    uint dim, stride;
    rw_gridparticle_buffer.GetDimensions(dim,stride);

    if(id.x<dim)
    {
        uint3 index = get_grid_index_from_position(rw_gridparticle_buffer[id.x].position);
        rw_gridparticle_buffer[id.x].cellindex =0;     

        if(
            check_inside_grid(index)
        )
        {
            uint linearindex = get_linear_index_from_grid_index(index);
            
            uint count;
            InterlockedAdd(rw_cellgrid_buffer[linearindex].particles_in_cell_current_count,1,count);
            rw_cellgrid_buffer[linearindex].particles_in_cell[count] = id.x;
            rw_gridparticle_buffer[id.x].cellindex = linearindex;
        }
        else
        {
            rw_gridparticle_buffer[id.x].cellindex = 1000000000; 
        }
    }

}

[numthreads(64, 1, 1)]
void update_particles(uint3 id : SV_DispatchThreadID) 
{
    
    uint dim, stride;
    rw_gridparticle_buffer.GetDimensions(dim,stride);

    if(id.x<dim)
    {

        float dt = 0.05*g_delta_time;
        float t = g_global_time;

        GridParticle p;
        p = rw_gridparticle_buffer[id.x];

        if(p.time>=0.0)
        {
            float3 force = float3(0.0,-1000.0,0.0);
            p.position = p.position + p.velocity * dt + force * dt * dt; 

            static float L=4.0;
            if(p.position.x<-L) p.position.x=-L;
            if(p.position.x>L) p.position.x=L;
            if(p.position.y<-L) p.position.y=-L;
            if(p.position.y>L) p.position.y=L;    
            if(p.position.z<-L) p.position.z=-L;
            if(p.position.z>L) p.position.z=L; 
            
            uint offset[27] = {
                -1-N-N*N,0-N-N*N,1-N-N*N,
                -1-N*N,0-N*N,1-N*N,
                -1+N-N*N,0+N-N*N,1+N-N*N,

                -1-N,0-N,1-N,
                -1,0,1,
                -1+N,0+N,1+N,

                -1-N+N*N,0-N+N*N,1-N+N*N,
                -1+N*N,0+N*N,1+N*N,
                -1+N+N*N,0+N+N*N,1+N+N*N   
            };


            uint linearindex = rw_gridparticle_buffer[id.x].cellindex;
            float3 delta = float3(0.0,0.0,0.0);
            float smoothie = 0.0;
            
            for(int i=0;i<27;i++)
            {
                uint index_offsetted = linearindex+offset[i];
                uint3 index_offsetted_3d = get_grid_index_from_linear_index(index_offsetted);

                if(check_inside_grid(index_offsetted_3d))
                {
                    uint count = cellgrid_buffer[index_offsetted].particles_in_cell_current_count;
                    for(int j=0;j<count;j++)
                    {
                        uint nindex = cellgrid_buffer[index_offsetted].particles_in_cell[j];
                        if(nindex>=0 && nindex!=id.x)
                        {
                            if(rw_gridparticle_buffer[nindex].time>=0.0)
                            {

                                float3 nposition = rw_gridparticle_buffer[nindex].position;
                                float nsize = rw_gridparticle_buffer[nindex].size;
                                float3 eab = p.position-nposition;
                                float rab = length(eab);

                                if(rab<p.size+nsize)
                                {
                                    delta += 0.5*(p.size+nsize-rab)*normalize(eab);
                                }

                                smoothie+=1.0;
                            }
                        }
                    }
                }
            }

            p.position += delta/(smoothie+1.0);
            p.velocity = (p.position-rw_gridparticle_buffer[id.x].position)/dt;

            if(length(delta/(smoothie+1.0))>0.01)
            {
                p.velocity *=0.0;
            }

        }

        {
            p.time+=dt;
            //p.size -= 0.05*dt; p.size = clamp(p.size,0.2,1.0);
            if(p.time>p.lifetime)
            {   
                
                //p = spawn_particle(id.x,dim);
            }
        }

        rw_gridparticle_buffer[id.x]= p;


    }
}

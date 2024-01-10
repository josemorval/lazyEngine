#define COMPUTE_SHADER
#define GPU_PARTICLE
#include "./shaders/common.hlsl"

////////////////////////////////////////////////////////////////////////////////////////
// Fast analytical Perlin noise
float	Hash( float n )
{
	return frac( sin(n) * 43758.5453 );
}

float	FastNoise( float3 x )
{
	float3	p = floor(x);
	float3	f = frac(x);

	f = smoothstep( 0.0, 1.0, f );

	float	n = p.x + 57.0 * p.y + 113.0 * p.z;

	return lerp(	lerp(	lerp( Hash( n +   0.0 ), Hash( n +   1.0 ), f.x ),
							lerp( Hash( n +  57.0 ), Hash( n +  58.0 ), f.x ), f.y ),
					lerp(	lerp( Hash( n + 113.0 ), Hash( n + 114.0 ), f.x ),
							lerp( Hash( n + 170.0 ), Hash( n + 171.0 ), f.x ), f.y ), f.z );
}

// Fast analytical noise for screen-space perturbation
float	FastScreenNoise( float2 _XY )
{
	return 	Hash(  13.5798490 * _XY.x - 23.60165409 * _XY.y )
		  * Hash( -17.3468489 * _XY.y + 27.31765563 * _XY.x );
}

static const float3x3	DEFAULT_OCTAVE_TRANSFORM = float3x3(   0.00,  0.80,  0.60,
															  -0.80,  0.36, -0.48,
															  -0.60, -0.48,  0.64 );


float	FBM( float3 _Position, uint _OctavesCount, float3x3 _Rotation = DEFAULT_OCTAVE_TRANSFORM )
{
	float2	N = 0.0;
	float	A = 1.0;
	for ( uint OctaveIndex=0; OctaveIndex < _OctavesCount; OctaveIndex++ )
	{
		N += A * float2( 2.0 * FastNoise( _Position ) - 1.0, 1.0 );
		A *= 0.5;
		_Position = mul( _Rotation, _Position );
	}

	return N.x / N.y;
}

float	Turbulence( float3 _Position, uint _OctavesCount, float3x3 _Rotation = DEFAULT_OCTAVE_TRANSFORM )
{
	float2	N = 0.0;
	float	A = 1.0;
	for ( uint OctaveIndex=0; OctaveIndex < _OctavesCount; OctaveIndex++ )
	{
		N += A * float2( abs( 2.0 * FastNoise( _Position ) - 1.0 ), 1.0 );
		A *= 0.5;
		_Position = mul( _Rotation, _Position );
	}

	return N.x / N.y;
}


////////////////////////////////////////////////////////////////////////////////////////
// Simple cellular noise with a single point per grid cell

// Generate a location within a grid cell given the integer cell index
float3	GenerateRandomLocation( float3 _GridCellIndex )
{
	return _GridCellIndex
		 + float3(	Hash( 0.894205 + 17.219 * _GridCellIndex.x - 19.1965 * _GridCellIndex.y + 7.0689 * _GridCellIndex.z ),
					Hash( 0.136515 - 23.198 * _GridCellIndex.x + 7.95145 * _GridCellIndex.y + 12.123 * _GridCellIndex.z ),
					Hash( 0.654318 + 19.161 * _GridCellIndex.y - 15.6317 * _GridCellIndex.y - 51.561 * _GridCellIndex.z )
				 );
}

float	Cellular( float3 _Position, float3 _InvGridCellSize )
{
	_Position = _InvGridCellSize * (_Position + 137.56);

	float3	nPosition = floor( _Position );

	float	ClosestDistance = 100000.0;
	for ( int z=-1; z <= 1; z++ )
		for ( int y=-1; y <= 1; y++ )
			for ( int x=-1; x <= 1; x++ )
			{
				float3	nCellIndex = nPosition + float3( x, y, z );
				float3	Location = GenerateRandomLocation( nCellIndex );	// Interest point within cell
				float3	Delta = _Position - Location;
				float	SqDistance = dot( Delta, Delta );
				ClosestDistance = lerp( ClosestDistance, SqDistance, step( SqDistance, ClosestDistance ) );
			}

	return sqrt( ClosestDistance );
}

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

[numthreads(1, 1, 1)]
void populate_grid(uint3 id : SV_DispatchThreadID)
{
    //CellGrid cg = rw_cellgrid_buffer[id.x];
    //cg.particles_in_cell_max_count = 20;
    //cg.particles_in_cell_current_count = 0;
    //rw_cellgrid_buffer[id.x] = cg;
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
        prevpos = rw_particle_buffer[id.x].position;
        p.velocity = 0.99999*(p.position-prevpos)/dt;
    }

    rw_particle_buffer[id.x]= p;
}

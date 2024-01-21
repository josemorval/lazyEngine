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
    float4x4 g_inv_view_matrix;
    float4x4 g_projection_matrix;
    float4x4 g_light_view_matrix;
    float4x4 g_light_inv_view_matrix;
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

struct GridParticle
{
    float3 position;
    float3 velocity;
    float time;
    float lifetime;
    float size;
    uint cellindex;
};


struct CellGrid
{
    uint particles_in_cell[MAX_PARTICLE_ARRAY_SIZE];
    uint particles_in_cell_max_count;
    uint particles_in_cell_current_count;
};


#ifdef GPU_PARTICLE
    StructuredBuffer<Particle> particle_buffer : register(t2);
    StructuredBuffer<GridParticle> gridparticle_buffer : register(t3);
    StructuredBuffer<CellGrid> cellgrid_buffer : register(t4);

    #ifdef COMPUTE_SHADER
    RWStructuredBuffer<Particle> rw_particle_buffer : register(u2);
    RWStructuredBuffer<GridParticle> rw_gridparticle_buffer : register(u3);
    RWStructuredBuffer<CellGrid> rw_cellgrid_buffer : register(u4);
    #endif
#endif

#ifdef VOXELIZER
    Texture3D<float4> voxel_texture : register(t5);
    RWTexture3D<float4> rw_voxel_texture: register(u5);
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


///////////////////////////////////
///NOISE LIBRARY///////////////////
///////////////////////////////////

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
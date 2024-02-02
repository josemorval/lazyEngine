#include "./shaders/common.hlsl"

RWByteAddressBuffer indirect_args : register(u0);

[numthreads(1, 1, 1)]
void fillindirect() 
{
    indirect_args.Store2(0,uint2(2000*(0.5+0.5*sin(g_global_time)),1));
    
}


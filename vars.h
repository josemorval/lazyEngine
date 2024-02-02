#pragma once
#ifndef _VARS_H_
#define _VARS_H_
#endif

#include "lib3d.h"
#include "libmath.h"

float BACKGROUND_COL[4] = { 0.1,0.1,0.1,1.0 };

struct GlobalConstantsBuffer {
	float4x4 view_matrix;
	float4x4 inv_view_matrix;
	float4x4 projection_matrix;
	float4x4 light_view_matrix;
	float4x4 light_inv_view_matrix;
	float4x4 light_projection_matrix;
	float global_time;
	float delta_time;
};

struct TransformBuffer {
	float4x4 translation_matrix[64];
	float4x4 rotation_matrix[64];
	float4x4 scale_matrix[64];
};

struct CustomDataBuffer {
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
	int cellindex;
};

GlobalConstantsBuffer* gcb = nullptr;
TransformBuffer* tb = nullptr;
CustomDataBuffer* cdb = nullptr;

lib3d::Viewport* viewport = nullptr;
lib3d::Rasterizer* rasterizer = nullptr;
lib3d::Rasterizer* rasterizer_nocull = nullptr;
lib3d::Rasterizer* rasterizer_wireframe = nullptr;
lib3d::AlphaBlending* alpha_blending;
lib3d::DepthStencil* nowrite_depthstencil;
lib3d::DepthStencil* write_depthstencil;
lib3d::RenderTarget2D* rendertarget_main;
lib3d::Mesh* mesh_quad;
lib3d::Mesh* mesh_cube;
lib3d::Mesh* mesh_sphere;
lib3d::ConstantBuffer* constantbuffer_main;
lib3d::ConstantBuffer* transformbuffer_main;
lib3d::ConstantBuffer* customdatabuffer_main;
lib3d::SamplerState* linearwrap;
lib3d::SamplerState* pointwrap;
lib3d::SamplerState* linearclamp;
lib3d::SamplerState* pointclamp;


#ifdef _DEBUG
lib3d::GPUTimer* gpu_timer;
#endif

void update_globalconstants()
{	

#ifdef _DEBUG
	gcb->delta_time = lib3d::global_time_use / 1000.0;
#else
	gcb->delta_time = 0.016;
#endif// DEBUG

	gcb->global_time += gcb->delta_time;
	
	constantbuffer_main->map();
		GlobalConstantsBuffer* _gcb = (GlobalConstantsBuffer*)constantbuffer_main->get_data();
		_gcb->view_matrix = gcb->view_matrix;
		_gcb->inv_view_matrix = gcb->inv_view_matrix;
		_gcb->projection_matrix = gcb->projection_matrix;
		_gcb->light_view_matrix = gcb->light_view_matrix;
		_gcb->light_inv_view_matrix = gcb->light_inv_view_matrix;
		_gcb->light_projection_matrix = gcb->light_projection_matrix;
		_gcb->global_time = gcb->global_time;
		_gcb->delta_time = gcb->delta_time;
	constantbuffer_main->unmap();
}
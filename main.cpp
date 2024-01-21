#include "libmem.h"
#include "lib3d.h"
#include "vars.h"

#ifdef _DEBUG
#include "libimgui.h"
#include "libmath.h"
#include "libmidi.h"
#include "liblua.h"
#endif // _DEBUG

extern "C" int _fltused = 0;

#ifdef _DEBUG
extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);	
#endif

#ifdef _DEBUG
int WinMain(HINSTANCE handle_instance, HINSTANCE handle_previnstance, LPSTR lpsz_argument, int nfunster_stil)
#else
void WinMainCRTStartup()
#endif
{
#ifdef _DEBUG
	lib3d::setup_device(handle_instance);
#else
	lib3d::setup_device(nullptr);
#endif// DEBUG
	
#ifdef _DEBUG
	libmidi::setup_device();
	liblua::initialize_lua();
#endif // _DEBUG

	//Create a viewport
	allocation(viewport, lib3d::Viewport, lib3d::WIDTH, lib3d::HEIGHT);

	//Create several rasterizers (one of them for wireframe)
	allocation(rasterizer, lib3d::Rasterizer);
	allocation(rasterizer_nocull, lib3d::Rasterizer, D3D11_FILL_SOLID, D3D11_CULL_NONE);
	allocation(rasterizer_wireframe, lib3d::Rasterizer, D3D11_FILL_WIREFRAME, D3D11_CULL_NONE);

	//Create a alpha blending mode 
	allocation(alpha_blending, lib3d::AlphaBlending);

	//Several depth stencil setups (write or not write to depth buffer)
	allocation(nowrite_depthstencil, lib3d::DepthStencil, true, D3D11_DEPTH_WRITE_MASK_ZERO, D3D11_COMPARISON_LESS_EQUAL);
	allocation(write_depthstencil, lib3d::DepthStencil, true, D3D11_DEPTH_WRITE_MASK_ALL, D3D11_COMPARISON_LESS_EQUAL);

	//Here we take the reference for the backbuffer (to draw the things) and create a depth map
	allocation(rendertarget_main, lib3d::RenderTarget2D);
	rendertarget_main->get_backbuffer();

	//A simple quad mesh
	allocation(mesh_quad, lib3d::Mesh, lib3d::quad_vertices, lib3d::quad_indices, sizeof(lib3d::quad_vertices), sizeof(lib3d::quad_indices));
	allocation(mesh_cube, lib3d::Mesh, lib3d::cube_vertices, lib3d::cube_indices, sizeof(lib3d::cube_vertices), sizeof(lib3d::cube_indices));

#ifdef  _DEBUG
	allocation(mesh_sphere, lib3d::Mesh, lib3d::sphere_vertices, lib3d::sphere_indices, sizeof(lib3d::sphere_vertices), sizeof(lib3d::sphere_indices));
#endif //  _DEBUG

#ifdef _DEBUG
	//allocation(gpu_timer, lib3d::GPUTimer);
#endif // DEBUG
	//Constant buffer for store view and projection matrices and other thins
	allocation(constantbuffer_main, lib3d::ConstantBuffer, sizeof(GlobalConstantsBuffer));

	GlobalConstantsBuffer _gcb; gcb = &_gcb;
	gcb->projection_matrix = compute_perspective_matrix(1.0, 1.0 * lib3d::WIDTH / lib3d::HEIGHT, 0.1f, 10.0f);
	gcb->light_projection_matrix = compute_orthographic_matrix(5.0 * lib3d::WIDTH / lib3d::HEIGHT, 5.0, 0.1f, 10.0f);

#ifdef _DEBUG
	allocation(transformbuffer_main, lib3d::ConstantBuffer, sizeof(TransformBuffer));
	allocation(customdatabuffer_main, lib3d::ConstantBuffer, sizeof(CustomDataBuffer));
	TransformBuffer _tb; tb = &_tb;
	for (int i = 0; i < 64; i++)
	{
		tb->translation_matrix[i] = compute_translation_matrix(0.0f, 0.0f, 0.0f);
		tb->rotation_matrix[i] = compute_rotation_matrix(0.0f, float3(0.0f, 1.0f, 0.0f));
		tb->scale_matrix[i] = compute_scale_matrix(0.0f, 0.0f, 0.0f);
	}
	transformbuffer_main->map();
		TransformBuffer* _tbp = (TransformBuffer*)transformbuffer_main->get_data();
		for (int i = 0; i < 64; i++)
		{
			_tbp->translation_matrix[i] = tb->translation_matrix[i];
			_tbp->rotation_matrix[i] = tb->rotation_matrix[i];
			_tbp->scale_matrix[i] = tb->scale_matrix[i];
		}
	transformbuffer_main->unmap();

	CustomDataBuffer _cdb = { float4(0.0,0.0,0.0,0.0),float4(0.0,0.0,0.0,0.0), float4(0.0,0.0,0.0,0.0) }; cdb = &_cdb;
	customdatabuffer_main->map();
		CustomDataBuffer* _cdbp = (CustomDataBuffer*)customdatabuffer_main->get_data();
		_cdbp->custom_data0 = cdb->custom_data0;
		_cdbp->custom_data1 = cdb->custom_data1;
		_cdbp->custom_data2 = cdb->custom_data2;
	customdatabuffer_main->unmap();
#endif

	//Set global constant buffer
	constantbuffer_main->attach(0);

#ifdef _DEBUG
	transformbuffer_main->attach(1);
	customdatabuffer_main->attach(2);

	//Initialize imgui
	libimgui::initialize();

	bool show_lua_errors_window = false;
	bool show_shader_errors_window = false;
	bool show_lua_reference_window = false;

	liblua::init();
#endif // _DEBUG

	do
	{		
#ifdef _DEBUG
		lib3d::start_global_timer();
#endif
		MSG msg;
		while (PeekMessageA(&msg, nullptr, 0, 0, PM_REMOVE))
		{

#ifdef _DEBUG

			if (ImGui_ImplWin32_WndProcHandler(lib3d::window, msg.message, msg.wParam, msg.lParam)) {
				continue;
			}

#endif // _DEBUG
			// Control focus app behaviour
			if (lib3d::window == GetForegroundWindow() && !lib3d::is_focused) lib3d::is_focused = true;//lib3d::swapchain->SetFullscreenState(true, nullptr);
			if (lib3d::window != GetForegroundWindow() && lib3d::is_focused) lib3d::is_focused = false;

			if (lib3d::is_focused)
			{	
				if (GetAsyncKeyState(VK_ESCAPE) & 0x8000) 
				{
					ExitProcess(0);
				}
			}

			TranslateMessage(&msg);
			DispatchMessageA(&msg);
		}
#ifdef _DEBUG
		liblua::check_lua_changes();

		bool shader_failed = false;
		bool lua_failed = false;

		//imgui logic
		{
			ImGui_ImplDX11_NewFrame();
			ImGui_ImplWin32_NewFrame();
			ImGui::NewFrame();

			//Lua based imgui
			liblua::imgui();

			if (ImGui::BeginMainMenuBar())
			{
				if (ImGui::Button("Load")) 
				{
					liblua::lua_filename_level = libimgui::open_file_dialog();
					liblua::load_new_level();
				}

				if (ImGui::BeginMenu("View")) 
				{
					ImGui::MenuItem("Shader errors", nullptr, &show_shader_errors_window);
					ImGui::MenuItem("Lua errors", nullptr, &show_lua_errors_window);
					ImGui::MenuItem("Lua reference", nullptr, &show_lua_reference_window);
				ImGui::EndMenu();
				}
				ImGui::SameLine(lib3d::WIDTH/2.0 - ImGui::CalcTextSize("____________________").x/2.0); // Ajusta la posición X según tus necesidades
				ImGui::Text("[ %.2f ms ] %d x %d",lib3d::global_time_use, lib3d::WIDTH, lib3d::HEIGHT);

				ImGui::EndMainMenuBar();
			}

			//Lua errors
			if (show_lua_errors_window)
			{
				ImGui::Begin("Lua errors");
				ImGui::TextWrapped(liblua::lua_error_log.c_str());
				ImGui::TextWrapped(liblua::lua_softerrors_log.c_str());
				ImGui::End();
			}

			//Shader errors
			if (show_shader_errors_window)
			{

				ImGui::Begin("Shader errors");
				for (int i = 0; i < MAX_LIST_SIZE; i++)
				{
					if (lib3d::fx_list[i] != nullptr)
					{
						ImGui::TextWrapped(lib3d::fx_list[i]->error_vertex_shader.c_str());
						ImGui::TextWrapped(lib3d::fx_list[i]->error_pixel_shader.c_str());
						ImGui::TextWrapped(lib3d::fx_list[i]->error_geometry_shader.c_str());
					}

					if (lib3d::computefx_list[i] != nullptr)
					{
						ImGui::TextWrapped(lib3d::computefx_list[i]->error_compute_shader.c_str());
					}
				}
				ImGui::End();
			}

			if (show_lua_reference_window)
			{
				ImGui::Begin("Lua reference");
				ImGui::TextWrapped(liblua::list_lua_bindings.c_str());
				ImGui::End();
			}


			if (liblua::lua_softerrors_log != "" || liblua::lua_error_log != "")
			{
				lua_failed = true;
				libimgui::draw_lua_error_square();
			}
			
			{
				for (int i = 0; i < MAX_LIST_SIZE; i++)
				{
					if (lib3d::fx_list[i] != nullptr)
					{
						if (lib3d::fx_list[i]->error_vertex_shader != "")
						{
							shader_failed = true;
							break;
						}
						if (lib3d::fx_list[i]->error_pixel_shader != "")
						{
							shader_failed = true;
							break;
						}
						if (lib3d::fx_list[i]->error_geometry_shader != "")
						{
							shader_failed = true;
							break;
						}
					}

					if (lib3d::computefx_list[i] != nullptr)
					{
						if (lib3d::computefx_list[i]->error_compute_shader != "")
						{
							shader_failed = true;
							break;
						}
					}
				}
				if (shader_failed)
				{
					libimgui::draw_shader_error_square();
				}
			}

			ImGui::Render();

		}

#endif // _DEBUG

		update_globalconstants();
		rendertarget_main->clear_rendertarget(BACKGROUND_COL);

#ifdef _DEBUG
		//Lua based render
		liblua::render();

		if (lua_failed) rendertarget_main->clear_rendertarget(BACKGROUND_COL);
#endif

		rendertarget_main->set_rendertarget();

#ifdef _DEBUG
		libimgui::end_frame();
#endif // _DEBUG

		lib3d::swapchain->Present(1, 0);
		
#ifdef _DEBUG
		//Lua cleanup (clean resources etc)
		liblua::cleanup();
		lib3d::stop_global_timer();
#endif // _DEBUG

	} 
	while (true);

	ExitProcess(0);  // actually not needed in this setup, but may be smaller..
}

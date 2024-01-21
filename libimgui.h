#pragma once
#ifndef _LIBIMGUI_H_
#define _LIBIMGUI_H_
#endif

#include <windows.h>
#include <commdlg.h>
#include "lib3d.h"

#include "imgui.h"
#include "imgui_impl_win32.h"
#include "imgui_impl_dx11.h"
#include "lib3d.h"

namespace libimgui
{ 

    void initialize()
    {
		ImGui::CreateContext();
		ImGuiIO& io = ImGui::GetIO();
		io.Fonts->AddFontFromFileTTF("./fonts/Roboto-Medium.ttf", 10.0);
		io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;

		io.MouseDrawCursor = true;
		io.FontGlobalScale = 2.0;

		//imgui style definition
		{
			auto& colors = ImGui::GetStyle().Colors;
			colors[ImGuiCol_WindowBg] = ImVec4{ 0.1f, 0.1f, 0.13f, 1.0f };
			colors[ImGuiCol_MenuBarBg] = ImVec4{ 0.16f, 0.16f, 0.21f, 1.0f };

			// Border
			colors[ImGuiCol_Border] = ImVec4{ 0.44f, 0.37f, 0.61f, 0.29f };
			colors[ImGuiCol_BorderShadow] = ImVec4{ 0.0f, 0.0f, 0.0f, 0.24f };

			// Text
			colors[ImGuiCol_Text] = ImVec4{ 1.0f, 1.0f, 1.0f, 1.0f };
			colors[ImGuiCol_TextDisabled] = ImVec4{ 0.5f, 0.5f, 0.5f, 1.0f };

			// Headers
			colors[ImGuiCol_Header] = ImVec4{ 0.13f, 0.13f, 0.17, 1.0f };
			colors[ImGuiCol_HeaderHovered] = ImVec4{ 0.19f, 0.2f, 0.25f, 1.0f };
			colors[ImGuiCol_HeaderActive] = ImVec4{ 0.16f, 0.16f, 0.21f, 1.0f };

			// Buttons
			colors[ImGuiCol_Button] = ImVec4{ 0.13f, 0.13f, 0.17, 1.0f };
			colors[ImGuiCol_ButtonHovered] = ImVec4{ 0.19f, 0.2f, 0.25f, 1.0f };
			colors[ImGuiCol_ButtonActive] = ImVec4{ 0.16f, 0.16f, 0.21f, 1.0f };
			colors[ImGuiCol_CheckMark] = ImVec4{ 0.74f, 0.58f, 0.98f, 1.0f };

			// Popups
			colors[ImGuiCol_PopupBg] = ImVec4{ 0.1f, 0.1f, 0.13f, 0.92f };

			// Slider
			colors[ImGuiCol_SliderGrab] = ImVec4{ 0.44f, 0.37f, 0.61f, 0.54f };
			colors[ImGuiCol_SliderGrabActive] = ImVec4{ 0.74f, 0.58f, 0.98f, 0.54f };

			// Frame BG
			colors[ImGuiCol_FrameBg] = ImVec4{ 0.13f, 0.13, 0.17, 1.0f };
			colors[ImGuiCol_FrameBgHovered] = ImVec4{ 0.19f, 0.2f, 0.25f, 1.0f };
			colors[ImGuiCol_FrameBgActive] = ImVec4{ 0.16f, 0.16f, 0.21f, 1.0f };

			// Tabs
			colors[ImGuiCol_Tab] = ImVec4{ 0.16f, 0.16f, 0.21f, 1.0f };
			colors[ImGuiCol_TabHovered] = ImVec4{ 0.24, 0.24f, 0.32f, 1.0f };
			colors[ImGuiCol_TabActive] = ImVec4{ 0.2f, 0.22f, 0.27f, 1.0f };
			colors[ImGuiCol_TabUnfocused] = ImVec4{ 0.16f, 0.16f, 0.21f, 1.0f };
			colors[ImGuiCol_TabUnfocusedActive] = ImVec4{ 0.16f, 0.16f, 0.21f, 1.0f };

			// Title
			colors[ImGuiCol_TitleBg] = ImVec4{ 0.16f, 0.16f, 0.21f, 1.0f };
			colors[ImGuiCol_TitleBgActive] = ImVec4{ 0.16f, 0.16f, 0.21f, 1.0f };
			colors[ImGuiCol_TitleBgCollapsed] = ImVec4{ 0.16f, 0.16f, 0.21f, 1.0f };

			// Scrollbar
			colors[ImGuiCol_ScrollbarBg] = ImVec4{ 0.1f, 0.1f, 0.13f, 1.0f };
			colors[ImGuiCol_ScrollbarGrab] = ImVec4{ 0.16f, 0.16f, 0.21f, 1.0f };
			colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4{ 0.19f, 0.2f, 0.25f, 1.0f };
			colors[ImGuiCol_ScrollbarGrabActive] = ImVec4{ 0.24f, 0.24f, 0.32f, 1.0f };

			// Seperator
			colors[ImGuiCol_Separator] = ImVec4{ 0.44f, 0.37f, 0.61f, 1.0f };
			colors[ImGuiCol_SeparatorHovered] = ImVec4{ 0.74f, 0.58f, 0.98f, 1.0f };
			colors[ImGuiCol_SeparatorActive] = ImVec4{ 0.84f, 0.58f, 1.0f, 1.0f };

			// Resize Grip
			colors[ImGuiCol_ResizeGrip] = ImVec4{ 0.44f, 0.37f, 0.61f, 0.29f };
			colors[ImGuiCol_ResizeGripHovered] = ImVec4{ 0.74f, 0.58f, 0.98f, 0.29f };
			colors[ImGuiCol_ResizeGripActive] = ImVec4{ 0.84f, 0.58f, 1.0f, 0.29f };

			// Docking
			colors[ImGuiCol_DockingPreview] = ImVec4{ 0.44f, 0.37f, 0.61f, 1.0f };

			auto& style = ImGui::GetStyle();
			style.TabRounding = 4;
			style.ScrollbarRounding = 9;
			style.WindowRounding = 7;
			style.GrabRounding = 3;
			style.FrameRounding = 3;
			style.PopupRounding = 4;
			style.ChildRounding = 4;
		}

		ImGui_ImplDX11_Init(lib3d::device, lib3d::inmediate);
		ImGui_ImplWin32_Init(lib3d::window);
    }

	void end_frame()
	{
		ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
	}

    std::string open_file_dialog() 
    {
        ShowCursor(true);

        OPENFILENAME ofn;
        char szFile[260] = { 0 };

        ZeroMemory(&ofn, sizeof(ofn));
        ofn.lStructSize = sizeof(ofn);
        ofn.hwndOwner = NULL;
        ofn.lpstrFile = szFile;
        ofn.nMaxFile = sizeof(szFile);
        ofn.lpstrFilter = "All Files\0*.*\0";
        ofn.nFilterIndex = 1;
        ofn.lpstrFileTitle = NULL;
        ofn.nMaxFileTitle = 0;
        ofn.lpstrInitialDir = NULL;
        ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_NOCHANGEDIR;

        if (GetOpenFileName(&ofn) == TRUE) {
            ShowCursor(false);
            return ofn.lpstrFile;
        }
        else {
            ShowCursor(false);
            return "";
        }
    }

    void point_sampler_callback(const ImDrawList* cmd_list, const ImDrawCmd* cmd)
    {
        D3D11_SAMPLER_DESC desc;
        ZeroMemory(&desc, sizeof(desc));
        desc.Filter = D3D11_FILTER_MIN_MAG_MIP_POINT; // Usar filtrado point
        desc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
        desc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
        desc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
        desc.ComparisonFunc = D3D11_COMPARISON_ALWAYS;
        desc.MinLOD = 0;
        desc.MaxLOD = D3D11_FLOAT32_MAX;
        ID3D11SamplerState* pSamplerState;
        lib3d::device->CreateSamplerState(&desc, &pSamplerState);
        lib3d::inmediate->PSSetSamplers(0, 1, &pSamplerState);
    }

    void linear_sampler_callback(const ImDrawList* cmd_list, const ImDrawCmd* cmd)
    {
        D3D11_SAMPLER_DESC desc;
        ZeroMemory(&desc, sizeof(desc));
        desc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR; // Usar filtrado point
        desc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
        desc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
        desc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
        desc.ComparisonFunc = D3D11_COMPARISON_ALWAYS;
        desc.MinLOD = 0;
        desc.MaxLOD = D3D11_FLOAT32_MAX;
        ID3D11SamplerState* pSamplerState;
        lib3d::device->CreateSamplerState(&desc, &pSamplerState);
        lib3d::inmediate->PSSetSamplers(0, 1, &pSamplerState);
    }

	void generate_chessboard_pattern(ImDrawList* cmd_list, int dimx, int dimy)
	{
		int checkerSize = 8;

		int numSquaresX = dimx / checkerSize;
		int numSquaresY = dimy / checkerSize;

		int lastRowSize = dimy % checkerSize;
		int lastColSize = dimx % checkerSize;

		ImU32 color1 = IM_COL32(200, 200, 200, 50); 
		ImU32 color2 = IM_COL32(100, 100, 100, 50); 

		for (int y = 0; y <= numSquaresY; ++y) {
			for (int x = 0; x <= numSquaresX; ++x) {
				int currentSize = (x < numSquaresX || x == 0) ? checkerSize : lastColSize;
				int currentHeight = (y < numSquaresY || y == 0) ? checkerSize : lastRowSize;

				ImVec2 squarePos = ImVec2(ImGui::GetCursorScreenPos().x + x * checkerSize, ImGui::GetCursorScreenPos().y + y * checkerSize);
				ImU32 color = (x + y) % 2 == 0 ? color1 : color2;
				cmd_list->AddRectFilled(squarePos, ImVec2(squarePos.x + currentSize, squarePos.y + currentHeight), color);
			}
		}
	}

	void draw_shader_error_square()
	{
		ImDrawList* drawList = ImGui::GetBackgroundDrawList();

		float alpha = sin(5.0*ImGui::GetTime()) * 0.5f + 0.5f; 
		alpha *= 1.0;
		ImVec2 squarePos = ImVec2(lib3d::WIDTH  - 20, 40);
		ImVec2 squareSize = ImVec2(10, 10);

		drawList->AddRectFilled(squarePos, ImVec2(squarePos.x + squareSize.x, squarePos.y + squareSize.y), IM_COL32(255, 0, 0, (int)(alpha * 255)));
	}

	void draw_lua_error_square()
	{
		ImDrawList* drawList = ImGui::GetBackgroundDrawList();

		float alpha = sin(5.0 * ImGui::GetTime()) * 0.5f + 0.5f;
		alpha *= 1.0;
		ImVec2 squarePos = ImVec2(lib3d::WIDTH - 40, 40);
		ImVec2 squareSize = ImVec2(10, 10);

		drawList->AddRectFilled(squarePos, ImVec2(squarePos.x + squareSize.x, squarePos.y + squareSize.y), IM_COL32(100, 0, 255, (int)(alpha * 255)));
	}


}


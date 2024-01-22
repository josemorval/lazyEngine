#pragma once
#ifndef _LIBLUA_H_
#define _LIBLUA_H_
#endif

#include "libimgui.h"
#include "lib3d.h"
#include "libmath.h"
#include "libmidi.h"
#include "vars.h"

extern "C" {
#include "lua.h"
#include "lualib.h"
#include "lauxlib.h"
}

#include <windows.h>
#include <iostream>
#include <cstring>

#define FAST_OBJ_IMPLEMENTATION
#include "fast_obj.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"


#define UPDATE_SOFTERROR  lua_Debug ar;\
                          lua_getstack(L, 1, &ar);\
                          lua_getinfo(L, "nSl", &ar);\
                          const char* file = ar.short_src;\
                          int line = ar.currentline;\
                          lua_softerrors_log += file;\
                          lua_softerrors_log += ":";\
                          lua_softerrors_log += std::to_string(line);\
                          lua_softerrors_log += ": got nil value";\
                          lua_softerrors_log += "\n";\

namespace liblua
{
    lua_State* lstate;
    bool has_lua_error = false;
    std::string lua_error_log;
    std::string lua_softerrors_log;
    std::string list_lua_bindings = "";
    std::string lua_filename_level;
    std::string folder_path = "./levels"; 
    FILETIME last_folder_write_time;

    void push_functions();

    void init()
    {
        if (!has_lua_error)
        {
            lib3d::clear_all_list();
            gcb->global_time = 0.0f;

            lua_getglobal(lstate, "init");
            if(lua_pcall(lstate, 0, 0, 0)!=0)
            {
                const char* errorMsg = lua_tostring(lstate, -1);
                lua_error_log += '\n';
                lua_error_log += errorMsg;
                lua_error_log = lua_error_log.substr((lua_error_log.length() > 200) ? lua_error_log.length() - 200 : 0);
                has_lua_error = true;
            }
        }
    }

    void render()
    {
        if (!has_lua_error)
        {
            lua_getglobal(lstate, "render");
            if(lua_pcall(lstate, 0, 0, 0))
            {
                const char* errorMsg = lua_tostring(lstate, -1);
                lua_error_log += '\n';
                lua_error_log += errorMsg;
                lua_error_log = lua_error_log.substr((lua_error_log.length() > 200) ? lua_error_log.length() - 200 : 0);
                has_lua_error = true;
            }
        }
    }

    void imgui()
    {
        if (!has_lua_error)
        {
            lua_getglobal(lstate, "imgui");
            if(lua_pcall(lstate, 0, 0, 0)!=0)
            {
                const char* errorMsg = lua_tostring(lstate, -1);
                lua_error_log += '\n';
                lua_error_log += errorMsg;
                lua_error_log = lua_error_log.substr((lua_error_log.length() > 200) ? lua_error_log.length() - 200 : 0);
                has_lua_error = true;
            }
        }
    }

    void cleanup()
    {
        if (!has_lua_error)
        {
            lua_getglobal(lstate, "cleanup");
            if (lua_pcall(lstate, 0, 0, 0) != 0)
            {
                const char* errorMsg = lua_tostring(lstate, -1);
                lua_error_log += '\n';
                lua_error_log += errorMsg;
                lua_error_log = lua_error_log.substr((lua_error_log.length() > 200) ? lua_error_log.length() - 200 : 0);
                has_lua_error = true;
            }
        }
    }


    bool check_folder_changes()
    {
        WIN32_FIND_DATA findFileData;
        HANDLE hFind = FindFirstFile((folder_path + "\\*").c_str(), &findFileData);

        if (hFind == INVALID_HANDLE_VALUE)
        {
            return false;
        }

        do
        {
            if (!(findFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
            {
                std::string filename = folder_path + "\\" + findFileData.cFileName;

                HANDLE file = CreateFile(filename.c_str(), FILE_READ_ATTRIBUTES, FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE, NULL,
                    OPEN_EXISTING, FILE_FLAG_BACKUP_SEMANTICS, NULL);

                FILETIME file_write_time;
                GetFileTime(file, NULL, NULL, &file_write_time);

                // Compara la fecha de modificación del archivo con la última fecha de modificación de la carpeta
                if (CompareFileTime(&file_write_time, &last_folder_write_time) > 0)
                {
                    // Ha habido cambios en la carpeta, realiza las operaciones necesarias
                    std::cout << "Se detectaron cambios en la carpeta.\n";
                    // Puedes agregar aquí la lógica adicional que necesites
                    last_folder_write_time = file_write_time;
                    return true;
                }

                CloseHandle(file);
            }
        } while (FindNextFile(hFind, &findFileData) != 0);

        FindClose(hFind);
        return false;
    }

    void load_new_level()
    {
        has_lua_error = false;
        lua_error_log = "";
        if (luaL_dofile(lstate, lua_filename_level.c_str()))
        {
            const char* errorMsg = lua_tostring(lstate, -1);
            lua_error_log += '\n';
            lua_error_log += errorMsg;
            lua_error_log = lua_error_log.substr((lua_error_log.length() > 200) ? lua_error_log.length() - 200 : 0);
            has_lua_error = true;
        }

        init();
    }

    void initialize_lua()
    {
        lstate = luaL_newstate();
        luaL_openlibs(lstate);
        push_functions();

        GetSystemTimeAsFileTime(&last_folder_write_time);

        lua_filename_level = "./levels/main.lua";

        has_lua_error = false;
        lua_error_log = "";
        if (luaL_dofile(lstate, lua_filename_level.c_str()))
        {
            const char* errorMsg = lua_tostring(lstate, -1);
            lua_error_log += '\n';
            lua_error_log += errorMsg;
            lua_error_log = lua_error_log.substr((lua_error_log.length() > 200) ? lua_error_log.length() - 200 : 0);
            has_lua_error = true;
        }
    }

    void check_lua_changes()
    {
        lua_softerrors_log = "";

        if (check_folder_changes())
        {
            lua_gc(lstate, LUA_GCCOLLECT, 0);

            has_lua_error = false;
            lua_error_log = "";
            if (luaL_dofile(lstate, lua_filename_level.c_str()))
            {
                const char* errorMsg = lua_tostring(lstate, -1);
                lua_error_log += '\n';
                lua_error_log += errorMsg;
                lua_error_log = lua_error_log.substr((lua_error_log.length() > 200) ? lua_error_log.length() - 200 : 0);
                has_lua_error = true;
            }

            init();
        }
    }

#pragma region imgui bindings

    static int imgui_begin(lua_State* L)
    {
        const char* _label = lua_tostring(L, 1);
        if (_label == nullptr || lua_isnoneornil(L, 1))
        {
            UPDATE_SOFTERROR
            ImGui::Begin("undefined");
            return 0;
        }
        else
        {
            ImGui::Begin(_label);
            return 0;
        }
    }

    static int imgui_end(lua_State* L)
    {
        ImGui::End();
        return 0;
    }

    static int imgui_button(lua_State* L) {
        const char* label = lua_tostring(L, 1);
        if (label == nullptr || lua_isnoneornil(L, 1))
        {
            UPDATE_SOFTERROR
            lua_pushboolean(L, false); 
        }
        else
        {
            lua_pushboolean(L, ImGui::Button(label));
        }
        return 1;
    }

    static int imgui_text(lua_State* L) {
        const char* label = lua_tostring(L, 1);
        if (label == nullptr || lua_isnoneornil(L, 1))
        {
            UPDATE_SOFTERROR
        }
        else
        {
            ImGui::Text("%s", label);
        }
        return 0;
    }

    static int imgui_textwrapped(lua_State* L) {
        const char* label = lua_tostring(L, 1);
        if (label == nullptr || lua_isnoneornil(L, 1))
        {
            UPDATE_SOFTERROR
        }
        else
        {
            ImGui::TextWrapped("%s", label);
        }
        return 0;
    }

    static int imgui_sliderint(lua_State* L)
    {
        const char* _label = lua_tostring(L, 1);
        int _value = lua_tointeger(L, 2);
        int _minValue = lua_tonumber(L, 3);
        int _maxValue = lua_tonumber(L, 4);
        ImGui::SliderInt(_label, &_value, _minValue, _maxValue);
        lua_pushinteger(L, _value);
        return 1;
    }

    static int imgui_sliderfloat(lua_State* L) 
    {
        const char* _label = lua_tostring(L, 1);
        float _value = lua_tonumber(L,2);
        float _minValue = lua_tonumber(L, 3);
        float _maxValue = lua_tonumber(L, 4);
        ImGui::SliderFloat(_label, &_value, _minValue, _maxValue);
        lua_pushnumber(L, _value);
        return 1;
    }

    static int imgui_sliderfloat2(lua_State* L)
    {
        const char* _label = lua_tostring(L, 1);
        float _value[2] = { lua_tonumber(L, 2), lua_tonumber(L, 3) };
        float _minValue = lua_tonumber(L, 4);
        float _maxValue = lua_tonumber(L, 5);
        ImGui::SliderFloat2(_label, &_value[0], _minValue, _maxValue);
        lua_pushnumber(L, _value[0]);
        lua_pushnumber(L, _value[1]);
        return 2;
    }

    static int imgui_sliderfloat3(lua_State* L)
    {
        const char* _label = lua_tostring(L, 1);
        float _value[3] = { lua_tonumber(L, 2), lua_tonumber(L, 3), lua_tonumber(L, 4)};
        float _minValue = lua_tonumber(L, 5);
        float _maxValue = lua_tonumber(L, 6);
        ImGui::SliderFloat3(_label, &_value[0], _minValue, _maxValue);
        lua_pushnumber(L, _value[0]);
        lua_pushnumber(L, _value[1]);
        lua_pushnumber(L, _value[2]);
        return 3;
    }

    static int imgui_sliderfloat4(lua_State* L)
    {
        const char* _label = lua_tostring(L, 1);
        float _value[4] = { lua_tonumber(L, 2), lua_tonumber(L, 3), lua_tonumber(L, 4), lua_tonumber(L, 5)};
        float _minValue = lua_tonumber(L, 6);
        float _maxValue = lua_tonumber(L, 7);
        ImGui::SliderFloat4(_label, &_value[0], _minValue, _maxValue);
        lua_pushnumber(L, _value[0]);
        lua_pushnumber(L, _value[1]);
        lua_pushnumber(L, _value[2]);
        lua_pushnumber(L, _value[3]);
        return 3;
    }

    static int imgui_dragfloat(lua_State* L)
    {
        const char* _label = lua_tostring(L, 1);
        float _value = lua_tonumber(L, 2);
        float _speed = lua_tonumber(L, 3);
        float _minValue = lua_tonumber(L, 4);
        float _maxValue = lua_tonumber(L, 5);
        ImGui::DragFloat(_label, &_value, _speed,_minValue,_maxValue);
        lua_pushnumber(L, _value);
        return 1;
    }

    static int imgui_dragfloat2(lua_State* L)
    {
        const char* _label = lua_tostring(L, 1);
        float _value[2] = { lua_tonumber(L, 2), lua_tonumber(L, 3) };
        float _speed = lua_tonumber(L, 3);
        float _minValue = lua_tonumber(L, 4);
        float _maxValue = lua_tonumber(L, 5);
        ImGui::DragFloat2(_label, &_value[0], _speed, _minValue, _maxValue);
        lua_pushnumber(L, _value[0]);
        lua_pushnumber(L, _value[1]);
        return 2;
    }

    static int imgui_dragfloat3(lua_State* L)
    {
        const char* _label = lua_tostring(L, 1);
        float _value[3] = { lua_tonumber(L, 2), lua_tonumber(L, 3), lua_tonumber(L, 4) };
        float _speed = lua_tonumber(L, 5);
        float _minValue = lua_tonumber(L, 6);
        float _maxValue = lua_tonumber(L, 7);
        ImGui::DragFloat3(_label, &_value[0], _speed, _minValue, _maxValue);
        lua_pushnumber(L, _value[0]);
        lua_pushnumber(L, _value[1]);
        lua_pushnumber(L, _value[2]);
        return 3;
    }

    static int imgui_dragfloat4(lua_State* L)
    {
        const char* _label = lua_tostring(L, 1);
        float _value[4] = { lua_tonumber(L, 2), lua_tonumber(L, 3), lua_tonumber(L, 4), lua_tonumber(L, 5)};
        float _speed = lua_tonumber(L, 6);
        float _minValue = lua_tonumber(L, 7);
        float _maxValue = lua_tonumber(L, 8);
        ImGui::DragFloat4(_label, &_value[0], _speed, _minValue, _maxValue);
        lua_pushnumber(L, _value[0]);
        lua_pushnumber(L, _value[1]);
        lua_pushnumber(L, _value[2]);
        lua_pushnumber(L, _value[3]);
        return 3;
    }

    static int imgui_checkbox(lua_State* L) 
    {
        const char* label = lua_tostring(L, 1);
        bool value = lua_toboolean(L, 2);
        ImGui::Checkbox(label, &value);
        lua_pushboolean(L, value);
        return 1;
    }

    static int imgui_sameline(lua_State* L) 
    {
        float value = lua_tonumber(L, 1);
        ImGui::SameLine(value);
        return 0;
    }

    static int imgui_newline(lua_State* L)
    {
        ImGui::NewLine();
        return 0;
    }

    static int imgui_spacing(lua_State* L)
    {
        ImGui::Spacing();
        return 0;
    }

    static int imgui_setnextwindowsize(lua_State* L) 
    {
        int _dimx = lua_tointeger(L, 1);
        int _dimy = lua_tointeger(L, 2);
        ImGui::SetNextWindowSize(ImVec2(_dimx,_dimy), ImGuiCond_FirstUseEver);
        return 0;
    }

    static int imgui_progressbar(lua_State* L)
    {
        float _val = lua_tonumber(L, 1);
        int _dimx = lua_toboolean(L, 2);
        int _dimy = lua_toboolean(L, 3);
        ImGui::ProgressBar(_val, ImVec2(_dimx,_dimy));
        return 0;
    }

    static int imgui_treenode(lua_State* L)
    {
        const char* _label = lua_tostring(L, 1);
        bool _b = ImGui::TreeNode(_label);
        lua_pushboolean(L, _b);
        return 1;
    }

    static int imgui_treepop(lua_State* L)
    {
        ImGui::TreePop();
        return 0;
    }

    static int imgui_begintable(lua_State* L)
    {
        const char* _label = lua_tostring(L, 1);
        int _columns = lua_tonumber(L, 2);

        bool _b = ImGui::BeginTable(_label, _columns);
        lua_pushboolean(L, _b);
        return 1;
    }

    static int imgui_tablenextcolumn(lua_State* L)
    {
        ImGui::TableNextColumn();
        return 0;
    }

    static int imgui_endtable(lua_State* L)
    {
        ImGui::EndTable();
        return 0;
    }

    static int imgui_tableheadersrow(lua_State* L)
    {
        ImGui::TableHeadersRow();
        return 0;
    }

    static int imgui_tablesetupcolum(lua_State* L)
    {
        const char* _label = lua_tostring(L, 1);
        ImGui::TableSetupColumn(_label);
        return 0;
    }

    static int imgui_image_rendertarget2D(lua_State* L) {
        int index = lua_tointeger(L, 1);
        int dimx = lua_tointeger(L, 2);
        int dimy = lua_tointeger(L, 3);
        const char* label = lua_tostring(L, 4);
        if (lib3d::rendertarget2D_list[index] != nullptr)
        {
            ImDrawList* draw_list = ImGui::GetWindowDrawList();
            libimgui::generate_chessboard_pattern(draw_list, dimx, dimy);
            draw_list->AddCallback(libimgui::point_sampler_callback, nullptr);
            draw_list->AddImage(lib3d::rendertarget2D_list[index]->srv, ImGui::GetCursorScreenPos(), ImVec2(ImGui::GetCursorScreenPos().x + dimx, ImGui::GetCursorScreenPos().y + dimy));
            draw_list->AddCallback(libimgui::linear_sampler_callback, nullptr);
            ImGui::InvisibleButton(label, ImVec2(dimx, dimy));
        }
        return 0;
    }

    static int imgui_image_texture2D(lua_State* L) {
        int index = lua_tointeger(L, 1);
        int dimx = lua_tointeger(L, 2);
        int dimy = lua_tointeger(L, 3);
        const char* label = lua_tostring(L, 4);
        if (lib3d::texture2D_list[index] != nullptr) {
            ImDrawList* draw_list = ImGui::GetWindowDrawList();
            libimgui::generate_chessboard_pattern(draw_list, dimx, dimy);
            draw_list->AddCallback(libimgui::point_sampler_callback, nullptr);
            draw_list->AddImage(lib3d::texture2D_list[index]->srv, ImGui::GetCursorScreenPos(), ImVec2(ImGui::GetCursorScreenPos().x + dimx, ImGui::GetCursorScreenPos().y + dimy));
            draw_list->AddCallback(libimgui::linear_sampler_callback, nullptr);
            ImGui::InvisibleButton(label, ImVec2(dimx, dimy));
        }

        return 0;
    }

    static int imgui_image_renderdepth2D(lua_State* L) {
        int index = lua_tointeger(L, 1);
        int dimx = lua_tointeger(L, 2);
        int dimy = lua_tointeger(L, 3);
        const char* label = lua_tostring(L, 4);
        if (lib3d::renderdepth2D_list[index] != nullptr)
        {
            ImDrawList* draw_list = ImGui::GetWindowDrawList();
            libimgui::generate_chessboard_pattern(draw_list, dimx, dimy);
            draw_list->AddCallback(libimgui::point_sampler_callback, nullptr);
            draw_list->AddImage(lib3d::renderdepth2D_list[index]->srv, ImGui::GetCursorScreenPos(), ImVec2(ImGui::GetCursorScreenPos().x + dimx, ImGui::GetCursorScreenPos().y + dimy));
            draw_list->AddCallback(libimgui::linear_sampler_callback, nullptr);
            ImGui::InvisibleButton(label, ImVec2(dimx, dimy));
        }
        return 0;
    }

    static int imgui_iskeypressed(lua_State* L)
    {
        int _key = lua_tonumber(L, 1);
        bool _is_key_pressed = ImGui::IsKeyDown((ImGuiKey)_key);
        lua_pushboolean(L, _is_key_pressed);
        return 1;
    }

    static int imgui_getmousepos(lua_State* L)
    {
        float x = ImGui::GetMousePos().x;
        float y = ImGui::GetMousePos().y;

        lua_pushnumber(L, x);
        lua_pushnumber(L, y);
        return 2;
    }
    

#pragma endregion
#pragma region rendertarget2d bindings

    static int create_rendertarget2D(lua_State* L)
    {
        int _dimx = lua_tointeger(L, 1);
        int _dimy = lua_tointeger(L, 2);
        int list_index = -1;
        lib3d::RenderTarget2D* rendertarget2d = new lib3d::RenderTarget2D(_dimx, _dimy);
        add_to_list(lib3d::rendertarget2D_list, rendertarget2d);
        lua_pushinteger(L, list_index);
        return 1;
    }

    static int create_empty_rendertarget2D(lua_State* L)
    {
        int list_index = -1;
        lib3d::RenderTarget2D* rendertarget2d = new lib3d::RenderTarget2D();
        add_to_list(lib3d::rendertarget2D_list, rendertarget2d);
        lua_pushinteger(L, list_index);
        return 1;
    }

    static int destroy_rendertarget2D(lua_State* L)
    {
        int _list_index = lua_tointeger(L, 1);
        if (lib3d::rendertarget2D_list[_list_index] != nullptr)
        {
            lib3d::rendertarget2D_list[_list_index]->release();
            lib3d::rendertarget2D_list[_list_index] = nullptr;
        }
        return 0;
    }

    static int attach_srv_rendertarget2D(lua_State* L)
    {
        int list_index = lua_tointeger(L, 1);
        int _slot = lua_tointeger(L, 2);
        lib3d::rendertarget2D_list[list_index]->attach_srv(_slot);
        return 0;
    }

    static int attach_uav_rendertarget2D(lua_State* L)
    {
        int list_index = lua_tointeger(L, 1);
        int _slot = lua_tointeger(L, 2);
        lib3d::rendertarget2D_list[list_index]->attach_uav(_slot);
        return 0;
    }

    static int set_rendertarget_rendertarget2D(lua_State* L)
    {
        int list_index = lua_tointeger(L, 1);
        lib3d::rendertarget2D_list[list_index]->set_rendertarget();
        return 0;
    }

    static int set_rendertarget_and_depth_rendertarget2D(lua_State* L)
    {
        int list_index = lua_tointeger(L, 1);
        int _index_depth = lua_tointeger(L, 2);

        lib3d::rendertarget2D_list[list_index]->set_rendertarget_and_depth(*lib3d::renderdepth2D_list[_index_depth]);
        return 0;
    }

    static int clear_rendertarget_rendertarget2D(lua_State* L)
    {
        int list_index = lua_tointeger(L, 1);
        float _colr = lua_tonumber(L, 2);
        float _colg = lua_tonumber(L, 3);
        float _colb = lua_tonumber(L, 4);
        float _cola = lua_tonumber(L, 5);
        float _clear_col[4] = { _colr,_colg,_colb,_cola };
        lib3d::rendertarget2D_list[list_index]->clear_rendertarget(_clear_col);
        return 0;
    }

    static int copy_resource_rendertarget2D(lua_State* L)
    {
        int list_index = lua_tointeger(L, 1);
        int _index_source = lua_tointeger(L, 1);

        lib3d::rendertarget2D_list[list_index]->copy_resource(*lib3d::rendertarget2D_list[_index_source]);
        return 0;
    }

#pragma endregion
#pragma region rendertarget3d bindings

    static int create_rendertarget3D(lua_State* L)
    {
        int _dimx = lua_tointeger(L, 1);
        int _dimy = lua_tointeger(L, 2);
        int _dimz = lua_tointeger(L, 3);
        int list_index = -1;
        lib3d::RenderTarget3D* rendertarget3d = new lib3d::RenderTarget3D(_dimx, _dimy, _dimz);
        add_to_list(lib3d::rendertarget3D_list, rendertarget3d);
        lua_pushinteger(L, list_index);
        return 1;
    }

    static int create_empty_rendertarget3D(lua_State* L)
    {
        int list_index = -1;
        lib3d::RenderTarget3D* rendertarget3d = new lib3d::RenderTarget3D();
        add_to_list(lib3d::rendertarget3D_list, rendertarget3d);
        lua_pushinteger(L, list_index);
        return 1;
    }

    static int destroy_rendertarget3D(lua_State* L)
    {
        int _list_index = lua_tointeger(L, 1);
        if (lib3d::rendertarget3D_list[_list_index] != nullptr)
        {
            lib3d::rendertarget3D_list[_list_index]->release();
            lib3d::rendertarget3D_list[_list_index] = nullptr;
        }
        return 0;
    }

    static int attach_srv_rendertarget3D(lua_State* L)
    {
        int list_index = lua_tointeger(L, 1);
        int _slot = lua_tointeger(L, 2);
        lib3d::rendertarget3D_list[list_index]->attach_srv(_slot);
        return 0;
    }

    static int attach_uav_rendertarget3D(lua_State* L)
    {
        int list_index = lua_tointeger(L, 1);
        int _slot = lua_tointeger(L, 2);
        lib3d::rendertarget3D_list[list_index]->attach_uav(_slot);
        return 0;
    }

    static int clear_rendertarget_rendertarget3D(lua_State* L)
    {
        int list_index = lua_tointeger(L, 1);
        float _colr = lua_tonumber(L, 2);
        float _colg = lua_tonumber(L, 3);
        float _colb = lua_tonumber(L, 4);
        float _cola = lua_tonumber(L, 5);
        float _clear_col[4] = { _colr,_colg,_colb,_cola };
        lib3d::rendertarget3D_list[list_index]->clear_rendertarget(_clear_col);
        return 0;
    }

    static int copy_resource_rendertarget3D(lua_State* L)
    {
        int list_index = lua_tointeger(L, 1);
        int _index_source = lua_tointeger(L, 1);

        lib3d::rendertarget3D_list[list_index]->copy_resource(*lib3d::rendertarget3D_list[_index_source]);
        return 0;
    }

#pragma endregion
#pragma region backbuffer bindings

    /*BACKBUFFER*/
    static int set_rendertarget_backbuffer(lua_State* L)
    {
        rendertarget_main->set_rendertarget();
        return 0;
    }

    static int set_rendertarget_and_depth_backbuffer(lua_State* L)
    {
        int _index_depth = lua_tointeger(L, 1);
        rendertarget_main->set_rendertarget_and_depth(*lib3d::renderdepth2D_list[_index_depth]);
        return 0;
    }

    static int clear_rendertarget_backbuffer(lua_State* L)
    {
        float _colr = lua_tonumber(L, 1);
        float _colg = lua_tonumber(L, 2);
        float _colb = lua_tonumber(L, 3);
        float _cola = lua_tonumber(L, 4);
        float _clear_col[4] = { _colr, _colg, _colb,_cola };
        rendertarget_main->clear_rendertarget(_clear_col);
        return 0;
    }

#pragma endregion
#pragma region renderdepth2d bindings

    /*RENDER DEPTH 2D*/
    static int create_renderdepth2D(lua_State* L)
    {
        int _dimx = lua_tointeger(L, 1);
        int _dimy = lua_tointeger(L, 2);
        int list_index = -1;
        lib3d::RenderDepth2D* renderdepth2d = new lib3d::RenderDepth2D(_dimx, _dimy);
        add_to_list(lib3d::renderdepth2D_list, renderdepth2d);
        lua_pushinteger(L, list_index);
        return 1;
    }

    static int destroy_renderdepth2D(lua_State* L)
    {
        int _list_index = lua_tointeger(L, 1);
        if (lib3d::renderdepth2D_list[_list_index] != nullptr)
        {
            lib3d::rendertarget2D_list[_list_index]->release();
            lib3d::renderdepth2D_list[_list_index] = nullptr;
        }
        return 0;
    }

    static int attach_srv_renderdepth2D(lua_State* L)
    {
        int list_index = lua_tointeger(L, 1);
        int _slot = lua_tointeger(L, 2);
        lib3d::renderdepth2D_list[list_index]->attach_srv(_slot);
        return 0;
    }

    static int clear_depth_renderdepth2D(lua_State* L)
    {
        int list_index = lua_tointeger(L, 1);
        lib3d::renderdepth2D_list[list_index]->clear_depth();
        return 0;
    }

    static int set_depth_renderdepth2D(lua_State* L)
    {
        int list_index = lua_tointeger(L, 1);
        lib3d::renderdepth2D_list[list_index]->set_depth();
        return 0;
    }

#pragma endregion
#pragma region buffer bindings

    static int create_buffer(lua_State* L)
    {
        int _size = lua_tointeger(L, 1);
        int _size_per_element = lua_tointeger(L, 2);
        bool _cpu_access = lua_tointeger(L, 3);

        int list_index = -1;
        lib3d::Buffer* buffer = new lib3d::Buffer(_size, _size_per_element,_cpu_access);
        add_to_list(lib3d::buffer_list, buffer);
        lua_pushinteger(L, list_index);
        return 1;
    }

    static int destroy_buffer(lua_State* L)
    {
        int _list_index = lua_tointeger(L, 1);
        if (lib3d::buffer_list[_list_index] != nullptr)
        {
            lib3d::buffer_list[_list_index]->release();
            lib3d::buffer_list[_list_index] = nullptr;
        }
        return 0;
    }

    static int attach_srv_buffer(lua_State* L)
    {
        int list_index = lua_tointeger(L, 1);
        int _slot = lua_tointeger(L, 2);
        lib3d::buffer_list[list_index]->attach_srv(_slot);
        return 0;
    }

    static int attach_uav_buffer(lua_State* L)
    {
        int list_index = lua_tointeger(L, 1);
        int _slot = lua_tointeger(L, 2);
        lib3d::buffer_list[list_index]->attach_uav(_slot);
        return 0;
    }

    static int get_data_buffer(lua_State* L)
    {
        int list_index = lua_tointeger(L, 1);
        if (lib3d::buffer_list[list_index] != nullptr)
        {
            D3D11_MAPPED_SUBRESOURCE mappedResource;
            HRESULT hr = lib3d::inmediate->Map(lib3d::buffer_list[list_index]->buffer, 0, D3D11_MAP_READ, 0, &mappedResource);
            if (SUCCEEDED(hr))
            {
                lua_pushlightuserdata(L, (void*)mappedResource.pData);
                lib3d::inmediate->Unmap(lib3d::buffer_list[list_index]->buffer, 0);
                return 1;
            }

        }
        return 0;
    }

    static int move_pointer(lua_State* L)
    {
        lua_pushlightuserdata(L, (void*)((char*)lua_topointer(L, 1) + lua_tointeger(L, 2)));
        return 1;
    }

    static int convert_float_pointer(lua_State* L)
    {
        lua_pushnumber(L,*((float*)lua_topointer(L, 1)));
        return 1;
    }

    static int convert_bool_pointer(lua_State* L)
    {
        lua_pushboolean(L, *((bool*)lua_topointer(L, 1)));
        return 1;
    }

    static int convert_int_pointer(lua_State* L)
    {
        lua_pushinteger(L, *((int*)lua_topointer(L, 1)));
        return 1;
    }

#pragma endregion
#pragma region constantbuffer bindings

    static int create_constantbuffer(lua_State* L)
    {
        int _size = lua_tointeger(L, 1);
        int list_index = -1;
        lib3d::ConstantBuffer* buffer = new lib3d::ConstantBuffer(_size);
        add_to_list(lib3d::constantbuffer_list, buffer);
        lua_pushinteger(L, list_index);
        return 1;
    }

    static int destroy_constantbuffer(lua_State* L)
    {
        int _list_index = lua_tointeger(L, 1);
        if (lib3d::constantbuffer_list[_list_index] != nullptr)
        {
            lib3d::constantbuffer_list[_list_index]->release();
            lib3d::constantbuffer_list[_list_index] = nullptr;
        }
        return 0;
    }

    static int attach_constantbuffer(lua_State* L)
    {
        int list_index = lua_tointeger(L, 1);
        int _slot = lua_tointeger(L, 2);
        lib3d::constantbuffer_list[list_index]->attach(_slot);
        return 0;
    }

    static int update_constantbuffer(lua_State* L)
    {
        int _nargs = lua_gettop(L);
        int _index = lua_tointeger(L, 1);

        lib3d::constantbuffer_list[_index]->map();

        float* _variables = (float*)lib3d::constantbuffer_list[_index]->get_data();
        
        for (int i = 1; i < _nargs; i+=2)
        {   
            int _slot = lua_tointeger(L, i + 1);
            if (_slot < lib3d::constantbuffer_list[_index]->nelements)
            {
                _variables[_slot] = lua_tonumber(L, i + 2);
            }
        }

        lib3d::constantbuffer_list[_index]->unmap();
        
        return 0;
    }

#pragma endregion
#pragma region fx bindings

    static int create_fx(lua_State* L)
    {
        int list_index = -1;
        const char* lua_filename = lua_tostring(L, 1);
        bool _use_geo_shader = lua_toboolean(L, 2);
        int length = MultiByteToWideChar(CP_UTF8, 0, lua_filename, -1, nullptr, 0);
        wchar_t* w_filename = new wchar_t[length];
        MultiByteToWideChar(CP_UTF8, 0, lua_filename, -1, w_filename, length);
        lib3d::FX* fx = new lib3d::FX(w_filename, _use_geo_shader);
        add_to_list(lib3d::fx_list, fx);
        lua_pushinteger(L, list_index);
        return 1;
    }

    static int destroy_fx(lua_State* L)
    {
        int _list_index = lua_tointeger(L, 1);
        if (lib3d::fx_list[_list_index] != nullptr)
        {
            lib3d::fx_list[_list_index]->release();
            lib3d::fx_list[_list_index] = nullptr;
        }
        return 0;
    }

    static int use_fx(lua_State* L)
    {
        int list_index = lua_tointeger(L, 1);
        if (lib3d::fx_list[list_index] != nullptr)
        {
            lib3d::fx_list[list_index]->use();
        }
        return 0;
    }

#pragma endregion
#pragma region computefx bindings

    static int create_computefx(lua_State* L)
    {
        int list_index = -1;
        const char* lua_filename = lua_tostring(L, 1);
        const char* lua_kernel = lua_tostring(L, 2);
        int lua_filename_length = MultiByteToWideChar(CP_UTF8, 0, lua_filename, -1, nullptr, 0);
        int lua_kernel_length = MultiByteToWideChar(CP_UTF8, 0, lua_kernel, -1, nullptr, 0);
        wchar_t* w_filename = new wchar_t[lua_filename_length];
        MultiByteToWideChar(CP_UTF8, 0, lua_filename, -1, w_filename, lua_filename_length);
        lib3d::ComputeFX* computefx = new lib3d::ComputeFX(w_filename, lua_kernel);
        add_to_list(lib3d::computefx_list, computefx);
        lua_pushinteger(L, list_index);
        return 1;
    }

    static int destroy_computefx(lua_State* L)
    {
        int _list_index = lua_tointeger(L, 1);
        if (lib3d::computefx_list[_list_index] != nullptr)
        {
            lib3d::computefx_list[_list_index]->release();
            lib3d::computefx_list[_list_index] = nullptr;
        }
        return 0;
    }

    static int use_computefx(lua_State* L)
    {
        int list_index = lua_tointeger(L, 1);
        lib3d::computefx_list[list_index]->use();
        return 0;
    }

    static int dispatch_computefx(lua_State* L)
    {
        int list_index = lua_tointeger(L, 1);
        int _threadx = lua_tointeger(L, 2);
        int _thready = lua_tointeger(L, 3);
        int _threadz = lua_tointeger(L, 4);

        lib3d::computefx_list[list_index]->dispatch(_threadx, _thready, _threadz);
        return 0;
    }

#pragma endregion
#pragma region viewport bindings

    static int use_viewport(lua_State* L)
    {
        float _offsetx0 = lua_tonumber(L, 1);
        float _offsety0 = lua_tonumber(L, 2);
        float _width = lua_tonumber(L, 3);
        float _height = lua_tonumber(L, 4);
        viewport->set(_offsetx0, _offsety0, _width, _height);
        viewport->use();
        return 0;
    }

#pragma endregion
#pragma region rasterizer bindings

    static int use_rasterizer(lua_State* L)
    {
        rasterizer->use();
        return 0;
    }

    static int use_nocull_rasterizer(lua_State* L)
    {
        rasterizer_nocull->use();
        return 0;
    }

    static int use_wireframe_rasterizer(lua_State* L)
    {
        rasterizer_wireframe->use();
        return 0;
    }

#pragma endregion
#pragma region alphablending bindings

    static int use_alphablending(lua_State* L)
    {
        alpha_blending->use();
        return 0;
    }

    static int clear_alphablending(lua_State* L)
    {
        lib3d::clear_blending();
        return 0;
    }

#pragma endregion
#pragma region depthstencil bindings

    static int use_nowrite_depthstencil(lua_State* L)
    {
        nowrite_depthstencil->use();
        return 0;
    }

    static int use_write_depthstencil(lua_State* L)
    {
        write_depthstencil->use();
        return 0;
    }

    static int clear_depthstencil(lua_State* L)
    {
        lib3d::clear_depthstencil();
        return 0;
    }

#pragma endregion
#pragma region mesh bindings

    static int use_mesh(lua_State* L)
    {
        int list_index = lua_tointeger(L, 1);
        if (lib3d::mesh_list[list_index] != nullptr)
        {
            lib3d::mesh_list[list_index]->use();
        }
        return 0;
    }

    static int draw_mesh(lua_State* L)
    {
        int list_index = lua_tointeger(L, 1);
        if (lib3d::mesh_list[list_index] != nullptr)
        {
            lib3d::mesh_list[list_index]->draw();
        }
        return 0;
    }

    static int draw_instances_mesh(lua_State* L)
    {
        int list_index = lua_tointeger(L, 1);
        int _instances = lua_tointeger(L, 2);

        if (lib3d::mesh_list[list_index] != nullptr)
        {
            lib3d::mesh_list[list_index]->draw_instanced(_instances);
        }
        return 0;
    }

    static int vertices_count_mesh(lua_State* L)
    {
        int list_index = lua_tointeger(L, 1);
        if (lib3d::mesh_list[list_index] != nullptr)
        {
            lua_pushinteger(L,lib3d::mesh_list[list_index]->vertices_count);
        }
        else
        {
            lua_pushinteger(L, 0);
        }

        return 1;
    }

    static int indices_count_mesh(lua_State* L)
    {
        int list_index = lua_tointeger(L, 1);
        if (lib3d::mesh_list[list_index] != nullptr)
        {
            lua_pushinteger(L, lib3d::mesh_list[list_index]->indices_count);
        }
        else
        {
            lua_pushinteger(L, 0);
        }

        return 1;
    }

    static int attach_vertices_srv_mesh(lua_State* L)
    {
        int list_index = lua_tointeger(L, 1);
        int _slot = lua_tointeger(L, 2);
        lib3d::mesh_list[list_index]->attach_vertices_srv(_slot);
        return 0;
    }

    static int attach_indices_srv_mesh(lua_State* L)
    {
        int list_index = lua_tointeger(L, 1);
        int _slot = lua_tointeger(L, 2);
        lib3d::mesh_list[list_index]->attach_indices_srv(_slot);
        return 0;
    }

    static int use_cube(lua_State* L)
    {
        mesh_cube->use();
        return 0;
    }

    static int use_quad(lua_State* L)
    {
        mesh_quad->use();
        return 0;
    }

    static int use_sphere(lua_State* L)
    {
        mesh_sphere->use();
        return 0;
    }

    static int draw_cube(lua_State* L)
    {
        mesh_cube->draw();
        return 0;
    }

    static int draw_sphere(lua_State* L)
    {
        mesh_sphere->draw();
        return 0;
    }

    static int draw_quad(lua_State* L)
    {
        mesh_quad->draw();
        return 0;
    }

    static int draw_instances_quad(lua_State* L)
    {
        int _instances = lua_tointeger(L, 1);
        mesh_quad->draw_instanced(_instances);
        return 0;
    }

    static int draw_instances_sphere(lua_State* L)
    {
        int _instances = lua_tointeger(L, 1);
        mesh_sphere->draw_instanced(_instances);
        return 0;
    }

    static int draw_instances_cube(lua_State* L)
    {
        int _instances = lua_tointeger(L, 1);
        mesh_cube->draw_instanced(_instances);
        return 0;
    }

    static int emit_vertex(lua_State* L)
    {
        int _count = lua_tointeger(L, 1);
        int _instancecount = lua_tointeger(L, 2);

        lib3d::emit_vertex(_count, _instancecount);
        return 0;
    }

#pragma endregion
#pragma region gputimer bindings

    static int gputimer_begin_block(lua_State* L)
    {
        const char* _label = lua_tostring(L, 1);
        gpu_timer->new_block(_label);
        return 0;
    }

    static int gputimer_count(lua_State* L)
    {
        lua_pushinteger(L,gpu_timer->numQueryPoints);
        return 1;
    }

    static int gputimer_value(lua_State* L)
    {
        int _index = lua_tointeger(L, 1);
        lua_pushstring(L, gpu_timer->blockNames[_index].c_str());
        lua_pushnumber(L, gpu_timer->blockTimes[_index]);
        return 2;
    }

    static int gputimer_begin_profile(lua_State* L)
    {
        gpu_timer->begin_profile();
        return 0;
    }

    static int gputimer_end_profile(lua_State* L)
    {
        gpu_timer->end_profile();
        return 0;
    }
#pragma endregion
#pragma region misc bindings

    static int get_window_size(lua_State* L)
    {
        lua_pushinteger(L, lib3d::WIDTH);
        lua_pushinteger(L, lib3d::HEIGHT);
        return 2;
    }

    static int get_time(lua_State* L)
    {
        lua_pushnumber(L, gcb->global_time);
        return 1;
    }

    static int get_delta_time(lua_State* L)
    {
        lua_pushnumber(L, gcb->delta_time);
        return 1;
    }

    static int set_camera(lua_State* L)
    {
        float _x = lua_tonumber(L, 1);
        float _y = lua_tonumber(L, 2);
        float _z = lua_tonumber(L, 3);
        float _dirx = lua_tonumber(L, 4);
        float _diry = lua_tonumber(L, 5);
        float _dirz = lua_tonumber(L, 6);

        gcb->view_matrix = compute_view_matrix(float3(_x,_y,_z), float3(_x, _y, _z) + 0.1 * float3(_dirx, _diry, _dirz));
        gcb->inv_view_matrix = compute_inv_view_matrix(float3(_x, _y, _z), float3(_x, _y, _z) + 0.1 * float3(_dirx, _diry, _dirz));

        return 0;
    }

    static int set_light_camera(lua_State* L)
    {
        float _x = lua_tonumber(L, 1);
        float _y = lua_tonumber(L, 2);
        float _z = lua_tonumber(L, 3);
        float _dirx = lua_tonumber(L, 4);
        float _diry = lua_tonumber(L, 5);
        float _dirz = lua_tonumber(L, 6);

        gcb->light_view_matrix = compute_view_matrix(float3(_x, _y, _z), float3(_x, _y, _z) + 0.1 * float3(_dirx, _diry, _dirz));
        gcb->light_inv_view_matrix = compute_inv_view_matrix(float3(_x, _y, _z), float3(_x, _y, _z) + 0.1 * float3(_dirx, _diry, _dirz));
        return 0;
    }

    static int set_perspective(lua_State* L)
    {
        float _fov = lua_tonumber(L, 1);
        float _aspect = lua_tonumber(L, 2);
        float _znear = lua_tonumber(L, 3);
        float _zfar = lua_tonumber(L, 4);

        gcb->projection_matrix = compute_perspective_matrix(_fov,_aspect,_znear,_zfar);
        return 0;
    }

    static int set_orthographic(lua_State* L)
    {
        float _width = lua_tonumber(L, 1);
        float _height = lua_tonumber(L, 2);
        float _znear = lua_tonumber(L, 3);
        float _zfar = lua_tonumber(L, 4);

        gcb->projection_matrix = compute_orthographic_matrix(_width,_height,_znear,_zfar);
        return 0;
    }

    static int set_light_perspective(lua_State* L)
    {
        float _width = lua_tonumber(L, 1);
        float _height = lua_tonumber(L, 2);
        float _znear = lua_tonumber(L, 3);
        float _zfar = lua_tonumber(L, 4);

        gcb->light_projection_matrix = compute_orthographic_matrix(_width, _height, _znear, _zfar);
        return 0;
    }

    static int set_light_orthographic(lua_State* L)
    {
        float _width = lua_tonumber(L, 1);
        float _height = lua_tonumber(L, 2);
        float _znear = lua_tonumber(L, 3);
        float _zfar = lua_tonumber(L, 4);

        gcb->light_projection_matrix = compute_orthographic_matrix(_width, _height, _znear, _zfar);
        return 0;
    }

    static int transform_point_camera(lua_State* L)
    {
        float _x = lua_tonumber(L, 1);
        float _y = lua_tonumber(L, 2);
        float _z = lua_tonumber(L, 3);
        float _w = lua_tonumber(L, 4);

        float4 _result = mul(float4(_x, _y, _z, _w), gcb->view_matrix);

        lua_pushnumber(L, _result.x);
        lua_pushnumber(L, _result.y);
        lua_pushnumber(L, _result.z);
        lua_pushnumber(L, _result.w);

        return 4;
    }

    static int inv_transform_point_camera(lua_State* L)
    {
        float _x = lua_tonumber(L, 1);
        float _y = lua_tonumber(L, 2);
        float _z = lua_tonumber(L, 3);
        float _w = lua_tonumber(L, 4);

        float4 _result = mul(gcb->inv_view_matrix, float4(_x, _y, _z, _w));

        lua_pushnumber(L, _result.x);
        lua_pushnumber(L, _result.y);
        lua_pushnumber(L, _result.z);
        lua_pushnumber(L, _result.w);

        return 4;
    }

    static int create_particle_buffer(lua_State* L)
    {
        int _size = lua_tointeger(L, 1);
        bool _cpu_access = lua_tointeger(L, 2);
        int list_index = -1;
        lib3d::Buffer* buffer = new lib3d::Buffer(_size, sizeof(Particle),_cpu_access);
        add_to_list(lib3d::buffer_list, buffer);
        lua_pushinteger(L, list_index);
        return 1;
    }

    static int set_rendertarget_depth_and_uavs(lua_State* L)
    {
        bool _use_backbuffer = lua_tointeger(L, 1);
        int _num_rtvs = 0;
        int _num_uavs_2d = 0;
        int _num_uavs_3d = 0;

        if (_use_backbuffer) _num_rtvs = 1;

        if (lua_istable(L, 2))
        {
            int _n = lua_rawlen(L, 2);
            _num_rtvs += _n;
        }

        if (lua_istable(L, 3))
        {
            int _n = lua_rawlen(L, 3);
            _num_uavs_2d += _n;
        }

        if (lua_istable(L, 4))
        {
            int _n = lua_rawlen(L, 4);
            _num_uavs_3d += _n;
        }

        int _depth_index = lua_tointeger(L, 5);
        int _offset_uav = lua_tointeger(L, 6);
        ID3D11DepthStencilView* _dsv = lib3d::renderdepth2D_list[_depth_index]->dsv;
        ID3D11RenderTargetView** _rtvs = new ID3D11RenderTargetView*[_num_rtvs];
        {
            _rtvs[0] = rendertarget_main->rtv;
            for (int i = 1; i < _num_rtvs; i++)
            {
                _rtvs[i] = lib3d::rendertarget2D_list[lua_rawgeti(L, 2, i)]->rtv;
            }
        }
        ID3D11UnorderedAccessView** _uavs = new ID3D11UnorderedAccessView * [_num_uavs_2d+ _num_uavs_3d];
        {
            for (int i = 0; i < _num_uavs_2d; i++)
            {
                _uavs[i] = lib3d::rendertarget2D_list[lua_rawgeti(L, 3, i+1)]->uav;
            }

            for (int i = 0; i < _num_uavs_3d; i++)
            {
                _uavs[i+ _num_uavs_2d] = lib3d::rendertarget3D_list[lua_rawgeti(L, 4, i + 1)]->uav;
            }
        }

        lib3d::set_renders_and_uavs(_rtvs, _num_rtvs, _dsv, _uavs, _offset_uav, _num_uavs_2d + _num_uavs_3d);
        
        return 0;
    }

    static int clean_srv(lua_State* L)
    {
        int _slot = lua_tointeger(L, 1);
        lib3d::clean_srv(_slot);
        return 0;
    }

    static int clean_uav(lua_State* L)
    {
        int _slot = lua_tointeger(L, 1);
        lib3d::clean_uav(_slot);
        return 0;
    }

    static int clean_constantbuffers(lua_State* L)
    {
        int _slot = lua_tointeger(L, 1);
        lib3d::clean_constantbuffers(_slot);
        return 0;
    }

    static int set_translation_transform(lua_State* L)
    {
        int _id = lua_tointeger(L, 1);
        float _x = lua_tonumber(L, 2);
        float _y = lua_tonumber(L, 3);
        float _z = lua_tonumber(L, 4);
        tb->translation_matrix[_id] = compute_translation_matrix(_x, _y, _z);
        return 0;
    }

    static int get_translation_transform(lua_State* L)
    {
        int _id = lua_tointeger(L, 1);
        lua_pushnumber(L, tb->translation_matrix[_id].row(3).x);
        lua_pushnumber(L, tb->translation_matrix[_id].row(3).y);
        lua_pushnumber(L, tb->translation_matrix[_id].row(3).z);
        return 3;
    }

    static int set_scale_transform(lua_State* L)
    {
        int _id = lua_tointeger(L, 1);
        float _x = lua_tonumber(L, 2);
        float _y = lua_tonumber(L, 3);
        float _z = lua_tonumber(L, 4);
        tb->scale_matrix[_id] = compute_scale_matrix(_x, _y, _z);
        return 0;
    }

    static int get_scale_transform(lua_State* L)
    {
        int _id = lua_tointeger(L, 1);
        lua_pushnumber(L, tb->scale_matrix[_id].row(0).x);
        lua_pushnumber(L, tb->scale_matrix[_id].row(1).y);
        lua_pushnumber(L, tb->scale_matrix[_id].row(2).z);
        return 3;
    }

    static int set_rotation_transform(lua_State* L)
    {
        int _id = lua_tointeger(L, 1);
        float _vx = lua_tonumber(L, 2);
        float _vy = lua_tonumber(L, 3);
        float _vz = lua_tonumber(L, 4);
        float _angle = lua_tonumber(L, 5);

        float3 _v = normalize(float3(_vx, _vy, _vz));

        tb->rotation_matrix[_id] = compute_rotation_matrix(_angle, float3(_v.x, _v.y, _v.z));

        return 0;
    }

    static int get_rotation_transform(lua_State* L)
    {
        int _id = lua_tointeger(L, 1);
        float4 _v = extract_rotation_info(tb->rotation_matrix[_id]);
        lua_pushnumber(L, _v.x);
        lua_pushnumber(L, _v.y);
        lua_pushnumber(L, _v.z);
        lua_pushnumber(L, _v.w);

        return 4;
    }

    static int set_customdata(lua_State* L)
    {
        int _id = lua_tointeger(L, 1);
        int _comp = lua_tointeger(L, 2);
        float _val = lua_tonumber(L, 3);

        if (_id == 0)
        {
            if (_comp == 0) cdb->custom_data0.x = _val;
            if (_comp == 1) cdb->custom_data0.y = _val;
            if (_comp == 2) cdb->custom_data0.z = _val;
            if (_comp == 3) cdb->custom_data0.w = _val;
        }

        if (_id == 1)
        {
            if (_comp == 0) cdb->custom_data1.x = _val;
            if (_comp == 1) cdb->custom_data1.y = _val;
            if (_comp == 2) cdb->custom_data1.z = _val;
            if (_comp == 3) cdb->custom_data1.w = _val;
        }

        if (_id == 2)
        {
            if (_comp == 0) cdb->custom_data2.x = _val;
            if (_comp == 1) cdb->custom_data2.y = _val;
            if (_comp == 2) cdb->custom_data2.z = _val;
            if (_comp == 3) cdb->custom_data2.w = _val;
        }

        return 0;
    }

    static int get_customdata(lua_State* L)
    {
        int _id = lua_tointeger(L, 1);
        int _comp = lua_tointeger(L, 2);
        float _val = lua_tonumber(L, 3);

        if (_id == 0)
        {
            if (_comp == 0) lua_pushnumber(L, cdb->custom_data0.x);
            if (_comp == 1) lua_pushnumber(L, cdb->custom_data0.y);
            if (_comp == 2) lua_pushnumber(L, cdb->custom_data0.z);
            if (_comp == 3) lua_pushnumber(L, cdb->custom_data0.w);
        }

        if (_id == 1)
        {
            if (_comp == 0) lua_pushnumber(L, cdb->custom_data1.x);
            if (_comp == 1) lua_pushnumber(L, cdb->custom_data1.y);
            if (_comp == 2) lua_pushnumber(L, cdb->custom_data1.z);
            if (_comp == 3) lua_pushnumber(L, cdb->custom_data1.w);
        }

        if (_id == 2)
        {
            if (_comp == 0) lua_pushnumber(L, cdb->custom_data2.x);
            if (_comp == 1) lua_pushnumber(L, cdb->custom_data2.y);
            if (_comp == 2) lua_pushnumber(L, cdb->custom_data2.z);
            if (_comp == 3) lua_pushnumber(L, cdb->custom_data2.w);
        }

        return 1;
    }

    static int update_globalconstantbuffer(lua_State* L)
    {
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
        return 0;
    }

    static int update_transformbuffer(lua_State* L)
    {
        transformbuffer_main->map();
        TransformBuffer* _tbp = (TransformBuffer*)transformbuffer_main->get_data();
        for (int i = 0; i < 64; i++)
        {
            _tbp->translation_matrix[i] = tb->translation_matrix[i];
            _tbp->rotation_matrix[i] = tb->rotation_matrix[i];
            _tbp->scale_matrix[i] = tb->scale_matrix[i];
        }
        transformbuffer_main->unmap();
        return 0;
    }

    static int update_customdatabuffer(lua_State* L)
    {
        customdatabuffer_main->map();
        CustomDataBuffer* _cdbp = (CustomDataBuffer*)customdatabuffer_main->get_data();
        _cdbp->custom_data0 = cdb->custom_data0;
        _cdbp->custom_data1 = cdb->custom_data1;
        _cdbp->custom_data2 = cdb->custom_data2;
        customdatabuffer_main->unmap();
        return 0;
    }

    static int load_mesh(lua_State* L)
    {
        const char* _label = lua_tostring(L, 1);

        if (_label == "")
        {
            lua_pushinteger(L, -1);
            return 1;
        }

        fastObjMesh* m = fast_obj_read(_label);
        
        int list_index = -1;
        if (m != nullptr)
        {
            float* vert_array = new float[m->index_count * 8];
            int* ind_array = new int[m->index_count];

            for (int i = 0; i < m->index_count; i++)
            {
                vert_array[8 * i + 0] = m->positions[3 * m->indices[i].p + 0];
                vert_array[8 * i + 1] = m->positions[3 * m->indices[i].p + 1];
                vert_array[8 * i + 2] = m->positions[3 * m->indices[i].p + 2];

                vert_array[8 * i + 3] = m->texcoords[2 * m->indices[i].t + 0];
                vert_array[8 * i + 4] = m->texcoords[2 * m->indices[i].t + 1];

                vert_array[8 * i + 5] = m->normals[3 * m->indices[i].n + 0];
                vert_array[8 * i + 6] = m->normals[3 * m->indices[i].n + 1];
                vert_array[8 * i + 7] = m->normals[3 * m->indices[i].n + 2];

                ind_array[i] = i;
            }


            list_index = -1;
            lib3d::Mesh* mesh = new lib3d::Mesh(vert_array, ind_array, (int)m->index_count * 8 * sizeof(float), (int)m->index_count * sizeof(int));
            add_to_list(lib3d::mesh_list, mesh);
        }
        
        lua_pushinteger(L, list_index);
        return 1;
    }

    static int load_texture(lua_State* L)
    {
        const char* _label = lua_tostring(L, 1);

        int channels;
        int width;
        int height;

        stbi_uc* imageData = stbi_load(_label, &width, &height, &channels, STBI_rgb_alpha);

        if (_label == "" || imageData==nullptr)
        {
            lua_pushinteger(L, -1);
            return 1;
        }

        int list_index = -1;
        lib3d:: Texture2D* texture2d = new lib3d::Texture2D(width, height, imageData);
        add_to_list(lib3d::texture2D_list, texture2d);

        lua_pushinteger(L, list_index);
        return 1;
    }


#pragma endregion

    //Don't know if this would be the best way...but it is cool
    void push_functions()
    {    
#include "luabindings_generated.h"
    }

}


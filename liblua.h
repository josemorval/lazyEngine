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
    FILETIME last_write_time;
    HANDLE file;
    bool has_lua_error = false;
    std::string lua_error_log;
    std::string lua_softerrors_log;
    lua_State* lstate;
    float time_since_last_check = 0.0;
    const float max_time_check = 3.0;
    std::string list_lua_bindings = "";
    std::string lua_filename_level;

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

    void load_new_level()
    {
        file = CreateFile(lua_filename_level.c_str(), FILE_READ_ATTRIBUTES, FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE, NULL,
            OPEN_EXISTING, FILE_FLAG_BACKUP_SEMANTICS, NULL
        );

        GetFileTime(file, NULL, NULL, &last_write_time);

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

        lua_filename_level = "./levels/main.lua";

        file = CreateFile(lua_filename_level.c_str(), FILE_READ_ATTRIBUTES, FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE, NULL,
            OPEN_EXISTING, FILE_FLAG_BACKUP_SEMANTICS, NULL
        );

        GetFileTime(file, NULL, NULL, &last_write_time);

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
        FILETIME currentWriteTime;
        GetFileTime(file, NULL, NULL, &currentWriteTime);

        if (CompareFileTime(&last_write_time, &currentWriteTime) != 0)
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

            last_write_time = currentWriteTime;
        }
    }

    ////////////////////////////
    /// IMGUI BINDINGS
    ///////////////////////////

    static int imgui_begin(lua_State* L)
    {
        const char* name = lua_tostring(L, 1);
        bool result = ImGui::Begin(name);
        lua_pushboolean(L, result);
        return 1;
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
        ImGui::SetNextWindowSize(ImVec2(_dimx,_dimy));
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


    static int imgui_image_rendertarget2D(lua_State* L) {
        int index = lua_tointeger(L, 1);
        int dimx = lua_tointeger(L, 2);
        int dimy = lua_tointeger(L, 2);
        if (lib3d::rendertarget2D_list[index] != nullptr)
        {
            ImDrawList* draw_list = ImGui::GetWindowDrawList();
            draw_list->AddCallback(libimgui::point_sampler_callback, nullptr);
            draw_list->AddImage(lib3d::rendertarget2D_list[index]->srv, ImGui::GetCursorScreenPos(), ImVec2(ImGui::GetCursorScreenPos().x + dimx, ImGui::GetCursorScreenPos().y + dimy));
            draw_list->AddCallback(libimgui::linear_sampler_callback, nullptr);

        }
        return 0;
    }

    static int imgui_image_renderdepth2D(lua_State* L) {
        int index = lua_tointeger(L, 1);
        int dimx = lua_tointeger(L, 2);
        int dimy = lua_tointeger(L, 3);
        if (lib3d::renderdepth2D_list[index] != nullptr)
        {
            ImDrawList* draw_list = ImGui::GetWindowDrawList();
            draw_list->AddCallback(libimgui::point_sampler_callback, nullptr);
            draw_list->AddImage(lib3d::renderdepth2D_list[index]->srv, ImGui::GetCursorScreenPos(), ImVec2(ImGui::GetCursorScreenPos().x + dimx, ImGui::GetCursorScreenPos().y + dimy));
            draw_list->AddCallback(libimgui::linear_sampler_callback, nullptr);

        }
        return 0;
    }

    ////////////////////////////
    /// 3D BINDINGS
    ///////////////////////////

    /*RENDER TARGET 2D*/
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

    /*RENDER TARGET 3D*/
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

    static int set_rendertarget_and_depthmain_backbuffer(lua_State* L)
    {
        int _index_depth = lua_tointeger(L, 1);
        rendertarget_main->set_rendertarget_and_depth(*depth_main);
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

    static int attach_srv_depthmain(lua_State* L)
    {
        int _slot = lua_tointeger(L, 1);
        depth_main->attach_srv(_slot);
        return 0;
    }

    static int clear_depth_depthmain(lua_State* L)
    {
        depth_main->clear_depth();
        return 0;
    }

    static int set_depth_depthmain(lua_State* L)
    {
        depth_main->set_depth();
        return 0;
    }

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

    /*BUFFER*/
    static int create_buffer(lua_State* L)
    {
        int _size = lua_tointeger(L, 1);
        int _size_per_element = lua_tointeger(L, 2);
        int list_index = -1;
        lib3d::Buffer* buffer = new lib3d::Buffer(_size, _size_per_element);
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

    /*FX*/
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

    /*COMPUTE FX*/
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

    /*VIEWPORT*/
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

    /*RASTERIZER*/
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

    /*ALPHA BLENDING*/
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

    /*DEPTH STENCIL*/
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

    /*MESH*/
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

    /*MISC*/
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

    static int create_particle_buffer(lua_State* L)
    {
        int _size = lua_tointeger(L, 1);
        int list_index = -1;
        lib3d::Buffer* buffer = new lib3d::Buffer(_size, sizeof(Particle));
        add_to_list(lib3d::buffer_list, buffer);
        lua_pushinteger(L, list_index);
        return 1;
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
        _gcb->projection_matrix = gcb->projection_matrix;
        _gcb->light_view_matrix = gcb->light_view_matrix;
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

    //Don't know if this would be the best way...but it is cool
    void push_functions()
    {    
#include "luabindings_generated.h"
    }

}

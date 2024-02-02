assets = {}

function assets.fx(_path,_use_geomety_shader)
    local _index = create_fx(_path,_use_geomety_shader)
    return
    {
        index = _index,
        use = function(self)
            use_fx(self.index)
        end
    }
end
function assets.computefx(_path,_kernel)
    local _index = create_computefx(_path, _kernel)
    return
    {
        index = _index,
        use = function(self)
            use_computefx(self.index)
        end,
        dispatch = function(self,_workgroup_x,_workgroup_y,_workgroup_z)
            dispatch_computefx( self.index, _workgroup_x,_workgroup_y,_workgroup_z )
        end
    }
end
function assets.rendertarget2D(_sx,_sy)
    local _index = create_rendertarget2D(_sx,_sy)
    return 
    {
        index = _index,
        sx = _sx,
        sy = _sy,
        srv = -1,
        uav = -1,
        get = function(self)
            return self.index
        end,
        read = function(self, _srv)
            self.srv = _srv
            attach_srv_rendertarget2D(self.index, self.srv)
        end,
        write = function(self, _uav)
            self.uav = _uav
            attach_uav_rendertarget2D(self.index, self.uav)
        end,
        unread = function(self)
            clean_srv(self.srv)
            self.srv = -1
        end,
        unwrite = function(self)
            clean_uav(self.uav)
            self.uav = -1
        end,
        gen_mipmaps = function(self)
            generate_mips_rendertarget2D(self.index)
        end,
        clear = function(self,r,g,b,a)
            clear_rendertarget_rendertarget2D(self.index,r,g,b,a)
        end,
        use = function(self,depth)
            depth = depth or nil
            if depth==nil then
                set_rendertarget_rendertarget2D(self.index)
            else
                set_rendertarget_and_depth_rendertarget2D(self.index,depth:get())
            end
        end
    }
end
function assets.rendertarget3D(_sx,_sy,_sz)
    local _index = create_rendertarget3D(_sx,_sy,_sz)
    return 
    {
        index = _index,
        sx = _sx,
        sy = _sy,
        sz = _sz,
        srv = -1,
        uav = -1,
        get = function(self)
            return self.index
        end,
        read = function(self, _srv)
            self.srv = _srv
            attach_srv_rendertarget3D(self.index, self.srv)
        end,
        write = function(self, _uav)
            self.uav = _uav
            attach_uav_rendertarget3D(self.index, self.uav)
        end,
        unread = function(self)
            clean_srv(self.srv)
            self.srv = -1
        end,
        unwrite = function(self)
            clean_uav(self.uav)
            self.uav = -1
        end,
        gen_mipmaps = function(self)
            generate_mips_rendertarget3D(self.index)
        end,
        clear = function(self,r,g,b,a)
            clear_rendertarget_rendertarget3D(self.index,r,g,b,a)
        end
    }
end
function assets.renderdepth2D(_sx,_sy)
    local _index = create_renderdepth2D(_sx,_sy)
    return 
    {
        index = _index,
        sx = _sx,
        sy = _sy,
        srv = -1,
        get = function(self)
            return self.index
        end,
        read = function(self, _srv)
            self.srv = _srv
            attach_srv_renderdepth2D(self.index, self.srv)
        end,
        unread = function(self)
            clean_srv(self.srv)
            self.srv = -1
        end,
        clear = function(self)
            clear_depth_renderdepth2D(self.index)
        end,
        use = function(self)
            set_depth_renderdepth2D(self.index)
        end
    }
end
function assets.texture2D(_path)
    local _index = load_texture2D(_path)
    return 
    {
        index = _index,
        srv = -1,
        get = function(self)
            return self.index
        end,
        read = function(self, _srv)
            self.srv = _srv
            attach_srv_rendertarget2D(self.index, self.srv)
        end,
        unread = function(self)
            clean_srv(self.srv)
            self.srv = -1
        end,
        gen_mipmaps = function(self)
            generate_mips_rendertarget2D(self.index)
        end
    }
end
function assets.constantbuffer(_size)
    local _index = create_constantbuffer(_size)
    return 
    {
        index = _index,
        srv = -1,
        get = function(self)
            return self.index
        end,
        bind = function(self,_srv)
            self.srv = _srv
            attach_constantbuffer(self.index,_srv)
        end,
        unbind = function(self)
            clean_constantbuffers(self.srv)
            self.srv = -1
        end,
        update = function(self, ...)
            update_constantbuffer( self.index, ... )
        end
    }
end
function assets.bufferindirect(_count,_size)
    local _index = create_buffer_indirect(_count,_size)
    return 
    {
        index = _index,
        srv = -1,
        uav = -1,
        pointer = nil,
        get = function(self)
            return self.index
        end,
        read = function(self, _srv)
            self.srv = _srv
            attach_srv_buffer(self.index, self.srv)
        end,
        write = function(self, _uav)
            self.uav = _uav
            attach_uav_buffer(self.index, self.uav)
        end,
        unread = function(self)
            clean_srv(self.srv)
            self.srv = -1
        end,
        unwrite = function(self)
            clean_uav(self.uav)
            self.uav = -1
        end,
        get_pointer = function(self)
            self.pointer = get_data_buffer( self.index )
        end,
        get_float = function(self,_offset)
            local _p = move_pointer( self.pointer, _offset )
            return convert_float_pointer( _p )
        end,
        get_bool = function(self,_offset)
            local _p = move_pointer( self.pointer, _offset )
            return convert_bool_pointer( _p )
        end,
        get_int = function(self,_offset)
            local _p = move_pointer( self.pointer, _offset )
            return convert_int_pointer( _p )
        end
    }
end
function assets.buffer(_count,_size,_has_cpu_access)
    local _index = create_buffer(_count,_size,_has_cpu_access)
    return 
    {
        index = _index,
        srv = -1,
        uav = -1,
        pointer = nil,
        get = function(self)
            return self.index
        end,
        read = function(self, _srv)
            self.srv = _srv
            attach_srv_buffer(self.index, self.srv)
        end,
        write = function(self, _uav)
            self.uav = _uav
            attach_uav_buffer(self.index, self.uav)
        end,
        unread = function(self)
            clean_srv(self.srv)
            self.srv = -1
        end,
        unwrite = function(self)
            clean_uav(self.uav)
            self.uav = -1
        end,
        get_pointer = function(self)
            self.pointer = get_data_buffer( self.index )
        end,
        get_float = function(self,_offset)
            local _p = move_pointer( self.pointer, _offset )
            return convert_float_pointer( _p )
        end,
        get_bool = function(self,_offset)
            local _p = move_pointer( self.pointer, _offset )
            return convert_bool_pointer( _p )
        end,
        get_int = function(self,_offset)
            local _p = move_pointer( self.pointer, _offset )
            return convert_int_pointer( _p )
        end
    }
end
function assets.mesh(_path)
    local _index = load_mesh(_path)
    return 
    {
        index = _index,
        vertices_srv = -1,
        indices_srv = -1,
        get = function(self)
            return self.index
        end,
        read_vertices = function(self, _vertices_srv)
            self.vertices_srv = _vertices_srv
            attach_vertices_srv_mesh(self.index, self.vertices_srv)
        end,
        unread_vertices = function(self)
            clean_srv(self.vertices_srv)
            self.vertices_srv = -1
        end,
        read_indices = function(self, _indices_srv)
            self.indices_srv = _indices_srv
            attach_indices_srv_mesh(self.index, self._indices_srv)
        end,
        unread_indices = function(self)
            clean_srv(self.indices_srv)
            self.indices_srv = -1
        end,
        use = function(self)
            use_mesh( self.index )
        end,
        draw = function(self)
            draw_mesh(self.index)
        end,
        draw_instances = function(self, _instances)
            draw_instances_mesh( self.index, _instances )
        end
    }
end
function assets.backbuffer()
    return
    {
        use = function(self, depth)
            depth = depth or nil
            if depth == nil then
                set_rendertarget_backbuffer()
            else
                set_rendertarget_and_depth_backbuffer(depth:get())
            end
        end,
        clear = function(self,r,g,b,a)
            clear_rendertarget_backbuffer(r,g,b,a)
        end
    }
end 

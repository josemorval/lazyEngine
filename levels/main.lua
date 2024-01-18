

function init()

    global_t = get_time()
    delta_t = 3.0 * get_delta_time()
    screenx,screeny = get_window_size()
    main_camera = {
        eye = Vector.new(0.0,0.0,-3.0),
        dir = Vector.new(0.0,0.0,1.0),
        lighteye = Vector.new(1.0,1.0,1.0),
        lightdir = Vector.new(-1.0,-1.0,-1.0),
    
        use = function(self)
            self.eye = Vector.new(4.0*math.cos(0.2*global_t),2.0,4.0*math.sin(0.2*global_t))
            self.dir = Vector.mul(-1.0,Vector.normalize(self.eye))
            self.eye.y = self.eye.y - 1.0

            self.lighteye = Vector.new(1.0,1.0,1.0)
            self.lightdir = Vector.mul(-1.0,Vector.normalize(self.lighteye))
                        
            set_camera(self.eye.x,self.eye.y,self.eye.z,self.dir.x,self.dir.y,self.dir.z)
            set_perspective(1.0,screenx/screeny,0.1,50.0)   
            set_light_camera(self.lighteye.x,self.lighteye.y,self.lighteye.z,self.lightdir.x,self.lightdir.y,self.lightdir.z)
            set_light_orthographic(10.0*screenx/screeny,10.0,0.1,50.0)   
            update_globalconstantbuffer()
        end
    }


    maindepth_texture = create_renderdepth2D(screenx,screeny)
    shadowmap_texture = create_renderdepth2D(screenx,screeny)
    standard_material = create_fx("./shaders/standard.hlsl", false)
    standardshadow_material = create_fx("./shaders/standard_shadow.hlsl", false)    
    geoquad_material = create_fx("./shaders/geoquad_material.hlsl", true)    

    gpuparticle_init = create_computefx("./shaders/compute_particle.cs", "init")
    gpuparticle_udpate = create_computefx("./shaders/compute_particle.cs", "update")
    
    number_particles = 500
    number_particles_drawing = 500

    gpuparticle_buffer = create_buffer(number_particles,36)

    custom_mesh = load_mesh("./models/pig.obj")

    attach_uav_buffer(gpuparticle_buffer,2)
    use_computefx(gpuparticle_init)
    dispatch_computefx(gpuparticle_init,math.ceil(number_particles/64),1,1)
    clean_uav(2)    
end
  
function imgui() 
    
    imgui_setnextwindowsize(400,100)
    imgui_begin("GPU particles")
        local _newvalue = imgui_sliderint("Simulating particles",number_particles,500,1000000)
        if _newvalue ~= number_particles then
            number_particles = _newvalue
            destroy_buffer(gpuparticle_buffer)
            gpuparticle_buffer = create_buffer(number_particles,36)
            attach_uav_buffer(gpuparticle_buffer,2)
            use_computefx(gpuparticle_init)
            dispatch_computefx(gpuparticle_init,math.ceil(number_particles/64),1,1)
            clean_uav(2)  
        end
        number_particles_drawing = imgui_sliderint("Drawing particles",number_particles_drawing,500,1000000)

    imgui_end()

end  

function render()
    global_t = get_time()
    delta_t = 3.0 * get_delta_time()

    use_computefx(gpuparticle_udpate)
    attach_uav_buffer(gpuparticle_buffer,2)
      dispatch_computefx(gpuparticle_udpate,math.ceil(number_particles/64),1,1)
    clean_uav(2)
    
    main_camera:use()

    render_scene()
end

function render_scene()    

    use_viewport(0,0,screenx,screeny)

    clear_depth_renderdepth2D(shadowmap_texture)
    set_depth_renderdepth2D(shadowmap_texture)

    clear_rendertarget_backbuffer(0.1,0.1,0.1,1.0)
    clear_depth_renderdepth2D(maindepth_texture)
    set_rendertarget_and_depth_backbuffer(maindepth_texture)

    set_translation_transform(0,0.0,-1.0,0.0)
    set_rotation_transform(0,0.0,1.0,0.0,0.0)
    set_scale_transform(0,2.0,0.2,2.0)
    update_transformbuffer()
    
    use_rasterizer()
    use_write_depthstencil()
    use_cube()
    use_fx(standard_material)
    draw_instances_cube(1)

    set_translation_transform(0,0.0,0.0,0.0)
    set_rotation_transform(0,0.0,1.0,0.0,global_t)
    set_scale_transform(0,0.5,0.5,0.5)
    update_transformbuffer()
    
    use_wireframe_rasterizer()
    use_write_depthstencil()
    use_mesh(custom_mesh)
    use_fx(standard_material)
    --draw_instances_mesh(custom_mesh,1)

    use_nocull_rasterizer()
    attach_srv_buffer(gpuparticle_buffer,2)
    use_fx(geoquad_material)
    emit_vertex(number_particles_drawing,1)
    clean_srv(2)

end


function cleanup()
end

---------------------
-- Mini vector lib --
---------------------
Vector = {}

function Vector.new(x, y, z)
    local vec = {x = x or 0, y = y or 0, z = z or 0}
    setmetatable(vec, {__index = Vector})
    return vec
end

function Vector.add(a, b)
    a.x = a.x + b.x
    a.y = a.y + b.y
    a.z = a.y + b.y
end

function Vector.substract(a, b)
    return Vector.new(a.x + b.x, a.y + b.y, a.z + b.z)
end

function Vector.normalize(v)
    local lq = math.sqrt(v.x*v.x + v.y*v.y + v.z*v.z)
    return Vector.new(v.x/lq,v.y/lq,v.z/lq)
end

function Vector.mul(s,v)
    local q = Vector.new(0,0,0)
    q.x = s*v.x
    q.y = s*v.y
    q.z = s*v.z
    return q
end
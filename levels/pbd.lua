

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
            self.eye = Vector.new(10.0*math.cos(0.3*global_t),5.0,10.0*math.sin(0.3*global_t))
            self.dir = Vector.mul(-1.0,Vector.normalize(self.eye))
            self.eye.y = self.eye.y - 1.5

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

    standard_material = create_fx("./shaders/standard.hlsl", false)
    billboard_material = create_fx("./shaders/pbd/billboard.hlsl", false)

    gpuparticle_initparticles = create_computefx("./shaders/pbd/compute_particle.cs", "init_particles")
    gpuparticle_initgrid = create_computefx("./shaders/pbd/compute_particle.cs", "init_grid")
    gpuparticle_populategrid = create_computefx("./shaders/pbd/compute_particle.cs", "populate_grid")
    gpuparticle_updateparticles = create_computefx("./shaders/pbd/compute_particle.cs", "update_particles")

    number_particles = 10000
    number_grid_cells = 30*30*30
    
    particle_buffer = create_buffer(number_particles,40)
    grid_buffer = create_buffer(number_grid_cells,208)

    custom_constantbuffer = create_constantbuffer(16*3)
    attach_constantbuffer(custom_constantbuffer,3)
    update_constantbuffer(custom_constantbuffer,0,2.1,1,3.12)

    attach_uav_buffer(particle_buffer,3)
    use_computefx(gpuparticle_initparticles)
    dispatch_computefx(gpuparticle_initparticles,math.ceil(number_particles/64),1,1)
    clean_uav(3)   
       
end
  
function imgui() 

    imgui_begin("GPU Performance")
    local count = gputimer_count()
    
    for i=0,count-2 do
        local name, time = gputimer_value(i)
        imgui_text(tostring(name) .. " " .. string.format("%.2f",time))
    end

    imgui_end()

end  

function render()

    gputimer_begin_profile()

    global_t = get_time()
    delta_t = 1.0 * get_delta_time()

    gputimer_begin_block("Initialize grid")
    use_computefx(gpuparticle_initgrid)
    attach_uav_buffer(grid_buffer,4)
    dispatch_computefx(gpuparticle_initgrid,math.ceil(number_grid_cells/64),1,1)
    clean_uav(4)

    gputimer_begin_block("-")
    use_computefx(gpuparticle_populategrid)
    attach_uav_buffer(particle_buffer,3) attach_uav_buffer(grid_buffer,4)
    dispatch_computefx(gpuparticle_populategrid,math.ceil(number_particles/64),1,1)
    clean_uav(3) clean_uav(4) 


    
    gputimer_begin_block("Update particles")
    for i=1,4 do
    --gputimer_begin_block("Update particles" .. " " .. i)
    use_computefx(gpuparticle_updateparticles)
    attach_uav_buffer(particle_buffer,3) attach_srv_buffer(grid_buffer,4)
    dispatch_computefx(gpuparticle_updateparticles,math.ceil(number_particles/64),1,1)
    clean_uav(3) clean_srv(4)
    end
    
    
    gputimer_begin_block("Render")


    main_camera:use()

    use_viewport(0,0,screenx,screeny)
    use_rasterizer()
    use_write_depthstencil()

    clear_rendertarget_backbuffer(0.1,0.1,0.1,1.0)
    clear_depth_renderdepth2D(maindepth_texture)
    set_rendertarget_and_depth_backbuffer(maindepth_texture)

    set_translation_transform(0,0.0,-5.0,0.0)
    set_rotation_transform(0,0.0,1.0,0.0,0.0)
    set_scale_transform(0,10.0,0.1,10.0)
    update_transformbuffer()

    use_cube()
    use_fx(standard_material)
    draw_instances_cube(1)

    attach_srv_buffer(particle_buffer,3)
    use_fx(billboard_material)
    use_quad()
    draw_instances_quad(number_particles)
    clean_srv(3)

    gputimer_end_profile()

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
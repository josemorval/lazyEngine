require("lualibs.vec")

function init()

    global_t = get_time()
    delta_t = 3.0 * get_delta_time()
    screenx,screeny = get_window_size()
    main_camera = {
        eye = vec.new(0.0,0.0,-3.0),
        dir = vec.new(0.0,0.0,1.0),
        lighteye = vec.new(1.0,1.0,1.0),
        lightdir = vec.new(-1.0,-1.0,-1.0),
    
        use = function(self)
            self.eye = vec.new(1.0,1.0,3.0)
            self.dir = vec.mul(-1.0,vec.normalize(self.eye))
            self.eye.y = self.eye.y - 0.0

            self.lighteye = vec.new(1.0,1.0,1.0)
            self.lightdir = vec.mul(-1.0,vec.normalize(self.lighteye))
                        
            set_camera(self.eye.x,self.eye.y,self.eye.z,self.dir.x,self.dir.y,self.dir.z)
            set_perspective(1.0,screenx/screeny,0.1,50.0)   
            set_light_camera(self.lighteye.x,self.lighteye.y,self.lighteye.z,self.lightdir.x,self.lightdir.y,self.lightdir.z)
            set_light_orthographic(10.0*screenx/screeny,10.0,0.1,50.0)   
            update_globalconstantbuffer()
        end
    }

    maindepth_texture = create_renderdepth2D(screenx,screeny)

    standard_material = create_fx("./shaders/basic/standard.hlsl", false)
    billboard_material = create_fx("./shaders/schain/billboard.hlsl", false)

    gpuparticle_init = create_computefx("./shaders/schain/compute_particle.cs", "init")
    gpuparticle_udpate = create_computefx("./shaders/schain/compute_particle.cs", "update")

    number_particles = 7
    gpuparticle_buffer = create_buffer(number_particles,36,true)

    attach_uav_buffer(gpuparticle_buffer,2)
    use_computefx(gpuparticle_init)
    dispatch_computefx(gpuparticle_init,math.ceil(number_particles/64),1,1)
    clean_uav(2)    
end
  
function imgui() 

    imgui_begin("Panel")
        
        p = get_data_buffer( gpuparticle_buffer )

        for i = 0, 4 do
            p = get_data_buffer( gpuparticle_buffer )
            po = move_pointer( p, 9*4*i )
            _x = convert_float_pointer( po )
            po = move_pointer( po, 4 )
            _y = convert_float_pointer( po )
            po = move_pointer( po, 4 )
            _z = convert_float_pointer( po )
    
            imgui_text( string.format("%.2f %.2f %.2f",_x,_y,_z) )
        end


        imgui_end()

end  

function render()
    global_t = get_time()
    delta_t = get_delta_time()

    use_computefx(gpuparticle_udpate)
    attach_uav_buffer(gpuparticle_buffer,2)
    for i=1,10 do
    dispatch_computefx(gpuparticle_udpate,math.ceil(number_particles/64),1,1)
    end
    clean_uav(2)

    main_camera:use()

    use_viewport(0,0,screenx,screeny)
    use_rasterizer()
    use_write_depthstencil()

    clear_rendertarget_backbuffer(0.1,0.1,0.1,1.0)
    clear_depth_renderdepth2D(maindepth_texture)
    set_rendertarget_and_depth_backbuffer(maindepth_texture)

    set_translation_transform(0,0.0,-1.0,0.0)
    set_rotation_transform(0,0.0,1.0,0.0,0.0)
    set_scale_transform(0,2.0,0.2,2.0)
    update_transformbuffer()

    use_cube()
    use_fx(standard_material)
    draw_instances_cube(1)

    use_nocull_rasterizer()
    attach_srv_buffer(gpuparticle_buffer,2)
    use_fx(billboard_material)
    use_quad()
    draw_instances_quad(number_particles)
    clean_srv(2)
end


function cleanup()
end
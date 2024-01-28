require("lualibs.vec")

function init()

    -- get usual variables
    global_t = get_time()
    delta_t = get_delta_time()
    screenx,screeny = get_window_size()
    
    mouseposx,mouseposy = imgui_getmousepos()
    deltamouseposx = 0
    deltamouseposy = 0

    -- first person camera controller
    main_camera = {
        eye = vec.new(-5.0,2.0,5.0),
        dir = vec.mul(-1.0,vec.normalize(vec.new(-5.0,2.0,5.0))),
        lighteye = vec.new(0.001,1.0,0.001),
        lightdir = vec.new(0.0,0.0,0.0),
        
        update = function(self)
            
            if imgui_ismousepressed(1) then
                local right = transform_view_to_world(1,0,0,0)
                local up = transform_view_to_world(0,1,0,0)
                local forward = transform_view_to_world(0,0,1,0)
            
                -- w key
                if imgui_iskeypressed(568) then
                    self.eye = vec.add(self.eye,vec.mul(0.1,forward))
                end
                -- a key
                if imgui_iskeypressed(546) then
                    self.eye = vec.add(self.eye,vec.mul(-0.1,right))
                end
                -- s key
                if imgui_iskeypressed(564) then
                    self.eye = vec.add(self.eye,vec.mul(-0.1,forward))
                end
                -- d key
                if imgui_iskeypressed(549) then
                    self.eye = vec.add(self.eye,vec.mul(0.1,right))
                end


                if math.abs(deltamouseposx)>1350.0 then deltamouseposx = 0 end
                if math.abs(deltamouseposy)>650.0 then deltamouseposy = 0 end
                    
                self.dir = vec.add(self.dir, vec.mul(deltamouseposx*0.001,right))
                self.dir = vec.add(self.dir, vec.mul(-deltamouseposy*0.001,up))
                self.dir = vec.normalize(self.dir)

                set_camera(self.eye.x,self.eye.y,self.eye.z,self.dir.x,self.dir.y,self.dir.z)

            end
        end,

        use = function(self)
            self.lighteye = vec.new(3.0,5.0,2.0)
            self.lightdir = vec.mul(-1.0,vec.normalize(self.lighteye))
            
            set_camera(self.eye.x,self.eye.y,self.eye.z,self.dir.x,self.dir.y,self.dir.z)
            set_perspective(1.0,screenx/screeny,0.1,50.0)   
            set_light_camera(self.lighteye.x,self.lighteye.y,self.lighteye.z,self.lightdir.x,self.lightdir.y,self.lightdir.z)
            set_light_orthographic(10.0*screenx/screeny,10.0,0.1,50.0)   
            update_globalconstantbuffer()
        end
    }

    -- create depth texture
    maindepth_texture = create_renderdepth2D(screenx,screeny)
    
    -- create shadow map
    shadowmap_texture = create_renderdepth2D(screenx,screeny)

    -- create voxel render target
    number_voxels = 512
    voxel_texture = create_rendertarget3D(number_voxels,number_voxels,number_voxels)
    
    -- camera for voxelization
    voxel_camera = {
        eye = vec.new(0.0,0.0,20.0),
        dir = vec.new(0.0,0.0,-1.0),
        use = function(self)            
            set_camera(self.eye.x,self.eye.y,self.eye.z,self.dir.x,self.dir.y,self.dir.z)
            set_orthographic(20,20,0.1,100.0)   
            update_globalconstantbuffer()
        end
    }

    -- standard (+ shadowmap) material
    standard_material = create_fx("./shaders/gi/standard.hlsl", false)
    standard_shadow_material = create_fx("./shaders/gi/standard_shadow.hlsl", false)
    -- voxelizer + voxel viewer material
    voxelize_material = create_fx("./shaders/gi/voxelize.hlsl", true)
    voxelviewer_material = create_fx("./shaders/gi/voxelviewer.hlsl", false)
    
    -- voxel constant buffer
    voxel_constant_buffer = create_constantbuffer( 16*2 )
    update_constantbuffer(
        voxel_constant_buffer,
        3, 10.0, 4, number_voxels
    )
    attach_constantbuffer(voxel_constant_buffer,3)

    -- create two cube entities. defined below
    cube = create_cube()
    ground = create_cube()


end
  
function imgui() 

end  

-- main logic loop
function render()
    global_t = get_time()
    delta_t = get_delta_time() 
    
    update_mousepos() 

    -----------------------
    -- shadowmap render
    -----------------------
    main_camera:update()
    main_camera:use()

    use_viewport(0,0,screenx,screeny)
    use_rasterizer()
    use_write_depthstencil()

    clear_depth_renderdepth2D(shadowmap_texture)
    set_depth_renderdepth2D(shadowmap_texture)

    use_fx(standard_shadow_material)
    render_scene() 
    
    -----------------------
    -- voxelization
    -----------------------
    voxel_camera:use()

    use_viewport(0,0,number_voxels,number_voxels)
    use_nocull_rasterizer()
    clear_depthstencil()

    --voxelize scene
    clear_rendertarget_rendertarget3D(voxel_texture,0.0,0.0,0.0,0.0)
    set_rendertarget_depth_and_uavs(false,{},{},{voxel_texture},-1,1)

    use_fx(voxelize_material)
    attach_srv_renderdepth2D(shadowmap_texture,0)
    render_scene() 
    clean_srv(0)

    -----------------------
    -- voxelization viewer
    -----------------------
    main_camera:use()
    
    use_viewport(0,0,screenx,screeny)
    use_write_depthstencil()
    
    -- ping-pong between normal/wireframe viz
    e,f = math.modf(0.2*global_t,1.0)
    if f > 0.5 then
        use_rasterizer()
    else
        use_wireframe_rasterizer()
    end
    
    -- sky color
    clear_rendertarget_backbuffer(0.4,0.7,1.0,1.0)
    clear_depth_renderdepth2D(maindepth_texture)
    set_rendertarget_and_depth_backbuffer(maindepth_texture)

    use_fx(voxelviewer_material)
    attach_srv_rendertarget3D(voxel_texture,1)
    use_cube()
    --draw_instances_cube(number_voxels * number_voxels * number_voxels)
    clean_srv(1)

    
end

-- as simple scene
function render_scene()    
    
    cube.rotation = vec.new(math.sin(global_t),math.cos(global_t),math.sin(global_t),0.5)
    cube.scale = vec.new(2.0,2.0,2.0)
    cube.position = vec.new(-2.0,0.0,0.0)
    cube:draw()    


    cube.rotation = vec.new(math.sin(global_t+1.0),math.cos(global_t+1.0),math.sin(global_t+1.0),0.5)
    cube.scale = vec.new(1.0,1.0,1.0)
    cube.position = vec.new(1.0,0.0,2.0)
    cube:draw()  

    cube.rotation = vec.new(math.sin(global_t+1.0),math.cos(global_t+1.0),math.sin(global_t+1.0),0.5)
    cube.scale = vec.new(-1.0,1.0,1.0)
    cube.position = vec.new(1.0,0.0,2.0)
    cube:draw()  

    
    cube.rotation = vec.new(math.sin(global_t+1.0),math.cos(global_t+1.0),math.sin(global_t+1.0),0.5)
    cube.scale = vec.new(-1.0,1.0,1.0)
    cube.position = vec.new(1.0,0.3,-1.0)
    cube:draw()  

    ground.position = vec.new(0.0,-1.0,0.0)
    ground.scale = vec.new(5.0,0.2,5.0)
    ground:draw()

end


function cleanup()
end


---------------------
-- Helpers ----------
---------------------
function create_cube()
    return {
        position = vec.new(0,0,0),
        rotation = vec.new(0,1,0,0),
        scale = vec.new(1,1,1), 
        draw = function(self)
            use_cube()
            set_translation_transform(0,self.position.x,self.position.y,self.position.z)
            set_rotation_transform(0,self.rotation.x,self.rotation.y,self.rotation.z,self.rotation.w)
            set_scale_transform(0,self.scale.x,self.scale.y,self.scale.z)
            update_transformbuffer()
            draw_instances_cube(1)
        end
    }
end

function transform_view_to_world(x,y,z,w)
    local vx,vy,vz,vw = inv_transform_point_camera(x,y,z,w)
    v = vec.new(vx,vy,vz)
    return v
end

function update_mousepos()
    newmouseposx, newmouseposy = imgui_getmousepos()
    deltamouseposx = newmouseposx - mouseposx
    deltamouseposy = newmouseposy - mouseposy
    mouseposx = newmouseposx
    mouseposy = newmouseposy
end
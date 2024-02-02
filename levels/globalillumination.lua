require("lualibs.vec")
require("lualibs.assets")

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
        eye = vec.new(2.5,2.0,0.5),
        dir = vec.new(-0.9,-0.45,-0.21),
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
            self.lighteye = vec.mul(1.0,vec.mul(1.3,vec.new(2.0,2.0,1.0)))
            self.lightdir = vec.mul(-1.0,vec.normalize(self.lighteye))
            
            set_camera(self.eye.x,self.eye.y,self.eye.z,self.dir.x,self.dir.y,self.dir.z)
            set_perspective(1.0,screenx/screeny,0.1,50.0)   
            set_light_camera(self.lighteye.x,self.lighteye.y,self.lighteye.z,self.lightdir.x,self.lightdir.y,self.lightdir.z)
            set_light_orthographic(8.0*screenx/screeny,8.0,0.1,50.0)   
            update_globalconstantbuffer()
        end
    }

    -- create backbuffer texture
    backbuffer = assets.backbuffer()
    
    -- create depth texture
    maindepth = assets.renderdepth2D(screenx,screeny)
    
    -- create shadow map
    shadowmap = assets.renderdepth2D(screenx,screeny)

    -- create voxel render target
    number_voxels = 64
    voxel_texture =  assets.rendertarget3D(number_voxels,number_voxels,number_voxels)
    
    -- camera for voxelization
    voxel_camera = {
        eye = vec.new(0.0,0.0,10.0),
        dir = vec.new(0.0,0.0,-1.0),
        size = 5,
        use = function(self)            
            set_camera(self.eye.x,self.eye.y,self.eye.z,self.dir.x,self.dir.y,self.dir.z)
            set_orthographic(self.size,self.size,0.1,100.0)   
            update_globalconstantbuffer()
        end
    }

    -- standard (+ shadowmap) material
    standard_material = assets.fx("./shaders/gi/standard.hlsl", false)
    standard_shadow_material = assets.fx("./shaders/gi/standard_shadow.hlsl", false)
    -- voxelizer + voxel viewer material
    voxelize_material = assets.fx("./shaders/gi/voxelize.hlsl", true)
    voxelviewer_material = assets.fx("./shaders/gi/voxelviewer.hlsl", false)
    
    -- voxel constant buffer
    voxel_constant_buffer = assets.constantbuffer( 16*2 )
    voxel_constant_buffer:update(3,2.0*voxel_camera.size, 4, number_voxels)
    voxel_constant_buffer:bind(3)

    -- pig model
    sponza_model = create_mesh("./models/sponza.obj")

    -- create two cube entities. defined below
    cube = create_cube()
    sphere = create_sphere()
    

end
  
function imgui() 
    imgui_begin("Inspector")
    imgui_text(string.format("%.2f %.2f %.2f",main_camera.eye.x,main_camera.eye.y,main_camera.eye.z))
    imgui_text(string.format("%.2f %.2f %.2f",main_camera.dir.x,main_camera.dir.y,main_camera.dir.z))
    imgui_end()
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

    shadowmap:clear()
    shadowmap:use()

    standard_shadow_material:use()
    render_scene() 
    
    -----------------------
    -- voxelization
    -----------------------
    voxel_camera:use()

    use_viewport(0,0,number_voxels,number_voxels)
    use_nocull_rasterizer()
    clear_depthstencil()

    --voxelize scene
    voxel_texture:clear(0.0,0.0,0.0,0.0)
    set_rendertarget_depth_and_uavs(false,{},{},{voxel_texture:get()},-1,1)

    voxelize_material:use()
    shadowmap:read(0)
    render_scene() 
    shadowmap:unread()

    --generate_mips_rendertarget3D(voxel_texture)
    -----------------------
    -- voxelization viewer
    -----------------------
    main_camera:use()
    
    use_viewport(0,0,screenx,screeny)
    use_write_depthstencil()
    
    -- sky color
    backbuffer:clear(0.4,0.7,1.0,1.0)
    maindepth:clear()
    backbuffer:use(maindepth)

    local debug = true

    if debug then
        voxelviewer_material:use()
        voxel_texture:read(1)
        use_cube()
        draw_instances_cube(number_voxels*number_voxels*number_voxels)
        voxel_texture:unread()
    end

    -----------------------
    -- gi render
    -----------------------
    
    if not debug then        
        use_viewport(0,0,screenx,screeny)
        use_write_depthstencil()

        standard_material:use()
        shadowmap:read(0)
        voxel_texture:read(1)
        render_scene() 
        shadowmap:unread()
        voxel_texture:unread()
    end
end

-- as simple scene
function render_scene()    
    
    --sponza_model.position = vec.new(0.0,0.0,0.0)
    --sponza_model.scale = vec.new(0.002,0.002,0.002)
    --sponza_model:draw()

    cube.position = vec.new(0.0,0.0,0.0)
    cube.scale = vec.new(2.0,0.1,2.0)
    cube.rotation = vec.new(0.0,1.0,0.0,0.0)
    cube:draw()

    cube.position = vec.new(0.0,2.0,0.0)
    cube.scale = vec.new(2.0,0.1,2.0)
    cube.rotation = vec.new(0.0,1.0,0.0,0.0)
    cube:draw()

    cube.position = vec.new(0.0,1.0,-1.0)
    cube.scale = vec.new(2.0,0.1,2.0)
    cube.rotation = vec.new(1.0,0.0,0.0,1.57)
    cube:draw()

    cube.position = vec.new(0.0,1.0,1.0)
    cube.scale = vec.new(2.0,0.1,2.0)
    cube.rotation = vec.new(1.0,0.0,0.0,1.57)
    cube:draw()

    cube.position = vec.new(-1.0,1.0,0.0)
    cube.scale = vec.new(2.0,0.1,2.0)
    cube.rotation = vec.new(0.0,0.0,1.0,1.57)
    cube:draw()

    sphere.position = vec.new(0.0,0.0,0.0)
    sphere.scale = vec.new(1.0,1.0,1.0)
    sphere.rotation = vec.new(0.0,1.0,0.0,0.0)
    sphere:draw()

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

function create_sphere()
    return {
        position = vec.new(0,0,0),
        rotation = vec.new(0,1,0,0),
        scale = vec.new(1,1,1), 
        draw = function(self)
            use_sphere()
            set_translation_transform(0,self.position.x,self.position.y,self.position.z)
            set_rotation_transform(0,self.rotation.x,self.rotation.y,self.rotation.z,self.rotation.w)
            set_scale_transform(0,self.scale.x,self.scale.y,self.scale.z)
            update_transformbuffer()
            draw_instances_sphere(1)
        end
    }
end

function create_mesh(path)
    return {
        position = vec.new(0,0,0),
        rotation = vec.new(0,1,0,0),
        scale = vec.new(1.0,1.0,1.0), 
        mesh_index = load_mesh(path),
        draw = function(self)
            use_mesh(self.mesh_index)
            set_translation_transform(0,self.position.x,self.position.y,self.position.z)
            set_rotation_transform(0,self.rotation.x,self.rotation.y,self.rotation.z,self.rotation.w)
            set_scale_transform(0,self.scale.x,self.scale.y,self.scale.z)
            update_transformbuffer()
            draw_instances_mesh(self.mesh_index,1)
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
require("lualibs.vec")

function init()

    -- get some system variables
    global_t = get_time()
    delta_t = get_delta_time()
    screenx,screeny = get_window_size()
    
    -- setup a canera together with a directional light
    main_camera = {
        eye = vec.new(0.0,0.0,0.0),
        dir = vec.new(0.0,0.0,0.0),
        lighteye = vec.new(0.0,1.0,0.0),
        lightdir = vec.new(0.0,0.0,0.0),
        use = function(self)
            self.eye = vec.new(-5*math.cos(global_t),2,5*math.sin(global_t))    
            self.dir = vec.mul(-1.0,vec.normalize(self.eye))
            
            self.lighteye = vec.new(10.0,10.0,10.0)
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
    
    -- standard material
    standard_material = create_fx("./shaders/basic/standard.hlsl", false)

    -- create two entities, two cubes. one moving and the other as a ground
    cube = create_cube()
    ground = create_cube()
end
  
function imgui() 
        -- remove/comment if you dont like
        imgui_setnextwindowsize(300,200)
        imgui_begin("Information")
            imgui_textwrapped("This main.lua file is always opened at lazyengine starts. This lua serves as a template for doing things.")
            imgui_end()
end  

-- main logic loop
function render()
    global_t = get_time()
    delta_t = 3.0 * get_delta_time()    
    main_camera:use()
    render_scene()
end

-- here we setup the scene elements to be renderered
function render_scene()    
    use_viewport(0,0,screenx,screeny)

    clear_rendertarget_backbuffer(0.1,0.1,0.1,1.0)
    clear_depth_renderdepth2D(maindepth_texture)
    set_rendertarget_and_depth_backbuffer(maindepth_texture)

    cube:draw()    

    ground.position = vec.new(0.0,-1.0,0.0)
    ground.scale = vec.new(5.0,0.2,5.0)
    ground.rotation = vec.new(0,1,0,0)
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
        rotation = vec.new(0,0,1,0),
        scale = vec.new(1,1,1), 
        draw = function(self)
            use_rasterizer()
            use_fx(standard_material)
            use_cube()
            set_translation_transform(0,self.position.x,self.position.y,self.position.z)
            set_rotation_transform(0,self.rotation.x,self.rotation.y,self.rotation.z,self.rotation.w)
            set_scale_transform(0,self.scale.x,self.scale.y,self.scale.z)
            update_transformbuffer()
            draw_cube()
        end
    }
end

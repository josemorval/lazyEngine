require("lualibs.vec")
require("lualibs.assets")

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

    depth = assets.renderdepth2D(screenx,screeny)
    backbuffer = assets.backbuffer()
    triangledrawer_material = assets.fx("./shaders/trianglesdrawer/trianglesdrawer.hlsl",true)
    pig = assets.mesh("./models/pig.obj")

    number_triangles = 200
    time_profi = 0.0

end
  
function imgui() 
    imgui_begin("Inspector")
    imgui_text(string.format("%s %.2f ms",label,time_profi))
    imgui_text("Triangle count")
        number_triangles=imgui_sliderint( "##Number triangles", number_triangles,1,500000 )
        label, time = gputimer_value( 0 )
        time_profi = 0.1*time + 0.9*time_profi
    imgui_end()
end  

-- main logic loop
function render()
    global_t = get_time()
    delta_t = get_delta_time()    
    
    use_viewport(0,0,screenx,screeny)
    main_camera:use()
    backbuffer:clear(0.0,0.0,0.0,1.0)
    depth:clear()
    backbuffer:use(depth)

    gputimer_begin_profile()
    gputimer_begin_block( "Render" )
    triangledrawer_material:use()
    pig:read_vertices(0)
    emit_vertex(number_triangles,1)
    pig:unread_vertices()

    gputimer_end_profile()


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

function create_quad()
    return {
        position = vec.new(0,0,0),
        rotation = vec.new(0,1,0,0),
        scale = vec.new(1,1,1), 
        draw = function(self)
            use_quad()
            set_translation_transform(0,self.position.x,self.position.y,self.position.z)
            set_rotation_transform(0,self.rotation.x,self.rotation.y,self.rotation.z,self.rotation.w)
            set_scale_transform(0,self.scale.x,self.scale.y,self.scale.z)
            update_transformbuffer()
            draw_instances_quad(1)
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
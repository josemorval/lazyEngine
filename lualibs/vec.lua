vec = {}
function vec.new(x, y, z, w)
    local v = {x or 0, y or 0, z or 0, w or 0}

    local comps = {"x", "y", "z", "w"}

    local mt = {
        __index = function(t, k)
            if type(k) == "string" then
                for i, comp in ipairs(comps) do
                    if comp == k then
                        return t[i]
                    end
                end
            end
        end,

        __newindex = function(t, k, value)
            if type(k) == "string" then
                for i, comp in ipairs(comps) do
                    if comp == k then
                        t[i] = value
                        return
                    end
                end
            end
        end
    }
    setmetatable(v, mt)
    return v
end

function vec.length(v)
    local l = 0
    for i = 1, #v do
        l = l + v[i] * v[i]
    end
    return math.sqrt(l)
end

function vec.normalize(v)
    local l = vec.length(v)
    local u = vec.new(0,0,0,0)
    for i = 1, #v do
        u[i] = v[i] / l
    end
    return u
end

function vec.mul(a, v)
    local u = vec.new(0,0,0,0)
    for i = 1, #v do
        u[i] = a * v[i]
    end
    return u
end

function vec.add(u, v)
    return vec.new(u.x+v.x,u.y+v.y,u.z+v.z,u.w+v.w)
end
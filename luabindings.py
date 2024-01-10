import re

def extract_lua_functions(file_path):
    lua_functions = []
    with open(file_path, 'r') as file:
        content = file.read()
        pattern = re.compile(r'static int (\w+)\(lua_State\* L\)')
        matches = re.findall(pattern, content)
        lua_functions.extend(matches)
    return lua_functions

def generate_push_functions(lua_functions):
    push_functions_code = ''
    push_functions_code = f'list_lua_bindings = "";\n'
    for func_name in lua_functions:
        push_functions_code += f'list_lua_bindings+="{func_name}\\n";\n'
        push_functions_code += f'lua_register(lstate, "{func_name}", {func_name});\n'
    return push_functions_code

if __name__ == "__main__":
    source_file_path = "liblua.h"
    lua_functions = extract_lua_functions(source_file_path)
    push_functions_code = generate_push_functions(lua_functions)

    with open("luabindings_generated.h", 'w') as output_file:
        output_file.write(push_functions_code)
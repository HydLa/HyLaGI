local main_projects = { "core" }
local lib_projects = { "common", "parser", "math_source_converter", "hierarchy", "simulator", "backend", "io",
    "interval", "symbolic_expression", "utility", "debug" }
local test_projects = { "unit_tests" }

local projects = {}

for _, project in ipairs(main_projects) do
    table.insert(projects, project)
end
for _, project in ipairs(lib_projects) do
    table.insert(projects, project)
end
for _, project in ipairs(test_projects) do
    table.insert(projects, project)
end

add_rules("mode.debug", "mode.release")

target("core", function()
    set_default(true)

    set_kind("binary")
    set_filename("hylagi")
    set_targetdir("$(projectdir)/bin")

    add_rules("mathematica")
    add_rules("python")

    if is_mode("debug") then 
        add_cxxflags("-Og")
    end

    lib_dirs = {"backend", "simulator", "hierarchy", "parser", "common", "io", "interval", "debug", "symbolic_expression", "utility"}
    libhydla_libs = {}

    -- add hydla_ prefix to all libraries
    for _, lib in ipairs(lib_dirs) do
        table.insert(libhydla_libs, "hydla_" .. lib)
    end
    
    add_deps(lib_dirs, {inherit = false})

    add_includedirs(lib_dirs)

    add_includedirs("/usr/include/boost")
    add_includedirs("/usr/local/include/boost")

    add_includedirs("backend/mathematica")
    add_includedirs("simulator/symbolic_simulator", "simulator/hybrid_automata")
    add_includedirs("parser/error")

    add_linkdirs("$(buildir)/$(os)/$(arch)/$(mode)")
    add_linkdirs("/usr/local/lib")
    add_linkdirs("/usr/lib")

    add_links(libhydla_libs)

    if is_plat("linux") then
        add_links("boost_program_options")
        add_links("m", "rt")
        if is_arch(".+64") then
            add_links("WSTP64i4")
        else
            add_links("WSTP32i4")
        end
    elseif is_plat("macosx") then
        add_links("boost_program_options-mt")
        add_links("m", "WSTPi4")
    end

    add_defines("COMMIT_ID=\"$(shell git show -s --format=\"%h)\"")
    add_defines("BRANCH_NAME=\"$(shell git rev-parse --abbrev-ref HEAD)\"")

    add_files("core/**.cpp")
end)

target("backend", function()
    set_kind("static")
    add_rules("hydla")

    add_deps("create_math_source", {inherit = false})

    add_includedirs("/usr/local/Wolfram/WolframEngine/13.2/SystemFiles/Links/WSTP/DeveloperKit/Linux-x86-64/CompilerAdditions")
    add_linkdirs("/usr/local/Wolfram/WolframEngine/13.2/SystemFiles/Links/WSTP/DeveloperKit/Linux-x86-64/CompilerAdditions")

    add_includedirs("backend")
    add_includedirs("backend/mathematica")
    add_includedirs("parser", "hierarchy", "simulator", "common", "simulator/symbolic_simulator", "symbolic_expression", "parser/error", "utility", "interval")

    add_files("backend/**.cpp")
    add_files("backend/mathematica/**.cpp")

    on_load(function (target)
        target:add("files", "backend/mathematica/math_source.cpp")
    end)

    after_clean(function (target)
        os.rm("$(projectdir)/src/backend/mathematica/math_source.cpp")
    end)
end)

target("simulator", function()
    set_kind("static")
    add_rules("hydla")

    add_includedirs("parser", "hierarchy", "common", "io", "solver", "backend", "backend/mathematica", "backend/reduce", "interval", "symbolic_expression", "utility", "parser/error")
    add_includedirs("simulator", "simulator/symbolic_simulator", "simulator/hybrid_automata")

    add_links("m", "WSTP64i4", "rt")

    add_files("simulator/**.cpp")
    add_files("simulator/symbolic_simulator/**.cpp")
    add_files("simulator/hybrid_automata/**.cpp")
end)

target("hierarchy", function()
    set_kind("static")
    
    add_rules("hydla")

    add_includedirs("hierarchy", "parser", "symbolic_expression")

    add_includedirs("/usr/include/boost")
    add_includedirs("/usr/local/include/boost")

    add_linkdirs("/usr/local/lib")
    add_linkdirs("/usr/lib")

    add_files("hierarchy/**.cpp")
end)

target("parser", function()
    set_kind("static")
    add_rules("hydla")

    add_includedirs("common", "parser/error", "utility", "symbolic_expression")

    add_includedirs("/usr/include/boost")
    add_includedirs("/usr/local/include/boost")

    add_linkdirs("/usr/local/lib")
    add_linkdirs("/usr/lib")

    add_files("parser/**.cpp")
end)

target("common", function()
    set_kind("static")
    add_rules("hydla")
    add_files("common/**.cpp")
end)

target("io", function()
    set_kind("static")
    add_rules("hydla")

    add_includedirs("io", "common", "parser", "simulator", "hierarchy", "backend", "symbolic_expression", "parser/error", "utility", "interval")

    add_linkdirs("/usr/local/lib")
    add_linkdirs("/usr/lib")

    add_files("io/**.cpp")
end)

target("interval", function()
    set_kind("static")
    add_rules("hydla")

    add_includedirs("interval", "parser", "simulator", "common", "hierarchy", "simulator/symbolic_simulator", "backend", "symbolic_expression", "parser/error")
    add_links("m", "rt")

    add_cxxflags("-DAFFINE_SIMPLE=1")

    add_files("interval/**.cpp")
end)

target("debug", function()
    set_kind("static")
    add_rules("hydla")

    add_rules("python")

    add_includedirs("debug", "parser", "simulator", "common", "hierarchy", "simulator/symbolic_simulator", "backend", "backend/mathematica", "symbolic_expression", "parser/error", "backend/reduce", "io")

    add_links("m", "WSTP64i4", "rt")

    add_cxxflags("-DAFFINE_SIMPLE=1")

    add_files("debug/**.cpp")
end)

target("symbolic_expression", function()
    set_kind("static")
    add_rules("hydla")

    add_includedirs("symbolic_expression", "common", "parser/error", "simulator")
    add_includedirs("/usr/include/boost")
    add_includedirs("/usr/local/include/boost")

    add_linkdirs("/usr/local/lib")
    add_linkdirs("/usr/lib")

    add_files("symbolic_expression/**.cpp")
end)

target("utility", function()
    set_kind("static")
    add_rules("hydla")
    add_files("utility/**.cpp")
end)

target("math_source_converter", function()
    set_kind("binary")
    set_filename("msc")

    set_targetdir("$(buildir)/math_source_converter")

    add_files("math_source_converter/**.cpp")
end)

target("create_math_source", function()
    add_deps("math_source_converter")

    set_targetdir("$(projectdir)/src/backend/mathematica")
    set_filename("math_source.cpp")

    add_files("backend/mathematica/math_source/load_first/*.m")
    add_files("backend/mathematica/math_source/*.m")

    on_build(function (target)
        os.mkdir(target:targetdir())
        
        local temp = path.join(target:targetdir(), "math_source.m")
        local temp_file = io.open(temp, "w")

        -- concat all files
        for _, sourcebatch in pairs(target:sourcebatches()) do
            for _, sourcefile in ipairs(sourcebatch.sourcefiles) do
                local file = io.open(sourcefile, "r")
                temp_file:write(file:read("*all"))
                file:close()
            end
        end

        temp_file:close()
        
        os.execv("$(buildir)/math_source_converter/msc", {"math_source"}, {stdin = temp, stdout = target:targetfile()})
    end)

    after_clean(function (target)
        os.rm("$(projectdir)/src/backend/mathematica/math_source.m")
        os.rm("$(projectdir)/src/backend/mathematica/math_source.cpp")
    end)
end)

rule("mathematica", function () 
    local versions = {"13.2", "13.1", "13.0", "12.3", "12.2", "12.1", "12.0", "11.3", "11.2", "11.1", "11.0"}
    local machine = "x86-64"

    if not is_arch("x86_64") then
        machine = "ARM64"
    end

    local math_path = "/usr/local/Wolfram/WolframEngine/"
    local ver = "13.2"

    if is_plat("macosx") then
        math_path = "/Applications/WolframEngine\\ 2.app/Contents"
        if not os.isdir(math_path) then
            math_path = "/Applications/Mathematica\\ 2.app/Contents"
        end
    elseif is_plat("linux") then
        if not os.isdir(math_path) then
            math_path = "/usr/local/Wolfram/Mathematica/"
        end
    end

    for _, version in ipairs(versions) do
        local path = math_path .. version
        if os.isdir(path) then
            ver = version
            break
        end
    end

    local path = ""
    local cxxflags = ""

    if is_plat("linux") then
        path = math_path .. ver .. "/SystemFiles/Links/WSTP/DeveloperKit/Linux-" .. machine .. "/CompilerAdditions"
        cxxflags = "-Wno-register -Wno-unused-command-line-argument -Wno-unused-variable"
    elseif is_plat("macosx") then
        path = math_path .. ver .. "/SystemFiles/Links/WSTP/DeveloperKit/MacOSX-" .. machine .. "/CompilerAdditions"
        cxxflags = "-framework Foundation -Wno-unused-command-line-argument -Wno-unused-variable"
    end

    on_load(function (target)
        target:add("includedirs", path)
        target:add("linkdirs", path)
        target:add("cxxflags", cxxflags)
    end)
end)

rule("python" , function ()
    on_load(function (target)
        target:add("ldflags", "$(shell python3-config --ldflags --embed)")
        target:add("cxxflags", "$(shell python3-config --includes)")
    end)
end)

rule("hydla", function () 
    on_load(function (target)
        target:add("arflags", "cru")
        target:set("prefixname", "libhydla_")
    end)
end)
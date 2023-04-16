set_project("HydLaGI")
set_version("0.9.5")
set_toolchains("clang")

local src_dir = "src"

set_languages("c++17")

includes(src_dir)

task("test", function()
    on_run(function(target)
        local oldir = os.cd("./system_test")
        os.exec("./system_test.sh")
        os.cd(oldir)
    end)

    set_menu { usage = "xmake test",
        description = "Do the system test." }
end)

task("math-check", function()
    on_run(function(target)
        local oldir = os.cd("./system_test")
        os.exec("./math_check.sh")
        os.cd(oldir)
    end)

    set_menu { usage = "xmake math-check",
        description = "Check the math library." }
end)


task("doc", function()
    on_run(function(target)
        os.exec("doxygen doc/doxygen.conf")
    end)

    set_menu { usage = "xmake doc",
        description = "Generate the document.", }
end)

task("doc-clean", function()
    on_run(function(target)
        os.rm("doc/html")
    end)
    set_menu { usage = "xmake doc-clean",
        description = "Clean the document." }
end)


target("crossfire")
    set_arch("x86")
    set_kind("shared")
    set_languages("c11", "c++23")
    add_defines("UNICODE")
    add_links("user32")
    add_files("src/*.cpp")
    set_symbols("debug")
	add_files("src/thirdparty/*.cpp")
    add_files("vendor/**.cpp")
    add_files("src/thirdparty/*.c")
    add_includedirs("include/")
    add_includedirs("vendor/")
    add_defines("ASMJIT_STATIC")
	add_includedirs("include/thirdparty/")
    after_build(function (target)
        local localbuild = false
        if localbuild then
            os.cp(target:targetfile(), "F:\\csgoded\\csgo\\addons\\")
            os.cp(target:targetfile(), "F:\\csgoclient\\csgo\\addons\\")
        end
        os.cp(target:targetfile(), "./dist/csgo/addons/")
    end)
    after_load(function (target)
        if os.getenv("XMAKE_IN_PROJECT_GENERATOR") then
            target:set("toolchains", "clang")
        end
        for _, toolchain in ipairs(target:toolchains()) do
            toolchain:check()
        end
    end)

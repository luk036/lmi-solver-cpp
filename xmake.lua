set_languages("c++20")

add_rules("mode.debug", "mode.release", "mode.coverage")

if is_mode("release") then
	set_optimize("fastest")
end

add_requires("eigen", { alias = "eigen" })
add_requires("doctest", { alias = "doctest" })
add_requires("fmt", { alias = "fmt" })
add_requires("spdlog", { alias = "spdlog" })

if is_mode("coverage") then
	add_cxflags("-ftest-coverage", "-fprofile-arcs", { force = true })
end

if is_plat("linux") then
    set_warnings("all", "error")
    -- add_cxflags("-Wconversion", {force = true})
    -- add_cxflags("-Wno-unused-command-line-argument", {force = true})
    -- Check if we're on Termux/Android
    local termux_prefix = os.getenv("PREFIX")
    if termux_prefix then
        add_sysincludedirs(termux_prefix .. "/include/c++/v1", {public = true})
        add_sysincludedirs(termux_prefix .. "/include", {public = true})
    end
    -- Enable host-native tuning in release mode for auto-vectorization
    if is_mode("release") then
        add_cxflags("-march=native", "-mtune=native", { force = true })
    end
elseif is_plat("windows") then
    add_cxflags("/EHsc /utf-8 /W4 /WX", { force = true })
    -- Enable AVX2 in release mode for auto-vectorization
    if is_mode("release") then
        add_cxflags("/arch:AVX2", { force = true })
    end
end

-- ellalgo-cpp include
local ellalgo_dir = path.join(os.projectdir(), "../ellalgo-cpp")
local ellalgo_inc = path.join(ellalgo_dir, "include")

target("LmiSolver")
set_kind("static")
add_includedirs("include", { public = true })
add_packages("eigen", { public = true })
add_files("source/*.cpp")

target("EllAlgo")
set_kind("static")
add_includedirs(ellalgo_inc, { public = true })
add_files(path.join(ellalgo_dir, "source/*.cpp"))
add_packages("fmt", "spdlog")
set_group("Dependencies")

target("test_lmi_solver")
set_kind("binary")
add_deps("LmiSolver", "EllAlgo")
add_includedirs("include")
add_includedirs(ellalgo_inc)
add_packages("eigen", "doctest", "fmt", "spdlog")
add_files("test/source/*.cpp")
add_tests("default")

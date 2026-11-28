set_languages("c++20")

add_rules("mode.debug", "mode.release", "mode.coverage")
add_requires("eigen", "doctest", "fmt", "spdlog")

if is_mode("release") then
    set_optimize("fastest")
end

if is_mode("coverage") then
    add_cxflags("-ftest-coverage", "-fprofile-arcs", {force = true})
end

if is_plat("linux") then
    set_warnings("all", "error")
    -- Termux/Android ships its libc++ headers under $PREFIX
    local prefix = os.getenv("PREFIX")
    if prefix then
        add_cxflags("-Wno-unused-command-line-argument", {force = true})
        add_sysincludedirs(prefix .. "/include/c++/v1", prefix .. "/include", {public = true})
    end
    if is_mode("release") then
        add_cxflags("-march=native", "-mtune=native", {force = true})
    end
elseif is_plat("windows") then
    add_cxflags("/EHsc /utf-8 /W4 /WX /wd4702", {force = true})
    if is_mode("release") then
        add_cxflags("/arch:AVX2", {force = true})
    end
end

-- sibling cutting-plane driver
local ellalgo_dir = path.join(os.projectdir(), "../ellalgo-cpp")
local ellalgo_inc = path.join(ellalgo_dir, "include")

target("LmiSolver")
    set_kind("static")
    add_includedirs("include", {public = true})
    add_packages("eigen", {public = true})
    add_files("source/*.cpp")

target("EllAlgo")
    set_kind("static")
    add_includedirs(ellalgo_inc, {public = true})
    add_files(path.join(ellalgo_dir, "source/*.cpp"))
    add_packages("fmt", "spdlog")
    set_group("Dependencies")

target("test_lmi_solver")
    add_deps("LmiSolver", "EllAlgo")
    add_files("test/source/*.cpp")
    add_packages("eigen", "doctest", "fmt", "spdlog")
    add_tests("default")

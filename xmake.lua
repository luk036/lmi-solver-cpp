set_languages("c++17")

add_rules("mode.debug", "mode.release", "mode.coverage")

if is_mode("release") then
    set_optimize("fastest")
end

add_requires("eigen", {alias = "eigen"})
add_requires("doctest", {alias = "doctest"})

if is_mode("coverage") then
    add_cxflags("-ftest-coverage", "-fprofile-arcs", {force = true})
end

if is_plat("linux") then
    set_warnings("all", "error")
elseif is_plat("windows") then
    add_cxflags("/EHsc /utf-8 /W4 /WX /wd4459 /wd4819 /wd4996 /wd4267", {force = true})
end

-- Local CMake deps (offline-friendly)
local deps = path.join(os.projectdir(), "build/_deps")
local mode_dir = is_mode("release") and "Release" or "Debug"

-- Header-only: doctest
local doctest_dir = path.join(deps, "doctest-src")
local doctest_h = path.join(doctest_dir, "doctest", "doctest.h")

-- Compiled: fmt
local fmt_dir = path.join(deps, "fmt-src")
local fmt_lib_dir = path.join(deps, "fmt-build", mode_dir)

-- Compiled: spdlog
local spdlog_dir = path.join(deps, "spdlog-src")

-- ellalgo-cpp include
local ellalgo_inc = path.join(os.projectdir(), "../ellalgo-cpp/include")
local ellalgo_src_inc = path.join(deps, "ellalgo-src/include")

target("LmiSolver")
    set_kind("static")
    add_includedirs("include", {public = true})
    add_packages("eigen", {public = true})
    add_files("source/*.cpp")

target("EllAlgo")
    set_kind("static")
    add_includedirs(ellalgo_src_inc, {public = true})
    add_includedirs(ellalgo_inc, {public = true})
    add_includedirs(path.join(fmt_dir, "include"), {public = true})
    add_includedirs(path.join(spdlog_dir, "include"), {public = true})
    add_files(path.join(deps, "ellalgo-src/source/*.cpp"))
    set_group("Dependencies")

target("test_lmi_solver")
    set_kind("binary")
    add_deps("LmiSolver", "EllAlgo")
    add_includedirs("include")
    add_includedirs(ellalgo_inc)
    add_includedirs(path.join(fmt_dir, "include"))
    add_packages("eigen")
    if os.isfile(doctest_h) then
        add_includedirs(path.join(doctest_dir))
    end
    add_files("test/source/*.cpp")
    add_linkdirs(fmt_lib_dir)
    add_links("fmt")
    add_tests("default")

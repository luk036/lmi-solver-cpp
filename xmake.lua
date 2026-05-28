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

target("LmiSolver")
    set_kind("static")
    add_includedirs("include", {public = true})
    add_packages("eigen", {public = true})
    add_files("source/*.cpp")

target("test_lmi_solver")
    set_kind("binary")
    add_deps("LmiSolver")
    add_includedirs("include")
    add_packages("eigen")
    add_packages("doctest")
    add_files("test/source/*.cpp")
    remove_files("test/source/test_lmi_integration.cpp")
    remove_files("test/source/test_lmi_old.cpp")
    add_tests("default")

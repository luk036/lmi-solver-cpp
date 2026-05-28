{
    files = {
        [[build\.objs\test_lmi_solver\windows\x64\release\test\source\main.cpp.obj]],
        [[build\.objs\test_lmi_solver\windows\x64\release\test\source\test_lmi.cpp.obj]],
        [[build\windows\x64\release\LmiSolver.lib]]
    },
    values = {
        [[C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Tools\MSVC\14.38.33130\bin\HostX64\x64\link.exe]],
        {
            "-nologo",
            "-dynamicbase",
            "-nxcompat",
            "-machine:x64",
            [[-libpath:build\windows\x64\release]],
            "/opt:ref",
            "/opt:icf",
            "LmiSolver.lib"
        }
    }
}
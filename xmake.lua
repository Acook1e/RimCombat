set_xmakever("3.0.5")

PROJECT_NAME = "RimCombat"

set_project(PROJECT_NAME)
set_version("0.0.1")
set_languages("cxx23")
set_toolchains("clang-cl")

includes("extern/CommonLibSSE-NG")

add_rules("mode.debug", "mode.release")

add_defines("UNICODE", "_UNICODE")

if is_mode("debug") then
    set_optimize("none")
    add_defines("DEBUG")
elseif is_mode("release") then
    set_optimize("fastest")
    add_defines("NDEBUG")
    set_symbols("debug")
end

add_requires("simpleini")
add_requires("spdlog", { configs = { header_only = false, wchar = true, std_format = true } })

target(PROJECT_NAME)

    add_deps("commonlibsse-ng")
    add_rules("commonlibsse-ng.plugin", {
        name = PROJECT_NAME,
        author = "Acook1e",
        description = "Rim Combat - Overhaul Combat Experience"
    })

    add_packages("spdlog")
    add_packages("simpleini")

    add_includedirs("src/include/")
    set_pcxxheader("src/include/pch.h")
    add_headerfiles("src/include/**.h")
    add_files("src/**.cpp")
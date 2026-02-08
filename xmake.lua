set_xmakever("3.0.5")

PROJECT_NAME = "RimCombat"

set_project(PROJECT_NAME)
set_version("1.0.0")
set_languages("cxx23")
set_toolchains("clang-cl")

add_defines("WIN32_LEAN_AND_MEAN", "NOMINMAX")
add_defines("UNICODE", "_UNICODE")

includes("extern/CommonLibSSE-NG")

add_rules("mode.debug", "mode.release")

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
        description = "Rim Combat - Overhaul Combat Experience",
        options = {
            address_library = true,
            signature_scanning = false,
            struct_dependent = false
        }
    })

    add_packages("spdlog")
    add_packages("simpleini")

    add_files("src/**.cpp")
    add_includedirs("include/")
    add_headerfiles("include/**.h")
    set_pcxxheader("include/PCH.h")
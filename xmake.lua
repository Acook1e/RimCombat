set_xmakever("3.0.5")

PROJECT_NAME = "RimCombat"

set_project(PROJECT_NAME)
set_version("0.4.1")
set_languages("cxx23")
set_toolchains("clang-cl")

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

add_requires("nlohmann_json", "magic_enum", "microsoft-detours")
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

    add_packages("nlohmann_json")
    add_packages("magic_enum")
    add_packages("microsoft-detours")
    add_packages("spdlog")

    add_files("src/**.cpp")
    add_includedirs("include/")
    add_headerfiles("include/**.h")
    set_pcxxheader("include/PCH.h")

    after_build(function (target)
        os.vcp(target:targetfile(), "dist/Core/SKSE/Plugins/")
        os.vcp(target:symbolfile(), "dist/Core/SKSE/Plugins/")

        import("utils.archive")
        local dist_dir = path.absolute("dist")
        local build_dir = path.absolute("build")
        local zip_name = target:name() .. "-" .. (target:version() or "1.0.0") .. ".7z"
        local zip_path = path.join(build_dir, zip_name)

        local old = os.cd(dist_dir)
        local files = os.files("**")
        os.cd(old)

        os.tryrm(zip_path)
        archive.archive(zip_path, files, { curdir = dist_dir })
        cprint("${dim}packaged %s", zip_name)
    end)

add_rules("mode.debug", "mode.release")
add_repositories("k_xmake_repo https://github.com/kociumba/k_xmake_repo.git")
set_languages("cxxlatest")
includes("@builtin/xpack")

add_requires("libuiohook", {configs = {shared = true}})
add_requires("imgui", {configs = {opengl3 = true, sdl3 = true, freetype = true}})
add_requires("opengl", "screen_capture_lite", "clip", "magic_enum", "nlohmann_json")

target("kadr")
    set_kind("binary")
    add_files("src/**.cpp", "assets/resources.rc")
    add_headerfiles("src/**.h")
    add_packages("libuiohook",
        "imgui",
        "opengl",
        "screen_capture_lite",
        "clip",
        "magic_enum",
        "nlohmann_json")

    add_extrafiles("assets/**")

    if is_plat("windows") and is_mode("release") then 
        add_rules("win.sdk.application")
        add_ldflags("/ENTRY:mainCRTStartup", {force = true})
    end

    after_build(function (target)
            local outdir = target:targetdir()
            local srcdir = path.join(os.projectdir(), "assets")

            os.tryrm(path.join(outdir, "assets"))
            if os.isdir(srcdir) then
                cprint("copying assets -> %s", outdir)
                os.cp(srcdir, outdir)
            end
        end)

xpack("kadr")
    set_title("kadr")
    set_description("")
    set_author("kociumba")

    set_formats("zip", "targz")

    set_bindir("kadr")
    add_targets("kadr")
    add_installfiles("assets/**.png", {prefixdir = "kadr/assets"})

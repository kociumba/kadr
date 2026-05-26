add_rules("mode.debug", "mode.release")
-- set_policy("build.progress_style", "multirow")
add_repositories("k_xmake_repo https://github.com/kociumba/k_xmake_repo.git")
set_languages("cxxlatest")
includes("@builtin/xpack")

add_requires("libuiohook @default", { configs = { shared = true } })
add_requires("imgui v1.92.7", { configs = { opengl3 = true, sdl3 = true, freetype = true } })
add_requires(
    "opengl",
    "screen_capture_lite 17.1.2745",
    "clip v1.15",
    "magic_enum v0.9.7",
    "nlohmann_json v3.12.0",
    "SDL3_mixer @default")

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
        "nlohmann_json",
        "SDL3_mixer")

    -- for now will have to serve for allowing clangd to work
    if is_mode("debug") and os.getenv("ZED_TERM") then
        set_toolchains("clang")
    end

    add_extrafiles("assets/**")

    if is_plat("windows") and is_mode("release") then
        add_rules("win.sdk.application")
        add_ldflags("/ENTRY:mainCRTStartup", { force = true })
    end

    on_load( function (target)
          local name = path.filename(target:targetfile())
          target:add("defines", "KADR_EXEC=\"" .. name .. "\"")
        end)

    after_build( function (target)
            local outdir = target:targetdir()
            local srcdir = path.join(os.projectdir(), "assets")

            os.tryrm(path.join(outdir, "assets"))
            if os.isdir(srcdir) then
                cprint("copying assets -> %s", outdir)
                os.cp(srcdir, outdir)
            end
        end)

    before_build( function (target)
        local git_hash = os.iorun("git rev-parse --short HEAD"):trim()
        local git_count = os.iorun("git rev-list --count HEAD"):trim()
        local git_branch = os.iorun("git rev-parse --abbrev-ref HEAD"):trim()
        local is_dirty = os.iorun("git status --porcelain"):trim() ~= ""

        local version_tag = "0.0.0"
            local tag_result = try
            {
                function ()
                    return os.iorun("git describe --tags --abbrev=0"):trim()
                end
            }
            if tag_result and tag_result:startswith("v") then
                version_tag = tag_result:sub(2)
            end

        local build_file = path.join(target:autogendir(), ".build_cache")
        local build_num = 1
        if os.isfile(build_file) then
            build_num = tonumber(io.readfile(build_file)) or 1
        end
        build_num = build_num + 1
        io.writefile(build_file, tostring(build_num))

        target:add("defines", "VERSION_MAJOR=" .. (version_tag:match("^(%d+)") or "0"))
        target:add("defines", "VERSION_MINOR=" .. (version_tag:match("^%d+%.(%d+)") or "0"))
        target:add("defines", "VERSION_PATCH=" .. (version_tag:match("^%d+%.%d+%.(%d+)") or "0"))

        target:add("defines", string.format('VERSION_TAG="%s"', version_tag))
        target:add("defines", string.format('GIT_HASH="%s"', git_hash))
        target:add("defines", string.format('GIT_COMMITS="%s"', git_count))
        target:add("defines", string.format('BUILD_NUMBER="%s"', build_num))
        target:add("defines", string.format('GIT_BRANCH="%s"', git_branch))

        if is_dirty then
            target:add("defines", "GIT_DIRTY=1")
        end

        local suffix = is_dirty and "-dirty" or ""
        local full_version = string.format("%s+%s.%s+%s%s",
            version_tag, git_count, git_hash, build_num, suffix)

        target:add("defines", string.format('BUILD_IDENTIFIER="%s"', full_version))
    end)

xpack("kadr")
    set_title("kadr")
    set_description("")
    set_author("kociumba")

    if is_plat("windows") then
        set_formats("zip")
    else
        set_formats("targz")
    end

    set_bindir("kadr")
    add_targets("kadr")
    add_installfiles(
        "assets/kadr_icon.png",
        "assets/shutter.wav",
        "assets/Geist-VariableFont_wght.ttf",
        { prefixdir = "kadr/assets" })

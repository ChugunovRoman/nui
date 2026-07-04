#!/usr/bin/env python3
"""
Generate Visual Studio .vcxproj files for SDL2, SDL_ttf, FreeType, pugixml.
Usage: python tools/gen_vcxproj.py
"""

import os
import glob

SCRIPT_DIR = os.path.dirname(os.path.abspath(__file__))
ROOT_DIR = os.path.dirname(SCRIPT_DIR)
EXTERNALS = os.path.join(ROOT_DIR, "Externals")

GUID_SDL2     = "{AAAA1111-1111-1111-1111-111111111111}"
GUID_SDL2MAIN = "{AAAA2222-2222-2222-2222-222222222222}"
GUID_FREETYPE = "{AAAA3333-3333-3333-3333-333333333333}"
GUID_SDLTTF   = "{AAAA4444-4444-4444-4444-444444444444}"
GUID_PUGIXML  = "{AAAA5555-5555-5555-5555-555555555555}"

VCXPROJ_HEADER = """\
<?xml version="1.0" encoding="utf-8"?>
<Project DefaultTargets="Build" xmlns="http://schemas.microsoft.com/developer/msbuild/2003">
  <ItemGroup Label="ProjectConfigurations">
    <ProjectConfiguration Include="Debug|Win32"><Configuration>Debug</Configuration><Platform>Win32</Platform></ProjectConfiguration>
    <ProjectConfiguration Include="Release|Win32"><Configuration>Release</Configuration><Platform>Win32</Platform></ProjectConfiguration>
    <ProjectConfiguration Include="Debug|x64"><Configuration>Debug</Configuration><Platform>x64</Platform></ProjectConfiguration>
    <ProjectConfiguration Include="Release|x64"><Configuration>Release</Configuration><Platform>x64</Platform></ProjectConfiguration>
  </ItemGroup>
  <PropertyGroup Label="Globals">
    <VCProjectVersion>17.0</VCProjectVersion>
    <ProjectGuid>{guid}</ProjectGuid>
    <RootNamespace>{name}</RootNamespace>
    <WindowsTargetPlatformVersion>10.0</WindowsTargetPlatformVersion>
  </PropertyGroup>
  <Import Project="$(VCTargetsPath)\\Microsoft.Cpp.Default.props" />
"""

VCXPROJ_CONFIG = """\
  <PropertyGroup Condition="'$(Configuration)|$(Platform)'=='{config}|{platform}'" Label="Configuration">
    <ConfigurationType>{type}</ConfigurationType>
    <UseDebugLibraries>{debug}</UseDebugLibraries>
    <PlatformToolset>v143</PlatformToolset>
    <CharacterSet>Unicode</CharacterSet>
    <WholeProgramOptimization>{wpo}</WholeProgramOptimization>
  </PropertyGroup>
"""

VCXPROJ_COMPILE = """\
  <ItemDefinitionGroup Condition="'$(Configuration)|$(Platform)'=='{config}|{platform}'">
    <ClCompile>
      <WarningLevel>Level3</WarningLevel>
      <SDLCheck>true</SDLCheck>
      <PreprocessorDefinitions>{defines}%(PreprocessorDefinitions)</PreprocessorDefinitions>
      <ConformanceMode>true</ConformanceMode>
      <LanguageStandard>stdcpp17</LanguageStandard>
      <AdditionalIncludeDirectories>{includes}%(AdditionalIncludeDirectories)</AdditionalIncludeDirectories>
      <RuntimeLibrary>{rtlib}</RuntimeLibrary>
      <MultiProcessorCompilation>true</MultiProcessorCompilation>
      <AdditionalOptions>{extra_opts} %(AdditionalOptions)</AdditionalOptions>
    </ClCompile>
    <Lib>
      <AdditionalLibraryDirectories>%(AdditionalLibraryDirectories)</AdditionalLibraryDirectories>
    </Lib>
  </ItemDefinitionGroup>
"""

VCXPROJ_FOOTER = """\
  <Import Project="$(VCTargetsPath)\\Microsoft.Cpp.targets" />
  <ImportGroup Label="ExtensionTargets" />
</Project>
"""

FILTERS_HEADER = """\
<?xml version="1.0" encoding="utf-8"?>
<Project ToolsVersion="4.0" xmlns="http://schemas.microsoft.com/developer/msbuild/2003">
"""

def find_files(base, patterns):
    """Find files matching patterns relative to base."""
    files = []
    for pat in patterns:
        full = os.path.join(base, pat)
        files.extend(glob.glob(full))
    return sorted(set(os.path.relpath(f, base).replace("/", "\\") for f in files))

def make_filter_name(filepath):
    """Get directory part for filters."""
    parts = filepath.replace("/", "\\").split("\\")
    return "\\".join(parts[:-1]) if len(parts) > 1 else ""

def write_vcxproj(path, name, guid, config_type, includes, defines, sources, extra_opts=""):
    """Write a .vcxproj file."""
    configs = ["Debug|Win32", "Release|Win32", "Debug|x64", "Release|x64"]

    with open(path, "w", encoding="utf-8") as f:
        f.write(VCXPROJ_HEADER.format(guid=guid, name=name))

        for cfg in configs:
            config, platform = cfg.split("|")
            is_debug = config == "Debug"
            f.write(VCXPROJ_CONFIG.format(
                config=config, platform=platform,
                type=config_type,
                debug="true" if is_debug else "false",
                wpo="false" if is_debug else "true"
            ))

        f.write('  <Import Project="$(VCTargetsPath)\\Microsoft.Cpp.props" />\n')

        # Output directories - match main project output
        for cfg in configs:
            config, platform = cfg.split("|")
            f.write(f'  <PropertyGroup Condition="\'$(Configuration)|$(Platform)\'==\'{config}|{platform}\'">\n')
            f.write(f'    <OutDir>$(SolutionDir)bin\\$(Configuration)\\$(Platform)\\</OutDir>\n')
            f.write(f'    <IntDir>$(SolutionDir)obj\\$(Configuration)\\{name}\\</IntDir>\n')
            f.write(f'  </PropertyGroup>\n')

        for cfg in configs:
            config, platform = cfg.split("|")
            is_debug = config == "Debug"
            rtlib = "MultiThreadedDebug" if is_debug else "MultiThreaded"
            defs = defines + ("_DEBUG;" if is_debug else "NDEBUG;")
            if config_type == "StaticLibrary":
                defs += "_LIB;"
            f.write(VCXPROJ_COMPILE.format(
                config=config, platform=platform,
                defines=defs,
                includes=includes,
                rtlib=rtlib,
                extra_opts=extra_opts
            ))

        # Source files
        f.write('  <ItemGroup>\n')
        for src in sources:
            f.write(f'    <ClCompile Include="{src}" />\n')
        f.write('  </ItemGroup>\n')

        f.write(VCXPROJ_FOOTER)

def write_filters(path, sources):
    """Write a .vcxproj.filters file."""
    # Collect unique filter directories
    filters = set()
    for src in sources:
        fn = make_filter_name(src)
        if fn:
            filters.add(fn)

    with open(path, "w", encoding="utf-8") as f:
        f.write(FILTERS_HEADER)

        # Filter definitions
        f.write('  <ItemGroup>\n')
        for flt in sorted(filters):
            f.write(f'    <Filter Include="{flt}"><UniqueIdentifier>{{{flt}}}</UniqueIdentifier></Filter>\n')
        f.write('  </ItemGroup>\n')

        # Source file assignments
        f.write('  <ItemGroup>\n')
        for src in sources:
            fn = make_filter_name(src)
            if fn:
                f.write(f'    <ClCompile Include="{src}"><Filter>{fn}</Filter></ClCompile>\n')
            else:
                f.write(f'    <ClCompile Include="{src}" />\n')
        f.write('  </ItemGroup>\n')

        f.write('</Project>\n')

# ── SDL2 ────────────────────────────────────────────────────────

def gen_sdl2():
    sdl2 = os.path.join(EXTERNALS, "SDL2")
    src = os.path.join(sdl2, "src")

    # Minimal sources for window + input + software rendering
    patterns = [
        "SDL*.c",
        "dynapi/SDL_dynapi.c",
        "atomic/SDL_*.c",
        "audio/SDL_*.c",
        "audio/directsound/SDL_*.c",
        "audio/disk/SDL_*.c",
        "audio/dummy/SDL_*.c",
        "audio/wasapi/SDL_*.c",
        "audio/winmm/SDL_*.c",
        "core/windows/SDL_*.c",
        "cpuinfo/SDL_*.c",
        "dynapi/SDL_*.c",
        "render/SDL_*.c",
        "render/direct3d/SDL_*.c",
        "render/direct3d11/SDL_*.c",
        "render/direct3d12/SDL_*.c",
        "render/metal/SDL_*.c",
        "render/opengl/SDL_*.c",
        "render/opengles/SDL_*.c",
        "render/opengles2/SDL_*.c",
        "render/psp/SDL_*.c",
        "render/software/SDL_*.c",
        "render/vitagxm/SDL_*.c",
        "render/vulkan/SDL_*.c",
        "events/SDL_*.c",
        "file/SDL_*.c",
        "filesystem/windows/SDL_*.c",
        "haptic/SDL_*.c",
        "haptic/windows/SDL_*.c",
        "hidapi/SDL_*.c",
        "hidapi/windows/*.c",
        "joystick/SDL_*.c",
        "joystick/controller_type.c",
        "joystick/hidapi/SDL_*.c",
        "joystick/virtual/SDL_*.c",
        "joystick/windows/SDL_*.c",
        "libm/*.c",
        "loadso/windows/SDL_*.c",
        "locale/SDL_*.c",
        "locale/windows/SDL_*.c",
        "main/windows/SDL_*.c",
        "misc/SDL_*.c",
        "misc/windows/SDL_*.c",
        "power/SDL_*.c",
        "power/windows/SDL_*.c",
        "sensor/SDL_*.c",
        "sensor/dummy/SDL_*.c",
        "sensor/windows/SDL_*.c",
        "stdlib/SDL_*.c",
        "thread/SDL_*.c",
        "thread/windows/SDL_*.c",
        "thread/generic/SDL_syscond.c",
        "timer/SDL_*.c",
        "timer/windows/SDL_*.c",
        "video/SDL_*.c",
        "video/dummy/SDL_*.c",
        "video/windows/SDL_*.c",
        "video/yuv2rgb/*.c",
    ]

    sources = find_files(src, patterns)
    # Paths relative to SDL2_proj/ (where .vcxproj lives) -> ../SDL2/src/
    sources = ["..\\SDL2\\src\\" + s for s in sources]

    includes = f"..\\SDL2\\include;"
    defines = (
        "SDL_STATIC_LIB=1;"
        "SDL_BUILD_MAJOR_VERSION=2;"
        "SDL_BUILD_MINOR_VERSION=30;"
        "SDL_BUILD_MICRO_VERSION=12;"
        "DLL_EXPORT=0;"
        "_CRT_SECURE_NO_WARNINGS;"
        "HAVE_LIBC=1;"
    )

    proj_dir = os.path.join(EXTERNALS, "SDL2_proj")
    os.makedirs(proj_dir, exist_ok=True)

    write_vcxproj(
        os.path.join(proj_dir, "SDL2.vcxproj"),
        "SDL2", GUID_SDL2, "StaticLibrary",
        includes, defines, sources
    )
    write_filters(
        os.path.join(proj_dir, "SDL2.vcxproj.filters"),
        sources
    )

    # SDL2main
    main_sources = ["..\\SDL2\\src\\main\\windows\\SDL_windows_main.c"]
    write_vcxproj(
        os.path.join(proj_dir, "SDL2main.vcxproj"),
        "SDL2main", GUID_SDL2MAIN, "StaticLibrary",
        includes, defines, main_sources
    )
    write_filters(
        os.path.join(proj_dir, "SDL2main.vcxproj.filters"),
        main_sources
    )

    print(f"[gen] SDL2: {len(sources)} files -> SDL2_proj/SDL2.vcxproj")
    print(f"[gen] SDL2main: 1 file -> SDL2_proj/SDL2main.vcxproj")

# ── FreeType ────────────────────────────────────────────────────

def gen_freetype():
    ft = os.path.join(EXTERNALS, "SDL_ttf", "external", "freetype")
    src = os.path.join(ft, "src")

    patterns = [
        "autofit/autofit.c",
        "bdf/bdf.c",
        "cache/ftcache.c",
        "cff/cff.c",
        "cid/type1cid.c",
        "gxvalid/gxvalid.c",
        # gzip disabled: vendored zlib has build issues in vcxproj
        # "gzip/ftgzip.c",
        "lzw/ftlzw.c",
        "otvalid/otvalid.c",
        "pcf/pcf.c",
        "pfr/pfr.c",
        "psaux/psaux.c",
        "pshinter/pshinter.c",
        "psnames/psnames.c",
        "raster/raster.c",
        "sfnt/sfnt.c",
        "smooth/smooth.c",
        "truetype/truetype.c",
        "type1/type1.c",
        "type42/type42.c",
        "winfonts/winfnt.c",
        "sdf/sdf.c",
        "sdf/ftbsdf.c",
        "sdf/ftsdf.c",
        "sdf/ftsdfcommon.c",
        "sdf/ftsdfrend.c",
        "svg/svg.c",
        "svg/ftsvg.c",
        "base/ftbase.c",
        "base/ftbbox.c",
        "base/ftbdf.c",
        "base/ftbitmap.c",
        "base/ftcid.c",
        "base/ftdebug.c",
        "base/ftfstype.c",
        "base/ftgasp.c",
        "base/ftglyph.c",
        "base/ftgxval.c",
        "base/ftinit.c",
        "base/ftmm.c",
        "base/ftotval.c",
        "base/ftpfr.c",
        "base/ftstroke.c",
        "base/ftsynth.c",
        "base/ftsystem.c",
        "base/fttype1.c",
        "base/ftwinfnt.c",
    ]

    sources = [s.replace("/", "\\") for s in patterns]
    # Paths relative to SDL_ttf_proj/ -> ../SDL_ttf/external/freetype/src/
    sources = ["..\\SDL_ttf\\external\\freetype\\src\\" + s for s in sources]
    # Add gzip stub (FreeType references gzip functions but we don't use them)
    sources.append("ftgzip_stub.c")

    includes = "..\\SDL_ttf\\external\\freetype\\include;..\\SDL_ttf\\external\\freetype\\src\\gzip;"
    defines = (
        "FT2_BUILD_LIBRARY=1;"
        "_CRT_SECURE_NO_WARNINGS;"
        "FT_CONFIG_OPTION_SYSTEM_ZLIB=0;"
    )
    # Undefine ZLIB support (ftoption.h defines it but we don't have working zlib)
    extra_compile_options = "/UFT_CONFIG_OPTION_USE_ZLIB"

    proj_dir = os.path.join(EXTERNALS, "SDL_ttf_proj")
    os.makedirs(proj_dir, exist_ok=True)

    write_vcxproj(
        os.path.join(proj_dir, "freetype.vcxproj"),
        "freetype", GUID_FREETYPE, "StaticLibrary",
        includes, defines, sources,
        extra_opts=extra_compile_options
    )
    write_filters(
        os.path.join(proj_dir, "freetype.vcxproj.filters"),
        sources
    )

    print(f"[gen] FreeType: {len(sources)} files -> SDL_ttf_proj/freetype.vcxproj")

# ── SDL_ttf ─────────────────────────────────────────────────────

def gen_sdl_ttf():
    sdlttf = os.path.join(EXTERNALS, "SDL_ttf")
    sdl2 = os.path.join(EXTERNALS, "SDL2")
    ft = os.path.join(sdlttf, "external", "freetype")

    sources = ["..\\SDL_ttf\\SDL_ttf.c"]
    includes = "..\\SDL2\\include;..\\SDL_ttf\\external\\freetype\\include;..\\SDL_ttf;"
    defines = (
        "_CRT_SECURE_NO_WARNINGS;"
        "SDL_TTF_USE_HARFBUZZ=0;"
    )

    proj_dir = os.path.join(EXTERNALS, "SDL_ttf_proj")
    os.makedirs(proj_dir, exist_ok=True)

    write_vcxproj(
        os.path.join(proj_dir, "SDL_ttf.vcxproj"),
        "SDL_ttf", GUID_SDLTTF, "StaticLibrary",
        includes, defines, sources
    )
    write_filters(
        os.path.join(proj_dir, "SDL_ttf.vcxproj.filters"),
        sources
    )

    print(f"[gen] SDL_ttf: {len(sources)} file -> SDL_ttf_proj/SDL_ttf.vcxproj")

# ── pugixml ─────────────────────────────────────────────────────

def gen_pugixml():
    pugi = os.path.join(EXTERNALS, "pugixml")

    sources = ["..\\pugixml\\src\\pugixml.cpp"]
    includes = "..\\pugixml\\src;"
    defines = "_CRT_SECURE_NO_WARNINGS;PUGIXML_STATIC_CRT=1;"

    proj_dir = os.path.join(EXTERNALS, "pugixml_proj")
    os.makedirs(proj_dir, exist_ok=True)

    write_vcxproj(
        os.path.join(proj_dir, "pugixml.vcxproj"),
        "pugixml", GUID_PUGIXML, "StaticLibrary",
        includes, defines, sources
    )
    write_filters(
        os.path.join(proj_dir, "pugixml.vcxproj.filters"),
        sources
    )

    print(f"[gen] pugixml: {len(sources)} file -> pugixml_proj/pugixml.vcxproj")

# ── Main ────────────────────────────────────────────────────────

if __name__ == "__main__":
    print("Generating .vcxproj files for Externals...\n")
    gen_sdl2()
    gen_freetype()
    gen_sdl_ttf()
    gen_pugixml()
    print("\nDone! Open nui.sln to build.")

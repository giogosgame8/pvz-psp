"""Build the native PSP EBOOT with PSPDEV; works without GNU make on Windows."""
from pathlib import Path
import argparse
import os
import subprocess

ROOT = Path(__file__).resolve().parents[1]


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("--pspdev", type=Path, default=os.environ.get("PSPDEV"))
    parser.add_argument("--msys-bin", type=Path)
    launcher = parser.add_mutually_exclusive_group()
    launcher.add_argument("--launcher-media", type=Path, help="Directory containing ICON0.PNG, PIC1.PNG and SND0.AT3")
    launcher.add_argument("--no-launcher-media", action="store_true", help="Build without the optional personal XMB media")
    memory = parser.add_mutually_exclusive_group()
    memory.add_argument("--low-memory", action="store_true", help="Force the smaller-cache / 30 FPS profile on every model for diagnostics; normal builds auto-detect PSP-1000")
    memory.add_argument("--psp1000-60fps-test", action="store_true", help="Experimental: keep PSP-1000's smaller caches but target 60 FPS; leaves the normal build intact")
    args = parser.parse_args()
    if not args.pspdev:
        parser.error("Set PSPDEV or pass --pspdev")
    dev = args.pspdev.resolve()
    sdk = dev / "psp" / "sdk"
    env = os.environ.copy()
    env["PSPDEV"] = str(dev)
    env["PATH"] = str(dev / "bin") + os.pathsep + (str(args.msys_bin) + os.pathsep if args.msys_bin else "") + env["PATH"]
    suffix = ".exe" if os.name == "nt" else ""
    def run(tool, *arguments):
        command = [str(dev / "bin" / (tool + suffix)), *map(str, arguments)]
        print(" ".join(command), flush=True)
        subprocess.run(command, check=True, env=env, cwd=ROOT)
    build = ROOT / ("build-psp1000-60fps-test" if args.psp1000_60fps_test else "build-low-memory" if args.low_memory else "build")
    build.mkdir(exist_ok=True)
    include = ["-I" + str(dev / "psp" / "include"), "-I" + str(sdk / "include"), "-I" + str(ROOT / "src")]
    objects = []
    defines = ["-DLOW_MEMORY=1"] if args.low_memory else ["-DTEST_PSP1000_60FPS=1"] if args.psp1000_60fps_test else []
    for name in ("game", "garden", "disc_font", "textdb", "movie", "profile", "platform", "campaign_test", "progress", "render", "asset_cache", "music", "adaptive_music", "foley", "audio", "main"):
        target = build / (name + ".o")
        run("psp-gcc", "-std=gnu11", "-O2", "-G0", "-Wall", "-Wextra", "-Wno-misleading-indentation", "-D_PSP_FW_VERSION=600", *defines, *include,
            "-c", ROOT / "src" / (name + ".c"), "-o", target)
        objects.append(target)
    model_stub=build / "model_import.o"
    run("psp-gcc", *include, "-c", ROOT / "src/model_import.S", "-o", model_stub)
    objects.append(model_stub)
    elf = build / "pvz.elf"
    run("psp-gcc", "-G0", "-L" + str(dev / "psp" / "lib"), "-L" + str(sdk / "lib"),
        "-Wl,-zmax-page-size=128", *objects, "-lpspgu", "-lpsppower", "-lpsprtc", "-lpspaudiolib", "-lpspaudio", "-ljpeg", "-lz", "-lm",
        "-lpspdebug", "-lpspdisplay", "-lpspge", "-lpspctrl", "-lpspnet", "-lpspnet_apctl", "-o", elf)
    run("psp-fixup-imports", elf)
    stripped = build / "pvz-strip.elf"
    run("psp-strip", elf, "-o", stripped)
    sfo = build / "PARAM.SFO"
    run("mksfoex", "-d", "MEMSIZE=" + ("0" if args.low_memory else "1"), "Plants vs. Zombies", sfo)
    run("pack-pbp", build / "EBOOT.PBP", sfo, "NULL", "NULL", "NULL", "NULL", "NULL", stripped, "NULL")
    media = args.launcher_media or ROOT / "private" / "launcher0111"
    if not args.no_launcher_media and (args.launcher_media or media.is_dir()):
        from launcher_media import repack
        repack(build / "EBOOT.PBP", build / "EBOOT.PBP", media)
        print("Embedded XMB icon, cover and music from", media)
    print("Built", build / "EBOOT.PBP")


if __name__ == "__main__":
    main()

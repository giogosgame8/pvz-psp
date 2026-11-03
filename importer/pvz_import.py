"""Import the PvZ resources from an owned BLUS30852 PS3 disc image.

This is an importer, not a game or a PSP executable. No game files are included.
The input is always opened read-only. Requires Python 3.10 or newer.
SPDX-License-Identifier: Apache-2.0
"""
from __future__ import annotations

import argparse
import hashlib
import json
import re
import struct
import sys
import zlib
from pathlib import Path, PurePosixPath, PureWindowsPath

from iso9660 import ISO, SECTOR, parse_sfo

BUILD = "BLUS30852"
GAME_ROOT = "/PS3_GAME/USRDIR/NPUA_30059/USRDIR/"
ARCHIVES = ("COMPRESSED.PAK", "EN.PAK", "MAIN.PAK", "SOUNDS.PAK")
MUSIC = ("MAINMUSIC.XM", "MAINMUSIC_DRUMS.XM", "MAINMUSIC_HIHATS.XM", "ZOMBIESONYOURLAWN.OGG")
MAX_ENTRY = 256 * 1024 * 1024
OMIT_SUFFIXES = {".bat", ".cmd", ".ps1", ".exe", ".com", ".dll", ".bin", ".self", ".sprx", ".pup"}


def relative_member(name: str) -> PurePosixPath:
    """Reject member names unsafe on Windows or POSIX before writing anything."""
    normalized = name.replace("\\", "/")
    windows = PureWindowsPath(name)
    parts = normalized.split("/")
    reserved = re.compile(r"^(CON|PRN|AUX|NUL|COM[1-9]|LPT[1-9])(?:\.|$)", re.I)
    if (not name or windows.drive or windows.root or normalized.startswith("/")
            or any(part in ("", ".", "..") or part.endswith((" ", "."))
                   or reserved.match(part) or any(ord(c) < 32 or c in '<>:"|?*' for c in part)
                   for part in parts)):
        raise ValueError(f"Unsafe archive member name: {name!r}")
    return PurePosixPath(*parts)


def pak_index(data: bytes) -> list[dict]:
    """Read PS3 console PAK records, little-endian, with per-file zlib sizes.

    Format reference: PopGameTool/PopStudio, Package/Pak (Apache-2.0).
    This implementation has explicit bounds, path, and decompression checks.
    """
    if data[:8] != bytes.fromhex("c04ac0ba00000000"):
        raise ValueError("Unsupported PAK signature or version")
    offset, rows, seen = 8, [], set()
    while True:
        if offset >= len(data):
            raise ValueError("Truncated PAK directory")
        flag = data[offset]
        offset += 1
        if flag == 0x80:
            break
        if flag != 0 or offset >= len(data):
            raise ValueError("Unsupported PAK directory flag")
        length = data[offset]
        offset += 1
        if not length or offset + length + 16 > len(data):
            raise ValueError("Truncated PAK file record")
        name = data[offset:offset + length].decode("utf-8", "strict")
        offset += length
        stored, expanded, timestamp = struct.unpack_from("<IIQ", data, offset)
        offset += 16
        path = relative_member(name)
        key = str(path).casefold()
        if key in seen:
            raise ValueError(f"Duplicate PAK path: {name}")
        seen.add(key)
        if max(stored, expanded) > MAX_ENTRY:
            raise ValueError(f"Oversized PAK member: {name}")
        rows.append({"path": str(path), "stored_bytes": stored, "expanded_bytes": expanded})
        if len(rows) > 100000:
            raise ValueError("PAK directory entry limit exceeded")
    for row in rows:
        if offset + 2 > len(data):
            raise ValueError("Truncated PAK alignment prefix")
        padding = int.from_bytes(data[offset:offset + 2], "little")
        offset += 2 + padding
        row["offset"] = offset
        if offset + row["stored_bytes"] > len(data):
            raise ValueError(f"PAK extent outside file: {row['path']}")
        offset += row["stored_bytes"]
    if offset != len(data):
        raise ValueError("Unexpected trailing bytes in PAK")
    return rows


def member_bytes(data: bytes, row: dict) -> bytes:
    payload = data[row["offset"]:row["offset"] + row["stored_bytes"]]
    if row["expanded_bytes"]:
        decoder = zlib.decompressobj()
        payload = decoder.decompress(payload, row["expanded_bytes"] + 1)
        if (len(payload) != row["expanded_bytes"] or not decoder.eof
                or decoder.unused_data or decoder.unconsumed_tail):
            raise ValueError(f"Invalid compressed data or expanded size: {row['path']}")
    return payload


def sha256_file(path: Path) -> str:
    digest = hashlib.sha256()
    with path.open("rb") as stream:
        for block in iter(lambda: stream.read(4 * 1024 * 1024), b""):
            digest.update(block)
    return digest.hexdigest()


def inspect_or_import(source: Path, destination: Path | None) -> dict:
    if destination is not None:
        if destination.exists() or destination.is_symlink():
            raise ValueError("Output must be a new directory; existing files are never overwritten")
        destination = destination.resolve()
    iso = ISO(source)
    created = False
    try:
        files = {entry["path"]: entry for entry in iso.walk() if not entry["directory"]}
        sfo = files.get("/PS3_GAME/PARAM.SFO")
        if sfo is None or sfo["size"] > 1024 * 1024:
            raise ValueError("No supported disc PARAM.SFO")
        metadata = parse_sfo(iso.read(sfo["lba"] * SECTOR, sfo["size"]))
        if metadata.get("TITLE_ID") != BUILD or metadata.get("APP_VER") != "01.00":
            raise ValueError(f"Unsupported disc build: {metadata.get('TITLE_ID')} {metadata.get('APP_VER')}")
        required = [GAME_ROOT + "DATA/" + name for name in ARCHIVES]
        required += [GAME_ROOT + "SOUNDS/" + name for name in MUSIC]
        if any(path not in files for path in required):
            raise ValueError("Expected PvZ archives or music files are missing")
        report = {"schema": 1, "status": "resources_only_not_playable",
                  "title": metadata["TITLE"], "disc_title_id": BUILD,
                  "app_version": metadata["APP_VER"], "embedded_game": "NPUA_30059",
                  "source_bytes": iso.size, "source_sha256": sha256_file(source),
                  "archives": [], "music": [], "entries_verified": 0,
                  "entries_written": 0, "decoded_archive_bytes": 0,
                  "dds_textures": 0, "raw_reanimations": 0}
        for name, archive_path in zip(ARCHIVES, required[:len(ARCHIVES)]):
            file = files[archive_path]
            if file["size"] > 512 * 1024 * 1024:
                raise ValueError("Oversized PAK file")
            data = iso.read(file["lba"] * SECTOR, file["size"])
            rows = pak_index(data)
            archive = {"archive": name, "sha256": hashlib.sha256(data).hexdigest(),
                       "source_bytes": len(data), "files": []}
            for row in rows:
                payload = member_bytes(data, row)
                path = PurePosixPath(row["path"])
                omitted = path.suffix.lower() in OMIT_SUFFIXES
                info = {"path": str(path), "bytes": len(payload),
                        "sha256": hashlib.sha256(payload).hexdigest(),
                        "omitted": omitted}
                if payload[:4] == b"DDS " and len(payload) >= 128:
                    height, width = struct.unpack_from("<II", payload, 12)
                    info["texture"] = {"width": width, "height": height,
                                       "fourcc": payload[84:88].decode("ascii", "replace")}
                    report["dds_textures"] += 1
                if str(path).lower().endswith(".reanim"):
                    report["raw_reanimations"] += 1
                if destination is not None and not omitted:
                    if not created:
                        destination.mkdir(parents=True, exist_ok=False)
                        created = True
                    target = destination / "archives" / name.removesuffix(".PAK").lower() / Path(*path.parts)
                    target.parent.mkdir(parents=True, exist_ok=True)
                    with target.open("xb") as output:
                        output.write(payload)
                    report["entries_written"] += 1
                archive["files"].append(info)
                report["entries_verified"] += 1
                report["decoded_archive_bytes"] += len(payload)
            report["archives"].append(archive)
            print(f"Verified {name}: {len(rows)} entries", file=sys.stderr)
        for name in MUSIC:
            file = files[GAME_ROOT + "SOUNDS/" + name]
            payload = iso.read(file["lba"] * SECTOR, file["size"])
            report["music"].append({"path": name, "bytes": len(payload),
                                    "sha256": hashlib.sha256(payload).hexdigest()})
            if destination is not None:
                target = destination / "music" / name
                target.parent.mkdir(parents=True, exist_ok=True)
                with target.open("xb") as output:
                    output.write(payload)
        # In inspect mode nothing is written. A successful import gets its manifest last.
        if destination is not None:
            (destination / "manifest.json").write_text(json.dumps(report, indent=2), encoding="utf-8")
        return report
    finally:
        iso.file.close()


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("iso", type=Path, help="Your owned BLUS30852 version 01.00 PS3 ISO")
    parser.add_argument("--output", type=Path, help="New local resource directory; omit for read-only verification")
    parser.add_argument("--report", type=Path, help="Write the metadata-only verification report")
    args = parser.parse_args()
    try:
        if args.report and args.report.resolve() == args.iso.resolve():
            raise ValueError("The report cannot overwrite the input ISO")
        if args.report and args.report.exists():
            raise ValueError("The report path already exists; choose a new filename")
        report = inspect_or_import(args.iso, args.output)
        if args.report:
            args.report.parent.mkdir(parents=True, exist_ok=True)
            with args.report.open("x", encoding="utf-8") as stream:
                json.dump(report, stream, indent=2)
        print(json.dumps({key: value for key, value in report.items() if key not in ("archives", "music")}, indent=2))
        print("Resources verified. This tool does not produce a playable PSP game.")
        return 0
    except (OSError, ValueError, zlib.error, struct.error) as error:
        print(f"Import failed: {error}", file=sys.stderr)
        print("If an output directory was created, treat it as incomplete unless manifest.json exists.", file=sys.stderr)
        return 1


if __name__ == "__main__":
    raise SystemExit(main())

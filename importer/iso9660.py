"""Read-only ISO9660 inventory and PS3 metadata inspection. Uses Python stdlib.

SPDX-License-Identifier: Apache-2.0
"""
from __future__ import annotations

import argparse
import json
import struct
from pathlib import Path

SECTOR = 2048


def parse_sfo(data: bytes) -> dict:
    if data[:4] != b"\0PSF":
        return {"error": "PARAM.SFO has no readable PSF signature", "header_hex": data[:16].hex()}
    _, version, key_table, data_table, count = struct.unpack_from("<5I", data)
    result = {}
    for index in range(count):
        key_offset, kind, length, capacity, value_offset = struct.unpack_from("<HHIII", data, 20 + 16 * index)
        key_start = key_table + key_offset
        key_end = data.index(0, key_start)
        key = data[key_start:key_end].decode("utf-8", "replace")
        value = data[data_table + value_offset:data_table + value_offset + length]
        if kind == 0x0404 and len(value) >= 4:
            result[key] = int.from_bytes(value[:4], "little")
        elif kind == 0x0204:
            result[key] = value.rstrip(b"\0").decode("utf-8", "replace")
        else:
            result[key] = {"format": kind, "hex": value.hex()}
    return result


class ISO:
    def __init__(self, path: Path):
        self.path = path
        self.file = path.open("rb")
        self.size = path.stat().st_size
        self.descriptors = []
        self.root = None
        self.volume_id = None
        for sector in range(16, 256):
            data = self.read(sector * SECTOR, SECTOR)
            if len(data) != SECTOR or data[1:6] != b"CD001":
                continue
            kind = data[0]
            self.descriptors.append({"sector": sector, "type": kind})
            if kind == 1:
                self.volume_id = data[40:72].decode("ascii", "replace").strip()
                logical_block = int.from_bytes(data[128:130], "little")
                if logical_block != SECTOR:
                    raise ValueError(f"Unsupported block size {logical_block}")
                self.root = self.record(data[156:156 + data[156]])
            if kind == 255:
                break
        if self.root is None:
            raise ValueError("No ISO9660 primary volume descriptor")

    def read(self, offset: int, size: int) -> bytes:
        if offset < 0 or size < 0 or offset + size > self.size:
            raise ValueError(f"Extent outside ISO: offset={offset}, size={size}")
        self.file.seek(offset)
        return self.file.read(size)

    @staticmethod
    def record(data: bytes) -> dict:
        name_len = data[32]
        name = data[33:33 + name_len].decode("ascii", "replace")
        return {"name": name.split(";")[0], "lba": int.from_bytes(data[2:6], "little"),
                "size": int.from_bytes(data[10:14], "little"), "directory": bool(data[25] & 2)}

    def walk(self) -> list[dict]:
        rows = []
        visited = set()

        def visit(directory: dict, parent: str, depth: int):
            identity = (directory["lba"], directory["size"])
            if identity in visited or depth > 24:
                return
            visited.add(identity)
            if directory["size"] > 16 * 1024 * 1024:
                raise ValueError("Implausibly large directory")
            data = self.read(directory["lba"] * SECTOR, directory["size"])
            offset = 0
            while offset < len(data):
                length = data[offset]
                if length == 0:
                    offset = (offset // SECTOR + 1) * SECTOR
                    continue
                if length < 34 or offset + length > len(data):
                    raise ValueError(f"Unreadable directory record at {parent}, byte {offset}")
                entry = self.record(data[offset:offset + length])
                offset += length
                if entry["name"] in ("\0", "\1"):
                    continue
                entry["path"] = parent + "/" + entry.pop("name")
                rows.append(entry)
                if len(rows) > 100000:
                    raise ValueError("Directory record limit exceeded")
                if entry["directory"]:
                    visit(entry, entry["path"], depth + 1)
        visit(self.root, "", 0)
        return rows


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("iso", type=Path)
    parser.add_argument("--report", type=Path, required=True)
    args = parser.parse_args()
    iso = ISO(args.iso)
    try:
        rows = iso.walk()
        result = {"source": str(args.iso.resolve()), "bytes": iso.size,
                  "volume_id": iso.volume_id, "descriptors": iso.descriptors, "entries": rows}
        metadata = {}
        for entry in rows:
            if entry["directory"]:
                continue
            name = entry["path"].upper()
            if name.endswith(("PARAM.SFO", "PS3_DISC.SFB")) or name.endswith((".PAK", "EBOOT.BIN")):
                header = iso.read(entry["lba"] * SECTOR, min(entry["size"], 64))
                entry["header_hex"] = header.hex()
                entry["header_ascii"] = "".join(chr(b) if 32 <= b < 127 else "." for b in header)
            if name.endswith("PARAM.SFO") and entry["size"] <= 1024 * 1024:
                metadata[entry["path"]] = parse_sfo(iso.read(entry["lba"] * SECTOR, entry["size"]))
        result["metadata"] = metadata
        args.report.parent.mkdir(parents=True, exist_ok=True)
        args.report.write_text(json.dumps(result, indent=2, ensure_ascii=False), encoding="utf-8")
        print(json.dumps({"volume_id": iso.volume_id, "bytes": iso.size, "entry_count": len(rows),
                          "metadata": metadata, "report": str(args.report),
                          "files": [{k: v for k, v in row.items() if k != "header_hex"} for row in rows if not row["directory"]]},
                         indent=2, ensure_ascii=True))
    finally:
        iso.file.close()


if __name__ == "__main__":
    main()

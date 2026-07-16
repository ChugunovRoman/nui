#!/usr/bin/env python3
"""Convert PNG to ICO (Windows icon format).

Usage: python tools/png_to_ico.py input.png output.ico

Generates multi-size ICO with 16, 32, 48, 64, 128, 256 px variants.
Requires Pillow: pip install Pillow
"""
import sys
import struct

try:
    from PIL import Image
except ImportError:
    print("ERROR: Pillow not installed. Run: pip install Pillow")
    sys.exit(1)


def png_to_ico(png_path: str, ico_path: str) -> None:
    sizes = [16, 32, 48, 64, 128, 256]
    img = Image.open(png_path).convert("RGBA")

    png_data_list = []
    for size in sizes:
        resized = img.resize((size, size), Image.LANCZOS)
        import io
        buf = io.BytesIO()
        resized.save(buf, format="PNG")
        png_data_list.append(buf.getvalue())

    # ICO header: reserved(2) + type(2) + count(2)
    header = struct.pack("<HHH", 0, 1, len(sizes))

    # Directory entries (16 bytes each)
    entries = []
    data_offset = 6 + 16 * len(sizes)
    for i, size in enumerate(sizes):
        w = 0 if size >= 256 else size
        h = 0 if size >= 256 else size
        entry = struct.pack("<BBBBHHII",
                            w, h, 0, 0, 1, 32,
                            len(png_data_list[i]), data_offset)
        entries.append(entry)
        data_offset += len(png_data_list[i])

    with open(ico_path, "wb") as f:
        f.write(header)
        for entry in entries:
            f.write(entry)
        for data in png_data_list:
            f.write(data)

    print(f"Created {ico_path} with {len(sizes)} sizes: {sizes}")


if __name__ == "__main__":
    if len(sys.argv) != 3:
        print(f"Usage: {sys.argv[0]} input.png output.ico")
        sys.exit(1)
    png_to_ico(sys.argv[1], sys.argv[2])

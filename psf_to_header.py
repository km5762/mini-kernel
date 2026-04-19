import struct
import argparse
from dataclasses import dataclass
from typing import List


# ----------------------------
# Bitmap structure
# ----------------------------


@dataclass
class Bitmap:
    data: bytes
    width: int
    height: int
    stride: int
    color_format: str
    color: int


# ----------------------------
# PSF parsing (same as before)
# ----------------------------

PSF2_MAGIC = 0x864AB572
PSF1_MAGIC = 0x0436


class PSFFont:
    def __init__(self, glyphs, width, height):
        self.glyphs = glyphs
        self.width = width
        self.height = height


def load_psf(path: str) -> PSFFont:
    with open(path, "rb") as f:
        data = f.read()

    magic = struct.unpack_from("<I", data, 0)[0]

    if magic == PSF2_MAGIC:
        (
            magic,
            version,
            headersize,
            flags,
            length,
            charsize,
            height,
            width,
        ) = struct.unpack_from("<IIIIIIII", data, 0)

        glyphs = []
        offset = headersize

        for _ in range(length):
            glyphs.append(data[offset : offset + charsize])
            offset += charsize

        return PSFFont(glyphs, width, height)

    magic16 = struct.unpack_from("<H", data, 0)[0]
    if magic16 == PSF1_MAGIC:
        mode = data[2]
        charsize = data[3]
        length = 256 if (mode & 1) == 0 else 512

        width = 8
        height = charsize

        offset = 4
        glyphs = []

        for _ in range(length):
            glyphs.append(data[offset : offset + charsize])
            offset += charsize

        return PSFFont(glyphs, width, height)

    raise ValueError("Invalid PSF file")


# ----------------------------
# Convert glyph → bitmap
# ----------------------------


def glyph_to_bitmap(glyph: bytes, width: int, height: int) -> Bitmap:
    stride = (width + 7) // 8
    out = bytearray(stride * height)

    for y in range(height):
        for x in range(width):
            src_byte = glyph[y * stride + (x // 8)]
            bit = 7 - (x % 8)

            if src_byte & (1 << bit):
                out[y * stride + (x // 8)] |= 1 << bit

    return Bitmap(
        data=bytes(out),
        width=width,
        height=height,
        stride=stride,
        color_format="MONO_1BPP",
        color=0xFFFFFFFF,
    )


def psf_to_bitmaps(psf: PSFFont) -> List[Bitmap]:
    return [glyph_to_bitmap(g, psf.width, psf.height) for g in psf.glyphs]


# ----------------------------
# C HEADER GENERATION
# ----------------------------


def write_header(bitmaps: List[Bitmap], path: str):
    with open(path, "w") as f:
        f.write("#pragma once\n")
        f.write("#include <stdint.h>\n")
        f.write("#include <stddef.h>\n\n")

        f.write("enum color_format {\n    MONO_1BPP\n};\n\n")

        f.write("struct bitmap {\n")
        f.write("    unsigned char *data;\n")
        f.write("    size_t width;\n")
        f.write("    size_t height;\n")
        f.write("    size_t stride;\n")
        f.write("    enum color_format color_format;\n")
        f.write("    uint32_t color;\n")
        f.write("};\n\n")

        # write raw data arrays
        for i, bm in enumerate(bitmaps):
            f.write(f"static const unsigned char glyph_{i}_data[] = {{\n    ")

            for j, b in enumerate(bm.data):
                f.write(f"0x{b:02x}, ")
                if (j + 1) % 12 == 0:
                    f.write("\n    ")

            f.write("\n};\n\n")

        # write bitmap structs
        f.write(f"static const struct bitmap font_bitmaps[] = {{\n")

        for i, bm in enumerate(bitmaps):
            f.write("    {\n")
            f.write(f"        (unsigned char*)glyph_{i}_data,\n")
            f.write(f"        {bm.width},\n")
            f.write(f"        {bm.height},\n")
            f.write(f"        {bm.stride},\n")
            f.write("        MONO_1BPP,\n")
            f.write(f"        0x{bm.color:08x}\n")
            f.write("    },\n")

        f.write("};\n\n")

        f.write(f"static const size_t font_glyph_count = {len(bitmaps)};\n")


# ----------------------------
# CLI ENTRY POINT
# ----------------------------


def main():
    parser = argparse.ArgumentParser(description="Convert PSF font to C bitmap header")
    parser.add_argument("input", help="Input .psf file")
    parser.add_argument("output", help="Output .h file")

    args = parser.parse_args()

    font = load_psf(args.input)
    bitmaps = psf_to_bitmaps(font)

    write_header(bitmaps, args.output)

    print(f"Converted {len(bitmaps)} glyphs -> {args.output}")


if __name__ == "__main__":
    main()

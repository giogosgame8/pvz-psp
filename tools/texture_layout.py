"""Lossless PSP GE texture tiling (16-byte by 8-row blocks)."""
from functools import lru_cache
from operator import itemgetter
from struct import Struct


PACK_LOW = tuple(value >> 4 for value in range(256))
PACK_HIGH = tuple(value & 240 for value in range(256))
UNPACK_LOW = tuple((value & 15) * 17 for value in range(256))
UNPACK_HIGH = tuple((value >> 4) * 17 for value in range(256))

def pack_rgba4444(image):
 """Pack little-endian RGBA4444 using compiled channel operations.

 Each output byte combines non-overlapping nibbles, so saturating addition is
 exactly equivalent to bitwise OR. LA storage interleaves those two bytes and
 is independent of the computer's byte order.
 """
 from PIL import Image,ImageChops
 red,green,blue,alpha=image.split()
 lower=ImageChops.add(red.point(PACK_LOW),green.point(PACK_HIGH))
 upper=ImageChops.add(blue.point(PACK_LOW),alpha.point(PACK_HIGH))
 return Image.merge('LA',(lower,upper)).tobytes()


@lru_cache(maxsize=32)
def _block_layout(width, height, tiled_to_linear):
    """Cache the order so splitting, gathering and joining run in C per frame."""
    chunks_per_row = width // 8
    total_chunks = chunks_per_row * height
    if tiled_to_linear:
        indices = (((y // 8) * chunks_per_row + x) * 8 + y % 8
                   for y in range(height) for x in range(chunks_per_row))
    else:
        indices = ((y + offset) * chunks_per_row + x
                   for y in range(0, height, 8) for x in range(chunks_per_row)
                   for offset in range(8))
    return Struct('16s' * total_chunks), itemgetter(*indices)


def _reorder_blocks(raw, width, height, tiled_to_linear):
    if width < 8 or height < 8 or width % 8 or height % 8 or len(raw) != width * height * 2:
        raise ValueError('Invalid padded texture')
    if width == 8:
        return bytes(raw)
    unpack, reorder = _block_layout(width, height, tiled_to_linear)
    return b''.join(reorder(unpack.unpack(raw)))


def tile_rgba4444(raw, width, height):
    return _reorder_blocks(raw, width, height, False)

def unpack_rgba4444(raw,width,height):
 """Decode both interleaved bytes with compiled, exact nibble lookup tables."""
 from PIL import Image
 if width<1 or height<1 or len(raw)!=width*height*2:raise ValueError('Invalid linear texture')
 lower,upper=Image.frombytes('LA',(width,height),raw).split()
 return Image.merge('RGBA',(lower.point(UNPACK_LOW),lower.point(UNPACK_HIGH),upper.point(UNPACK_LOW),upper.point(UNPACK_HIGH)))


def untile_rgba4444(raw, width, height):
    return _reorder_blocks(raw, width, height, True)

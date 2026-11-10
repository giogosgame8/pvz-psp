"""Lossless PSP GE texture tiling (16-byte by 8-row blocks)."""

def pack_rgba4444(image):
 """Pack little-endian RGBA4444 using compiled channel operations.

 Each output byte combines non-overlapping nibbles, so saturating addition is
 exactly equivalent to bitwise OR. LA storage interleaves those two bytes and
 is independent of the computer's byte order.
 """
 from PIL import Image,ImageChops
 red,green,blue,alpha=image.split()
 low=[v>>4 for v in range(256)];high=[v&240 for v in range(256)]
 lower=ImageChops.add(red.point(low),green.point(high))
 upper=ImageChops.add(blue.point(low),alpha.point(high))
 return Image.merge('LA',(lower,upper)).tobytes()
def tile_rgba4444(raw,width,height):
 row=width*2
 if width<8 or height<8 or width%8 or height%8 or len(raw)!=row*height:raise ValueError('Invalid padded texture')
 source=memoryview(raw);out=bytearray(len(raw));dest=memoryview(out);at=0
 for y in range(0,height,8):
  for x in range(0,row,16):
   for dy in range(8):
    start=(y+dy)*row+x;dest[at:at+16]=source[start:start+16];at+=16
 return bytes(out)

def unpack_rgba4444(raw,width,height):
 """Decode both interleaved bytes with compiled, exact nibble lookup tables."""
 from PIL import Image
 if width<1 or height<1 or len(raw)!=width*height*2:raise ValueError('Invalid linear texture')
 lower,upper=Image.frombytes('LA',(width,height),raw).split()
 low=[(v&15)*17 for v in range(256)];high=[(v>>4)*17 for v in range(256)]
 return Image.merge('RGBA',(lower.point(low),lower.point(high),upper.point(low),upper.point(high)))
def untile_rgba4444(raw,width,height):
 row=width*2;blocks=width//8;out=bytearray(len(raw));dest=memoryview(out);source=memoryview(raw)
 for y in range(height):
  for bx in range(blocks):
   address=((y//8)*blocks+bx)*128+(y%8)*16;start=y*row+bx*16;dest[start:start+16]=source[address:address+16]
 return bytes(out)

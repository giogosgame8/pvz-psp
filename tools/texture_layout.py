"""Lossless PSP GE texture tiling (16-byte by 8-row blocks)."""
def tile_rgba4444(raw,width,height):
 row=width*2
 if width<8 or height<8 or width%8 or height%8 or len(raw)!=row*height:raise ValueError('Invalid padded texture')
 source=memoryview(raw);out=bytearray(len(raw));dest=memoryview(out);at=0
 for y in range(0,height,8):
  for x in range(0,row,16):
   for dy in range(8):
    start=(y+dy)*row+x;dest[at:at+16]=source[start:start+16];at+=16
 return bytes(out)
def untile_rgba4444(raw,width,height):
 row=width*2;blocks=width//8;out=bytearray(len(raw));dest=memoryview(out);source=memoryview(raw)
 for y in range(height):
  for bx in range(blocks):
   address=((y//8)*blocks+bx)*128+(y%8)*16;start=y*row+bx*16;dest[start:start+16]=source[address:address+16]
 return bytes(out)

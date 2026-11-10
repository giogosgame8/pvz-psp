"""Limit affine rendering work while preserving the original pixel coordinates."""
import math
from collections import OrderedDict
from concurrent.futures import ThreadPoolExecutor
from threading import Lock
from PIL import Image
from preparation_workers import worker_count


def render_frames(function, indices):
    indices = list(indices)
    workers = min(worker_count('graphics'), len(indices))
    if workers <= 1:
        return [function(index) for index in indices]
    with ThreadPoolExecutor(max_workers=workers) as pool:
        return list(pool.map(function, indices))


class AffineLayerCache:
    """Bounded reuse of transformed, immutable textures within one preparation.

    Only transformed layers are cached; destination backgrounds remain separate.
    Retain the source object so an evicted texture's id cannot alias a new image.
    Pillow work happens outside the lock, allowing independent frames to render.
    """
    def __init__(self, max_bytes=32*1048576):
        self.max_bytes=max_bytes;self.bytes=0;self.hits=0;self.misses=0
        self.layers=OrderedDict();self.lock=Lock()

    def get(self,size,image,matrix,inverse,alpha):
        key=(id(image),size,tuple(matrix),tuple(inverse),alpha)
        with self.lock:
            if key in self.layers:
                value=self.layers[key];self.layers.move_to_end(key);self.hits+=1
                return value[1]
            self.misses+=1
        layer=affine_layer(size,image,matrix,inverse,alpha)
        if layer is None:return None
        cost=layer[0].width*layer[0].height*4
        if cost<=self.max_bytes:
            with self.lock:
                if key not in self.layers:
                    while self.layers and self.bytes+cost>self.max_bytes:
                        _,old=self.layers.popitem(last=False);self.bytes-=old[2]
                    self.layers[key]=(image,layer,cost);self.bytes+=cost
        return layer

    def clear(self):
        with self.lock:self.layers.clear();self.bytes=0


def affine_layer(size, image, matrix, inverse, alpha=1):
    a, b, c, d, x, y = matrix
    corners = ((x, y), (x + a * image.width, y + c * image.width),
               (x + b * image.height, y + d * image.height),
               (x + a * image.width + b * image.height,
                y + c * image.width + d * image.height))
    left = max(0, math.floor(min(p[0] for p in corners)) - 2)
    top = max(0, math.floor(min(p[1] for p in corners)) - 2)
    right = min(size[0], math.ceil(max(p[0] for p in corners)) + 2)
    bottom = min(size[1], math.ceil(max(p[1] for p in corners)) + 2)
    if left >= right or top >= bottom:
        return
    # Keep output origin (0, 0) and the original inverse coefficients: translating
    # the matrix for a tight crop can change bicubic rounding at some pixels.
    layer = image.transform((right, bottom), Image.Transform.AFFINE, inverse,
                            resample=Image.Resampling.BICUBIC)
    layer = layer.crop((left, top, right, bottom))
    if alpha < 1:
        layer.putalpha(layer.getchannel('A').point(lambda value: round(value * alpha)))
    return layer,left,top


def composite_affine(canvas, image, matrix, inverse, alpha=1, cache=None):
    layer=(cache.get(canvas.size,image,matrix,inverse,alpha) if cache is not None
           else affine_layer(canvas.size,image,matrix,inverse,alpha))
    if layer is not None:canvas.alpha_composite(layer[0],layer[1:])

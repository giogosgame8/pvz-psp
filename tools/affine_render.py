"""Limit affine rendering work while preserving the original pixel coordinates."""
import math
from collections import OrderedDict
from concurrent.futures import ThreadPoolExecutor
from contextlib import contextmanager
from threading import Lock, local
from PIL import Image
from preparation_workers import worker_count


_frame_workers = local()


@contextmanager
def frame_worker_session():
    """Reuse graphics workers for the calling thread's preparation run."""
    if getattr(_frame_workers, 'session', None) is not None:
        yield
        return
    workers = worker_count('graphics')
    with ThreadPoolExecutor(max_workers=workers) as executor:
        _frame_workers.session = workers, executor
        try:
            yield
        finally:
            del _frame_workers.session


def render_frames(function, indices):
    indices = list(indices)
    if len(indices) <= 1:
        return [function(index) for index in indices]
    session = getattr(_frame_workers, 'session', None)
    if session is not None:
        workers, executor = session
        if workers == 1:
            return [function(index) for index in indices]
        return list(executor.map(function, indices))
    workers = min(worker_count('graphics'), len(indices))
    if workers <= 1:
        return [function(index) for index in indices]
    with ThreadPoolExecutor(max_workers=workers) as pool:
        return list(pool.map(function, indices))


class CompactCanvas:
    """Collect clipped layers in source coordinates and composite their occupied rectangle."""

    def __init__(self, size):
        self.size = size
        self.layers = []

    def alpha_composite(self, image, position):
        self.layers.append((image, *position))

    def finish(self):
        if not self.layers:
            return Image.new('RGBA', (1, 1)), 0, 0
        left = min(x for _, x, _ in self.layers)
        top = min(y for _, _, y in self.layers)
        right = max(x + image.width for image, x, _ in self.layers)
        bottom = max(y + image.height for image, _, y in self.layers)
        canvas = Image.new('RGBA', (right - left, bottom - top))
        for image, x, y in self.layers:
            canvas.alpha_composite(image, (x - left, y - top))
        return canvas, left, top


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
    source = image.convert('RGBa') if image.mode == 'RGBA' else image
    layer = source.transform((right, bottom), Image.Transform.AFFINE, inverse,
                            resample=Image.Resampling.BICUBIC)
    layer = layer.crop((left, top, right, bottom))
    if source is not image:
        # Pillow's RGBA transform follows the same path. Converting after the crop
        # avoids unpremultiplying the transparent space before the layer.
        layer = layer.convert('RGBA')
    if alpha < 1:
        layer.putalpha(layer.getchannel('A').point(lambda value: round(value * alpha)))
    return layer,left,top


def composite_affine(canvas, image, matrix, inverse, alpha=1, cache=None):
    layer=(cache.get(canvas.size,image,matrix,inverse,alpha) if cache is not None
           else affine_layer(canvas.size,image,matrix,inverse,alpha))
    if layer is not None:canvas.alpha_composite(layer[0],layer[1:])

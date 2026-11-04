"""Limit affine rendering work while preserving the original pixel coordinates."""
import math
from concurrent.futures import ThreadPoolExecutor
from PIL import Image
from preparation_workers import worker_count


def render_frames(function, indices):
    indices = list(indices)
    workers = min(worker_count('graphics'), len(indices))
    if workers <= 1:
        return [function(index) for index in indices]
    with ThreadPoolExecutor(max_workers=workers) as pool:
        return list(pool.map(function, indices))


def composite_affine(canvas, image, matrix, inverse, alpha=1):
    a, b, c, d, x, y = matrix
    corners = ((x, y), (x + a * image.width, y + c * image.width),
               (x + b * image.height, y + d * image.height),
               (x + a * image.width + b * image.height,
                y + c * image.width + d * image.height))
    left = max(0, math.floor(min(p[0] for p in corners)) - 2)
    top = max(0, math.floor(min(p[1] for p in corners)) - 2)
    right = min(canvas.width, math.ceil(max(p[0] for p in corners)) + 2)
    bottom = min(canvas.height, math.ceil(max(p[1] for p in corners)) + 2)
    if left >= right or top >= bottom:
        return
    # Keep output origin (0, 0) and the original inverse coefficients: translating
    # the matrix for a tight crop can change bicubic rounding at some pixels.
    layer = image.transform((right, bottom), Image.Transform.AFFINE, inverse,
                            resample=Image.Resampling.BICUBIC)
    layer = layer.crop((left, top, right, bottom))
    if alpha < 1:
        layer.putalpha(layer.getchannel('A').point(lambda value: round(value * alpha)))
    canvas.alpha_composite(layer, (left, top))

import json
from math import *

WATERFALL_RESOLUTION = 65536

# Open colormap file
data = json.load(open('/usr/share/sdrpp/colormaps/classic.json'))
map = data['map']

def hexToRGB(color):
    colc = color[1:]
    val = int(colc, 16)
    return [(val >> 16) & 0xFF, (val >> 8) & 0xFF, (val >> 0) & 0xFF]

def clamp(val, low, high):
    if val < low:
        val = low
    if val > high:
        val = high
    return val

def htons(x):
    a = (x >> 0) & 0xFF
    b = (x >> 8) & 0xFF
    return (a << 8) | b


rgbMap = []
colorCount = len(map)

for c in map:
    rgbMap.append(hexToRGB(c))

array = []

for i in range(WATERFALL_RESOLUTION):
    lowerId = floor((i / WATERFALL_RESOLUTION) * colorCount)
    upperId = ceil((i / WATERFALL_RESOLUTION) * colorCount)

    lowerId = clamp(lowerId, 0, colorCount - 1)
    upperId = clamp(upperId, 0, colorCount - 1)
    ratio = ((i / WATERFALL_RESOLUTION) * colorCount) - lowerId
    r = int((rgbMap[lowerId][0] * (1.0 - ratio) + rgbMap[upperId][0] * ratio) * (31.0/255.0))
    g = int((rgbMap[lowerId][1] * (1.0 - ratio) + rgbMap[upperId][1] * ratio) * (63.0/255.0))
    b = int((rgbMap[lowerId][2] * (1.0 - ratio) + rgbMap[upperId][2] * ratio) * (31.0/255.0))
    
    array.append(htons((r << 11) | (g << 5) | b))

print('#pragma once')
print('#include <stdint.h>')
print('')
print('inline const uint16_t colormap[%d] = {' % WATERFALL_RESOLUTION);

for col in array:
    print('    0x%06X,' % col)

print('};')

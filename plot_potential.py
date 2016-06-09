#!/usr/bin/env python2
import matplotlib.pyplot as plt
import numpy as np
import colormaps
import scipy.misc
import sys

plt.register_cmap(name='viridis', cmap=colormaps.viridis)
plt.set_cmap(colormaps.viridis)

def pic_yx_to_ram_yx(pic_y, pic_x):
    ram_y = (pic_y/420.*84 - 15)*44./17.5 + 0x94
    ram_x = (pic_x/576.*84 - 15)*48./25 + 0x18
    return ram_y, ram_x

#points = [(0x85, 0xc4),
#          (0x7d, 0x94),
#          (0x15, 0x94),
#          (0x15, 0xc0)]

p_nokey = [(0x6d, 0xc5),
          (0x85, 0xc4),
          (0x85, 0xbf),
          (0x85, 0xb9),
          (0x85, 0xb4),
          (0x85, 0xaf),
          (0x85, 0xa9),
          (0x85, 0xa4),

          (0x85, 0x94),
          (0x7d, 0x94),
          (0x75, 0x94),
          (0x6d, 0x94),
          (0x65, 0x94),
          (0x5d, 0x94),
          (0x55, 0x94),
          (0x4d, 0x94),
          (0x45, 0x94),
          (0x3d, 0x94),
          (0x35, 0x94),
          (0x2d, 0x94),
          (0x25, 0x94),
          (0x1d, 0x94),
          (0x15, 0x94),

          (0x15, 0xa0),
          (0x15, 0xa5),
          (0x15, 0xab),
          (0x15, 0xb0),
          (0x15, 0xb5),
          (0x15, 0xbb),
          (0x15, 0xc0)]
p_key = list(reversed(p_nokey)) + [
    (0x7a, 0xc0),
    (0x68, 0xc0),
    (0x55, 0xc0),
    (0x4d, 0xc0),
    (0x4d, 0xd3),
    (0x4d, 0x0e)]

l_nokey = [(0x64, 0xc5),
           (0x85, 0xc0),
           (0x85, 0x94),
           (0x15, 0x94),
           (0x15, 0xc0),
           (0x09, 0xcf),
           ]
l_key = list(reversed(l_nokey))[:-1] + [
    (0x4d, 0xc0),
    (0x4d, 0xeb),
    (0x99, 0xeb),
    ]

def modulus(v, yscale=.5):
    return (v[0]**2+(v[1]*yscale)**2)**.5

def to_line(points):
    lines = []
    total_len = 0.
    for i in range(len(points)-1):
        v = points[i+1] - points[i]
        mv = modulus(v)
        lines.append((points[i], v, total_len, mv))
        total_len += mv
    lines.append((points[-1], None, total_len, 0))
    return (lines, total_len)

lines_ = (list(np.array(p,dtype=np.float64) for p in l_nokey),
         list(np.array(p,dtype=np.float64) for p in l_key))
lines = (to_line(lines_[0]), to_line(lines_[1]))

def fun(y, x, key):
    if 0xba < y < 0xeb and 0x2a < x < 0x43:
        return 0
    elif key == 0:
        points = p_nokey
    else:
        points = p_key
    r = 0
    for i in range(len(points)):
        d = (((x-points[i][0])*2)**2 + (y-points[i][1])**2)**.7
        r += max(i/float(len(points)*.8 + .2)*(1 - d/50.), 0)
    return r

def progress(line, p):
    ptis = []
    points, total_len = line
#    y__, x__ = pic_yx_to_ram_yx(100, 400)
#    p = np.array((x__, y__), dtype=np.float64)
    for i in range(len(points)-1):
        x1, y1 = p_ = points[i][0]
        vx1, vy1 = v_ = points[i][1]
        x2, y2 = p
        vx2 = -vy1
        vy2 = vx1
        t1 = (vx2*(y1-y2)-vy2*(x1-x2))/(vy2*vx1 - vx2*vy1)
        if 0 < t1 < 1:
            ptis.append((p_ + v_*t1, t1, i))
        ptis.append((points[i][0], 0, i))
    ptis.append((points[-1][0], 0, -1))
    min_dist = float("inf")
    min_pti = None
    for pti in ptis:
        d = modulus(pti[0] - p)
        if d < min_dist:
            min_dist = d
            min_pti = pti
    if min_dist > 10:
        return 0
    i = min_pti[2]
    return (points[i][2]+min_pti[1]*points[i][3]) / total_len

def fun2(y, x, key):
    if 0xba < y < 0xeb and 0x2a < x < 0x43:
        return -1
    return progress(lines[key], np.array((x, y)))

stride = 1

if len(sys.argv) == 2 and sys.argv[1] == "array":
    print "static double potential[2][0x100][0x100] = {{"
    a = np.zeros((0x100,0x100), dtype=np.float64)
    b = np.zeros((0x100,0x100), dtype=np.float64)
    get_real_coords = lambda a, b: (a, b)
    paint = False
else:
    a = np.zeros((420,576), dtype=np.float64)
    b = np.zeros((420,576), dtype=np.float64)
    get_real_coords = pic_yx_to_ram_yx
    paint = True

for i in range(0, a.shape[1], stride):
    for j in range(0, a.shape[0], stride):
        y, x = get_real_coords(j, i)
        a[j:j+stride, i:i+stride] = fun2(y, x, 0)
        b[j:j+stride, i:i+stride] = fun2(y, x, 1)

if paint:
    dpi = 96.
    w = a.shape[1]*4/dpi
    h = a.shape[0]*4/dpi

    plt.figure(figsize=(w, h), dpi=dpi)
    plt.subplot(1, 3, 1)
    plt.imshow(a, origin='lower')
    plt.grid(color='r', linestyle='-', linewidth=1)
    plt.subplot(1, 3, 2)
    plt.imshow(b, origin='lower')
    plt.grid(color='r', linestyle='-', linewidth=1)
    plt.subplot(1, 3, 3)
    plt.imshow(scipy.misc.imread("/Users/adria/Desktop/Montezuma's Revenge (1984) (Parker Bros).png")[::-1], origin='lower')
    plt.grid(color='r', linestyle='-', linewidth=1)
    plt.savefig('shaping.pdf')
else:
    def print_arr(a):
        print ",\n".join(("{"+",".join("%s"%a[j,i] for i in range(a.shape[0]))+"}")
                    for j in range(a.shape[1]))
    print_arr(a)
    print "}, {"
    print_arr(b)
    print "}};"

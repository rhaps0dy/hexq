#!/usr/bin/env python2
import matplotlib
matplotlib.use('Agg')
import matplotlib.pyplot as plt
import numpy as np
import colormaps
import scipy.misc
import sys
import cPickle

W = 7.2
matplotlib.rcParams.update({'font.size': 28,
                            'figure.figsize': (W, (W/1.61803398875)*.86),
                            'lines.linewidth': 1})

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

l_nokey = [(0x64, 0xc9),
           (0x85, 0xc9),
           (0x85, 0x94),
           (0x15, 0x94),
           (0x15, 0xc0),
           (0x09, 0xcf),
           ]
l_key = list(reversed(l_nokey))[:-1] + [
    (0x48, 0xc9),
    (0x48, 0xfb),
    (0x99, 0xfb),
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
        # Notation inverted in thesis, 1 is 2 and 2 is 1
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
#    if 0xba < y < 0xeb and 0x2a < x < 0x43:
#        return -1
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

    def forceAspect(ax,aspect=1):
        im = ax.get_images()
        extent =  im[0].get_extent()
        ax.set_aspect(abs((extent[1]-extent[0])/(extent[3]-extent[2]))/aspect)

    fig, (ax1, ax2, ax3) = plt.subplots(1, 3, figsize=(w, h), dpi=dpi)
    y1, x1 = pic_yx_to_ram_yx(0, 0)
    y2, x2 = pic_yx_to_ram_yx(a.shape[0], a.shape[1])
    grid_interval = np.arange(0, 160, 30)
    ax1.imshow(a, origin='lower', extent=[x1,x2,y1,y2])
    ax1.grid(color='r', linestyle='-', linewidth=1)
    ax1.set_xticks(grid_interval)
    forceAspect(ax1, a.shape[1]/float(a.shape[0]))
    ax2.imshow(b, origin='lower', extent=[x1,x2,y1,y2])
    ax2.grid(color='r', linestyle='-', linewidth=1)
    ax2.set_xticks(grid_interval)
    forceAspect(ax2, a.shape[1]/float(a.shape[0]))
    ax3.imshow(scipy.misc.imread("montezuma_screen_1.png")[::-1], origin='lower', extent=[x1,x2,y1,y2])
    ax3.grid(color='r', linestyle='-', linewidth=1)
    ax3.set_xticks(grid_interval)
    forceAspect(ax3, a.shape[1]/float(a.shape[0]))
    plt.setp(ax2.get_yticklabels(), visible=False)
    plt.setp(ax3.get_yticklabels(), visible=False)
    fig.savefig('shaping.pdf', bbox_inches='tight')
else:
    def print_arr(a):
        print ",\n".join(("{"+",".join("%s"%a[j,i] for i in range(a.shape[0]))+"}")
                    for j in range(a.shape[1]))
    print_arr(a)
    print "}, {"
    print_arr(b)
    print "}};"

    joint = np.empty(shape=(2,0x100,0x100), dtype=np.float64)
    joint[0,...] = a
    joint[1,...] = b
    with open("shaping.pkl", "wb") as f:
        cPickle.dump(joint, f)

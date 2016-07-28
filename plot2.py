#!/usr/bin/env python2

import matplotlib
matplotlib.use('Agg')
import matplotlib.pyplot as plt
import sys

matplotlib.rcParams.update({'font.size': 22})
#$plt.ylim(ymax=1)
#$3plt.ylim(ymin=-0.5)
#plt.xlim(xmin=0)
#plt.xlim(xmax=100)

N = 1000
j = 0
colors = 'rbgmk'
for fname in sys.argv[1:-1]:
    reward = []
    length = []
    ratio = []

    max_reward = 0
    with open(fname, 'r') as f:
        i = 0
        avg_reward = 0
        avg_length = 0
        for line in f.readlines():
            l = line.split(",")
            if len(l) != 2:
                continue
            r = float(l[0])
            max_reward = max(max_reward, r)
            avg_reward += r
            avg_length += float(l[1])
            if i == N:
                reward.append(avg_reward/i)
                length.append(avg_length/i)
                ratio.append(avg_reward/avg_length);
                avg_reward = 0
                avg_length = 0
                i = 0
            i += 1
        if i != 0:
            reward.append(avg_reward/i)
            length.append(avg_length/i)
            ratio.append(avg_reward/avg_length);
    plt.plot(reward, colors[j])
    j += 1
plt.savefig(sys.argv[-1])
#plt.plot(xx, reward, 'r-', xx, ratio, 'g-')
#xx, length, 'b-', 


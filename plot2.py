#!/usr/bin/env python2

import matplotlib
matplotlib.use('Agg')
import matplotlib.pyplot as plt
import sys
import os
import math

W = 7
matplotlib.rcParams.update({'font.size': 10,
                            'figure.figsize': (W, (W/1.61803398875)*.86),
                            'lines.linewidth': 1.5})
#$plt.ylim(ymax=1)
#$3plt.ylim(ymin=-0.5)

N = 1000
j = 0
#colors = 'mbgrk'
colors = ["#83063b", "#bb7784", "#023fa5", "#7d87b9",
          "#11c638", "#8dd593", "#ef9708", "#f0b98d"]
fnames = []
for a in sys.argv[1:]:
    fnames.append(os.path.join(a, 'rewards_nophi.txt'))
    fnames.append(os.path.join(a, 'rewards.txt'))
fnames.append(sys.argv[1]+'.pdf')
mlen = 99999
rewards = []
for fname in fnames[:-1]:
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
    reward.append(avg_reward/i)
    rewards.append(reward)
    mlen = min(mlen, len(reward))
    j += 1
for j in reversed(range(len(rewards))):
    plt.plot(rewards[j], colors[j])
#mlen = int(math.ceil(mlen/10.))*10
plt.xlim(xmin=0, xmax=mlen-1)
plt.savefig(fnames[-1], bbox_inches='tight')
#plt.plot(xx, reward, 'r-', xx, ratio, 'g-')
#xx, length, 'b-', 


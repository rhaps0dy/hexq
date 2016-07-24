#!/usr/bin/env python2

import matplotlib.pyplot as plt
import sys

N = 1000

reward = []
length = []
ratio = []

max_reward = 0
with open(sys.argv[1], 'r') as f:
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
            reward.append(avg_reward/i+1)
            length.append(avg_length/i+1)
            ratio.append(avg_reward/avg_length);
            avg_reward = 0
            avg_length = 0
            i = 0
        i += 1
    if i != 0:
        reward.append(avg_reward/i+1)
        length.append(avg_length/i+1)
        ratio.append(avg_reward/avg_length);

print "Max reward: ", max_reward

#xx = list(range(len(reward)))

plt.plot(reward)
#plt.plot(xx, reward, 'r-', xx, ratio, 'g-')
#xx, length, 'b-', 
plt.show()


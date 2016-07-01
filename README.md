Compile with CMake:

```
cmake .
make
```

#HEXQ

Contains an implementation of
[HEXQ (Hengst, 2002)](www.demo.cs.brandeis.edu/icml02ws/hengst.ps) that works
with the toy problem in the paper.

#Sarsa

Also contains the results for the shaped Sarsa algorithm in "Solving
Montezuma's Revenge with Planning and Reinforcement Learning.

Files:

- `plot_potential.py`: makes the potential function described in Subsection
  3.2.2, and saves it as `shaping.pdf`.
- `montezuma_potential.h`: autogenerated potential function, using
  `plot_potential.py array > montezuma_potential.h`.
- `montezuma_mdp.cpp`: edit the ALE options in the end of the file to change
  whether to show or not the screen, whether to record, ...
- `plain_sarsa.cpp`: is the main entry point. It is so called because HEXQ in
  the same repository is implemented as non-plain, hierarchical, Sarsa. Its
  executable, `plain_sarsa`, is the entry point for the tabular learner. Change
  the experiment name in
  [line 105](https://github.com/rhaps0dy/hexq/blob/master/plain_sarsa.cpp#L105),
  and the epsilon (annealing or no annealing, value) in
  [line 131](https://github.com/rhaps0dy/hexq/blob/master/plain_sarsa.cpp#L131).
  The program saves the Q-value table every 1000 episodes. If invoked as
  `./plain_sarsa <q_table_file>`, it will load the table and start with it. If
  invoked as `./plain_sarsa <file1> <file2> ...`, it will run each Q table for a
  few episodes and give the achieved reward.
  
Within an experiment directory, there are the saved Q tables and two files:
`rewards.txt` and `rewards_nophi.txt`. They are the rewards in each episode,
with and without the potential-based shaping reward, respectively. The first
column of the file has the reward, and the second column the episode length.
They can be plotted using `plot.py`.

Inside the `runs` directory there are the rewards, with and without potential,
that are showed in the thesis.



import numpy as np
import matplotlib.pyplot as plt

# a=np.loadtxt('RunningTime.txt')
a=np.loadtxt('RunningTime_higher.txt')
plt.scatter(a[:,1],a[:,2])
plt.plot(a[:,1],a[:,2])
plt.ylabel('Running Time (in Seconds)')
# plt.xticks(np.arange(1,10,1))

plt.xlabel('Number of Processors')
plt.title('$N=10^7$')
plt.show()
speedup=np.ones(a[:,2].size)*a[0,2]
speedup=speedup/a[:,2]
plt.scatter(a[:,1],speedup)
plt.plot(a[:,1],speedup)
# plt.xticks(np.arange(1,10,1))
plt.xlabel('Number of Processors')
plt.ylabel('Speedup($t_s/t_p$)')
plt.show()

plt.bar(np.arange(len(a[:,1])),speedup)
plt.xticks(np.arange(len(a[:,1])),(a[:,1].astype(int)))
plt.xlabel('Number of Processors')
plt.ylabel('Speedup($t_s/t_p$)')
plt.show()


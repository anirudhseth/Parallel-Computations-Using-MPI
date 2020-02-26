import numpy as np
import pandas as pd
import matplotlib.pyplot as plt

def trueU():
    x=np.linspace(0,1,1000)
    return x*x-x
u=np.loadtxt('outputError.txt')

from sklearn.metrics import mean_squared_error
x=trueU()
mse=[]
for i in range(np.shape(u)[0]):
    mse.append(mean_squared_error(u[i,:], x))
b=np.arange(100)
b=b*10000
plt.plot(b,mse)
plt.title('Mean Squared Error after every 10000 step')
plt.xlabel('Iteration')
plt.legend()
plt.show()

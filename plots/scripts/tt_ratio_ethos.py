import matplotlib.pyplot as plt
import numpy as np
import itertools
from scipy.interpolate import InterpolatedUnivariateSpline

files = ['/home/archidiacono/ethos_class/output/lcdm_cl_lensed.dat', '/home/archidiacono/ethos_class/output/ethos_fig1_n1_cl_lensed.dat','/home/archidiacono/ethos_class/outcamb/ethos_fig1_n1_lensedCls.dat',
'/home/archidiacono/ethos_class/output/ethos_fig1_n2_cl_lensed.dat','/home/archidiacono/ethos_class/outcamb/ethos_fig1_n2_lensedCls.dat',
'/home/archidiacono/ethos_class/output/ethos_fig1_n3_cl_lensed.dat','/home/archidiacono/ethos_class/outcamb/ethos_fig1_n3_lensedCls.dat',
'/home/archidiacono/ethos_class/output/ethos_fig1_n4_cl_lensed.dat','/home/archidiacono/ethos_class/outcamb/ethos_fig1_n4_lensedCls.dat']


data = []
for data_file in files:
    data.append(np.loadtxt(data_file))

fig, ax = plt.subplots()

index, curve0 = 0, data[0]
#ax.plot(curve[:, 0], abs(curve[:, 1])*7.42835025e12, linestyle=':', color='k')

index, curve = 1, data[1]
ax.plot(curve0[:, 0], curve[:, 1]/curve0[:,1]-1.,linestyle='--',color='r')

index, curve = 2, data[2]
interpolation = InterpolatedUnivariateSpline(curve[:,0],curve[:,1])
interpolated = interpolation(curve0[:,0])
ax.plot(curve0[:, 0], interpolated[:]/curve0[:,1]/7.42835025e12-1.,linestyle='-',color='r')

index, curve = 3, data[3]
ax.plot(curve0[:, 0], curve[:, 1]/curve0[:,1]-1.,linestyle='--',color='b')

index, curve = 4, data[4]
interpolation = InterpolatedUnivariateSpline(curve[:,0],curve[:,1])
interpolated = interpolation(curve0[:,0])
ax.plot(curve0[:, 0], interpolated[:]/curve0[:,1]/7.42835025e12-1., linestyle='-', color='b')

index, curve = 5, data[5]
ax.plot(curve0[:, 0], curve[:, 1]/curve0[:,1]-1.,linestyle='--',color='g')

index, curve = 6, data[6]
interpolation = InterpolatedUnivariateSpline(curve[:,0],curve[:,1])
interpolated = interpolation(curve0[:,0])
ax.plot(curve0[:, 0], interpolated[:]/curve0[:,1]/7.42835025e12-1.,linestyle='-',color='g')

index, curve = 7, data[7]
ax.plot(curve0[:, 0], curve[:, 1]/curve0[:,1]-1.,linestyle='--',color='k')

index, curve = 8, data[8]
interpolation = InterpolatedUnivariateSpline(curve[:,0],curve[:,1])
interpolated = interpolation(curve0[:,0])
ax.plot(curve0[:, 0], interpolated[:]/curve0[:,1]/7.42835025e12-1.,linestyle='-',color='k')

ax.legend(['class n=1','camb n=1','class n=2','camb n=2','class n=3','camb n=3','class n=4','camb n=4'], loc='best')

ax.set_xlabel('$\ell$', fontsize=16)
ax.set_ylabel('$C_{\ell}/C_{\ell}^{LCDM}$', fontsize=16)
plt.show()
plt.savefig('tt_ratio_ethos.pdf')

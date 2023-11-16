import numpy as np
import pandas as pd

from helpers import *

from scipy.optimize import curve_fit

from datetime import datetime

import matplotlib.pyplot as plt

def getEqualization( mu, sigma, emu, esigma ):
    equalization = sigma/mu
    error = np.sqrt( (emu/mu)**2 + (esigma/sigma)**2 )*equalization
    return equalization, error

# The fit function
def gaus(x,a,mean,sigma):
    return a*np.exp(-(x-mean)**2/(2*sigma**2))

# The fit strategy
def fitGainsDistribution(hdf, target='q', BINSIZE=0.03,RMIN=0.0,RMAX=1.5, p0=(0.5, 0.2)):

    nbins=int((RMAX-RMIN)/BINSIZE)
    ys,edges = np.histogram(getattr(hdf,target),bins=nbins,range=(RMIN,RMAX))
    xs=np.array([edges[i]+0.5*(edges[i+1]-edges[i]) for i in range(len(ys))])

    param = [np.max(ys), p0[0], p0[1] ]
    bounds = np.array([(param[0]*0.5,param[0]*1.5),
              (param[1]*0.2,param[1]*1.3),
              (param[2]*0.1,param[2]*1.4)])
    
    param,pcov = curve_fit(gaus, xs, ys, p0=param, bounds=(bounds[:,0],bounds[:,1]) )
    param_errors = np.diag(pcov)**0.5
    
    return xs, ys, param, param_errors


def makeplot( timestamp, figname, xs, ys, param, errors ):

    ys_fitted = gaus(xs, *param)
    out = plt.step( xs, ys, where='mid')
    _ = plt.errorbar(xs, ys, yerr=np.sqrt(ys), fmt='', linestyle='', color=out[0].get_color() )

    plt.plot(xs, ys_fitted, '-.', lw=3.0, label="Number of PMTs: {:d} \nMean: {:.2e} \nSigma: {:.1e} \nEqualization: {:.1f}%".format( np.sum(ys), param[1], param[2], (param[2]/param[1])*100) )

    plt.xlabel("Gains [10^7 electrons]", fontsize=12)
    plt.ylabel("# PMTs", fontsize=12)
    plt.legend(title = "%s" % datetime.fromtimestamp(timestamp), fontsize=12)
    plt.grid(alpha=0.5,linestyle="dashed")

    if figname != "":
        plt.savefig( figname, dpi=500 )

    return plt

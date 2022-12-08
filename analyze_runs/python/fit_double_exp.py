import numpy as np
import pandas as pd

import matplotlib.pyplot as plt
from matplotlib.dates import DateFormatter

from scipy.optimize import curve_fit
from datetime import datetime

# Make the reference plot
def dualexpo(x, a1, tau1, a2, tau2):
    return a1*np.exp(-x/tau1) + a2*np.exp(-x/tau2 ) 


def fitDualExp( df, function, p0, bounds , fig):
    """
    Perform a fit using the assigned fit routines
    Takes
        df: Formatted dataframe as result of loadData() function in helpers.py. 
            Assumes it is filtered to hold information for only one PMT 
        function: functional relation to fit to data
        p0: start values of the parameters 
        bounds: allowed range for the parameters
        fig: matplotlib instance
    Returns:
        ..
        ..
    """
    
    # Access to the figure axes
    ax = fig.get_axes()[0]

    
    fit_params=[]
    
    # fixed parameters
    pmt = df.pmt.values[0]
    voltage=df.voltage.values[0]
    tmin = 1610067905
        
    # Extract the data of interest. Convert timestamps in days
    t = np.array([ (tt-tmin) / (3600*24) for tt in df.index ]) #in days
    q = np.array(df["q"])
    eq = np.array(df["eq"])
    npoints = len(q)
    
    # Make the data plot
    if npoints ==0 or np.isnan(voltage):
        print("PMT {} has no correct data associated".format(pmt))
        return fit_params, fig
    label='\n'.join( ['Data PMT %d' % pmt,  
                      '  Operative voltage: %d V' % voltage, 
                      '  Number of time points: %d' % npoints])
    out = ax.errorbar( x=t, y=q, yerr=eq, fmt='o', label=label)
    
    
    # Now we workout the fit
    _sel= (q>0) & (q<0.85)
    
    if pmt==357: 
        _sel = (q>0)
    
    q  = q[np.where(_sel)]
    t  = t[np.where(_sel)]
    eq = eq[np.where(_sel)]
    n_fit_points = len(_sel)
    
    if n_fit_points==0:
        print("PMT {} has no valid data for fit".format(pmt))
        return fit_params, fig
    try:
        params,pcov = curve_fit(function, t, q, sigma=eq , p0=p0, bounds=bounds )
        perrors = np.diag(pcov)**0.5
    except:
        print("PMT {} has a failed fit".format(pmt))
        return fit_params, fig
                
    ndof = len(q)-len(params)
    chi2 = np.sum([(q[i]-function(tt, *params))**2 / eq[i] for i, tt in enumerate(t)])
    
    taumax = np.max( [params[1], params[3]] )
    argmax = np.argmax( [params[1], params[3]] )
        
    taumin = np.min( [params[1], params[3]] )
    argmin = np.argmin( [params[1], params[3]] )
    
    fit_params=[pmt, 
                params[2*argmax], #amax
                taumax, 
                params[2*argmin], #amin
                taumin, 
                perrors[2*argmax], # error amax
                perrors[2*argmax+1], # error taumax
                perrors[2*argmin], #error amin
                perrors[2*argmin+1], # error taumin
                npoints, # all points of the timeserire 
                n_fit_points, # valid points considered for the fit
                chi2, 
                ndof, 
                voltage ] 
                              
    # Now make the plot ( This is sadly too function specific ) 
    time = np.linspace(0, 310, 100) 
                                
    label='\n'.join( ['Fit $a_{max} e^{-t/\\tau_{max}} + a_{min} e^{-t/\\tau_{min}}$: ',
                            '    $a_{max}$: %.2f $\pm$ %.3f' % (params[2*argmax], perrors[2*argmax]),  
                            '    $\\tau_{max}$: %.2e $\pm$ %.3e' % (taumax, perrors[2*argmax+1]), 
                            '    $a_{min}$: %.2f $\pm$ %.3f' % (params[2*argmin], perrors[2*argmin]),  
                            '    $\\tau_{min}$: %.2e $\pm$ %.3e' % (taumin, perrors[2*argmin+1]), 
                            '  n points fit: %d' % ( n_fit_points ),
                            '  $\chi^{2}$: %.2f' % ( chi2 ),
                      ])
                      
    kargs = {'color':out[0].get_color(), 'lw':2 }
    ax.plot( time, [ dualexpo( t, *params ) for t in time ], label=label, **kargs)
    ax.set_ylabel("Fitted gain $q$ [$10^7$ electrons]", fontsize=16)
    ax.set_xlabel( "Elapsed time since %s  [days]" % datetime.fromtimestamp(tmin).strftime("%b %d %Y"), fontsize=16  )

    return fit_params, fig
import numpy as np
import matplotlib.pyplot as plt
import pandas as pd
import os
import sys
from matplotlib.backends import backend_pdf
from scipy.optimize import curve_fit
import scipy.stats as st
import ROOT as r
import warnings
warnings.filterwarnings("error")

###---------------------------------------------------------
## Functions parsing ROOT histograms

def get_bins_edges(bin_centers):

    bin_edges = []
    bin_edges.append(0.0) #first bin left edge

    for i in range(len(bin_centers)):
        length = 0
        if i == (len(bin_centers)-1):
            length = bin_centers[i]-bin_centers[i-1]
        else:
            length = bin_centers[i+1]-bin_centers[i]
        bin_edges.append(bin_centers[i]+length/2.)

    #print(bin_edges)
    return bin_edges

def get_histo_things(hcharge, max, min):
    
    binned_events = []
    bin_centers = []
    for i in range(1,hcharge.GetNbinsX()+1):
        binned_events.append(hcharge.GetBinContent(i))
        bin_centers.append(hcharge.GetXaxis().GetBinCenter(i))
    bin_edges = get_bins_edges(bin_centers)
    
    fbin_centers = [x for x in bin_centers if x < max]
    fbin_edges = [x for x in bin_edges if x < max]
    length = len(fbin_centers)
    fbinned_events = binned_events[:length]
    
    imin = 0
    for i in range(0, length):
        if fbin_centers[i] < min:
            imin = i
        
    xs = fbin_centers[imin+1:]
    ys = fbinned_events[imin+1:]
    
    return fbin_centers, fbin_edges, fbinned_events, xs, ys

###----------------------------------------------------------------
### Functions for fit setup

def gauss(x, a, x0, sigma):
    return a*np.exp(-(x-x0)**2/(2*sigma**2))

def poissonGauss(x, amplitude, n, mu, q, sigma ):
    return amplitude*(np.power(mu,n)*np.exp(-1.0*mu)/np.math.factorial(n)*np.exp(-1.0*(x-q*n)*(x-q*n)/(2.0*n*sigma*sigma))/(sigma*np.sqrt(2.0*np.pi*n)))

def IdealSER(x, mu, q, sigma, amplitude):  
    val = 0
    for n in range(1,5): #da 1 a 4
        val += poissonGauss(x, amplitude, n, mu, q, sigma)
    return val

###--------------------------------------------------------------
### # Get the timestamp
def getTimestamp(file):
    
    file1 = r.TFile(file,"READ")
    tree = file1.Get("bkgcalibration/event")
    tree.GetEntry(0)
    timestamp = tree.timestamp
    file1.Close()     

    return timestamp

###---------------------------------------------------------------
### Main loop

def fit_all(pdf, outfile, filepath, ma, mi):
    
    file1 = r.TFile(filepath,"READ")
    dir1 = file1.Get("bkgcalibration")

    for ch in range(0,360):
        
        hamp = "hamplitude"+str(ch)
        hamp1 = dir1.Get(hamp)
        nentries = int(hamp1.GetEntries())
        line = str(ch) + "," + str(nentries) + ","
        a1, ea1, wa1, xs, ys = get_histo_things(hamp1,ma,mi)
     
        fig = plt.figure(figsize=(8, 5))
        plt.hist(a1, bins=ea1, weights=wa1, histtype="step", label="Ch {} - v09_67_00\nEntries: {}".format(ch, nentries), lw=1.5)
 
        vtrials=np.arange(mi,ma,0.5)
        trials=0
        v=vtrials[0]
    
        param = []
        param_errors = []
        ys_fitted = []
        lab= "SER fit: \n"
    
        repeatfit=True
    
        while repeatfit:
            try:
                param = [ 0.2, v, 1.5, np.max(ys)] # mu, q, sigma, amplitude
                param, pcov = curve_fit(IdealSER, xs, ys, p0=param)
                param_errors = np.diag(pcov)**0.5
                ys_fitted = IdealSER(xs, *param)
                lab+="$\mu$: %.3f $\pm$ %3.e\n" % ( param[0], param_errors[0] ) 
                lab+="$q$: %.3f $\pm$ %3.e\n" % ( param[1], param_errors[1] )
                lab+="$\sigma$: %.3f $\pm$ %3.e\n" % ( param[2], param_errors[2] )
                lab+="$A$: %.0f $\pm$ %1.e\n" % ( param[3], param_errors[3] )
                plt.plot(xs, ys_fitted,'-', lw=2.0, color='red', label=lab)
                for i,p in enumerate(param):
                    line += str(p) + "," + str(param_errors[i]) + ","
                line += "0,48,0\n"
                repeatfit=False
            except:
                trials=trials+1
                if trials<len(vtrials):
                    v=vtrials[trials]
                    #print("ch {} - repeat fit using v: {:.2f}".format(ch,v))
                else: 
                    print("not working - ch {}".format(ch))
                    for i,p in enumerate(param):
                        line += "0,0,"
                    line += "0,48,-1\n"
                    repeatfit=False
        else: 
            repeatfit=False

        plt.grid(color='0.95')
        plt.xlabel(r'Amplitude [mV]')
        plt.ylabel("# events")
        plt.margins(x=0.00)
        plt.legend()
        
        outfile.write(line)
        pdf.savefig( fig )
        plt.close()
    
    return pdf


def main():

    user = os.environ.get("USER")
    args = sys.argv
   
    if (len(args) != 4):
        sys.exit()
    
    run = int(args[1])
    mi = int(args[2])
    ma = int(args[3])
    
    filepath ="/exp/icarus/data/users/{}/pmt-calibration/histograms/pulseDistributionHist_run{}.root".format(user,run)
    timestamp = getTimestamp(filepath)
    outpdf = "/exp/icarus/app/users/{}/pmt-calibration/gain/analyze_runs/python/figs/amplitude_{}.pdf".format(user,run)
    outdb = "/exp/icarus/data/users/{}/pmt-calibration/amplitudedb/bkgphamplitude_run{}_{}.csv".format(user,run,timestamp)

    print("Fitting amplitude histograms from run {} in [{},{}]".format(run,mi,ma))
   
    f = open(outdb,"w")
    f.write("pmt,nentries,mu,emu,q,eq,sigma,esigma,amplitude,eamplitude,chi2,ndf,fitstatus\n")

    pdf = backend_pdf.PdfPages(outpdf)
    fit_all(pdf,f,filepath, ma, mi)
    pdf.close()
    f.close()

if __name__ == "__main__":
    main()


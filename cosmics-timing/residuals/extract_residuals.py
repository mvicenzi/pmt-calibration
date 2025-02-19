import uproot
import pandas as pd
import numpy as np
import matplotlib.pyplot as plt
import os, sys
from scipy import stats
from scipy.optimize import curve_fit
from datetime import datetime
import argparse

date = datetime.today().strftime('%Y%m%d')

def loadSingleFile( tfile, treename, flatenndf=False ):
    ttree = uproot.open(tfile)
    data = pd.DataFrame(ttree[treename].arrays(library="pd"))
    return data

def getdiff( y, t):
    # max y is on top : cosmics are going towards decreasing y 
    return t[np.argmin(y)] - t[np.argmax(y)]

# Define a linear model: t = intercept + slope * y
def linear_model(x, intercept, slope):
    return intercept + slope * x

def fittime( y, t ):

    if(len(y)<4 or len(t)<4):
        print("Not enough data points for linear regression: y = %s, t = %s", y, t)
        return 0,0
    
    # initial guess
    y_range = np.max(y) - np.min(y)
    slope0 = (np.max(t) - np.min(t)) / y_range if y_range !=0 else 0
    intercept0 = np.median(t) - slope0 * np.median(y)
    initp0 = [intercept0, slope0]

    try:

        popt, _ = curve_fit(linear_model, y, t, p0=initp0)
        return popt[0], popt[1]
    
        #old implementation...
        #res= stats.linregress(y, t)
        #print( res.intercept, res.slope)
        #print(res.pvalue)
        #return res.intercept,  res.slope
    
    except Exception as e:
        print(e)
        return 0,0

def residuals( tobs, y, a, b ):
    return tobs -  ( a + b*y )

def readPlacements(file="/exp/icarus/data/users/mvicenzi/pmt-calibration/input/pmt_positions.csv"):
    geo = pd.read_csv(file,sep=",")
    geo.drop(columns=["entry","subentry"],inplace=True)
    return geo

#-----------------------#

def main(args):

    user = os.environ.get("USER") 
   
    RUN = int(args.run)
    PERIOD = args.period
    PECUT = args.pecut

    PATH = "/exp/icarus/data/users/mvicenzi/pmt-calibration/track_matches/"
    OUTPATH = "/exp/icarus/data/users/mvicenzi/pmt-calibration/residualsdb/" + PERIOD + "/"
    COSMICSDB = "/exp/icarus/data/users/mvicenzi/timing-database/pmt_cosmics_timing_data/"
    LASERDB = "/exp/icarus/data/users/mvicenzi/timing-database/pmt_laser_timing_data/"

    FILENAME = PATH + "run{}_matched_light_tracks.root".format(RUN)

    APPLY_LASER = int(args.applylaser)
    APPLY_COSMICS = int(args.applycosmics)
    LASERCORR = LASERDB + args.laserfile
    COSMICSCORR = COSMICSDB + args.cosmicsfile
    #COSMICSCORR = OUTPATH + args.cosmicsfile
    
    suffix = "nocorr"
    if APPLY_LASER:
        suffix = "laseronly"
    if APPLY_LASER and APPLY_COSMICS:
        suffix = "lasercosmics"

    OUTFILE = OUTPATH + "run{}_residuals_{}.csv".format(RUN,suffix)
    
    print("Reading {}".format(FILENAME))

    ### Get optical data
    dfw = loadSingleFile(FILENAME, "trackLightMatchW")
    dfe = loadSingleFile(FILENAME, "trackLightMatchE")
    df = pd.concat([dfe, dfw])
    del dfw
    del dfe

    print("Considering {} track-flash matches".format( len(df) ) )

    ## Explode the dataframe 
    df = df.explode(["pmt_time", "pmt_x", "pmt_y", "pmt_pe", "pmt_z", "pmt_amplitude","channel_id"])

    ## Import and use laser correction
    ## WARNING: DO NOT USE IF CORRECTIONS WERE ALREADY APPLIED AT PREVIOUS STAGES
    ## note: laser corrections are in ns!

    lasercorr = pd.read_csv(LASERCORR, sep=r'\s*,\s*', engine='python')
    lasercorr = lasercorr.rename(columns={'channel': 'channel_id'})
    lasercorr.set_index(["channel_id"], inplace=True)
    lasercorr["t_signal"] = lasercorr["t_signal"]/1e3  #convert ns to us

    df = df.join( lasercorr[["t_signal"]], on=["channel_id"])

    if APPLY_LASER:
        print("Applying laser corrections from {}...".format(LASERCORR))
        df['pmt_time'] = df['pmt_time'] - df['t_signal']  #CURRENTLY ADDING LASER CORRECTIONS!
        
        
    ## Import and use cosmic corrections
    ## WARNING: DO NOT USE IF CORRECTIONS WERE ALREADY APPLIED AT PREVIOUS STAGES
    ## note: cosmics corrections are in ns!

    cosmicscorr = pd.read_csv(COSMICSCORR, sep=r'\s*,\s*', engine='python')
    cosmicscorr = cosmicscorr.rename(columns={'channel': 'channel_id'})
    cosmicscorr.set_index(["channel_id"])
    cosmicscorr["mean_residual_ns"] = cosmicscorr["mean_residual_ns"]/1e3  #convert ns to us

    df = df.join( cosmicscorr[["mean_residual_ns"]], on=["channel_id"])

    if APPLY_COSMICS:
        print("Applying cosmics corrections from {}...".format(COSMICSCORR))
        df['pmt_time'] = df['pmt_time'] - df['mean_residual_ns']  #CURRENTLY ADDING COSMICS CORRECTIONS!    
        
    # drop unneed columns
    df = df.drop(columns=["t_signal"])
    df = df.drop(columns=["mean_residual_ns"])
    
    _sel = (df.pmt_pe > PECUT)
    meandf = df[_sel][["run", "event", "cryo", "flash_id", "pmt_time", "pmt_pe", "pmt_y"]].groupby(["run", "event", "cryo","flash_id", "pmt_y"]).apply( 
        lambda x : pd.Series( {
        "mean_time" : np.mean(x.pmt_time),
        "weight_mean_time" : np.average(x.pmt_time, weights=x.pmt_pe), 
        "error_mean_time": np.std(x.pmt_time) / np.sqrt(len(x.pmt_time)),
    }) ).reset_index()

    meandf = meandf.groupby(["run", "event", "cryo", "flash_id"]).agg(list)

    print("PE cut leaves {} tracks".format( len(meandf) ))
    
    N = 4 # minimum number of quotas available for a good fit (at least 1 pmt >300 PE in each) 
    meandf = meandf[meandf["pmt_y"].apply(lambda x: isinstance(x, (list, np.ndarray)) and len(x) >= N)]

    print("Minimum quotas cut leaves {} tracks".format( len(meandf ) ))
    
    meandf["diff_time"] = meandf.apply( lambda x : getdiff( x.pmt_y, x.mean_time ), axis=1 ) 
    meandf[["intercept", "slope"]] = meandf.apply(lambda x : fittime(x.pmt_y, x.mean_time ), axis=1, result_type="expand" )
    
    # Putting fit back in the exploded dataframe, then compute the residual
    # This should work for every channel_id
    # this is using all slopes, including possible "negative" ones
    dfg = df.join( meandf[["intercept", "slope"]], on=["run", "event", "cryo", "flash_id"], how='inner')
    dfg["residuals"] = dfg.apply( lambda x : residuals(x.pmt_time, x.pmt_y, x.intercept, x.slope), axis=1 ) 

    # TEST: only positive slopes
    #dfg = df.join( meandf[meandf.slope<0][["intercept", "slope"]], on=["run", "event", "cryo", "flash_id"], how='inner')

    # Keep only the residuals on relevant PMT for that event
    dfg = dfg[(dfg.pmt_pe>PECUT)]
    dfg.to_csv("output/dump_run{}_test.csv".format(RUN))
    
    print("Flash-tracks used for the computation of residuals: {}".format( len(dfg.groupby(["run", "event", "cryo", "flash_id"])) ) )
    
    # now group the residual by channel, computing the mean residual for each of them
    # this is the final output which can then be saved!
    us_to_ns = 1e3
    thisdfg = dfg.groupby(["channel_id"]).apply(
        lambda x : pd.Series( { 
            'x': np.mean(x.pmt_x),
            'y': np.mean(x.pmt_y),
            'z': np.mean(x.pmt_z),
            'entries' : len(x.residuals), 
            'pecut' : PECUT,
            'mean_residual_ns' : np.mean(x.residuals)*us_to_ns,
            'std_residual_ns' : np.std(x.residuals)*us_to_ns,
            'emean_ns' : np.std(x.residuals)*us_to_ns/len(x.residuals)
        })).reset_index()
    
    print("Saving residuals to {}...".format(OUTFILE))
    thisdfg.to_csv(OUTFILE, index=False, float_format='%.4f')
    
    rdf = pd.read_csv(OUTFILE)

    # list of PMT channels at 0 voltage from HV files + disconnected pmts:
    offCHs = []
    if PERIOD == "Run_2" or PERIOD=="Run_1":
        offCHs = [350, 248, 215, 190, 161, 139, 127, 103, 131, 59, 52, 21, 5, 71]
    else: # for Run_3 onwards...
        offCHs = [215, 103, 71 ]
    
    geo = readPlacements()
    
    dictionary = {'channel_id':[ a for a in offCHs],
        'x':[ geo["pmt_x"].iloc[a] for a in offCHs],
        'y':[ geo["pmt_y"].iloc[a] for a in offCHs],
        'z':[ geo["pmt_z"].iloc[a] for a in offCHs],
        'entries': [ 0 for a in offCHs],
        'pecut': [ 0. for a in offCHs],
        'mean_residual_ns': [ 0. for a in offCHs],
        'std_residual_ns': [ 0. for a in offCHs],
        'emean_ns': [ 0. for a in offCHs]        
    }

    addf = pd.DataFrame(dictionary)
    rdf = pd.concat([rdf,addf], ignore_index=True)
    rdf.sort_values(by="channel_id", inplace=True)
    thisdfg.to_csv(OUTFILE, index=False, float_format='%.4f')
    
    rdf.to_csv(OUTFILE, index=False, float_format='%.4f')


if __name__ == "__main__":
    
    args = argparse.ArgumentParser()
    args.add_argument("-r", "--run", default=-1)
    args.add_argument("-p", "--period", default="Run_3")
    args.add_argument("-l", "--applylaser", default=True)
    args.add_argument("-c", "--applycosmics", default=False)
    args.add_argument("-f", "--laserfile", default="")
    args.add_argument("-g", "--cosmicsfile", default="")
    args.add_argument("-t", "--pecut", default="")

    main(args.parse_args())



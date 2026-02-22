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
    data = pd.DataFrame(ttree[treename].arrays(library="np"))
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
        return 0,0,0
    
    # initial guess
    y_range = np.max(y) - np.min(y)
    slope0 = (np.max(t) - np.min(t)) / y_range if y_range !=0 else 0
    intercept0 = np.median(t) - slope0 * np.median(y)
    initp0 = [intercept0, slope0]

    try:

        popt, _ = curve_fit(linear_model, y, t, p0=initp0)
        return popt[0], popt[1], 1
    
        #old implementation...
        #res= stats.linregress(y, t)
        #print( res.intercept, res.slope)
        #print(res.pvalue)
        #return res.intercept,  res.slope
    
    except Exception as e:
        print("Fitting failed:", e)
        return 0,0,0

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
    PECUT = float(args.pecut)
    PMT_TIME_FIELD = args.timing_field
    TRACK_TYPE = args.track_type
    RESIDUAL_RANGE = float(args.residual_range)

    # Determine timing definition for filename
    TDEF = "rise"
    if "start" in PMT_TIME_FIELD:
        TDEF = "start"

    PATH = "/exp/icarus/data/users/mvicenzi/pmt-calibration/track_matches/"
    OUTPATH = "/exp/icarus/data/users/mvicenzi/pmt-calibration/residualsdb/" + PERIOD + "/"
    COSMICSDB = "/exp/icarus/data/users/mvicenzi/pmt-database/pmt_cosmics_timing_data/"
    LASERDB = "/exp/icarus/data/users/mvicenzi/pmt-database/pmt_laser_timing_data/"

    if TRACK_TYPE:
        FILENAME = PATH + "run{}_matched_light_tracks_{}.root".format(RUN, TRACK_TYPE)
    else:
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

    if TRACK_TYPE:
        OUTFILE = OUTPATH + "run{}_residuals_{}_{}_{}.csv".format(RUN, TDEF, TRACK_TYPE, suffix)
    else:
        OUTFILE = OUTPATH + "run{}_residuals_{}_{}.csv".format(RUN, TDEF, suffix)
    
    print("Reading {}".format(FILENAME))

    ### Get optical data
    dfw = loadSingleFile(FILENAME, "trackLightMatchW")
    dfe = loadSingleFile(FILENAME, "trackLightMatchE")
    df = pd.concat([dfe, dfw])
    del dfw
    del dfe

    print("Considering {} track-flash matches".format( len(df) ) )

    ## Explode the dataframe 
    df = df.explode(["pmt_time_start", "pmt_time_rise", "pmt_x", "pmt_y", "pmt_pe", "pmt_z", "pmt_amplitude","channel_id"])

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
        df[PMT_TIME_FIELD] = df[PMT_TIME_FIELD] - df['t_signal']  #CURRENTLY ADDING LASER CORRECTIONS!
        
        
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
        df[PMT_TIME_FIELD] = df[PMT_TIME_FIELD] - df['mean_residual_ns']  #CURRENTLY ADDING COSMICS CORRECTIONS!    
        
    # drop unneed columns
    df = df.drop(columns=["t_signal"])
    df = df.drop(columns=["mean_residual_ns"])
    
    _sel = (df.pmt_pe > PECUT)
    meandf = df[_sel][["run", "event", "cryo", "flash_id", PMT_TIME_FIELD, "pmt_pe", "pmt_y"]].groupby(["run", "event", "cryo","flash_id", "pmt_y"]).apply( 
        lambda x : pd.Series( {
        "mean_time" : np.mean(x[PMT_TIME_FIELD]),
        "weight_mean_time" : np.average(x[PMT_TIME_FIELD], weights=x.pmt_pe), 
        "error_mean_time": np.std(x[PMT_TIME_FIELD]) / np.sqrt(len(x[PMT_TIME_FIELD])),
    }) ).reset_index()

    meandf = meandf.groupby(["run", "event", "cryo", "flash_id"]).agg(list)

    print("PE cut leaves {} tracks".format( len(meandf) ))
    
    N = 4 # minimum number of quotas available for a good fit (at least 1 pmt >300 PE in each)
    MIN_MEASUREMENTS_PER_QUOTA = 2  # Average measurements per quota
    meandf = meandf[meandf["pmt_y"].apply(lambda x: isinstance(x, (list, np.ndarray)) and len(x) >= N)]

    print("Minimum quotas cut leaves {} tracks".format( len(meandf ) ))
    
    # Filter fits by average measurements per quota
    meandf = meandf[meandf.apply(lambda x: len(x.mean_time) / len(x.pmt_y) >= MIN_MEASUREMENTS_PER_QUOTA if isinstance(x.pmt_y, (list, np.ndarray)) else False, axis=1)]
    
    print("Minimum measurements per quota cut leaves {} tracks".format( len(meandf ) )) 
    meandf[["intercept", "slope", "status"]] = meandf.apply(lambda x : fittime(x.pmt_y, x.mean_time ), axis=1, result_type="expand" )
    
    # Putting fit back in the exploded dataframe, then compute the residual
    # This should work for every channel_id
    # this is using all slopes, including possible "negative" ones
    dfg = df.join( meandf[["intercept", "slope", "status"]], on=["run", "event", "cryo", "flash_id"], how='inner')

    # TEST: only positive slopes
    #dfg = df.join( meandf[meandf.slope<0][["intercept", "slope"]], on=["run", "event", "cryo", "flash_id"], how='inner')

    dfg["residuals"] = dfg.apply( lambda x : residuals(x[PMT_TIME_FIELD], x.pmt_y, x.intercept, x.slope), axis=1 ) 

    # Keep only the residuals on relevant PMT for that event
    # also rejects events with residuals outside the specified range
    dfg = dfg[(dfg.pmt_pe>PECUT) & (dfg.status>0) & (dfg.residuals<RESIDUAL_RANGE) & (dfg.residuals>-RESIDUAL_RANGE)]
    dfg.to_csv("dumps/dump_run{}_{}_{}.csv".format(RUN, TDEF, suffix))
    
    print("Flash-tracks used for the computation of residuals: {}".format( len(dfg.groupby(["run", "event", "cryo", "flash_id"])) ) )
    
    # now group the residual by channel, computing the mean residual for each of them
    # this is the final output which can then be saved!
    us_to_ns = 1e3
    thisdfg = dfg.groupby(["channel_id"]).apply(
        lambda x : pd.Series( { 
            'x': np.mean(x.pmt_x),
            'y': np.mean(x.pmt_y),
            'z': np.mean(x.pmt_z),
            'entries' : int(len(x.residuals)), 
            'pecut' : PECUT,
            'mean_residual_ns' : np.mean(x.residuals)*us_to_ns,
            'median_residual_ns' : np.median(x.residuals)*us_to_ns,
            'std_residual_ns' : np.std(x.residuals)*us_to_ns,
            'emean_ns' : np.std(x.residuals)*us_to_ns/len(x.residuals)
        })).reset_index()
    
    thisdfg['entries'] = thisdfg['entries'].astype(int)
    
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
    
    rdf.to_csv(OUTFILE, index=False, float_format='%.4f')


if __name__ == "__main__":
    
    args = argparse.ArgumentParser()
    args.add_argument("-r", "--run", default=-1)
    args.add_argument("-p", "--period", default="Run_3")
    args.add_argument("-l", "--applylaser", default=True)
    args.add_argument("-c", "--applycosmics", default=False)
    args.add_argument("-f", "--laserfile", default="")
    args.add_argument("-g", "--cosmicsfile", default="")
    args.add_argument("-t", "--pecut", default="150")
    args.add_argument("--timing-field", default="pmt_time_start", help="PMT timing field: 'pmt_time_start' or 'pmt_time_rise'")
    args.add_argument("--track-type", default="", help="Track type suffix (e.g., 'STD')")
    args.add_argument("--residual-range", default="1.0", help="Residual outlier range in microseconds (default: 1.0)")

    main(args.parse_args())



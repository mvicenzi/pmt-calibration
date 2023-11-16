import uproot3 as uproot
import pandas as pd
import numpy as np
import matplotlib.pyplot as plt
import os, sys
from scipy import stats
from datetime import datetime
date = datetime.today().strftime('%Y%m%d')

def loadSingleFile( tfile, treename, flatenndf=False ):
    ttree = uproot.open(tfile)
    data = ttree[treename].arrays(outputtype=pd.DataFrame,flatten=flatenndf)
    return data

def loadFiles( filelist, treename1, treename2, maxfiles=100, flatenndf=False):
    data1 = pd.DataFrame()
    data2 = pd.DataFrame()
    for i,tfile in enumerate(filelist[0:maxfiles]):
        if i%10 == 0:
            print("{} files processed".format(i))
        ttree = uproot.open(tfile)
        data1 = pd.concat([data1, ttree[treename1].arrays(outputtype=pd.DataFrame,flatten=flatenndf)])
        data2 = pd.concat([data2, ttree[treename2].arrays(outputtype=pd.DataFrame,flatten=flatenndf)])
    return data1, data2

def getdiff( y, t):
    # max y is on top : cosmics are going towards decreasing y 
    return t[np.argmin(y)] - t[np.argmax(y)]

def fittime( y, t ):
    try:
        res= stats.linregress(y, t)
        #print( res.intercept, res.slope)
        return res.intercept,  res.slope
    except:
        return 0,0

def compute_residuals( tobs, y, a, b ):
    return tobs -  ( a + b*y )

#-----------------------#

def main():

    user = os.environ.get("USER")
    args = sys.argv
   
    if (len(args) != 3):
        sys.exit()
    
    RUN = int(args[1])
    CORR = int(args[2])
    MATCHES = "output/run{}_matched_light_tracks.txt".format(RUN)
    LIGHTINFO = "inputs/run{}_tracks_BNBMAJORITY_files.txt".format(RUN)
    OUTFILE = "output/residuals/Run_2/run{}_residuals_test8ns_fixEast.csv".format(RUN,CORR)
    FILENAMES = [ line.strip() for line in open(LIGHTINFO, "r") ]

    print("Extracting residuals from {}.".format(RUN))
    #print(" and correcting with cosmics from {}".format(CORR))

    #maxim = 100
    maxim = len(FILENAMES)
    print(maxim)

    ## Get the light data and combine the two cryostats
    dfw, dfe = loadFiles(FILENAMES, "simpleLightAna/opflashCryoW_flashtree", "simpleLightAna/opflashCryoE_flashtree", maxim)
    dfw["cryo"] = 1
    dfe["cryo"] = 0
    dfw.drop(columns=["multiplicity","multiplicity_right","multiplicity_left","sum_pe","sum_pe_right","sum_pe_left"],inplace=True)
    dfe.drop(columns=["multiplicity","multiplicity_right","multiplicity_left","sum_pe","sum_pe_right","sum_pe_left"],inplace=True)
    df = pd.concat([dfe, dfw])

    del dfw
    del dfe

    ## Now match with the selected tracks
    dfmatches = pd.read_csv(MATCHES)
    dfmatches.rename(columns={'flashID':"flash_id"}, inplace=True)
    dfmatches.set_index(["run", "event", "cryo", "flash_id"], inplace=True)
    df = (df.join( dfmatches, on=["run", "event", "cryo", "flash_id"], how='inner'))
    df["channel_id"] = df.pmt_y.apply( lambda x : np.arange(len(x)) )

    print("Consider {} tracks".format( len(df) ) )

#    fig, ax = plt.subplots(1,2, figsize=(12, 4.3))

#    ax[0].plot( [df.trackEndZ, df.trackStartZ], [df.trackEndY, df.trackStartY], color='black', lw=0.1 )
#    ax[1].plot( [df.trackEndX, df.trackStartX], [df.trackEndY, df.trackStartY], color='black', lw=0.1 )

    # consider steeper angle
#    _sel_dir_z= (df.trackDirZ > -0.3) & (df.trackDirZ < 0.3 ) 
#    _sel_dir_x = (df.trackDirX > -0.1) & (df.trackDirX < 0.1 )
#    _seldir= _sel_dir_z
    #ax[0].plot( [dfmatches[_seldir].trackEndZ, dfmatches[_seldir].trackStartZ], [dfmatches[_seldir].trackEndY, dfmatches[_seldir].trackStartY], color='red', lw=0.1 )
    #ax[1].plot( [dfmatches[_seldir].trackEndX, dfmatches[_seldir].trackStartX], [dfmatches[_seldir].trackEndY, dfmatches[_seldir].trackStartY], color='red', lw=0.1 )
#    print( len(df), len(df[_seldir]) )
#    ax[0].set_ylabel("Y [cm]" )  
#    ax[0].set_xlabel("Z [cm]" )  
#    ax[1].set_ylabel("Y [cm]" )  
#    ax[1].set_xlabel("X [cm]" )  
#    plt.savefig("figs/run{}_tracks_standard_selection.png".format(RUN),dpi=300)
    #plt.show()

    ## Explode the dataframe 
    df = df.explode(["time_pmt", "pmt_x", "pmt_y", "pe_pmt", "pmt_z", "channel_id"])

    ### REMOVE 8ns phase correction from WE-TOP-C
    wetopc = [238, 239, 235, 236, 237, 230, 233, 234, 232, 231, 220, 223, 224, 222, 221]
    df.loc[(df['channel_id'].isin(wetopc))&(df['pe_pmt']>0), 'time_pmt'] += 0.008 #in us

    eetopb = [78, 79, 75, 76, 77, 68, 69, 65, 66, 67, 60, 63, 64, 62, 61]
    df.loc[(df['channel_id'].isin(eetopb))&(df['pe_pmt']>0), 'time_pmt'] -= 0.008 #in us

    eetopc = [58, 59, 55, 56, 57, 50, 53, 54, 52, 51, 40, 43, 44, 42, 41]
    df.loc[(df['channel_id'].isin(eetopc))&(df['pe_pmt']>0), 'time_pmt'] -= 0.008 #in us

    ewbotb = [110, 111, 112, 113, 114, 118, 116, 117, 115, 119, 100, 101, 102, 104, 103]
    df.loc[(df['channel_id'].isin(ewbotb))&(df['pe_pmt']>0), 'time_pmt'] += 0.006 #in us

    ## WARNING: adding cosmics corrections
    #COSMICSCORR = "output/residuals/Run_2/run{}_residuals_test8ns_fixEast.csv".format(CORR)
    #cosmics = pd.read_csv(COSMICSCORR).set_index("channel_id")
    #df = df.join( cosmics["mean_residual_ns"], on="channel_id" )
    #df["time_pmt"] = df["time_pmt"]-df["mean_residual_ns"]/1e3 #convert ns to us

    _pecut=300
    _sel = df.pe_pmt > _pecut
    meandf = df[_sel][["run", "event", "cryo", "flash_id", "time_pmt", "pe_pmt", "pmt_y"]].groupby(["run", "event", "cryo", "flash_id", "pmt_y"]).apply( 
        lambda x : pd.Series( {
        "mean_time" : np.mean(x.time_pmt),
        "weight_mean_time" : np.average(x.time_pmt, weights=x.pe_pmt), 
        "error_mean_time": np.std(x.time_pmt) / np.sqrt(len(x.time_pmt)),
        }) ).reset_index()

    meandf = meandf.groupby(["run", "event", "cryo", "flash_id"]).agg(list)

    print("PE cut leaves {} tracks".format( len(meandf) ))
    
    meandf["diff_time"] = meandf.apply( lambda x : getdiff( x.pmt_y, x.mean_time ), axis=1 ) 
    meandf[["intercept", "slope"]] = meandf.apply(lambda x : fittime(x.pmt_y, x.mean_time ), axis=1, result_type="expand" )
    
    # Putting fit back in the exploded dataframe, then compute the residual
    # This should work for every channel_id
    
    # TEST: only positive slopes
    #dfg = df.join( meandf[meandf.slope<0][["intercept", "slope"]], on=["run", "event", "cryo", "flash_id"], how='inner')
    
    # all slopes
    dfg = df.join( meandf[["intercept", "slope"]], on=["run", "event", "cryo", "flash_id"], how='inner')
    dfg["residuals"] = dfg.apply( lambda x : compute_residuals(x.time_pmt, x.pmt_y, x.intercept, x.slope), axis=1 ) 
    
    # Keep only the residuals on relevant PMT for that event
    PECUT = 300
    dfg = dfg[dfg.pe_pmt>PECUT]
    
    print("Using the above slopes leaves {} tracks".format( len(dfg.groupby(["run", "event", "cryo", "flash_id"])) ) )
    
    #dfg.to_csv("dump_run{}_test8ns.csv".format(RUN))

    # ## Group and save residuals
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
    
    #add off channels
    geo = loadSingleFile(FILENAMES[0], "simpleLightAna/geotree")
    x = geo.pmt_x.values[0] ; y = geo.pmt_y.values[0] ; z = geo.pmt_z.values[0]

    offCHs = [350, 248, 215, 190, 161, 139, 127, 103, 131, 59, 52, 21, 5, 71]
    dict = {'channel_id':[ a for a in offCHs],
        'x':[ x[a] for a in offCHs],
        'y':[ y[a] for a in offCHs],
        'z':[ z[a] for a in offCHs],
        'entries': [ 0 for a in offCHs],
        'pecut': [ 0. for a in offCHs],
        'mean_residual_ns': [ 0. for a in offCHs],
        'std_residual_ns': [ 0. for a in offCHs],
        'emean_ns': [ 0. for a in offCHs]        
    }

    addf = pd.DataFrame(dict)
    thisdfg = pd.concat([thisdfg,addf], ignore_index=True)
    thisdfg.sort_values(by="channel_id", inplace=True)

    thisdfg.to_csv(OUTFILE, index=False, float_format='%.4f')
    
    selected_channel = 28
    residuals = dfg[dfg.channel_id==selected_channel].residuals.values
    
    fig = plt.figure(dpi=100)
    
    lab = "Channel ID "+str(selected_channel)+"\n"
    lab += "Entries: {}\n".format(len(residuals))
    lab += "Mean: {:.2f} ns\n".format(np.mean(residuals*1e3))
    lab += "Std: {:.2f} ns".format(np.std(residuals*1e3))
    
    plt.hist( residuals*1e3, bins=50, range=(-10,20), histtype='step', label=lab)
    #plt.ylabel("# entries", fontsize=16)
    plt.xlabel("Residuals [ns]", fontsize=14)
    plt.tight_layout()
    plt.grid(linestyle="dashed", alpha=0.5)
    plt.legend(fontsize=12)
    plt.savefig("figs/run{}_channel_{}_residuals_TEST8ns_fixEast.png".format(RUN,selected_channel,CORR),dpi=100)
    #plt.show()
    
    # Plotting full distribution 
    fig = plt.figure(dpi=100)
    
    rmin=-10
    rmax=10
    r=(rmin,rmax)
    s=0.5
    b=int((rmax-rmin)/s)
    
    lab = "Run {}\nMean: {:.2f} ns\nStd: {:.2f} ns".format(RUN,np.mean(thisdfg.mean_residual_ns),np.std(thisdfg.mean_residual_ns))
    
    plt.hist(thisdfg.mean_residual_ns, bins=b, linewidth=2, range=r, histtype="step", label=lab)
    
    plt.xlabel("Time residual [ns]")
    plt.ylabel("# PMTs")
    plt.legend()
    plt.grid(linestyle="dashed", alpha=0.5)
    plt.savefig("figs/run{}_residuals_TEST8ns_fixEast.png".format(RUN,CORR),dpi=100)
    #plt.show()
    
    slopes = meandf["slope"].values
    len(slopes)
    
    fig = plt.figure(dpi=100)
    
    plt.hist( slopes*1e3, bins=50, range=(-0.1,0.075), histtype='step')
    plt.ylabel("# Flash-Track matches", fontsize=14)
    plt.xlabel("Fitted slope [cm ns$^{-1}$]", fontsize=14)
    
    plt.axvline(x=0.,color="red",linestyle="dotted")
    
    #plt.tight_layout()
    plt.grid(linestyle="dashed",alpha=0.5)
    #plt.legend()
    plt.savefig("figs/run{}_slope_distribution_TEST8ns_fixEast.png".format(RUN,CORR),dpi=100)
    #plt.show()

if __name__ == "__main__":
    main()



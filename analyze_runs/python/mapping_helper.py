import numpy as np
import pandas as pd
import os, re
from datetime import datetime

def convert(chID, map_file):
    
    dmap = pd.read_csv(map_file, sep=',')
    pmtID = dmap.loc[dmap['channel_id'].isin(chID)]
    
    return pmtID["pmt_id"]

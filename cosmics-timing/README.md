# Cosmics timing corrections
This calibration step exploits the abundant downward-going cosmic muons crossing the detector.
Considering vertical downward-going muons, a linear relationship is expected between the PMT y-coordinate and the time light is collected.
Deviations can arise depending on the relative position between the track and each tube, but dditional selection criteria (PE cut, opposite wall averaging) can be applied event-by-event to recover the linear relationship.
Time residuals can therefore be computed between single PMT times and the expected value at that quota from the linear fit.
Considering a large sample of tracks, a distribution of residuals is obtained for each PMT channel.
The means of these distributions are subsequently used as channel-by-channel correction factors to further align the PMT times.

## How to run
The code is splitted into three parts:
* [track-matching](./track-matching): This directory contains script/macros to collect calibration ntuples and apply the track selection.
  The result is a `.root` file containing flash-matched tracks that can be used for the extracting the time residuals.
  The code is flexible enough to be able to add/remove previously-applied laser or cosmics corrections.
* [residuals](./residuals): This directory contains Python notebooks and scripts that extract the timing residuals.
  The result is a `.csv` file with mean time residuals per channel.
  Additional notebooks are available for event-diplay plotting and quick checks on the residuals.
* [validation](./validation): This directory contains more advanced python notebooks for further validation of the residuals.
   
  


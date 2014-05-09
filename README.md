Author: daid kahl
Name: ffpeaks version 0.5
Runs by: ROOT macro
Does: FINDS and FITS peaks with Gaussians, and then linearizes the results
What: Calibrate your silicon strip detectors with an alpha source!
Credits: Some ideas from the examples multifit.C and peaks.C by Rene Brun 
Last updated: 05 Jun 2012 20:21:47 

Call with no arguements to see options:
daid@titanium ~/active/progs/alpha-calib-root % root 

*****************************
* Welcome to ROOT v5.34/18 *
*****************************

root [0] .x ffpeaks.C()
Usage:
ffpeaks.C(const Int_t np=0, char run[100], const char *ch_name, const Int_t ch_start, const Int_t ch_stop=ch_start, const Int_t interactive=1, const Int_t minimum, const Int_t maximum)
        np: number of peaks.  Default: 0 (this message).
        run: Name of run file (DO NOT PUT '.root'). Default: nothing.  Must be in double quotes!
                ch_start: channel to start analyzing.
        [ch_stop]: channel to stop analyzing.  Default: ch_stop=ch_start.
        [interactive]: Toggle on interactivity; 1 is interactive, 0 is automated.  Default: 1
        [minimum]: Minimum value (increase to remove pedestal); Default: 0
        [maximum]: Maximum value; Default: 4096


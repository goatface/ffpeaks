/*
            DO WHAT THE FUCK YOU WANT TO PUBLIC LICENSE
                    Version 2, December 2004

 Copyright (C) 2010, 2011, 2012, 2015 daid kahl; 2021 Thomas Chillery

 Everyone is permitted to copy and distribute verbatim or modified
 copies of this license document, and changing it is allowed as long
 as the name is changed.

            DO WHAT THE FUCK YOU WANT TO PUBLIC LICENSE
   TERMS AND CONDITIONS FOR COPYING, DISTRIBUTION AND MODIFICATION

  0. You just DO WHAT THE FUCK YOU WANT TO.
*/

// Name: ffpeaks version 1.0
// Runs by: ROOT6 macro
// Does: FINDS and FITS peaks with Gaussians, and then linearizes the results
// What: Calibrate your silicon strip detectors with an alpha source!
// Credits: Some ideas from the examples multifit.C and peaks.C by Rene Brun 
// Last updated: 14 Jan 2022 12:27:28  

// some includes we need
#include "TCanvas.h" // draws stuff
#include "TMath.h" // does math
#include "TH1.h"  // makes historgrams
#include "TF1.h" // makes lines
#include "TSpectrum.h" // finds peaks
#include <iomanip> // setprecision call (output is not crazy)

// output to user on null call
void Usage(){ 
   cout << "ffpeaks v. 0.62\n";
   cout << "\tAuthor: daid kahl\n";
   cout << "Usage:\n";
   cout << "ffpeaks.C(const Int_t np=0, char run[100], const char *ch_name, const Int_t ch_start, const Int_t ch_stop=ch_start, const Int_t interactive=1, const Int_t minimum, const Int_t maximum)" <<endl;
   cout << "\tnp: number of peaks.  Default: 0 (this message).\n";
   cout << "\trun: Name of run file (DO NOT PUT '.root'). Default: nothing.  Must be in double quotes!\n";
   cout << "\tch_name: name of the histogram base (\"strip_ch1\" base is \"strip_ch\").  Must be in double quotes!\n";
   cout << "\tch_start: channel to start analyzing.\n";
   cout << "\t[ch_stop]: channel to stop analyzing.  Default: ch_stop=ch_start.\n";
   cout << "\t[interactive]: Toggle on interactivity; 1 is interactive, 0 is automated.  Default: 1\n"; 
   cout << "\t[minimum]: Minimum value (increase to remove pedestal); Default: 0\n";
   cout << "\t[maximum]: Maximum value; Default: 4096\n";
   cout << "\t\tNOTE! [minimum] and [maximum] take *bin* value and not the *value*.  Please check your histogram binning!\n";
}


void ffpeaks(const Int_t np=0,char run[100]=" ",const char *ch_name=" ",const Int_t ch_start=0,const Int_t ch_stop=0, const Int_t interactive=1, const Int_t minimum=0, const Int_t maximum=4096) {
   // A few other options
   // Even if you are not a coding expert, you should be free to change these as you need!!
   // calibsets and calibpeaks all may need to be changed if you don't use CRIB alpha sources 2 and 3 (or some strange condition)


   //halfwidth is used for the Gaussian fitting. 
   // 	Spectra from different detectors and data runs often need this to be tuned.
   
   // 70 um SSD
   //Int_t halfwidth=10;
   Int_t halfwidth=50;
   // ACTIVE TARGET 500 um SSD
   //Int_t halfwidth=150;
   //for pad
   //Int_t halfwidth=20;
   //for strips
   //Int_t halfwidth=150;
   //for pedestal
   //Int_t halfwidth=5;
   
   //npeaks_help is the number of helper peaks (added to the main npeaks you WANT to find
   // 	If one peak is much smaller and is not found before the shoulder of a larger peak, increase this!
   // 	The code has an automagically user interaction way to discard the false peaks spectra-by-spectra
   
   const Int_t npeaks_help=0; 
   
   //[peak_autofind]: Toggle on peak finding; 1 is automated, 0 is direct user input.  Default: 1
   
   const Int_t peak_autofind=1;
   
   //[linearize]: Toggle on linear fitting; 1 is linear fitting ON, 0 is linear fitting OFF.  Default: 1
   
   const Int_t linearize=1;
   
   //[printpeaks]: Toggle on printing peaks to file; 1 is peak printing ON, 0 is peak printing OFF.  Default: 0
   //  Disable this unless you need to debug or confirm.  
   //  You DO NOT want these output to the calibration file as a general rule.
   
   const Int_t printpeaks=0;

   // You can rebin your histogram here.  Binning that is too small for raw data finds to many peaks in the same region

   const Int_t rebin=2;
   
  if (np==0){
      Usage();
      return;
   }
   char run_name[105];
   sprintf(run_name,"%s.root",run);
   TFile *f;
   if (!(f = TFile::Open(run_name))) // Open the file or quit
     return; // break;
   char foutname[104];
   sprintf(foutname,"%s-%s-ch%d-%d.txt",run,ch_name,ch_start,ch_stop);
   ofstream fout;
   fout << setprecision(8); // good for column output data
   fout.open(foutname);
   TCanvas *c1 = new TCanvas("c1");
   Int_t npeaks;
   Int_t nfound;
   char name[100];
   for (Int_t ch_no=ch_start;ch_no<ch_stop+1;ch_no++){
     top:  
     Int_t pdiscard[101];
     sprintf(name,"%s%d",ch_name,ch_no);
     cout << "Beginning peak finding in histogram " << name << endl;
     fout << "# " << name << " " << run_name ;
     npeaks = TMath::Abs(np);
     TH1F *h = (TH1F*)(f->Get(name))->Clone("h");
     h->Rebin(rebin);
     h->GetXaxis()->SetRange(minimum,maximum); 
     h->GetXaxis()->SetRangeUser(minimum,maximum); 
     TH1F *h2 = (TH1F*)h->Clone("h2");
     //h2->Rebin(10);
     Float_t xpeaks[np];
     if ( peak_autofind == 1){
       //Use TSpectrum to find the peak candidates
       TSpectrum *s = new TSpectrum(npeaks+npeaks_help);// change the scale factor to find more peaks (if not successful with 1)
       nfound = s->Search(h,2,"",0.10);
       printf("Found %d candidate peaks to fit\n",nfound);
       Double_t *xpeaksfound = s->GetPositionX();
       // check the np (number of peaks entered by user) versus nfound (number of peaks the routine found)
       if (np <=0){ 
       	cout << "No peaks found!" << endl;
          return;
       }
       if (nfound < np){
            cout << "Could not find " << np << " peaks as user input requests!" << endl;
            cout << "Continue with this channel anyway? (yes/no): ";
            char input;
            std::cin >> input;
            switch (input){
            case 'y': case 'Y': case 'yes': case 'Yes': { 
              const Int_t np=nfound; continue; // redfinition of np might give invalid pointer error...needs testing!
            }
            //case 'y': case 'Y': case 'yes': case 'Yes': const Int_t np=nfound; continue; // redfinition of np might give invalid pointer error...needs testing!
            case 'n': case 'N': case 'no': case 'No': { 
              ch_no++; if (linearize) fout << "\n0.0\t\t1.0\n"; goto top;
            }
            default: cout << "User input error...exiting." << endl; 
            }
       }
       // by default we will try to keep ALL peaks
       for (Int_t p=0;p<nfound;p++)
          pdiscard[p]=1;
       // if there are too many peaks, we should get rid of some
       h->Draw();
       c1->Update();
       if (nfound > np){
       cout << "More than " << np << " peaks found!" << endl;
         for (Int_t p=0;p<nfound;p++){
           cout << "Peak number " << p+1 << " centered near " << xpeaksfound[p] << endl;}
         cout << "Please discard " << nfound-np << " peak(s)." << endl;
         Int_t discard;
         for (Int_t p=np;p<nfound;p++){
           cout << "Discard peak number: ";
           std::cin >> discard;
           pdiscard[discard-1]=0;
         }
       }
       Int_t pkeep=0;
       for (Int_t p=0;p<nfound;p++){
         if (pdiscard[p] == 1){
            xpeaks[pkeep]=xpeaksfound[p];
            pkeep++;
         }
       }
     }
     else {
       for (Int_t n=0;n<npeaks;n++){
         cout << "Input approximate centroid for peak " << n << ": " ;
	 cin >> xpeaks[n];
       }
     }
     
     // fit the peaks we found and decided to keep
     npeaks = 0;
     const Int_t npar = np;
     const Int_t npar3 = np * 3;
     Double_t par[npar3];
     TF1 *fit[npar];
     Double_t xfit[np];
     for (Int_t p=0;p<np;p++) {
        sprintf(name,"fit%d",p);
        fit[p] = new TF1(name,"gaus",xpeaks[p]-halfwidth,xpeaks[p]+halfwidth);
        h2->Fit(name,"QR+");
        fit[p]->GetParameters(&par[p*3]);
	xfit[p] = par[p*3+1];
        cout << "Centroid for peak " << p+1 << ": " << xfit[p] << endl;
        npeaks++;
     }
     // find the minimum and maxium X values among the peaks found  -- this is just for the plot range
     Float_t pmin=xpeaks[0],pmax=0;
     for (Int_t p=0;p<np;p++){
          if (pmax<xpeaks[p])
          	pmax=xpeaks[p];
          if (pmin>xpeaks[p])
          	pmin=xpeaks[p];
     }
     // set up the fit parameters for an arbitrary number of peaks np>=1
     string fitparams="gaus(0)";
     char fitparam[8];
     for (Int_t p=1;p<np;p++){
          sprintf(fitparam,"+gaus(%d)",p*3);
          fitparams.append(fitparam);
     }
     // plot all three peaks we found
     TF1 *total = new TF1("total",fitparams.c_str(),pmin-halfwidth,pmax+halfwidth);   
     total->SetParameters(par);
     h2->Fit(total,"QR+");
     Double_t xmin = h2->GetXaxis()->GetXmin();
     h2->GetXaxis()->SetRange(pmin-xmin-halfwidth,pmax-xmin+halfwidth);
     h2->Draw();
     // pause for the user to see the resulting fits for sanity checks -- only executes if this is not the last run and if run as interactive (default).
     if (ch_no<ch_stop  && interactive==1 ){
       cout << "Done fitting histogram " << name << endl << "Press a keystroke and <CR> to continue:" << endl;
       std::string dummy;
       if (nfound != np) // this is a stupid hack to clear the buffer, sorry.
         cin.ignore();
       std::getline(std::cin, dummy);
       cout << dummy  << endl;
     }
     // a crappy bubble sort on xfit, since the TSpectrum default gives them in order of peak height, not peak value
     // however, alpha calibration parameter order is fixed from lowest to highest energy
     // hence, bubble sort
     // if np is large, should make a more efficient sorting routine
     Double_t temp;
     for (Int_t i=1;i<=np;i++){
       for (Int_t j=0;j<(np-1);j++){
         if (xfit[j+1] < xfit[j])      // ascending order
           { 
             temp = xfit[j];             // swap elements
             xfit[j] = xfit[j+1];
             xfit[j+1] = temp;
           }
       }
     }
     // can check the bubble sort
     /*
     for (Int_t p=0;p<np;p++){
	cout << "xfit[" << p << "]=" << xfit[p] << endl;
     }*/
     
     // run linear calibration on the peaks we found
     if (linearize==1){
	Double_t datapoints[np];
	 for (Int_t i=0;i<np;i++){
	   datapoints[i]=xfit[i];
	 } 
	// CRIB Specific settings for our two main alpha sources.  
	// If you have an unusual condition, or a different alpha set, you MUST CHANGE THIS
	const Int_t calibsets=2; // there are two calibration sets, but only one will apply to any detector
	// don't worry, we chi square fit to determine automagically the alpha source used!
        //Double_t calibpeaksall[calibsets][3] = {4.780,5.480,5.795,3.180,5.480,5.795}; // set 0 is CRIB alpha 2, set 1 is CRIB alpha 3
        Double_t calibpeaksall[calibsets][3] = {4.780,5.480,5.795,3.148,5.462,5.771}; // set 0 is CRIB alpha 2, set 1 is CRIB alpha 3
        Double_t calibpeaks[np]; 
        Double_t linpar[calibsets*2];
        Double_t linchi2[calibsets];
        TF1 *linear[calibsets]; 
        TGraph *Graph[calibsets];
        // fit both calibration sets
        for (Int_t i=0;i<calibsets;i++){
          sprintf(name,"linear%d",i);
          linear[i] = new TF1(name,"pol1");
          for (Int_t j=0;j<3;j++){
            calibpeaks[j]=calibpeaksall[i][j];
          }
          Double_t *x = datapoints;
          Double_t *y = calibpeaks;
          Graph[i] = new TGraph(np,x,y);
          Graph[i]->Fit(name,"Q");
          linear[i]->GetParameters(&linpar[i*2]);
          linchi2[i]=linear[i]->GetChisquare();
        }
        Int_t calibuse;
        Double_t minchi2 = 1000;
        // determine which calibration set has a better Chi^2
        for (Int_t i=0;i<calibsets;i++){
           if (linchi2[i] < minchi2){
             minchi2 = linchi2[i];
             calibuse = i;
           }
        }
        // plot and output the results
        Graph[calibuse]->Draw("A*");
        c1->Update();
        fout << " CRIB alpha source " << calibuse+2 << " Chisquare: "  << linchi2[calibuse] << endl;
        fout << -linpar[calibuse*2]/linpar[calibuse*2+1] << "\t" << linpar[calibuse*2+1] << endl;
    } //if linearize==1
    if (printpeaks==1){
	fout << endl;
        for (Int_t n=0;n<np;n++){
           fout << xfit[n] << " ";
	}
	fout << endl;
    }
   } //for closure ch_no
  fout.close();
} // ffpeaks closure

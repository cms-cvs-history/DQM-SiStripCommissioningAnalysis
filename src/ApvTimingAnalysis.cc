#include "DQM/SiStripCommissioningAnalysis/interface/ApvTimingAnalysis.h"
#include "FWCore/MessageLogger/interface/MessageLogger.h"
#include "TProfile.h"
#include <iostream>
#include <iomanip>
#include <sstream>
#include <cmath>

using namespace std;

// ----------------------------------------------------------------------------
// temporarily is wrapping orginal analysis() method
void ApvTimingAnalysis::analysis( const TProfile* const histo, 
				  ApvTimingAnalysis::Monitorables& mons ) { 
  vector<const TProfile*> histos; 
  histos.push_back( const_cast<const TProfile*>(histo) );
  vector<unsigned short> monitorables;
  analysis( histos, monitorables );
  mons.coarse_ = monitorables[0];
  mons.fine_   = monitorables[1];
  mons.delay_  = 24*monitorables[0] + monitorables[0];
}

// ----------------------------------------------------------------------------
// temporarily is wrapping orginal analysis() method
void ApvTimingAnalysis::Monitorables::print( stringstream& ss ) { 
  ss << " PLL coarse/fine delays:  " 
     << coarse_ << "/" 
     << fine_ << "\n"
     << " Timing delay/error [ns]: " 
     << setprecision(1) << delay_ << "+/-" 
     << setprecision(1) << error_ << "\n"
     << " Base/Peak/Height [adc]:  " 
     << setprecision(1) << base_ << "/" 
     << setprecision(1) << peak_ << "/" 
     << setprecision(1) << height_ << "\n";
}

// ----------------------------------------------------------------------------

void ApvTimingAnalysis::analysis( const vector<const TProfile*>& histos, vector<unsigned short>& monitorables ) {
  
   edm::LogInfo("Commissioning|Analysis") << "[ApvTimingAnalysis::analysis]";

  //extract root histogram
  //check 
  if (histos.size() != 1) { edm::LogError("Commissioning|Analysis") << "[ApvTimingAnalysis::analysis]: Requires \"const vector<const TH1F*>& \" argument to have size 1. Actual size: " << histos.size() << ". Monitorables set to 0."; 
  
  monitorables.clear(); monitorables.reserve(2); monitorables.push_back(0); monitorables.push_back(0);
  return; }

   //containers
  pair< unsigned short, unsigned short > coarse_fine = pair< unsigned short, unsigned short >(0,0);
  const TProfile* histo = histos[0];

 //check 
  if ((unsigned short)histo->GetNbinsX() <= 2) { edm::LogError("Commissioning|Analysis") << "[ApvTimingAnalysis::analysis]: Too few bins in histogram. Number of bins: " << (unsigned short)histo->GetNbinsX() << ". Minimum required: 2.";
  monitorables.clear(); monitorables.push_back(0); monitorables.push_back(0);
  return; }

  vector<unsigned short> binContent; binContent.reserve(((unsigned short)histo->GetNbinsX() - 2)); binContent.resize(((unsigned short)histo->GetNbinsX() - 2), 0);
  
  float maxderiv=-9999.;
  unsigned short ideriv = 0;

  for (unsigned short k = 2; k < (unsigned short)histo->GetNbinsX(); k++) { // k is bin number
    
    //fill vector with histogram contents
    binContent.push_back((unsigned int)(histo->GetBinContent(k)));
    
    //calculate the 2nd derivative of the readout...
    
    float deriv = (unsigned int)histo->GetBinContent(k+1) - (unsigned int)histo->GetBinContent(k-1);
    	if (deriv>maxderiv)
     {
       maxderiv=deriv;
       ideriv=k;
     }
  }

 //calculate median
  sort(binContent.begin(), binContent.end());

  //calculate mean and mean2 of the readout within cutoffs

  float meanNoise = 0.;
  float mean2Noise = 0.;

  for (unsigned short k = (unsigned short)(binContent.size()*.1); k < (unsigned short)(binContent.size()*.9); k++) {
    meanNoise += binContent[k];
    mean2Noise += binContent[k]*binContent[k];
 }

  meanNoise = meanNoise / (binContent.size() * 0.8);
  mean2Noise = mean2Noise / (binContent.size() * 0.8);
  float sigmaNoise = sqrt(fabs(meanNoise*meanNoise - mean2Noise));

    // check 35 elements after max dervivative are > meanNoise + 2*sigmaNoise
  for (unsigned short ii = 0; ii < 35; ii++) {
    if ((short)histo->GetBinContent(ideriv + ii) < (meanNoise + 2*sigmaNoise)) {
      LogDebug("Commissioning|Analysis") << "[ApvTimingAnalysis::analysis]: Warning: large noise levels or no ticks.";}
continue;

}
  
  ////Method 1: Take start of tick as the max derivative
  /*
  coarse_fine.first = (ideriv - 1)/24;
  coarse_fine.second = (ideriv - 1)%24;
  */

  unsigned short counter = 0;
  vector<unsigned short> ticks; //records bin number of first position of tick > 2*sigma

  ////Method 2: Take start of tick as start of 35 bins above mean + 2*SD of noise.
  /*
  // find tick positions..

  for (unsigned short k = 1; k < ((unsigned short)histo->GetNbinsX() + 1); k++) { // k is bin number

    if ((short)histo->GetBinContent(k) > (meanNoise + 2*sigmaNoise)) counter++;
    else {counter = 0;}
 
    if (counter > 35) { ticks.push_back(k-35); counter = 0; }
  }
  */
////Method 3: Take start of tick as position of max derivative within 35 bins above mean + 1*SD of noise.
  
  maxderiv = -9999.;
  ideriv = 0;

// find tick positions..

  for (unsigned short k = 2; k < (unsigned short)histo->GetNbinsX(); k++) { // k is bin number

    if ((short)histo->GetBinContent(k) > (meanNoise + 2.*sigmaNoise)) {
      counter++;

      //find the maximum derivative within the window...
      float deriv = (unsigned int)histo->GetBinContent(k+1) - (unsigned int)histo->GetBinContent(k-1);
      if (deriv > maxderiv) {maxderiv = deriv; ideriv = k;}
    }
    else {counter = 0; maxderiv = -9999.; ideriv = 0;}
 
    if (counter > 35) { ticks.push_back(ideriv); counter = 0; maxderiv = -9999.; ideriv = 0; }
  }

  // notify user if more than one tick is present in sample

  if (ticks.size() > 1) { 

  stringstream os;

  for (unsigned short num = 0; num < (ticks.size() - 1); num++) {os << ticks[num + 1] - ticks[num];
  if (num != (ticks.size() - 2)) os << ", ";
}

  if (ticks.size() > 2) {os << " PLL fine delay settings, respectively.";}
  else { os << " PLL fine delay settings.";}

  LogDebug("Commissioning|Analysis") << "[ApvTimingAnalysis::analysis]: Multiple ticks found in sample. Number of ticks: " << ticks.size() << " at a separation: " << os.str();
}

  else if (ticks.size() == 1) {
  coarse_fine.first = (ticks[0] - 1)/24;
  coarse_fine.second = (ticks[0] - 1)%24;
  }

  // or no ticks...
  else { 
    LogDebug("Commissioning|Analysis") << "[ApvTimingAnalysis::analysis]: No ticks found in sample.";
    coarse_fine.first = 0;
    coarse_fine.second = 0;
  }

  // set monitorables
  monitorables.resize(2,0);
  monitorables[0] = coarse_fine.first;
  monitorables[1] = coarse_fine.second;
}



#ifndef DQM_SiStripCommissioningAnalysis_DaqScopeModeAnalysis_H
#define DQM_SiStripCommissioningAnalysis_DaqScopeModeAnalysis_H

#include "DQM/SiStripCommissioningAnalysis/interface/CommissioningAnalysis.h"
#include "DataFormats/SiStripCommon/interface/SiStripConstants.h"
#include <boost/cstdint.hpp>
#include <sstream>
#include <vector>

class TH1;

/**
   @class DaqScopeModeAnalysis
   @author R.Bainbridge
   @brief Analysis for scope mode data.
*/

class DaqScopeModeAnalysis : public CommissioningAnalysis {
  
 public:
  
  DaqScopeModeAnalysis( const uint32_t& key );
  DaqScopeModeAnalysis();
  virtual ~DaqScopeModeAnalysis() {;}
  
  inline const float& entries() const;
  inline const float& mean() const; 
  inline const float& median() const; 
  inline const float& mode() const; 
  inline const float& rms() const; 
  inline const float& min() const; 
  inline const float& max() const; 
  
  inline const Histo& histo() const;
  
  void print( std::stringstream&, uint32_t not_used = 0 );
  
 private:
  
  void reset();
  void extract( const std::vector<TH1*>& );
  void analyse();
  
 private:
  
  float entries_;
  float mean_;
  float median_;
  float mode_;
  float rms_;
  float min_;
  float max_;
  
  /** Histogram of scope mode data. */
  Histo histo_;
  
};

const float& DaqScopeModeAnalysis::entries() const { return entries_; }
const float& DaqScopeModeAnalysis::mean() const { return mean_; }
const float& DaqScopeModeAnalysis::median() const { return median_; }
const float& DaqScopeModeAnalysis::mode() const { return mode_; }
const float& DaqScopeModeAnalysis::rms() const { return rms_; }
const float& DaqScopeModeAnalysis::min() const { return min_; }
const float& DaqScopeModeAnalysis::max() const { return max_; }
const DaqScopeModeAnalysis::Histo& DaqScopeModeAnalysis::histo() const { return histo_; }

#endif // DQM_SiStripCommissioningAnalysis_DaqScopeModeAnalysis_H



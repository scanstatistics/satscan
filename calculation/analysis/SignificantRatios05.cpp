//***************************************************************************
#include "SaTScan.h"
#pragma hdrstop
//***************************************************************************
#include "SignificantRatios05.h"
#include "SSException.h"
#include <cstring>

/** constructor */
SignificantRatios::SignificantRatios(unsigned int iNumReplications) : _numReplications(iNumReplications) {
    _ratios.resize(static_cast<unsigned int>(ceil((_numReplications+1)*0.05)), 0);
}

/** Adds loglikelihood ratio to array in decending order, if ratio is greater than existing ratio in list. */
bool SignificantRatios::add(double dRatio) {
  container_t::iterator itr = std::upper_bound(_ratios.begin(), _ratios.end(), dRatio, std::greater<double>());
  if (itr != _ratios.end()) {
      _ratios.insert(itr, dRatio);
      _ratios.pop_back();
      return true;
  } return false;
}

/** Returns loglikelihood ratio at the top 0.01, relative to the number of replications. */
SignificantRatios::alpha_t SignificantRatios::getAlpha01()  const {
  double index = floor(static_cast<double>(_numReplications + 1) * 0.01) - 1.0;
  return index >= 0.0 ? SignificantRatios::alpha_t(true,_ratios.at(static_cast<size_t>(index))) : std::make_pair(false,0.0);
}

/** Returns loglikelihood ratio at the top 0.05, relative to the number of replications. */
SignificantRatios::alpha_t SignificantRatios::getAlpha05()  const {
  double index = floor(static_cast<double>(_numReplications + 1) * 0.05) - 1.0;
  return index >= 0.0 ? SignificantRatios::alpha_t(true,_ratios.at(static_cast<size_t>(index))) : std::make_pair(false,0.0);
}

/** Returns loglikelihood ratio at the top 0.001, relative to the number of replications. */
SignificantRatios::alpha_t SignificantRatios::getAlpha001()  const {
  double index = floor(static_cast<double>(_numReplications + 1) * 0.001) - 1.0;
  return index >= 0.0 ? SignificantRatios::alpha_t(true,_ratios.at(static_cast<size_t>(index))) : std::make_pair(false,0.0);
}

/** Returns loglikelihood ratio at the top 0.0001, relative to the number of replications. */
SignificantRatios::alpha_t SignificantRatios::getAlpha0001()  const {
  double index = floor(static_cast<double>(_numReplications + 1) * 0.0001) - 1.0;
  return index >= 0.0 ? SignificantRatios::alpha_t(true,_ratios.at(static_cast<size_t>(index))) : std::make_pair(false,0.0);
}

/** Returns loglikelihood ratio at the top 0.00001, relative to the number of replications. */
SignificantRatios::alpha_t SignificantRatios::getAlpha00001()  const {
  double index = floor(static_cast<double>(_numReplications + 1) * 0.00001) - 1.0;
  return index >= 0.0 ? SignificantRatios::alpha_t(true,_ratios.at(static_cast<size_t>(index))) : std::make_pair(false,0.0);
}

/** Initialize each ratio of array to zero. */
void SignificantRatios::initialize() {
  if (_ratios.size()) memset(&_ratios[0], 0, _ratios.size() * sizeof(double));
}





//***************************************************************************
#include "SaTScan.h"
#pragma hdrstop
//***************************************************************************
#include "SignificantRatios05.h"
#include "SSException.h"

/** constructor */
CSignificantRatios05::CSignificantRatios05(unsigned int replications) : _replications(replications) {
  _ratios_list.resize(static_cast<unsigned int>(ceil((_replications+1)*0.05)), 0);
}

/** Initialize each ratio of array to zero. */
void CSignificantRatios05::Initialize() {
  if (_ratios_list.size())
    memset(&_ratios_list[0], 0, _ratios_list.size() * sizeof(double));
}

/** Adds loglikelihood ratio to array in decending order, if ratio is greater
    than existing ratio in list. */
bool CSignificantRatios05::AddRatio(double dRatio) {
  bool          bAdded=false;
  unsigned int  i;

  if (_ratios_list.size() && std::fabs(dRatio - _ratios_list.back()) > DBL_CMP_TOLERANCE && dRatio > _ratios_list.back()) {
     bAdded = true;
     //insert calculated ratio in descending order
     for (i=_ratios_list.size() - 1; i > 0 && dRatio > _ratios_list[i-1]; --i)
        _ratios_list[i] =  _ratios_list[i-1];
     _ratios_list[i] = dRatio;
  }

  return bAdded;
}

/** Returns loglikelihood ratio at the top 0.01, relative to the number of replications. */
double CSignificantRatios05::GetAlpha01()  const {
  return _ratios_list.size() ? _ratios_list[(_replications >= 99 ? static_cast<size_t>(_replications  * 0.01) : 0)] : 0;
}

/** Returns loglikelihood ratio at the top 0.001, relative to the number of replications. */
double CSignificantRatios05::GetAlpha001()  const {
  return _ratios_list.size() ? _ratios_list[(_replications >= 999 ? static_cast<size_t>(_replications  * 0.001) : 0)] : 0;
}

/** Returns loglikelihood ratio at the top 0.0001, relative to the number of replications. */
double CSignificantRatios05::GetAlpha0001()  const {
  return _ratios_list.size() ? _ratios_list[(_replications >= 9999 ? static_cast<size_t>(_replications  * 0.0001) : 0)] : 0;
}

/** Returns loglikelihood ratio at the top 0.00001, relative to the number of replications. */
double CSignificantRatios05::GetAlpha00001()  const {
  return _ratios_list.size() ? _ratios_list[(_replications >= 99999 ? static_cast<size_t>(_replications  * 0.00001) : 0)] : 0;
}

/** Returns loglikelihood ratio at the top 0.05, relative to the number of replications. */
double CSignificantRatios05::GetAlpha05() const {
  return _ratios_list.size() ? _ratios_list.back() : 0;
}





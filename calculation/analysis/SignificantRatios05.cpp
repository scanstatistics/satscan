//***************************************************************************
#include "SaTScan.h"
#pragma hdrstop
//***************************************************************************
#include "SignificantRatios05.h"

/** constructor */
CSignificantRatios05::CSignificantRatios05(unsigned int iNumReplications) {
  try {
    gvRatiosList.resize(static_cast<int>(ceil((iNumReplications+1)*0.05)) + 1, 0);
  }
  catch (ZdException &x) {
    x.AddCallpath("constructor()","CSignificantRatios05");
    throw;
  }
}

/** destrucutor */
CSignificantRatios05::~CSignificantRatios05() {}

/** Initialize each ratio of array to zero. */
void CSignificantRatios05::Initialize() {
  if (gvRatiosList.size())
    memset(&gvRatiosList[0], 0, gvRatiosList.size() * sizeof(double));
}

/** Adds loglikelihood ratio to array in decending order, if ratio is greater
    than existing ratio in list. */
bool CSignificantRatios05::AddRatio(double dRatio) {
  bool          bAdded=false;
  unsigned int  i;

  if (gvRatiosList.size() && dRatio > gvRatiosList.back()) {
     bAdded = true;
     //insert calculated ratio in descending order
     for (i=gvRatiosList.size() - 1; i > 0 && dRatio > gvRatiosList[i-1]; --i)
        gvRatiosList[i] =  gvRatiosList[i-1];
     gvRatiosList[i] = dRatio;
  }

  return bAdded;
}

/** Returns loglikelihood ratio at the top 1%. If number of replications is
    less than 99, zero is returned. */
double CSignificantRatios05::GetAlpha01()  const {
  return (gvRatiosList.size() >= 99 ? gvRatiosList[gvRatiosList.size()/5 - 1] : 0);
}

/** Returns loglikelihood ratio at the top 5%. If number of replications is
    less than 19, zero is returned. */
double CSignificantRatios05::GetAlpha05() const {
  return (gvRatiosList.size() >= 19 ? gvRatiosList[gvRatiosList.size() - 1] : 0);
}





//---------------------------------------------------------------------------
#ifndef DenominatorDataRandomizerH
#define DenominatorDataRandomizerH
//---------------------------------------------------------------------------
#include "Randomizer.h"

/** abstract denominator data randomizer class */
class AbstractDenominatorDataRandomizer : public AbstractRandomizer {
  protected:
    BinomialGenerator   gBinomialGenerator;

  public:
            AbstractDenominatorDataRandomizer();
    virtual ~AbstractDenominatorDataRandomizer();
};
//---------------------------------------------------------------------------
#endif

//---------------------------------------------------------------------------
#ifndef DenominaterDataRandomizerH
#define DenominaterDataRandomizerH
//---------------------------------------------------------------------------
#include "Randomizer.h"

/** abstract denominator data randomizer class */
class AbstractDenominaterDataRandomizer : public AbstractRandomizer {
  protected:
    BinomialGenerator   gBinomialGenerator;

  public:
            AbstractDenominaterDataRandomizer();
    virtual ~AbstractDenominaterDataRandomizer();
};
//---------------------------------------------------------------------------
#endif

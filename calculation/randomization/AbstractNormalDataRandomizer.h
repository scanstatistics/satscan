//******************************************************************************
#ifndef AbstractNormalDataRandomizerH
#define AbstractNormalDataRandomizerH
//******************************************************************************
/** */
class AbstractNormalDataRandomizer {
   public:
     AbstractNormalDataRandomizer() {}
     virtual ~AbstractNormalDataRandomizer() {}

    virtual void               AssignFromAttributes(RealDataSet& RealSet) = 0;
    virtual void               RemoveCase(int iTimeInterval, tract_t tTractIndex) = 0;
};
//******************************************************************************
#endif
 
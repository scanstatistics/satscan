//---------------------------------------------------------------------------
#ifndef MeasureDeterminantH
#define MeasureDeterminantH
#include "SaTScan.h"
typedef measure_t(*MEAURE_DETERMINANT)(measure_t, measure_t);
measure_t Standard(measure_t tMeasure, measure_t tMeasureSquared);
measure_t EnhancedMeasure(measure_t tMeasure, measure_t tMeasureSquared);

typedef measure_t(*_MEAURE_DETERMINANT_)(unsigned int, const measure_t*, const measure_t*);
measure_t _Standard_(unsigned int iStart, const measure_t* tMeasure, const measure_t* tMeasureSquared);
measure_t _EnhancedMeasure_(unsigned int iStart, const measure_t* tMeasure, const measure_t* tMeasureSquared);

typedef measure_t(*MEAURE_DETERMINANT_)(unsigned int, unsigned int, const measure_t*, const measure_t*);
measure_t Standard_(unsigned int iStart, unsigned int iEnd, const measure_t* tMeasure, const measure_t* tMeasureSquared);
measure_t EnhancedMeasure_(unsigned int iStart, unsigned int iEnd, const measure_t* tMeasure, const measure_t* tMeasureSquared);

class AbstractMeasureDeterminant {
  public:
    AbstractMeasureDeterminant() {}
    virtual ~AbstractMeasureDeterminant() {}

    virtual measure_t     GetMeasure(measure_t tMeasure, measure_t tSqMeasure) = 0;
    virtual measure_t     GetMeasure(unsigned int iStart, const measure_t* tMeasure, const measure_t* tMeasureSquared) = 0;
    virtual measure_t     GetMeasure(unsigned int iStart, unsigned int iEnd, const measure_t* tMeasure, const measure_t* tMeasureSquared) = 0;
};

class StandardMeasureDeterminant {
  public:
    StandardMeasureDeterminant() {}
    virtual ~StandardMeasureDeterminant() {}

    inline virtual measure_t     GetMeasure(measure_t tMeasure, measure_t tSqMeasure) {return tMeasure;}
    inline virtual measure_t     GetMeasure(unsigned int iStart, const measure_t* tMeasure, const measure_t* tMeasureSquared) {return tMeasure[iStart];}
    inline virtual measure_t     GetMeasure(unsigned int iStart, unsigned int iEnd, const measure_t* tMeasure, const measure_t* tMeasureSquared) {return tMeasure[iStart] - tMeasure[iEnd];}
};

//---------------------------------------------------------------------------
#endif

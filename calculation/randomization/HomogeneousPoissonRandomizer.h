//******************************************************************************
#ifndef __HomogeneousPoissonRandomizer_H
#define __HomogeneousPoissonRandomizer_H
//******************************************************************************
#include "DenominatorDataRandomizer.h"
#include "ObservableRegion.h"
#include <iostream>
#include <fstream>

typedef std::vector<ConvexPolygonObservableRegion> ObserverableRegionContainer_t;
class ObservationGroupingManager;
class GInfo;

/** Abstraction for Homogeneous Poisson data randomizers */
class HomogeneousPoissonRandomizer : public AbstractDenominatorDataRandomizer {
  protected:
    const CParameters                   & gParameters;
    const ObserverableRegionContainer_t & gPolygons;
    std::auto_ptr<GInfo>                  gCentroidsHandler;
    std::auto_ptr<ObservationGroupingManager> _groupings;

  public:
    HomogeneousPoissonRandomizer(const CParameters& Parameters, const ObserverableRegionContainer_t& Regions, long lInitialSeed=RandomNumberGenerator::glDefaultSeed);
    virtual ~HomogeneousPoissonRandomizer() {}

	ObservationGroupingManager          & getGroupInfo();
    GInfo                               & getCentroidHandler();
};

/** Homogeneous Poisson randomizer for null hypothesis. */
class HomogeneousPoissonNullHypothesisRandomizer : public HomogeneousPoissonRandomizer {
  public:
    HomogeneousPoissonNullHypothesisRandomizer(const CParameters & Parameters, const ObserverableRegionContainer_t& Regions, long lInitialSeed=RandomNumberGenerator::glDefaultSeed)
       : HomogeneousPoissonRandomizer(Parameters, Regions, lInitialSeed) {}
    virtual ~HomogeneousPoissonNullHypothesisRandomizer() {}

    virtual HomogeneousPoissonNullHypothesisRandomizer * Clone() const;

    virtual void  RandomizeData(const RealDataSet& RealSet, DataSet& SimSet, unsigned int iSimulation);
};
//******************************************************************************
#endif

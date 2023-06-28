//******************************************************************************
#include "SaTScan.h"
#pragma hdrstop
//******************************************************************************
#include "HomogeneousPoissonRandomizer.h"
#include "SaTScanData.h"
#include "SSException.h" 

HomogeneousPoissonRandomizer::HomogeneousPoissonRandomizer(const CParameters& Parameters, const ObserverableRegionContainer_t& Regions, long lInitialSeed)
                             :AbstractDenominatorDataRandomizer(lInitialSeed), gParameters(Parameters), gPolygons(Regions) {
  try {
	  _groupings.reset(new ObservationGroupingManager(gParameters.GetIsPurelyTemporalAnalysis(), gParameters.GetMultipleCoordinatesType()));
	  _groupings->setExpectedCoordinateDimensions(2);
     if (!gParameters.UseSpecialGrid())
       gCentroidsHandler.reset(new LocationsCentroidHandlerPassThrough(_groupings->getLocationsManager()));
  }
  catch (prg_exception& x) {
    x.addTrace("constructor()","HomogeneousPoissonRandomizer");
    throw;
  }
}

ObservationGroupingManager& HomogeneousPoissonRandomizer::getGroupInfo() {
   return *_groupings;
}

GInfo& HomogeneousPoissonRandomizer::getCentroidHandler() {
  if (!gCentroidsHandler.get()) throw prg_error("Centroid handler not allocated.","getCentroidHandler()");
  return *gCentroidsHandler;
}

HomogeneousPoissonNullHypothesisRandomizer * HomogeneousPoissonNullHypothesisRandomizer::Clone() const {
    return new HomogeneousPoissonNullHypothesisRandomizer(gParameters, gPolygons);
}

/** Creates randomized under the null hypothesis for Poisson model, assigning data to DataSet objects structures.
    Random number generator seed initialized based upon 'iSimulation' index. */
void HomogeneousPoissonNullHypothesisRandomizer::RandomizeData(const RealDataSet& RealSet, DataSet& SimSet, unsigned int iSimulation) {
  double areaOfRegion, rX, rY;
  count_t distributedCount=0, distributionTotal=RealSet.getTotalCases();
  measure_t randomizedArea=0, areaTotal=RealSet.getTotalPopulation();
  ObserverableRegionContainer_t::const_iterator itr=gPolygons.begin();
  ObservationGroupingManager::CoordinatesContainer_t vCoordiantes;

  SetSeed(iSimulation, SimSet.getSetIndex());
  vCoordiantes.reserve(distributionTotal);
  for (; itr != gPolygons.end() && (distributionTotal - distributedCount > 0); ++itr) {
     count_t distribution = gBinomialGenerator.GetBinomialDistributedVariable(distributionTotal - distributedCount,
                                                                              itr->getArea()/(areaTotal - randomizedArea), gRandomNumberGenerator);
     for (count_t t=0; t < distribution;) {
         itr->retrieveRandomPointInRegion(rX, rY, gRandomNumberGenerator);         
         //insert unique coordinates into collection - ordered by first coordinate, then second coordinate, etc.
         std::auto_ptr<Coordinates> pCoordinates(new Coordinates(rX, rY));
         ptr_vector<Coordinates>::iterator itrCoordinates;
         itrCoordinates = std::lower_bound(vCoordiantes.begin(), vCoordiantes.end(), pCoordinates.get(), CompareCoordinates());
         if (itrCoordinates != vCoordiantes.end() && *(pCoordinates.get()) == *(*itrCoordinates))
           continue; // Duplicate point generated.
         vCoordiantes.insert(itrCoordinates, pCoordinates.release());
          ++t;
     }
     distributedCount += distribution;
     randomizedArea += itr->getArea();
  }
  
  _groupings->assignExplicitCoordinates(vCoordiantes);

}

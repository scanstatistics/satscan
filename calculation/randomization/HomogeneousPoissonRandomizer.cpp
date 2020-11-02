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
     gTractHandler.reset(new TractHandler(gParameters.GetIsPurelyTemporalAnalysis(), gParameters.GetMultipleCoordinatesType()));
     gTractHandler->setCoordinateDimensions(2);
     if (!gParameters.UseSpecialGrid())
       gCentroidsHandler.reset(new CentroidHandlerPassThrough(*gTractHandler, gParameters.getUseLocationsNetworkFile() && gParameters.getNetworkFilePurpose() == NETWORK_DEFINITION));
  }
  catch (prg_exception& x) {
    x.addTrace("constructor()","HomogeneousPoissonRandomizer");
    throw;
  }
}

TractHandler& HomogeneousPoissonRandomizer::getTractHandler() {
   return *gTractHandler;
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
  TractHandler::CoordinatesContainer_t vCoordiantes;

  SetSeed(iSimulation, SimSet.getSetIndex());
  vCoordiantes.reserve(distributionTotal);
  for (; itr != gPolygons.end() && (distributionTotal - distributedCount > 0); ++itr) {
     count_t distribution = gBinomialGenerator.GetBinomialDistributedVariable(distributionTotal - distributedCount,
                                                                              itr->getArea()/(areaTotal - randomizedArea), gRandomNumberGenerator);
     for (count_t t=0; t < distribution;) {
         itr->retrieveRandomPointInRegion(rX, rY, gRandomNumberGenerator);         
         //insert unique coordinates into collection - ordered by first coordinate, then second coordinate, etc.
         std::auto_ptr<TractHandler::Coordinates> pCoordinates(new TractHandler::Coordinates(rX, rY));
         ptr_vector<TractHandler::Coordinates>::iterator itrCoordinates;
         itrCoordinates = std::lower_bound(vCoordiantes.begin(), vCoordiantes.end(), pCoordinates.get(), TractHandler::CompareCoordinates());
         if (itrCoordinates != vCoordiantes.end() && *(pCoordinates.get()) == *(*itrCoordinates))
           continue; // Duplicate point generated.
         vCoordiantes.insert(itrCoordinates, pCoordinates.release());
          ++t;
     }
     distributedCount += distribution;
     randomizedArea += itr->getArea();
  }
  
  gTractHandler->assignExplicitCoordinates(vCoordiantes);

}

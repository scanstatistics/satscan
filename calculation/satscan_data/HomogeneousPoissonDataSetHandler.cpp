//******************************************************************************
#include "SaTScan.h"
#pragma hdrstop
//******************************************************************************
#include "HomogeneousPoissonDataSetHandler.h"
#include "SaTScanData.h"
#include "DateStringParser.h"
#include "DataSource.h"
#include "SSException.h"

HomogeneousPoissonDataSetHandler::HomogeneousPoissonDataSetHandler(CSaTScanData& DataHub, TractHandler& TractHandler, GInfo& CentroidsHandler, BasePrint& Print) 
    :DataSetHandler(DataHub, Print), gTractHandler(TractHandler), gCentroidsHandler(CentroidsHandler) {
  try {
    // for each region defined, define regions from specifications
    for (size_t t=0; t < gParameters.getObservableRegions().size(); ++t) {
       // get each collection of inequalities from parameter settings
       InequalityContainer_t inequalities = ConvexPolygonBuilder::parse(gParameters.getObservableRegions()[t]);
       // build region from inequalities and add to collection of regions ...
       gPolygons.push_back(ConvexPolygonBuilder::buildConvexPolygon(inequalities));
    }
  }
  catch (prg_exception& x) {
    x.addTrace("constructor()","HomogeneousPoissonDataSetHandler");
    throw;
  }  
}

/** For each element in SimulationDataContainer_t, allocates appropriate data structures
    as needed by data set handler (probability model). */
SimulationDataContainer_t& HomogeneousPoissonDataSetHandler::AllocateSimulationData(SimulationDataContainer_t& Container) const {
  return Container;
}

/** Returns newly allocated data gateway object that references structures
    utilized in calculating most likely clusters (real data) for the Poisson
    probablity model, analysis type and possibly inclusion purely temporal
    clusters. Caller is responsible for destructing returned object. */
AbstractDataSetGateway & HomogeneousPoissonDataSetHandler::GetDataGateway(AbstractDataSetGateway& DataGatway) const {
  DataSetInterface Interface(gDataHub.GetNumTimeIntervals(), gDataHub.GetNumTracts() + gDataHub.GetTInfo()->getMetaManagerProxy().getNumMetaLocations());

  try {
    DataGatway.Clear();
    for (size_t t=0; t < gvDataSets.size(); ++t) {
      //get reference to dataset
      const RealDataSet& DataSet = *gvDataSets.at(t);
      //set total cases and measure
      Interface.SetTotalCasesCount(DataSet.getTotalCases());
      Interface.SetTotalMeasureCount(DataSet.getTotalMeasure());
      Interface.SetTotalMeasureAuxCount(DataSet.getTotalMeasureAux());
      DataGatway.AddDataSetInterface(Interface);
    }
  }
  catch (prg_exception& x) {
    x.addTrace("GetDataGateway()","HomogeneousPoissonDataSetHandler");
    throw;
  }  

  return DataGatway;
}

/** Returns indication of whether point is within any of defined regions. */
bool HomogeneousPoissonDataSetHandler::isPointInRegions(double x, double y) const {
  for (size_t t=0; t < gPolygons.size(); ++t)
      if (gPolygons[t].intersectsPoint(x, y, false)) return true;
  return false;
}

/** Returns a collection of cloned randomizers maintained by data set handler.
    All previous elements of list are deleted. */
RandomizerContainer_t& HomogeneousPoissonDataSetHandler::GetRandomizerContainer(RandomizerContainer_t& Container) const {
  try {
      Container.push_back(new HomogeneousPoissonNullHypothesisRandomizer(gParameters, gPolygons, gParameters.GetRandomizationSeed()));
    //Container = gvDataSetRandomizers;
  }
  catch (prg_exception& x) {
    x.addTrace("GetRandomizerContainer()","HomogeneousPoissonDataSetHandler");
    throw;
  }
  return Container;
}

/** Returns newly allocated data gateway object that references structures
    utilized in performing simulations (Monte Carlo) for the Poisson
    probablity model, analysis type and possibly inclusion purely temporal
    clusters. Caller is responsible for destructing returned object. */
AbstractDataSetGateway & HomogeneousPoissonDataSetHandler::GetSimulationDataGateway(AbstractDataSetGateway& DataGatway, const SimulationDataContainer_t& Container, const RandomizerContainer_t& rContainer) const {
  DataSetInterface Interface(gDataHub.GetNumTimeIntervals(), gDataHub.GetNumTracts() + gDataHub.GetTInfo()->getMetaManagerProxy().getNumMetaLocations());

  try {
    DataGatway.Clear();
    for (size_t t=0; t < gvDataSets.size(); ++t) {
      //get reference to datasets
      const RealDataSet& R_DataSet = *gvDataSets.at(t);
      const DataSet& S_DataSet = *Container.at(t);
      //set total cases and measure
      Interface.SetTotalCasesCount(R_DataSet.getTotalCases());
      Interface.SetTotalMeasureCount(R_DataSet.getTotalMeasure());
      Interface.SetTotalMeasureAuxCount(R_DataSet.getTotalMeasureAux());
      DataGatway.AddDataSetInterface(Interface);
    }
  }
  catch (prg_exception& x) {
    x.addTrace("GetSimulationDataGateway()","HomogeneousPoissonDataSetHandler");
    throw;
  }
  return DataGatway;
}

/** Randomizes data */
void HomogeneousPoissonDataSetHandler::RandomizeData(RandomizerContainer_t& Container, SimulationDataContainer_t& SimDataContainer, unsigned int iSimulationNumber) const {
  DataSetHandler::RandomizeData(Container, SimDataContainer, iSimulationNumber);
}

/** Reads the coordinate data source, storing data in RealDataSet object. As a
    means to help user clean-up their data, continues to read records as errors
    are encountered. Returns boolean indication of read success. */
bool HomogeneousPoissonDataSetHandler::ReadCoordinates(RealDataSet& DataSet, DataSource& Source) {
  short                 iScanCount=2;
  const long            uLocationIndex=0;
  bool                  bValid=true, bEmpty=true,bHasIdentifier=false;
  std::vector<double>   vCoordinates(2);
  count_t               tTotalCases=0;

  gTractHandler.setCoordinateDimensions(iScanCount);

  try {
    while (!gPrint.GetMaximumReadErrorsPrinted() && Source.ReadRecord()) {
         iScanCount = Source.GetNumValues();
         //if empty and this record has data, then this is the first record w/ data
         if (bEmpty) {
           bEmpty = false;
           //determine number of dimensions from first record
           //there must be at least two dimensions
           if (iScanCount < 2) {
             gPrint.Printf("Error: The first record of the coordinate file contains %s.\n",
                           BasePrint::P_READERROR, iScanCount == 2 ? "only x-coordinate" : "no coordinates");
             bValid = false;
             break; //stop reading records, the first record defines remaining records format
           }
           bHasIdentifier = iScanCount == 3;
         }

         //there must be at least two dimensions plus optional identifier
         if (iScanCount < (bHasIdentifier ? 3 : 2)) {
           gPrint.Printf("Error: Record %ld of the coordinate file contains %s.\n",
                          BasePrint::P_READERROR, Source.GetCurrentRecordIndex(), 
                          iScanCount == (bHasIdentifier ? 2 : 1) ? "only x-coordinate" : "no coordinates");
           bValid = false;
           continue;
         }
         //read and validate dimensions -- skip to next record if error reading coordinates
         if (! ReadCartesianCoordinates(Source, vCoordinates, iScanCount, bHasIdentifier ? 1 : 0)) {
           bValid = false;
           continue;
         }
         //validate that we read the correct number of coordinates
         if (iScanCount < gTractHandler.getCoordinateDimensions()) {
           //Note: since the first record defined the number of dimensions, this error could not happen.
           gPrint.Printf("Error: Record %ld in the coordinate file contains %d dimension%s but expecting %d.\n", BasePrint::P_READERROR,
                         Source.GetCurrentRecordIndex(), iScanCount, (iScanCount == 1 ? "" : "s"), gTractHandler.getCoordinateDimensions());
           bValid = false;
           continue;
         }

         // check that coordinates are within one of the regions
         if (!isPointInRegions(vCoordinates[0], vCoordinates[1])) {
             if (gParameters.GetCoordinatesDataCheckingType() == STRICTCOORDINATES) {
                gPrint.Printf("Error: Record %ld in the coordinate file contains a coordinate that is not within "
                             "any of the defined polygons.\n", BasePrint::P_READERROR, Source.GetCurrentRecordIndex());
                bValid = false;
             }
             continue;
         }
         
         //check that coordinates of case are not already defined
         if (gTractHandler.getCoordinatesExist(vCoordinates)) {
             gPrint.Printf("Error: Record %ld in the coordinate file contains a duplicate coordinate.\n"
                           "       In the continuous Poisson model, two cases cannot have exactly the same location.\n", BasePrint::P_READERROR, Source.GetCurrentRecordIndex());
             bValid = false;
         }

         //add the tract identifier and coordinates to tract handler
         if (bHasIdentifier) 
           gTractHandler.addLocation(Source.GetValueAt(uLocationIndex), vCoordinates);
         else {
             std::stringstream identifier;
             identifier << "point" << Source.GetCurrentRecordIndex();
             gTractHandler.addLocation(identifier.str().c_str(), vCoordinates);
         }
    }
    tTotalCases = gTractHandler.getLocations().size();
    //if invalid at this point then read encountered problems with data format,
    //inform user of section to refer to in user guide for assistance
    if (! bValid)
      gPrint.Printf("Please see the '%s' section in the user guide for help.\n", BasePrint::P_ERROR, gPrint.GetImpliedFileTypeString().c_str());
    //print indication if file contained no data
    else if (bEmpty || tTotalCases == 0) {
      gPrint.Printf("Error: The %s does not contain data.\n", BasePrint::P_ERROR, gPrint.GetImpliedFileTypeString().c_str());
      bValid = false;
    } else {
        DataSet.setTotalCases(tTotalCases);
        DataSet.setTotalMeasure(tTotalCases);
        measure_t tTotalMeasure=0;
        // This is actually the total adusted measure. We'll need to apply this ratio during scanning.
        DataSet.setTotalMeasureAux(tTotalCases/getTotalArea()); 
        DataSet.setTotalPopulation(getTotalArea());
    }
  }
  catch (prg_exception& x) {
    x.addTrace("ReadCounts()","HomogeneousPoissonDataSetHandler");
    throw;
  }
  return bValid;
}

/** Returns total scanning area. */
double HomogeneousPoissonDataSetHandler::getTotalArea() const {
    double d=0;
    for (size_t t=0; t < gPolygons.size(); ++t) d += gPolygons[t].getArea();
    return d;
}

/** Read the special grid data file as Cartesian coordinates.
   If invalid data is found in the file, an error message is printed,
   that record is ignored, and reading continues.
   Return value: true = success, false = errors encountered          */
bool HomogeneousPoissonDataSetHandler::ReadGridFile(DataSource& Source) {
  bool                          bValid=true, bEmpty=true;
  short                         iScanCount;
  std::vector<double>           vCoordinates;
  CentroidHandler             * pGridPoints;
  GInfo::FocusInterval_t        focusInterval;

  try {
    focusInterval.first = false;
    if ((pGridPoints = dynamic_cast<CentroidHandler*>(&gCentroidsHandler)) == 0)
      throw prg_error("Not a CentroidHandler type.", "ReadGridFileAsCartiesian()");
    pGridPoints->setDimensions(gTractHandler.getCoordinateDimensions());
    vCoordinates.resize(gTractHandler.getCoordinateDimensions(), 0);
    while (!gPrint.GetMaximumReadErrorsPrinted() && Source.ReadRecord()) {
        //there are records with data, but not necessarily valid
        bEmpty = false;
         //read and vaidate dimensions skip to next record if error reading coordinates as double
         if (! ReadCartesianCoordinates(Source, vCoordinates, iScanCount, 0)) {
           bValid = false;
           continue;
         }
        //validate that we read the correct number of coordinates as defined by coordinates system or coordinates file
        if (iScanCount < gTractHandler.getCoordinateDimensions()) {
          gPrint.Printf("Error: Record %ld in the grid file contains %d dimension%s but the\n"
                        "       coordinates file defined the number of dimensions as %d.\n",
                        BasePrint::P_READERROR, Source.GetCurrentRecordIndex(), iScanCount,
                        (iScanCount == 1 ? "" : "s"), gTractHandler.getCoordinateDimensions());
          bValid = false;
          continue;
        }
        // check that coordinates are within one of the regions
        if (!isPointInRegions(vCoordinates[0], vCoordinates[1])) {
            if (gParameters.GetCoordinatesDataCheckingType() == STRICTCOORDINATES) {
                gPrint.Printf("Error: Record %ld in the grid file contains a coordinate that is not within "
                                "any of the defined regions.\n", BasePrint::P_READERROR, Source.GetCurrentRecordIndex());
                bValid = false;
            }
            continue;
        }

        pGridPoints->addGridPoint(vCoordinates, focusInterval);
    }
    //if invalid at this point then read encountered problems with data format,
    //inform user of section to refer to in user guide for assistance
    if (! bValid)
      gPrint.Printf("Please see the 'grid file' section in the user guide for help.\n", BasePrint::P_ERROR);
    //print indication if file contained no data
    else if (bEmpty) {
      gPrint.Printf("Error: The Grid file does not contain any data.\n", BasePrint::P_ERROR);
      bValid = false;
    } else {
        pGridPoints->additionsCompleted();
        //record number of centroids read
        gDataHub.m_nGridTracts = pGridPoints->getNumGridPoints();
    }

    if (gDataHub.m_nGridTracts == 0) {
      gPrint.Printf("Error: The Grid file does not contain any data.\n", BasePrint::P_ERROR);
      bValid = false;
    }
  }
  catch (prg_exception& x) {
    x.addTrace("ReadGridFile()","HomogeneousPoissonDataSetHandler");
    throw;
  }
  return bValid;
}

/** Reads coordinates from data source record into passed vector. */
bool HomogeneousPoissonDataSetHandler::ReadCartesianCoordinates(DataSource& Source, std::vector<double>& vCoordinates, short& iScanCount, short iWordOffSet) {
  const char  * pCoordinate;
  int           i;

  for (i=0, iScanCount=0; i < (int)vCoordinates.size(); ++i, ++iWordOffSet)
     if ((pCoordinate = Source.GetValueAt(iWordOffSet)) != 0) {
        if (!string_to_type<double>(pCoordinate, vCoordinates[i])) {
            //unable to read word as double, print error to print direction and return false
            gPrint.Printf("Error: Value '%s' of record %ld in %s could not be read as ",
                          BasePrint::P_READERROR, pCoordinate, Source.GetCurrentRecordIndex(), gPrint.GetImpliedFileTypeString().c_str());
            //we can be specific about which dimension we are attending to read to                                    
            gPrint.Printf("%s-coordinate.\n", BasePrint::P_READERROR, (i == 0 ? "x" : "y"));
            return false;
        } else {
            iScanCount++; //track num successful scans, caller of function wants this information
        }
     }
  return true;          
}

/** Attempts to read coordinate file data into RealDataSet object. Returns boolean indication of read success. */
bool HomogeneousPoissonDataSetHandler::ReadCoordinatesFile(RealDataSet& DataSet) {
  try {
      gPrint.SetImpliedInputFileType(BasePrint::COORDFILE);
      std::auto_ptr<DataSource> Source(DataSource::GetNewDataSourceObject(gParameters.GetCoordinatesFileName(), gPrint));
    return ReadCoordinates(DataSet, *Source);
  }
  catch (prg_exception& x) {
    x.addTrace("ReadCoordinatesFile()","HomogeneousPoissonDataSetHandler");
    throw;
  }
}

/** Attempts to read data files into class RealDataSet objects. */
bool HomogeneousPoissonDataSetHandler::ReadData() {
  try {
    for (size_t t=0; t < GetNumDataSets(); ++t) {
       //read case data file
       if (GetNumDataSets() == 1) gPrint.Printf("Reading the coordinates file\n", BasePrint::P_STDOUT);
       else gPrint.Printf("Reading the coordinates file for data set %u\n", BasePrint::P_STDOUT, t + 1);
       if (!ReadCoordinatesFile(GetDataSet(t))) return false;
    }

    //signal insertions completed
    gTractHandler.additionsCompleted();

    if (gParameters.UseSpecialGrid()) {
       gPrint.Printf("Reading the grid file\n", BasePrint::P_STDOUT);
       gPrint.SetImpliedInputFileType(BasePrint::GRIDFILE);
       std::auto_ptr<DataSource> Source(DataSource::GetNewDataSourceObject(gParameters.GetSpecialGridFileName(), gPrint));
       if (!ReadGridFile(*Source)) return false;
    }
    SetRandomizers();
  }
  catch (prg_exception& x) {
    x.addTrace("ReadData()","HomogeneousPoissonDataSetHandler");
    throw;
  }
  return true;
}

/** Allocates randomizers for data set.  */
void HomogeneousPoissonDataSetHandler::SetRandomizers() {
  try {
    gvDataSetRandomizers.killAll();
    gvDataSetRandomizers.resize(gParameters.GetNumDataSets(), 0);
    // create the randomizer -- only implemented for one data set
    gvDataSetRandomizers.at(0) = new HomogeneousPoissonNullHypothesisRandomizer(gParameters, gPolygons, gParameters.GetRandomizationSeed());
  }
  catch (prg_exception& x) {
    x.addTrace("SetRandomizers()","HomogeneousPoissonDataSetHandler");
    throw;
  }
}
 

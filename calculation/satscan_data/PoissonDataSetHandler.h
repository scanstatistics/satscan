//******************************************************************************
#ifndef __PoissonDataSetHandler_H
#define __PoissonDataSetHandler_H
//******************************************************************************
#include "DataSetHandler.h"
#include "PoissonRandomizer.h"

/** Function object to compare container of std::pair<Julian, DatePrecisionType> objects. */
class ComparePopulationDates {
  public:
    /** Compare std::pair<Julian, DatePrecisionType> objects by Julian */
    bool operator() (const std::pair<Julian, DatePrecisionType>& lhs,
                     const std::pair<Julian, DatePrecisionType>& rhs) {return lhs.first < rhs.first;}
};

/** Data set handler for the Poisson probablity model, expanding on
    functionality of base class DataSetHandler.
    Defines processes for:
     - reading population data from file(s) into datasets' structures
     - allocation of dataset's randomizers
     - allocation of dataset objects used during simulations 
     - allocation of gateway objects used by analysis object to access data
       set structures for real data and simulation data */
class PoissonDataSetHandler : public DataSetHandler {
  protected:
    bool                                ConvertPopulationDateToJulian(const char * sDateString, int iRecordNumber, PopulationData::PopulationDate_t& PopulationDate);
    bool                                CreatePopulationData(RealDataSet& DataSet);
    bool                                ReadPopulationFile(RealDataSet& DataSet);
    virtual void                        SetRandomizers();

  public:
    PoissonDataSetHandler(CSaTScanData& DataHub, BasePrint& Print) : DataSetHandler(DataHub, Print) {}
    virtual ~PoissonDataSetHandler() {}

    virtual SimulationDataContainer_t & AllocateSimulationData(SimulationDataContainer_t& Container) const;
    virtual void                        assignMetaLocationData(RealDataContainer_t& Container) const;
    virtual const RealDataContainer_t & buildOliveiraDataSets();
    virtual AbstractDataSetGateway    & GetDataGateway(AbstractDataSetGateway& DataGatway) const;
    virtual AbstractDataSetGateway    & GetOliveraDataGateway(AbstractDataSetGateway& DataGatway, const SimulationDataContainer_t& Container) const;
    virtual AbstractDataSetGateway    & GetSimulationDataGateway(AbstractDataSetGateway& DataGatway, const SimulationDataContainer_t& Container, const RandomizerContainer_t& rContainer) const;
    virtual void                        RandomizeData(RandomizerContainer_t& Container, SimulationDataContainer_t& SimDataContainer, unsigned int iSimulationNumber) const;
    virtual bool                        ReadData();
};
//******************************************************************************
#endif


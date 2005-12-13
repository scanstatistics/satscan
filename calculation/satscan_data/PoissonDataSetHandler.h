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
    bool                                ConvertPopulationDateToJulian(const char * sDateString, int iRecordNumber,
                                                                      std::pair<Julian, DatePrecisionType>& PopulationDate);
    bool                                CreatePopulationData(size_t tSetIndex);
    bool                                ReadPopulationFile(size_t tSetIndex);
    virtual void                        SetRandomizers();

  public:
    PoissonDataSetHandler(CSaTScanData& DataHub, BasePrint& Print) : DataSetHandler(DataHub, Print) {}
    virtual ~PoissonDataSetHandler() {}

    virtual SimulationDataContainer_t&  AllocateSimulationData(SimulationDataContainer_t& Container) const;
    virtual AbstractDataSetGateway     & GetDataGateway(AbstractDataSetGateway& DataGatway) const;
    virtual AbstractDataSetGateway     & GetSimulationDataGateway(AbstractDataSetGateway& DataGatway, const SimulationDataContainer_t& Container) const;
    virtual double                      GetSimulationDataSetAllocationRequirements() const;
    virtual bool                        ReadData();
};
//******************************************************************************
#endif


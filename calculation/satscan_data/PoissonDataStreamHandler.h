//---------------------------------------------------------------------------
#ifndef PoissonDataStreamHandlerH
#define PoissonDataStreamHandlerH
//---------------------------------------------------------------------------
#include "DataStreamHandler.h"
#include "PoissonRandomizer.h"

/** Function object to compare container of std::pair<Julian, DatePrecisionType> objects. */
class ComparePopulationDates {
  public:
    /** Compare std::pair<Julian, DatePrecisionType> objects by Julian */
    bool operator() (const std::pair<Julian, DatePrecisionType>& lhs,
                     const std::pair<Julian, DatePrecisionType>& rhs) {return lhs.first < rhs.first;}
};

/** Data stream handler for the Poisson probablity model, expanding on
    functionality of base class DataStreamHandler.
    Defines processes for:
     - reading population data from file(s) into data stream structures
     - allocation of data stream randomizers
     - allocation of data stream objects used during simulations 
     - allocation of gateway objects used by analysis object to access data
       stream structures for real data and simulation data */
class PoissonDataStreamHandler : public DataStreamHandler {
  protected:
    bool                                ConvertPopulationDateToJulian(const char * sDateString, int iRecordNumber,
                                                                      std::pair<Julian, DatePrecisionType>& PopulationDate);
    bool                                CreatePopulationData(size_t tStream);
    bool                                ReadPopulationFile(size_t tStream);
    virtual void                        SetRandomizers();

  public:
    PoissonDataStreamHandler(CSaTScanData& Data, BasePrint& Print);
    virtual ~PoissonDataStreamHandler();

    virtual AbtractDataStreamGateway  * GetNewDataGateway() const;
    virtual AbtractDataStreamGateway  * GetNewSimulationDataGateway(const SimulationDataContainer_t& Container) const;
    virtual SimulationDataContainer_t & GetSimulationDataContainer(SimulationDataContainer_t& Container) const;
    virtual bool                        ReadData();
};
#endif

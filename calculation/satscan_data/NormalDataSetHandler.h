//******************************************************************************
#ifndef __NormalDataSetHandler_H
#define __NormalDataSetHandler_H
//******************************************************************************
#include "DataSetHandler.h"
#include "NormalRandomizer.h"

class NormalDataSetHandler : public DataSetHandler {
  private:
    CountFileReadStatus                 ReadCountsStandard(RealDataSet& DataSet, DataSource& Source);
    CountFileReadStatus                 ReadCountsWeighted(RealDataSet& DataSet, DataSource& Source);

  protected:
    virtual CountFileReadStatus         ReadCounts(RealDataSet& DataSet, DataSource& Source);
    virtual RecordStatusType            RetrieveCaseRecordData(DataSource& Source, tract_t& tid, 
                                                               count_t& nCount, Julian& nDate, 
                                                               measure_t& tContinuousVariable, double * pRateVariable,
                                                               std::vector<double> * pvCovariates);
    bool                                setIsWeighted();
    virtual void                        SetRandomizers();

  public:
    NormalDataSetHandler(CSaTScanData& DataHub, BasePrint& Print) : DataSetHandler(DataHub, Print) {}
    virtual ~NormalDataSetHandler() {}

    virtual SimulationDataContainer_t & AllocateSimulationData(SimulationDataContainer_t& Container) const;
    virtual void                        assignMetaData(RealDataContainer_t& Container) const;
    virtual AbstractDataSetGateway    & GetDataGateway(AbstractDataSetGateway& DataGatway) const;
    virtual AbstractDataSetGateway    & GetSimulationDataGateway(AbstractDataSetGateway& DataGatway, const SimulationDataContainer_t& Container, const RandomizerContainer_t& rContainer) const;
    virtual void                        RandomizeData(RandomizerContainer_t& Container, SimulationDataContainer_t& SimDataContainer, unsigned int iSimulationNumber) const;
    virtual bool                        ReadData();
    virtual void                        SetPurelyTemporalMeasureData(RealDataSet & DataSet);
    virtual void                        SetPurelyTemporalSimulationData(SimulationDataContainer_t& SimDataContainer);
};
//******************************************************************************
#endif


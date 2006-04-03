//******************************************************************************
#ifndef SpaceTimePermutationDataSetHandlerH
#define SpaceTimePermutationDataSetHandlerH
//******************************************************************************
#include "DataSetHandler.h"
#include "SpaceTimeRandomizer.h"

class SpaceTimePermutationDataSetHandler : public DataSetHandler {
  protected:
    virtual void                        AllocateCaseStructures(size_t tSetIndex);
    virtual bool                        ReadCounts(size_t iSetIndex, DataSource& Source, const char* szDescription);
    virtual void                        SetRandomizers();

  public:
    SpaceTimePermutationDataSetHandler(CSaTScanData& DataHub, BasePrint& Print);
    virtual ~SpaceTimePermutationDataSetHandler();

    virtual SimulationDataContainer_t & AllocateSimulationData(SimulationDataContainer_t& Container) const;
    virtual AbstractDataSetGateway    & GetDataGateway(AbstractDataSetGateway& DataGatway) const;
    virtual AbstractDataSetGateway    & GetSimulationDataGateway(AbstractDataSetGateway& DataGatway, const SimulationDataContainer_t& Container) const;
    virtual double                      GetSimulationDataSetAllocationRequirements() const;
    virtual bool                        ReadData();
};
//******************************************************************************
#endif


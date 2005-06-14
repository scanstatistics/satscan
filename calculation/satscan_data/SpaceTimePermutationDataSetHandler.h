//******************************************************************************
#ifndef SpaceTimePermutationDataSetHandlerH
#define SpaceTimePermutationDataSetHandlerH
//******************************************************************************
#include "DataSetHandler.h"
#include "SpaceTimeRandomizer.h"

class SpaceTimePermutationDataSetHandler : public DataSetHandler {
  protected:
    virtual void                        AllocateCaseStructures(size_t tSetIndex);
    virtual bool                        ReadCounts(size_t tSetIndex, FILE * fp, const char* szDescription);
    virtual void                        SetRandomizers();

  public:
    SpaceTimePermutationDataSetHandler(CSaTScanData& DataHub, BasePrint& Print);
    virtual ~SpaceTimePermutationDataSetHandler();

    virtual SimulationDataContainer_t & AllocateSimulationData(SimulationDataContainer_t& Container) const;
    virtual AbtractDataSetGateway     & GetDataGateway(AbtractDataSetGateway& DataGatway) const;
    virtual AbtractDataSetGateway     & GetSimulationDataGateway(AbtractDataSetGateway& DataGatway, const SimulationDataContainer_t& Container) const;
    virtual double                      GetSimulationDataSetAllocationRequirements() const;
    virtual bool                        ReadData();
};
//******************************************************************************
#endif


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

    virtual AbtractDataSetGateway     * GetNewDataGateway() const;
    virtual AbtractDataSetGateway     * GetNewSimulationDataGateway(const SimulationDataContainer_t& Container) const;
    virtual SimulationDataContainer_t & GetSimulationDataContainer(SimulationDataContainer_t& Container) const;
    virtual bool                        ReadData();
};
//******************************************************************************
#endif


//******************************************************************************
#ifndef __CoefficientsWriter_H
#define __CoefficientsWriter_H
//******************************************************************************
#include "AbstractDataFileWriter.h"

class CSaTScanData;
class MostLikelyClustersContainer;
class SimulationVariables;

/** optimized spatial window coefficients file writer. */
class CoefficientsWriter : public AbstractDataFileWriter {
  protected:
    static const char * CLUSTER_SIZE_FIELD;
    static const char * GINI_FIELD;
    static const char * CLIC_FIELD;
    static const char * FILE_EXT;
    std::auto_ptr<RecordBuffer> _recordBuffer;

    const CSaTScanData & _dataHub;
    const SimulationVariables & _simVars;

    void defineFields();

  public:
    CoefficientsWriter(const CSaTScanData& dataHub, const SimulationVariables& simVars);
    virtual ~CoefficientsWriter() {}

    virtual void write(const MostLikelyClustersContainer& mlc);
};
//******************************************************************************
#endif

//******************************************************************************
#include "SaTScan.h"
#pragma hdrstop
//******************************************************************************
#include "CoefficientsWriter.h"
#include "SSException.h"
#include "SaTScanData.h"
#include "MostLikelyClustersContainer.h"
#include "SimulationVariables.h"

const char * CoefficientsWriter::CLUSTER_SIZE_FIELD = "MAX_SIZE";
const char * CoefficientsWriter::GINI_FIELD = "GINI";
const char * CoefficientsWriter::CLIC_FIELD = "CLIC";
const char * CoefficientsWriter::FILE_EXT = ".ceo";

/** constructor */
CoefficientsWriter::CoefficientsWriter(const CSaTScanData& dataHub, const SimulationVariables& simVars)
                   :AbstractDataFileWriter(dataHub.GetParameters()), _dataHub(dataHub), _simVars(simVars) {
  try {
    defineFields();
    _recordBuffer.reset(new RecordBuffer(vFieldDefinitions));
    if (gParameters.getOutputCoefficientsAscii())
      gpASCIIFileWriter = new ASCIIDataFileWriter(gParameters, vFieldDefinitions, FILE_EXT, false);
    if (gParameters.getOutputCoefficientsDBase())
      gpDBaseFileWriter = new DBaseDataFileWriter(gParameters, vFieldDefinitions, FILE_EXT, false);
  } catch (prg_exception& x) {
    delete gpASCIIFileWriter; gpASCIIFileWriter=0;
    delete gpDBaseFileWriter; gpDBaseFileWriter=0;
    x.addTrace("constructor()","CoefficientsWriter");
    throw;
  }
}

/** Defines fields of output file. */
void CoefficientsWriter::defineFields() {
  unsigned short uwOffset=0;
  try {
    CreateField(vFieldDefinitions, CLUSTER_SIZE_FIELD, FieldValue::NUMBER_FLD, 10, 8, uwOffset, 4);
    CreateField(vFieldDefinitions, GINI_FIELD, FieldValue::NUMBER_FLD, 9, 4, uwOffset, 4);
    CreateField(vFieldDefinitions, CLIC_FIELD, FieldValue::NUMBER_FLD, 9, 4, uwOffset, 4);
  } catch (prg_exception& x) {
    x.addTrace("DefineFields()","CoefficientsWriter");
    throw;
  }
}

/** Writes data to each requested file type. */
void CoefficientsWriter::write(const MostLikelyClustersContainer& mlc) {
  try {
    _recordBuffer->SetAllFieldsBlank(true);
    _recordBuffer->GetFieldValue(CLUSTER_SIZE_FIELD).AsDouble() = mlc.getMaximumWindowSize();
    _recordBuffer->GetFieldValue(GINI_FIELD).AsDouble() = mlc.getGiniCoefficient(_dataHub, _simVars, gParameters.GetOptimizingSpatialCutOffPValue());
    _recordBuffer->GetFieldValue(CLIC_FIELD).AsDouble() = mlc.getClicCoefficient(_dataHub, _simVars, gParameters.GetOptimizingSpatialCutOffPValue());
    if (gpASCIIFileWriter) gpASCIIFileWriter->WriteRecord(*_recordBuffer);
    if (gpDBaseFileWriter) gpDBaseFileWriter->WriteRecord(*_recordBuffer);
  } catch (prg_exception& x) {
    x.addTrace("Write()","CoefficientsWriter");
    throw;
  }
}

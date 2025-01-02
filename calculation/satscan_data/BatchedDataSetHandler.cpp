//******************************************************************************
#include "SaTScan.h"
#pragma hdrstop
//******************************************************************************
#include "SaTScanData.h"
#include "BatchedDataSetHandler.h"
#include "SSException.h"
#include "DataSource.h" 

/** For each element in SimulationDataContainer_t, allocates appropriate data structures
    as needed by data set handler (probability model) and analysis type. */
SimulationDataContainer_t & BatchedDataSetHandler::AllocateSimulationData(SimulationDataContainer_t& Container) const {
    switch (gParameters.GetAnalysisType()) {
        case PURELYSPATIAL: 
            std::for_each(Container.begin(), Container.end(), std::mem_fun(&DataSet::allocateCaseData));
            std::for_each(Container.begin(), Container.end(), std::mem_fun(&DataSet::allocateMeasureData));
            std::for_each(Container.begin(), Container.end(), std::mem_fun(&DataSet::allocateMeasureData_Aux));
            std::for_each(Container.begin(), Container.end(), std::mem_fun(&DataSet::allocateMeasureData_Aux2));
            for (unsigned int i=0; i < Container.size(); ++i)
                Container[i]->allocatePositiveBatchData(static_cast<unsigned int>(gDataHub.GetDataSetHandler().GetDataSet(i).getTotalMeasure()));
            break;
        case SEASONALTEMPORAL:
        case PURELYTEMPORAL:
        case PROSPECTIVEPURELYTEMPORAL: 
            std::for_each(Container.begin(), Container.end(), std::mem_fun(&DataSet::allocateCaseData_PT));
            std::for_each(Container.begin(), Container.end(), std::mem_fun(&DataSet::allocateMeasureData_PT));
            std::for_each(Container.begin(), Container.end(), std::mem_fun(&DataSet::allocateMeasureData_PT_Aux));
            std::for_each(Container.begin(), Container.end(), std::mem_fun(&DataSet::allocateMeasureData_PT_Aux2)); 
            for (unsigned int i = 0; i < Container.size(); ++i)
                Container[i]->allocatePositiveBatchData_PT(static_cast<unsigned int>(gDataHub.GetDataSetHandler().GetDataSet(i).getTotalMeasure()));
            break;
        case SPACETIME:
        case PROSPECTIVESPACETIME: 
            std::for_each(Container.begin(), Container.end(), std::mem_fun(&DataSet::allocateCaseData));
            std::for_each(Container.begin(), Container.end(), std::mem_fun(&DataSet::allocateMeasureData));
            std::for_each(Container.begin(), Container.end(), std::mem_fun(&DataSet::allocateMeasureData_Aux));
            std::for_each(Container.begin(), Container.end(), std::mem_fun(&DataSet::allocateMeasureData_Aux2));
            for (unsigned int i = 0; i < Container.size(); ++i)
                Container[i]->allocatePositiveBatchData(static_cast<unsigned int>(gDataHub.GetDataSetHandler().GetDataSet(i).getTotalMeasure()));
            if (gParameters.GetIncludePurelyTemporalClusters()) {
                std::for_each(Container.begin(), Container.end(), std::mem_fun(&DataSet::allocateCaseData_PT));
                std::for_each(Container.begin(), Container.end(), std::mem_fun(&DataSet::allocateMeasureData_PT));
                std::for_each(Container.begin(), Container.end(), std::mem_fun(&DataSet::allocateMeasureData_PT_Aux));
                std::for_each(Container.begin(), Container.end(), std::mem_fun(&DataSet::allocateMeasureData_PT_Aux2)); 
                for (unsigned int i = 0; i < Container.size(); ++i)
                    Container[i]->allocatePositiveBatchData_PT(static_cast<unsigned int>(gDataHub.GetDataSetHandler().GetDataSet(i).getTotalMeasure()));
            }
            break;
        case SPATIALVARTEMPTREND       :
            throw prg_error("AllocateSimulationData() not implemented for spatial variation and temporal trends analysis.","AllocateSimulationData()");
        default                        :
            throw prg_error("Unknown analysis type '%d'.","AllocateSimulationData()", gParameters.GetAnalysisType());
    };
    return Container;
}

/** For each data set, assigns data at meta location indexes. */
void BatchedDataSetHandler::assignMetaData(RealDataContainer_t& Container) const {
    for (auto& data: Container) {
        data->setCaseDataMeta(gDataHub.getIdentifierInfo().getMetaManagerProxy());
        data->setMeasureDataMeta(gDataHub.getIdentifierInfo().getMetaManagerProxy());
        data->setMeasureDataAuxMeta(gDataHub.getIdentifierInfo().getMetaManagerProxy());
        data->setMeasureDataAux2Meta(gDataHub.getIdentifierInfo().getMetaManagerProxy());
        data->setBitsetDataMeta(gDataHub.getIdentifierInfo().getMetaManagerProxy());
        data->setPositiveBitsetDataMeta(gDataHub.getIdentifierInfo().getMetaManagerProxy());
    }
}

/** returns new data gateway for real data */
AbstractDataSetGateway & BatchedDataSetHandler::GetDataGateway(AbstractDataSetGateway& DataGatway) const {
    DataSetInterface Interface(
        gDataHub.GetNumTimeIntervals(),
        gDataHub.GetNumIdentifiers() + gDataHub.getIdentifierInfo().getMetaManagerProxy().getNumMeta(),
        gDataHub.getDataInterfaceIntervalStartIndex()
    );
    try {
        DataGatway.Clear();
        for (const auto& DataSet: gvDataSets) {
            //set total cases and measure
            Interface.SetTotalCasesCount(DataSet->getTotalCases());
            Interface.SetTotalMeasureCount(DataSet->getTotalMeasure());
            Interface.SetTotalMeasureAuxCount(DataSet->getTotalMeasureAux());
            Interface.SetTotalMeasureAux2Count(DataSet->getTotalMeasureAux2());
            //set pointers to data structures
            switch (gParameters.GetAnalysisType()) {
                case PURELYSPATIAL:
                    Interface.SetCaseArray(DataSet->getCaseData().GetArray());
                    Interface.SetMeasureArray(DataSet->getMeasureData().GetArray());
                    Interface.SetMeasureAuxArray(DataSet->getMeasureData_Aux().GetArray());
                    Interface.SetMeasureAux2Array(DataSet->getMeasureData_Aux2().GetArray());
                    Interface.setPositiveBatchIndexesArray(DataSet->getPositiveBatchData().GetArray());
                    break;
                case SEASONALTEMPORAL:
                case PROSPECTIVEPURELYTEMPORAL:
                case PURELYTEMPORAL:
                    Interface.SetPTMeasureArray(DataSet->getMeasureData_PT());
                    Interface.SetPTCaseArray(DataSet->getCaseData_PT());
                    Interface.SetPTMeasureAuxArray(DataSet->getMeasureData_PT_Aux());
                    Interface.SetPTMeasureAux2Array(DataSet->getMeasureData_PT_Aux2());
                    Interface.setPtPositiveBatchIndexesArray(DataSet->getPositiveBatchIndexes_PT());
                    break;
                case SPACETIME:
                case PROSPECTIVESPACETIME:
                    Interface.SetCaseArray(DataSet->getCaseData().GetArray());
                    Interface.SetMeasureArray(DataSet->getMeasureData().GetArray());
                    Interface.SetMeasureAuxArray(DataSet->getMeasureData_Aux().GetArray());
                    Interface.SetMeasureAux2Array(DataSet->getMeasureData_Aux2().GetArray());
                    Interface.setPositiveBatchIndexesArray(DataSet->getPositiveBatchData().GetArray());
                    if (gParameters.GetIncludePurelyTemporalClusters()) {
                        Interface.SetPTCaseArray(DataSet->getCaseData_PT());
                        Interface.SetPTMeasureArray(DataSet->getMeasureData_PT());
                        Interface.SetPTMeasureAuxArray(DataSet->getMeasureData_PT_Aux());
                        Interface.SetPTMeasureAux2Array(DataSet->getMeasureData_PT_Aux2());
                        Interface.setPtPositiveBatchIndexesArray(DataSet->getPositiveBatchIndexes_PT());
                    }
                    Interface.setBatchIndexesArray(DataSet->getBatchData().GetArray());
                    break;
                case SPATIALVARTEMPTREND:
                    throw prg_error("GetDataGateway() not implemented for purely spatial monotone analysis.","GetDataGateway()");
                default :
                    throw prg_error("Unknown analysis type '%d'.","GetDataGateway()",gParameters.GetAnalysisType());
            };
            DataGatway.AddDataSetInterface(Interface);
        }
    } catch (prg_exception& x) {
        x.addTrace("GetDataGateway()","BatchedDataSetHandler");
        throw;
    }
    return DataGatway;
}

void BatchedDataSetHandler::SetPurelyTemporalMeasureData(RealDataSet& DataSet) {
    try {
        DataSet.setMeasureData_PT();
        DataSet.setMeasureData_PT_Aux();
        DataSet.setMeasureData_PT_Aux2();
        DataSet.setPositiveBatchIndexes_PT(static_cast<unsigned int>(DataSet.getTotalMeasure()));
    } catch (prg_exception& x) {
        x.addTrace("SetPurelyTemporalMeasureData()", "BatchedDataSetHandler");
        throw;
    }
}

/** returns new data gateway for simulation data */
AbstractDataSetGateway & BatchedDataSetHandler::GetSimulationDataGateway(AbstractDataSetGateway& DataGatway, const SimulationDataContainer_t& Container, const RandomizerContainer_t& rContainer) const {
    DataSetInterface Interface(
        gDataHub.GetNumTimeIntervals(),
        gDataHub.GetNumIdentifiers() + gDataHub.getIdentifierInfo().getMetaManagerProxy().getNumMeta(),
        gDataHub.getDataInterfaceIntervalStartIndex()
    );
    try {
        DataGatway.Clear();
        for (size_t t=0; t < gvDataSets.size(); ++t) {
            //get reference to datasets
            const RealDataSet& R_DataSet = *gvDataSets.at(t);
            const DataSet& S_DataSet = *Container.at(t);
            //set total cases and measure
            Interface.SetTotalCasesCount(R_DataSet.getTotalCases());
            Interface.SetTotalMeasureCount(R_DataSet.getTotalMeasure());
            Interface.SetTotalMeasureAuxCount(S_DataSet.getTotalMeasureAux());
            Interface.SetTotalMeasureAux2Count(S_DataSet.getTotalMeasureAux2());
            //set pointers to data structures
            switch (gParameters.GetAnalysisType()) {
                case PURELYSPATIAL:
                    Interface.SetCaseArray(S_DataSet.getCaseData().GetArray());
                    Interface.SetMeasureArray(R_DataSet.getMeasureData().GetArray());
                    Interface.SetMeasureAuxArray(S_DataSet.getMeasureData_Aux().GetArray());
                    Interface.SetMeasureAux2Array(S_DataSet.getMeasureData_Aux2().GetArray());
                    Interface.setPositiveBatchIndexesArray(S_DataSet.getPositiveBatchData().GetArray());
                    break;
                case SEASONALTEMPORAL:
                case PROSPECTIVEPURELYTEMPORAL:
                case PURELYTEMPORAL:
                    Interface.SetPTCaseArray(S_DataSet.getCaseData_PT());
                    Interface.SetPTMeasureArray(R_DataSet.getMeasureData_PT());
                    Interface.SetPTMeasureAuxArray(S_DataSet.getMeasureData_PT_Aux());
                    Interface.SetPTMeasureAux2Array(S_DataSet.getMeasureData_PT_Aux2());
                    Interface.setPtPositiveBatchIndexesArray(S_DataSet.getPositiveBatchIndexes_PT());
                    break;
                case SPACETIME:
                case PROSPECTIVESPACETIME:
                    Interface.SetCaseArray(S_DataSet.getCaseData().GetArray());
                    Interface.SetMeasureArray(R_DataSet.getMeasureData().GetArray());
                    Interface.SetMeasureAuxArray(S_DataSet.getMeasureData_Aux().GetArray());
                    Interface.SetMeasureAux2Array(S_DataSet.getMeasureData_Aux2().GetArray());
                    Interface.setPositiveBatchIndexesArray(S_DataSet.getPositiveBatchData().GetArray());
                    if (gParameters.GetIncludePurelyTemporalClusters()) {
                        Interface.SetPTCaseArray(S_DataSet.getCaseData_PT());
                        Interface.SetPTMeasureArray(R_DataSet.getMeasureData_PT());
                        Interface.SetPTMeasureAuxArray(S_DataSet.getMeasureData_PT_Aux());
                        Interface.SetPTMeasureAux2Array(S_DataSet.getMeasureData_PT_Aux2());
                        Interface.setPtPositiveBatchIndexesArray(S_DataSet.getPositiveBatchIndexes_PT());
                    } 
                    Interface.setBatchIndexesArray(R_DataSet.getBatchData().GetArray());
                    break;
                case SPATIALVARTEMPTREND:
                    throw prg_error("GetSimulationDataGateway() not implemented for purely spatial monotone analysis.","GetSimulationDataGateway()");
                default :
                    throw prg_error("Unknown analysis type '%d'.","GetSimulationDataGateway()",gParameters.GetAnalysisType());
            };
            DataGatway.AddDataSetInterface(Interface);
        }
    } catch (prg_exception& x) {
        x.addTrace("GetSimulationDataGateway()","BatchedDataSetHandler");
        throw;
    }
    return DataGatway;
}

/** Randomizes data and assigns data at meta location indexes (if using meta locations file)*/
void BatchedDataSetHandler::RandomizeData(RandomizerContainer_t& Container, SimulationDataContainer_t& SimDataContainer, unsigned int iSimulationNumber) const {
    DataSetHandler::RandomizeData(Container, SimDataContainer, iSimulationNumber);
    if (gParameters.UseMetaLocationsFile() || gParameters.UsingMultipleCoordinatesMetaLocations()) {
        for (auto& simData : SimDataContainer) {
            simData->setCaseDataMeta(gDataHub.getIdentifierInfo().getMetaManagerProxy());
            simData->setMeasureDataAuxMeta(gDataHub.getIdentifierInfo().getMetaManagerProxy());
            simData->setMeasureDataAux2Meta(gDataHub.getIdentifierInfo().getMetaManagerProxy());
            simData->setPositiveBitsetDataMeta(gDataHub.getIdentifierInfo().getMetaManagerProxy());
        }
    }
}

/** Reads the count data source, storing data in RealDataSet object. As a
    means to help user clean-up their data, continues to read records as errors
    are encountered. Returns boolean indication of read success. */
DataSetHandler::CountFileReadStatus BatchedDataSetHandler::ReadCounts(RealDataSet& DataSet, DataSource& Source) {
    Julian                                Date;
    tract_t                               tractIndex;
    count_t                               trapCount, batchSize, positive, numPositive, tTotalCases=0, ** ppCounts;
    measure_t                             tTotalMeasure=0;
    BatchedRandomizer                   * pRandomizer;
    DataSetHandler::CountFileReadStatus   readStatus = DataSetHandler::NoCounts;

    try {
        // if randomization data created by reading from file, we'll need to use temporary randomizer to create real data set
        pRandomizer = dynamic_cast<BatchedRandomizer*>(gvDataSetRandomizers.at(DataSet.getSetIndex() - 1));
        if (!pRandomizer)
            throw prg_error("Data set randomizer not BatchedRandomizer type.", "ReadCounts()");
        ppCounts = DataSet.allocateCaseData().GetArray();
        //Read data, parse and if no errors, increment count for tract at date.
        while (!gPrint.GetMaximumReadErrorsPrinted() && Source.ReadRecord()) {
            DataSetHandler::RecordStatusType eRecordStatus = RetrieveCaseRecordData(Source, tractIndex, trapCount, Date, batchSize, positive);
            if (eRecordStatus == DataSetHandler::Accepted) {
                readStatus = readStatus == DataSetHandler::NoCounts ? DataSetHandler::ReadSuccess : readStatus;
                if (gParameters.GetAnalysisType() == SEASONALTEMPORAL)
                    Date = gDataHub.convertToSeasonalDate(Date);
                pRandomizer->AddBatch(tractIndex, trapCount, Date, batchSize, positive);
            } else if (eRecordStatus == DataSetHandler::Ignored)
                continue;
            else
                readStatus = DataSetHandler::ReadError;
        }
        if (readStatus == DataSetHandler::ReadSuccess)
            pRandomizer->AssignFromAttributes(DataSet);
        //DataSet.setTotalCases(tTotalCases);
    } catch (prg_exception& x) {
        x.addTrace("ReadCounts()","BatchedDataSetHandler");
        throw;
    }
    return readStatus;
}

/** Attempts to read case data file into class RealDataSet objects. */
bool BatchedDataSetHandler::ReadData() {
    DataSetHandler::CountFileReadStatus readStaus;
    try {
        SetRandomizers();
        size_t numDataSet = GetNumDataSets();
        for (size_t t=0; t < numDataSet; ++t) {
            printFileReadMessage(BasePrint::CASEFILE, t, numDataSet == 1);
            readStaus = ReadCaseFile(GetDataSet(t));
            printReadStatusMessage(readStaus, false, t, numDataSet == 1);
            if (readStaus == DataSetHandler::ReadError || (readStaus != DataSetHandler::ReadSuccess && numDataSet == 1))
                return false;
        }
        removeDataSetsWithNoData();
    } catch (prg_exception& x) {
        x.addTrace("ReadData()","BatchedDataSetHandler");
        throw;
    }
    return true;
}

/** Parses current file record contained in DataSource object in expected
    parts: location, # patients, date, survival time and censor attribute. Returns
    DataSetHandler::Accepted if no errors in data were found, else prints error
    messages to BasePrint object and returns RecordStatusType. */
DataSetHandler::RecordStatusType BatchedDataSetHandler::RetrieveCaseRecordData(DataSource& Source, tract_t& tid, count_t& trapCount, Julian& nDate, count_t& batchSize, count_t& positive) {
    try {
        DataSetHandler::RecordStatusType eStatus = RetrieveIdentifierIndex(Source, tid); //read and validate that tract identifier
        if (eStatus != DataSetHandler::Accepted) return eStatus;
        eStatus = RetrieveCaseCounts(Source, trapCount); // read and validate count
        if (eStatus != DataSetHandler::Accepted) return eStatus;
        eStatus = RetrieveCountDate(Source, nDate); // read and validate date
        if (eStatus != DataSetHandler::Accepted) return eStatus;
        // read batch size
        short batchIdx = gParameters.GetPrecisionOfTimesType() == NONE ? (short)2 : (short)3;
        if (!Source.GetValueAt(batchIdx)) {
            gPrint.Printf(
                "Error: Record %d of the %s is missing the batch size.\n",
                BasePrint::P_READERROR, Source.GetCurrentRecordIndex(), gPrint.GetImpliedFileTypeString().c_str()
            );
            return DataSetHandler::Rejected;
        }
        if (!string_to_type<count_t>(Source.GetValueAt(batchIdx), batchSize) || batchSize <= 0) {
            gPrint.Printf(
                "Error: The batch size value '%s' in record %ld of the %s is not valid.\n"
                "       Batch size must be an integer value greater than 0.\n",
                BasePrint::P_READERROR, Source.GetValueAt(batchIdx), Source.GetCurrentRecordIndex(), gPrint.GetImpliedFileTypeString().c_str()
            );
            return DataSetHandler::Rejected;
        }
        //read and validate positive attribute
        short positiveIdx = gParameters.GetPrecisionOfTimesType() == NONE ? (short)3 : (short)4;
        if (!Source.GetValueAt(positiveIdx)) {
            gPrint.Printf(
                "Error: Record %d of the %s is missing the positive/negative attribute.\n",
                BasePrint::P_READERROR, Source.GetCurrentRecordIndex(), gPrint.GetImpliedFileTypeString().c_str()
            );
            return DataSetHandler::Rejected;
        }
        if (!string_to_type<count_t>(Source.GetValueAt(positiveIdx), positive) || !(positive == 0 || positive == 1)) {
            gPrint.Printf(
                "Error: The value '%s' of record %ld, in the %s, could not be read as valid positive/negative attribute.\n"
                "       The value must be either 0 or 1.\n", BasePrint::P_READERROR,
                Source.GetValueAt(positiveIdx), Source.GetCurrentRecordIndex(), gPrint.GetImpliedFileTypeString().c_str()
            );
            return DataSetHandler::Rejected;
        }
    } catch (prg_exception& x) {
        x.addTrace("RetrieveCaseRecordData()","BatchedDataSetHandler");
        throw;
    }
    return DataSetHandler::Accepted;
}

/** sets purely temporal structures used in simulations */
void BatchedDataSetHandler::SetPurelyTemporalSimulationData(SimulationDataContainer_t& Container) {
    std::for_each(Container.begin(), Container.end(), std::mem_fun(&DataSet::setCaseData_PT));
    std::for_each(Container.begin(), Container.end(), std::mem_fun(&DataSet::setMeasureData_PT));
    std::for_each(Container.begin(), Container.end(), std::mem_fun(&DataSet::setMeasureData_PT_Aux));
    std::for_each(Container.begin(), Container.end(), std::mem_fun(&DataSet::setMeasureData_PT_Aux2));
    for (unsigned int i = 0; i < Container.size(); ++i)
        Container[i]->setPositiveBatchIndexes_PT(static_cast<unsigned int>(gDataHub.GetDataSetHandler().GetDataSet(i).getTotalMeasure()));
}

/** Allocates randomizers for each dataset. There are currently 2 randomization types
    for the Exponential model: null hypothesis and  purely temporal optimized null hypothesis. */
void BatchedDataSetHandler::SetRandomizers() {
    try {
        gvDataSetRandomizers.killAll();
        gvDataSetRandomizers.resize(gParameters.getNumFileSets(), 0);
        switch (gParameters.GetSimulationType()) {
            case STANDARD:
                gvDataSetRandomizers.at(0) = new BatchedRandomizer(gDataHub, gParameters.GetRandomizationSeed());
                break;
            case FILESOURCE :
            case HA_RANDOMIZATION :
            default : throw prg_error("Unknown simulation type '%d'.","SetRandomizers()", gParameters.GetSimulationType());
        };
        for (size_t t=1; t < gParameters.getNumFileSets(); ++t) // create more as needed
            gvDataSetRandomizers.at(t) = gvDataSetRandomizers.at(0)->Clone();
    } catch (prg_exception& x) {
        x.addTrace("SetRandomizers()","BatchedDataSetHandler");
        throw;
    }
}

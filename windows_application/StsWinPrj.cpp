//---------------------------------------------------------------------------
#include "stsSaTScan.h"
#pragma hdrstop
#include "stsFrmUpdateCheck.h"
//---------------------------------------------------------------------------

USERES("StsWinPrj.res");
USEFORM("stsMain.cpp", frmMainForm);
USEFORM("stsFrmAnalysisParameters.cpp", frmAnalysis);
USEFORM("stsFrmAbout.cpp", frmAbout);
USEFORM("stsFrmAnalysisRun.cpp", frmAnalysisRun);
USEUNIT("stsUtilityFunctions.cpp");
USEUNIT("stsCalculationThread.cpp");
USEUNIT("stsPrintWindow.cpp");
USEUNIT("stsDataImporter.cpp");
USEFORM("stsDlgDataImporter.cpp", BDlgDataImporter);
USEUNIT("stsOutputFileRegistry.cpp");
USEFORM("stsFrmDownloadProgress.cpp", frmDownloadProgress);
USEFORM("stsFrmUpdateCheck.cpp", frmUpdateCheck);
USEUNIT("stsBaseAnalysisChildForm.cpp");
USEFORM("stsFrmStartWindow.cpp", frmStartWindow);
USEFORM("stsFrmAdvancedParameters.cpp", frmAdvancedParameters);
USEUNIT("..\calculation\Parameters.cpp");
USEUNIT("..\calculation\Toolkit.cpp");
USEUNIT("..\calculation\SaTScanBasis.cpp");
USEUNIT("..\calculation\utility\ScanfFile.cpp");
USEUNIT("..\calculation\utility\UtilityFunctions.cpp");
USEUNIT("..\calculation\utility\JulianDates.cpp");
USEUNIT("..\calculation\utility\RandomNumberGenerator.cpp");
USEUNIT("..\calculation\utility\Salloc.cpp");
USEUNIT("..\calculation\utility\RandomDistribution.cpp");
USEUNIT("..\calculation\utility\SSException.cpp");
USEUNIT("..\calculation\utility\DBFFile.cpp");
USEUNIT("..\calculation\utility\MultipleDimensionArrayHandler.cpp");
USEUNIT("..\calculation\print\BasePrint.cpp");
USEUNIT("..\calculation\print\PrintScreen.cpp");
USEUNIT("..\calculation\print\PrintQueue.cpp");
USEUNIT("..\calculation\probability_model\ExponentialModel.cpp");
USEUNIT("..\calculation\probability_model\RankModel.cpp");
USEUNIT("..\calculation\probability_model\NormalModel.cpp");
USEUNIT("..\calculation\probability_model\BernoulliModel.cpp");
USEUNIT("..\calculation\probability_model\CalculateMeasure.cpp");
USEUNIT("..\calculation\probability_model\ProbabilityModel.cpp");
USEUNIT("..\calculation\probability_model\PoissonModel.cpp");
USEUNIT("..\calculation\probability_model\SpaceTimePermutationModel.cpp");
USEUNIT("..\calculation\satscan_data\Tracts.cpp");
USEUNIT("..\calculation\satscan_data\SaTScanData.cpp");
USEUNIT("..\calculation\satscan_data\SaTScanDataDisplay.cpp");
USEUNIT("..\calculation\satscan_data\GridTractCoordinates.cpp");
USEUNIT("..\calculation\satscan_data\MakeNeighbors.cpp");
USEUNIT("..\calculation\satscan_data\PurelySpatialData.cpp");
USEUNIT("..\calculation\satscan_data\PurelyTemporalData.cpp");
USEUNIT("..\calculation\satscan_data\SaTScanDataRead.cpp");
USEUNIT("..\calculation\satscan_data\SpaceTimeData.cpp");
USEUNIT("..\calculation\satscan_data\SVTTData.cpp");
USEUNIT("..\calculation\satscan_data\MaxWindowLengthIndicator.cpp");
USEUNIT("..\calculation\satscan_data\AdjustmentHandler.cpp");
USEUNIT("..\calculation\satscan_data\DataSet.cpp");
USEUNIT("..\calculation\satscan_data\DataSetInterface.cpp");
USEUNIT("..\calculation\satscan_data\DataSetGateway.cpp");
USEUNIT("..\calculation\satscan_data\DataSetHandler.cpp");
USEUNIT("..\calculation\satscan_data\PopulationData.cpp");
USEUNIT("..\calculation\satscan_data\SpaceTimePermutationDataSetHandler.cpp");
USEUNIT("..\calculation\satscan_data\NormalDataSetHandler.cpp");
USEUNIT("..\calculation\satscan_data\PoissonDataSetHandler.cpp");
USEUNIT("..\calculation\satscan_data\RankDataSetHandler.cpp");
USEUNIT("..\calculation\satscan_data\BernoulliDataSetHandler.cpp");
USEUNIT("..\calculation\satscan_data\ExponentialDataSetHandler.cpp");
USEUNIT("..\calculation\cluster\Cluster.cpp");
USEUNIT("..\calculation\cluster\SpaceTimeCluster.cpp");
USEUNIT("..\calculation\cluster\IncidentRate.cpp");
USEUNIT("..\calculation\cluster\PurelySpatialCluster.cpp");
USEUNIT("..\calculation\cluster\PurelyTemporalCluster.cpp");
USEUNIT("..\calculation\cluster\PurelySpatialMonotoneCluster.cpp");
USEUNIT("..\calculation\cluster\SVTTCluster.cpp");
USEUNIT("..\calculation\cluster\PurelySpatialProspectiveCluster.cpp");
USEUNIT("..\calculation\cluster\TimeIntervalRange.cpp");
USEUNIT("..\calculation\cluster\TimeTrend.cpp");
USEUNIT("..\calculation\cluster\NormalClusterData.cpp");
USEUNIT("..\calculation\cluster\ClusterData.cpp");
USEUNIT("..\calculation\cluster\ClusterDataFactory.cpp");
USEUNIT("..\calculation\cluster\MultiSetClusterData.cpp");
USEUNIT("..\calculation\cluster\AbstractClusterData.cpp");
USEUNIT("..\calculation\analysis\MeasureList.cpp");
USEUNIT("..\calculation\analysis\Analysis.cpp");
USEUNIT("..\calculation\analysis\SignificantRatios05.cpp");
USEUNIT("..\calculation\analysis\SpaceTimeIncludePureAnalysis.cpp");
USEUNIT("..\calculation\analysis\SpaceTimeIncludePurelySpatialAnalysis.cpp");
USEUNIT("..\calculation\analysis\SpaceTimeIncludePurelyTemporalAnalysis.cpp");
USEUNIT("..\calculation\analysis\SpaceTimeAnalysis.cpp");
USEUNIT("..\calculation\analysis\PurelySpatialAnalysis.cpp");
USEUNIT("..\calculation\analysis\PurelySpatialMonotoneAnalysis.cpp");
USEUNIT("..\calculation\analysis\PurelyTemporalAnalysis.cpp");
USEUNIT("..\calculation\analysis\SVTTAnalysis.cpp");
USEUNIT("..\calculation\Output\stsRunHistoryFile.cpp");
USEUNIT("..\calculation\Output\stsDBaseFileWriter.cpp");
USEUNIT("..\calculation\Output\stsASCIIFileWriter.cpp");
USEUNIT("..\calculation\Output\AsciiPrintFormat.cpp");
USEUNIT("..\calculation\randomization\SpaceTimeRandomizer.cpp");
USEUNIT("..\calculation\randomization\DenominatorDataRandomizer.cpp");
USEUNIT("..\calculation\randomization\PermutationDataRandomizer.cpp");
USEUNIT("..\calculation\randomization\PoissonRandomizer.cpp");
USEUNIT("..\calculation\randomization\Randomizer.cpp");
USEUNIT("..\calculation\randomization\BernoulliRandomizer.cpp");
USEUNIT("..\calculation\loglikelihood\BernoulliLikelihoodCalculation.cpp");
USEUNIT("..\calculation\loglikelihood\LikelihoodCalculation.cpp");
USEUNIT("..\calculation\loglikelihood\PoissonLikelihoodCalculation.cpp");
USEUNIT("..\calculation\loglikelihood\WilcoxonLikelihoodCalculation.cpp");
USEUNIT("..\calculation\loglikelihood\NormalLikelihoodCalculation.cpp");
USEUNIT("..\calculation\analysis_run\AnalysisRun.cpp");
USEUNIT("..\calculation\analysis_run\MostLikelyClustersContainer.cpp");
USELIB("..\xbase\xbase_2.0.0\xbase\xbase.lib");
USEUNIT("..\..\boost\boost_1_33_1\libs\thread\src\xtime.cpp");
USEUNIT("..\..\boost\boost_1_33_1\libs\thread\src\exceptions.cpp");
USEUNIT("..\..\boost\boost_1_33_1\libs\thread\src\mutex.cpp");
USEUNIT("..\..\boost\boost_1_33_1\libs\thread\src\once.cpp");
USEUNIT("..\..\boost\boost_1_33_1\libs\thread\src\recursive_mutex.cpp");
USEUNIT("..\..\boost\boost_1_33_1\libs\thread\src\thread.cpp");
USEUNIT("..\..\boost\boost_1_33_1\libs\thread\src\condition.cpp");
USEUNIT("..\..\boost\boost_1_33_1\libs\thread\src\tss_hooks.cpp");
USEUNIT("..\calculation\utility\stsMonteCarloSimFunctor.cpp");
USEUNIT("..\calculation\utility\contractor.cpp");
USEUNIT("..\calculation\utility\DateStringParser.cpp");
USEUNIT("..\calculation\ParameterFileAccess.cpp");
USEUNIT("..\calculation\ScanLineParameterFileAccess.cpp");
USEUNIT("..\calculation\IniParameterFileAccess.cpp");
USEUNIT("..\calculation\IniParameterSpecification.cpp");
USEUNIT("..\calculation\probability_model\OrdinalModel.cpp");
USEUNIT("..\calculation\randomization\OrdinalDataRandomizer.cpp");
USEUNIT("..\calculation\satscan_data\OrdinalDataSetHandler.cpp");
USEUNIT("..\calculation\cluster\AbstractClusterDataFactory.cpp");
USEUNIT("..\calculation\cluster\CategoricalClusterData.cpp");
USEUNIT("..\calculation\cluster\CategoricalClusterDataFactory.cpp");
USEUNIT("..\calculation\cluster\MultiSetCategoricalClusterData.cpp");
USEUNIT("..\calculation\cluster\NormalClusterDataFactory.cpp");
USEUNIT("..\calculation\loglikelihood\LoglikelihoodRatioUnifier.cpp");
USEUNIT("..\calculation\loglikelihood\OrdinalLikelihoodCalculation.cpp");
USEUNIT("..\calculation\randomization\ExponentialRandomizer.cpp");
USEUNIT("..\calculation\Output\AbstractDataFileWriter.cpp");
USEUNIT("..\calculation\Output\LoglikelihoodRatioWriter.cpp");
USEUNIT("..\calculation\Output\ClusterInformationWriter.cpp");
USEUNIT("..\calculation\Output\ClusterLocationsWriter.cpp");
USEUNIT("..\calculation\Output\LocationRiskEstimateWriter.cpp");
USEUNIT("..\calculation\analysis\IntermediateClustersContainer.cpp");
USEUNIT("..\calculation\analysis\SpaceTimeIncludePurelyTemporalCentricAnalysis.cpp");
USEUNIT("..\calculation\analysis\AbstractCentricAnalysis.cpp");
USEUNIT("..\calculation\analysis\PurelySpatialCentricAnalysis.cpp");
USEUNIT("..\calculation\analysis\SpaceTimeCentricAnalysis.cpp");
USEUNIT("..\calculation\analysis\SpaceTimeIncludePureCentricAnalysis.cpp");
USEUNIT("..\calculation\analysis\SpaceTimeIncludePurelySpatialCentricAnalysis.cpp");
USEUNIT("..\calculation\analysis\AbstractAnalysis.cpp");
USEUNIT("..\calculation\utility\stsMCSimJobSource.cpp");
USEUNIT("..\calculation\RunTimeComponents.cpp");
USEFORM("stsDlgExecuteOptions.cpp", dlgExecutionOptions);
USEUNIT("..\calculation\utility\stsCentricAlgoJobSource.cpp");
USEUNIT("..\calculation\utility\stsCentricAlgoFunctor.cpp");
USEUNIT("..\calculation\ParametersPrint.cpp");
USEUNIT("..\calculation\ParametersValidate.cpp");
USEUNIT("..\calculation\satscan_data\CentroidNeighbors.cpp");
USEUNIT("..\calculation\randomization\NormalRandomizer.cpp");
USEUNIT("..\calculation\randomization\RankRandomizer.cpp");
//---------------------------------------------------------------------------
WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int) {
        bool            bRunUpdate=false;
        AnsiString      sUpdateApplication, sUpdateDataParameter, sError;
        AnsiString      sOmitULA_Parameter(AnsiString("-ver_id=") + VERSION_ID);
        HINSTANCE       hReturn;

        try {
           Application->Initialize();
           BasisInit();
           BasisSetToolkit(new SaTScanToolkit(_argv[0]));
           ZdGetFileTypeArray()->AddElement( &(DBFFileType::GetDefaultInstance()) );
           Application->Title = "SaTScan - Software for the Spatial and Space-Time Scan Statistics";
                 Application->HelpFile = "";
           sUpdateApplication.printf("%s%s", ExtractFilePath(Application->ExeName).c_str(), TfrmUpdateCheck::gsUpdaterFilename);
           if (!access(sUpdateApplication.c_str(), 00)) {
             _sleep(1); // give updater moment to shutdown
             remove(sUpdateApplication.c_str());
           }
           Application->CreateForm(__classid(TfrmMainForm), &frmMainForm);
           Application->Run();
           if ((bRunUpdate = GetToolkit().GetRunUpdateOnTerminate()) == true)
             sUpdateDataParameter.printf("\"%s%s\" \"%s\" %s", ExtractFilePath(Application->ExeName).c_str(),
                                         GetToolkit().GetUpdateArchiveFilename().GetCString(), Application->ExeName.c_str(),
                                         sOmitULA_Parameter.c_str());
           BasisExit();
        }
        catch (ZdException &x) {
           DisplayBasisException(Application, x);
           BasisExit();
        }
        catch (Exception &exception) {
           Application->ShowException(&exception);
           BasisExit();
        }
        catch(...) {
           BasisExit();
        }
        // run update application if flag set
        if (bRunUpdate) {
          hReturn = ShellExecute(NULL, "open", sUpdateApplication.c_str(), sUpdateDataParameter.c_str(), NULL, SW_SHOWNORMAL);
          if ((int)hReturn <= 32) { // check that updater launched
            sError.printf("Update application was unable to launch (Error Code %d).\n\n"
                          "Please send an email to the address referenced in the about box indicating this situation\n"
                          "and the error code. Note that the latest release can be obtained at http://www.satscan.org/\n"
                          "should this problem persist. SaTScan will now restart.", (int)hReturn);
            Application->MessageBox(sError.c_str(), "Update Failed!", MB_OK);
            // relaunch SaTScan
            ShellExecute(NULL, "open", Application->ExeName.c_str(), NULL, NULL, SW_SHOWNORMAL);
          }
        }
        return 0;
}
//---------------------------------------------------------------------------

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
USEFORM("stsFrmOutPutViewer.cpp", frmOutputViewer);
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
USEUNIT("..\calculation\probability_model\SurvivalModel.cpp");
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
USEUNIT("..\calculation\satscan_data\DataStream.cpp");
USEUNIT("..\calculation\satscan_data\DataStreamInterface.cpp");
USEUNIT("..\calculation\satscan_data\DataStreamGateway.cpp");
USEUNIT("..\calculation\satscan_data\DataStreamHandler.cpp");
USEUNIT("..\calculation\satscan_data\PopulationData.cpp");
USEUNIT("..\calculation\satscan_data\SpaceTimePermutationDataStreamHandler.cpp");
USEUNIT("..\calculation\satscan_data\NormalDataStreamHandler.cpp");
USEUNIT("..\calculation\satscan_data\PoissonDataStreamHandler.cpp");
USEUNIT("..\calculation\satscan_data\RankDataStreamHandler.cpp");
USEUNIT("..\calculation\satscan_data\BernoulliDataStreamHandler.cpp");
USEUNIT("..\calculation\satscan_data\SurvivalDataStreamHandler.cpp");
USEUNIT("..\calculation\cluster\Cluster.cpp");
USEUNIT("..\calculation\cluster\SpaceTimeCluster.cpp");
USEUNIT("..\calculation\cluster\TimeEstimate.cpp");
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
USEUNIT("..\calculation\cluster\MultipleStreamClusterData.cpp");
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
USEUNIT("..\calculation\Output\stsRelativeRisk.cpp");
USEUNIT("..\calculation\Output\stsClusterData.cpp");
USEUNIT("..\calculation\Output\stsLogLikelihood.cpp");
USEUNIT("..\calculation\Output\stsOutputFileData.cpp");
USEUNIT("..\calculation\Output\stsAreaSpecificData.cpp");
USEUNIT("..\calculation\Output\stsOutputFileWriter.cpp");
USEUNIT("..\calculation\Output\stsDBaseFileWriter.cpp");
USEUNIT("..\calculation\Output\stsASCIIFileWriter.cpp");
USEUNIT("..\calculation\randomization\SpaceTimeRandomizer.cpp");
USEUNIT("..\calculation\randomization\ContinuousVariableRandomizer.cpp");
USEUNIT("..\calculation\randomization\DenominatorDataRandomizer.cpp");
USEUNIT("..\calculation\randomization\PermutationDataRandomizer.cpp");
USEUNIT("..\calculation\randomization\PoissonRandomizer.cpp");
USEUNIT("..\calculation\randomization\Randomizer.cpp");
USEUNIT("..\calculation\randomization\BernoulliRandomizer.cpp");
USEUNIT("..\calculation\loglikelihood\BernoulliLikelihoodCalculation.cpp");
USEUNIT("..\calculation\loglikelihood\LikelihoodCalculation.cpp");
USEUNIT("..\calculation\loglikelihood\PoissonLikelihoodCalculation.cpp");
USEUNIT("..\calculation\loglikelihood\WillcoxomLikelihoodCalculation.cpp");
USEUNIT("..\calculation\loglikelihood\ExponentialLikelihoodCalculation.cpp");
USEUNIT("..\calculation\loglikelihood\NormalLikelihoodCalculation.cpp");
USELIB("..\xbase\xbase_2.0.0\xbase\xbase.lib");
USELIB("C:\Program Files\Borland\CBuilder5\Lib\zdannex540.lib");
USELIB("C:\Program Files\Borland\CBuilder5\Lib\zd540.lib");
//---------------------------------------------------------------------------
WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int) {
        bool            bRunUpdate=false;
        AnsiString      sUpdateParameter;
        AnsiString      sOmitULA_Parameter(AnsiString("-ver_id=") + VERSION_ID);
        int i;

        try {
           Application->Initialize();
           BasisInit();
           BasisSetToolkit(new SaTScanToolkit(_argv[0]));
           ZdGetFileTypeArray()->AddElement( &(DBFFileType::GetDefaultInstance()) );
           Application->Title = "SaTScan";
           Application->HelpFile = "";
           sUpdateParameter.printf("%s%s", ExtractFilePath(Application->ExeName).c_str(), TfrmUpdateCheck::gsUpdaterFilename);
           if (!access(sUpdateParameter.c_str(), 00)) {
             _sleep(1); // give updater moment to shutdown
             i = remove(sUpdateParameter.c_str());
           }
           Application->CreateForm(__classid(TfrmMainForm), &frmMainForm);
                 Application->Run();
           if ((bRunUpdate = GetToolkit().GetRunUpdateOnTerminate()) == true)
             sUpdateParameter.printf
             (
               "\"%s%s\" \"%s\" %s"
              ,ExtractFilePath(Application->ExeName).c_str()
              ,GetToolkit().GetUpdateArchiveFilename().GetCString()
              ,Application->ExeName.c_str()
              ,sOmitULA_Parameter.c_str()
             );
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
        if (bRunUpdate)
          HINSTANCE hReturn = ShellExecute(NULL, "open", TfrmUpdateCheck::gsUpdaterFilename, sUpdateParameter.c_str(), NULL, SW_SHOWNORMAL);
        return 0;
}
//---------------------------------------------------------------------------

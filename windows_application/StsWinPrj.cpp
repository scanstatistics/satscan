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
USEUNIT("..\calculation\probability_model\BernoulliModel.cpp");
USEUNIT("..\calculation\probability_model\CalculateMeasure.cpp");
USEUNIT("..\calculation\probability_model\ProbabilityModel.cpp");
USEUNIT("..\calculation\probability_model\PoissonModel.cpp");
USEUNIT("..\calculation\probability_model\SpaceTimePermutationModel.cpp");
USEUNIT("..\calculation\satscan_data\Tracts.cpp");
USEUNIT("..\calculation\satscan_data\PopulationCategories.cpp");
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
USEUNIT("..\calculation\cluster\Cluster.cpp");
USEUNIT("..\calculation\cluster\SpaceTimeCluster.cpp");
USEUNIT("..\calculation\cluster\TimeIntervalAlive.cpp");
USEUNIT("..\calculation\cluster\TimeIntervalAll.cpp");
USEUNIT("..\calculation\cluster\TimeEstimate.cpp");
USEUNIT("..\calculation\cluster\IncidentRate.cpp");
USEUNIT("..\calculation\cluster\PurelySpatialCluster.cpp");
USEUNIT("..\calculation\cluster\PurelyTemporalCluster.cpp");
USEUNIT("..\calculation\cluster\PurelySpatialMonotoneCluster.cpp");
USEUNIT("..\calculation\cluster\SVTTCluster.cpp");
USEUNIT("..\calculation\cluster\PurelySpatialProspectiveCluster.cpp");
USEUNIT("..\calculation\cluster\TimeIntervalRange.cpp");
USEUNIT("..\calculation\cluster\TimeTrend.cpp");
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
USELIB("..\xbase\xbase_2.0.0\xbase\xbase.lib");
USELIB("..\..\zero dimension\zdannex540.lib");
USELIB("..\..\zero dimension\zd540.lib");
USEUNIT("..\calculation\satscan_data\AdjustmentHandler.cpp");
//---------------------------------------------------------------------------
WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int) {
        bool            bRunUpdate=false;
        AnsiString      sUpdateParameter;
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
             sUpdateParameter.printf("\"%s%s\" \"%s\"", ExtractFilePath(Application->ExeName).c_str(),
                                     GetToolkit().GetUpdateArchiveFilename().GetCString(), Application->ExeName.c_str());
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

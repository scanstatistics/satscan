//---------------------------------------------------------------------------
//#include "xbDlgException.h"      // curious but for some reason this is needed here AJV 9/5/2002

#include "stsSaTScan.h"
#pragma hdrstop
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
USEUNIT("..\calculation\utility\UtilityFunctions.cpp");
USEUNIT("..\calculation\utility\JulianDates.cpp");
USEUNIT("..\calculation\utility\RandomNumberGenerator.cpp");
USEUNIT("..\calculation\utility\Salloc.cpp");
USEUNIT("..\calculation\utility\RandomDistribution.cpp");
USEUNIT("..\calculation\utility\Sort.cpp");
USEUNIT("..\calculation\utility\SSException.cpp");
USEUNIT("..\calculation\print\BasePrint.cpp");
USEUNIT("..\calculation\print\PrintScreen.cpp");
USEUNIT("..\calculation\probability_model\BernoulliModel.cpp");
USEUNIT("..\calculation\probability_model\CalculateMeasure.cpp");
USEUNIT("..\calculation\probability_model\ProbabilityModel.cpp");
USEUNIT("..\calculation\probability_model\PoissonModel.cpp");
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
USEUNIT("..\calculation\cluster\Cluster.cpp");
USEUNIT("..\calculation\cluster\SpaceTimeCluster.cpp");
USEUNIT("..\calculation\cluster\TimeIntervalAlive.cpp");
USEUNIT("..\calculation\cluster\TimeIntervalAll.cpp");
USEUNIT("..\calculation\cluster\TimeEstimate.cpp");
USEUNIT("..\calculation\cluster\IncidentRate.cpp");
USEUNIT("..\calculation\cluster\PurelySpatialCluster.cpp");
USEUNIT("..\calculation\cluster\PurelyTemporalCluster.cpp");
USEUNIT("..\calculation\cluster\PurelySpatialMonotoneCluster.cpp");
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
USEUNIT("..\calculation\Parameters.cpp");
USEUNIT("..\calculation\probability_model\SpaceTimePermutationModel.cpp");
USELIB("C:\Program Files\Borland\CBuilder5\Lib\zdannex540.lib");
USELIB("C:\Program Files\Borland\CBuilder5\Lib\zd540.lib");
USEUNIT("..\calculation\SaTScanBasis.cpp");
USEUNIT("..\calculation\Output\stsRunHistoryFile.cpp");
USEUNIT("..\calculation\utility\DBFFile.cpp");
USEUNIT("..\calculation\Output\stsAreaSpecificDBF.cpp");
USEUNIT("..\calculation\Output\stsClusterLevelDBF.cpp");
USELIB("..\xbase\xbase_2.0.0\xbase\xbase.lib");
USEUNIT("..\calculation\Output\stsOutputDBF.cpp");
//---------------------------------------------------------------------------
WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int) {
        try {
           BasisInit();
           ZdFileName sAppName(Application->ExeName.c_str());
           chdir( sAppName.GetLocation() );
           ZdGetFileTypeArray()->AddElement( &(DBFFileType::GetDefaultInstance()) );
           Application->Initialize();
           Application->Title = "SaTScan";
           Application->HelpFile = "Stswinprj.HLP";
           Application->CreateForm(__classid(TfrmMainForm), &frmMainForm);
           Application->Run();
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
        return 0;
}
//---------------------------------------------------------------------------

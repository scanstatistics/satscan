//---------------------------------------------------------------------------

#include <vcl.h>
#pragma hdrstop
USERES("StsWinPrj.res");
USEFORM("Stswin.cpp", frmMainForm);
USEFORM("TfrmAnalysis.cpp", frmAnalysis);
USEFORM("TfrmAbout.cpp", frmAbout);
USEUNIT("Utils.cpp");
USEUNIT("CalcThread.cpp");
USEUNIT("Calc\tinfo.cpp");
USEUNIT("Calc\BernoulliModel.cpp");
USEUNIT("Calc\calcmsr.cpp");
USEUNIT("Calc\cats.cpp");
USEUNIT("Calc\Cluster.cpp");
USEUNIT("Calc\Data.cpp");
USEUNIT("Calc\data_display.cpp");
USEUNIT("Calc\data_read.cpp");
USEUNIT("Calc\Analysis.cpp");
USEUNIT("Calc\RNG.cpp");
USEUNIT("Calc\Salloc.cpp");
USEUNIT("Calc\SigRatios05.cpp");
USEUNIT("Calc\Sort.cpp");
USEUNIT("Calc\ST+PS+PTAnalysis.cpp");
USEUNIT("Calc\ST+PSAnalysis.cpp");
USEUNIT("Calc\ST+PTAnalysis.cpp");
USEUNIT("Calc\STanalysis.cpp");
USEUNIT("Calc\STCluster.cpp");
USEUNIT("Calc\STdata.cpp");
USEUNIT("Calc\TIAlive.cpp");
USEUNIT("Calc\TIAll.cpp");
USEUNIT("Calc\TimeEstimate.cpp");
USEUNIT("Calc\rate.cpp");
USEUNIT("Calc\model.cpp");
USEUNIT("Calc\Ginfo.cpp");
USEUNIT("Calc\LatLong.cpp");
USEUNIT("Calc\Loglikelihood.cpp");
USEUNIT("Calc\MakeNeighbors.cpp");
USEUNIT("Calc\MeasureList.cpp");
USEUNIT("Calc\GetWord.cpp");
USEUNIT("Calc\PSCluster.cpp");
USEUNIT("Calc\PoissonModel.cpp");
USEUNIT("Calc\PSanalysis.cpp");
USEUNIT("Calc\PSdata.cpp");
USEUNIT("Calc\PSManalysis.cpp");
USEUNIT("Calc\PSMCluster.cpp");
USEUNIT("Calc\PTanalysis.cpp");
USEUNIT("Calc\PTCluster.cpp");
USEUNIT("Calc\PTdata.cpp");
USEUNIT("Calc\Randdist.cpp");
USEFORM("xmfrmAnalysisRun.cpp", frmAnalysisRun);
USEUNIT("Calc\date.cpp");
USEUNIT("Calc\Param.cpp");
USEUNIT("Calc\display.cpp");
USEUNIT("Calc\BasePrint.cpp");
USEUNIT("PrintWindow.cpp");
USEUNIT("Calc\PrintScreen.cpp");
USEUNIT("Calc\SSException.cpp");
USEFORM("xbDlgException.cpp", BdlgException);
USEFORM("xmfrmFileViewer.cpp", frmOutputViewer);
//---------------------------------------------------------------------------
WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int)
{
        try
        {
                 Application->Initialize();
                 Application->Title = "SaTScan";
                 Application->HelpFile = "Stswinprj.HLP";
                 Application->CreateForm(__classid(TfrmMainForm), &frmMainForm);
                 Application->Run();
        }
        catch (Exception &exception)
        {
                 Application->ShowException(&exception);
        }
        return 0;
}
//---------------------------------------------------------------------------

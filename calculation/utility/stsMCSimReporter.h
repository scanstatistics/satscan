//---------------------------------------------------------------------------

#ifndef stsMCSimReporterH
#define stsMCSimReporterH
//---------------------------------------------------------------------------
#include "AnalysisRun.h"
//---------------------------------------------------------------------------


class stsMCSimReporter
{
  typedef double result_type;

  AnalysisRunner const & gAnalysisRunner;
  MostLikelyClustersContainer & gMLCs;
  BasePrint & gPrintDirection;
  const char * gszReplicationFormatString;

public:
  stsMCSimReporter(AnalysisRunner const & theAnalysisRunner, MostLikelyClustersContainer & theMLCs, BasePrint & thePrintDirection, const char * szReplicationFormatString)
   : gAnalysisRunner(theAnalysisRunner)
   , gMLCs(theMLCs)
   , gPrintDirection(thePrintDirection)
   , gszReplicationFormatString(szReplicationFormatString)
  {}
  void Report_SubcontractLet(long id,long how_many_job_results_registered,long how_many_jobs_total)
  {}
  void Report_ResultsRegistered(long id,result_type const & result,long how_many_job_results_registered,long how_many_jobs_total)
  {
    try
    {
      gMLCs.UpdateTopClustersRank(result);
      bool bFinish = gAnalysisRunner.CheckForEarlyTermination(id);
      //if (bFinish) {} else {}//to be added--bws17mar2004
      //if appropriate, estimate time required to complete all jobs and report it.

      gPrintDirection.SatScanPrintf(gszReplicationFormatString, id, how_many_jobs_total, result);
    }
    catch (ZdException & e)
    {
      e.AddCallpath("", "stsMCSimReporter");
      throw;
    }
  }
};



#endif

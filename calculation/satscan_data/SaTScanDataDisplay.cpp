#pragma hdrstop
#include "SaTScanData.h"

void CSaTScanData::DisplayCases(FILE* pFile)
{
  fprintf(pFile, "Case counts (m_pCases)\n\n");

  for (int i = 0; i < m_nTimeIntervals; i++)
    for (int j = 0; j < m_nTracts; j++)
      fprintf(pFile, "Case [%i][%i] = %i\n", i,j,m_pCases[i][j]);

  fprintf(pFile, "\n");
}

void CSaTScanData::DisplayControls(FILE* pFile)
{
  fprintf(pFile, "Control counts (m_pControls)\n\n");

  for (int i = 0; i < m_nTimeIntervals; i++)
    for (int j = 0; j < m_nTracts; j++)
      fprintf(pFile, "Controls [%i][%i] = %i\n", i,j,m_pControls[i][j]);

  fprintf(pFile, "\n");
}

void CSaTScanData::DisplaySimCases(FILE* pFile)
{
  fprintf(pFile, "Simulated Case counts (m_pSimCases)\n\n");

  for (int i = 0; i < m_nTimeIntervals; i++)
    for (int j = 0; j < m_nTracts; j++)
      fprintf(pFile, "Cases [%i][%i] = %i\n", i,j,m_pSimCases[i][j]);

  fprintf(pFile, "\n");
}

void CSaTScanData::DisplayMeasure(FILE* pFile)
{
  fprintf(pFile, "Measures (m_pMeasure)\n\n");

  for (int i = 0; i < m_nTimeIntervals; i++)
    for (int j = 0; j < m_nTracts; j++)
      fprintf(pFile, "Measure [%i][%i] = %f\n", i,j,m_pMeasure[i][j]);

  fprintf(pFile, "\n");
}

void CSaTScanData::DisplayNeighbors(FILE* pFile)
{
  fprintf(pFile, "Neighbors (m_pSorted)   m_nGridTracts=%i\n\n", m_nGridTracts);

  for (int i = 0; i < m_nGridTracts; i++)
  {
    fprintf(pFile, "Grid Point # %i : ", i);
    if (m_pSortedInt)
       for (int j = 0; j < m_NeighborCounts[0][i]; j++)
         fprintf(pFile, "%s ", gpTInfo->tiGetTid(m_pSortedInt[0][i][j]));
    else
       for (int j = 0; j < m_NeighborCounts[0][i]; j++)
         fprintf(pFile, "%s ", gpTInfo->tiGetTid(m_pSortedUShort[0][i][j]));
    fprintf(pFile, "(# of neighbors=%i)\n", m_NeighborCounts[0][i]);
  }

  fprintf(pFile,"\n");
}

void CSaTScanData::DisplaySummary(FILE* fp)
{
  fprintf(fp, "________________________________________________________________\n\n");
  fprintf(fp, "SUMMARY OF DATA\n\n");
  fprintf(fp, "Study period .........: %s - %s\n",
              m_pParameters->m_szStartDate,
              m_pParameters->m_szEndDate);
  fprintf(fp, "Number of census areas: %ld\n", (long) m_nTracts);
  if (m_pParameters->m_nModel != SPACETIMEPERMUTATION)
    fprintf(fp, "Total population .....: %.0f\n", m_nTotalPop);
  fprintf(fp, "Total cases ..........: %ld\n",  m_nTotalCasesAtStart);
  if (m_pParameters->m_nModel == POISSON)
    fprintf(fp, "Annual cases / %.0f.: %.1f\n",
                GetAnnualRatePop(), GetAnnualRateAtStart());
  fprintf(fp, "________________________________________________________________\n");
}

void CSaTScanData::DisplaySummary2(FILE* fp)
{
  fprintf(fp, "SUMMARY OF DATA\n\n");
//  fprintf(fp, "m_nTractsInput.........: %i\n", m_nTractsInput);
  fprintf(fp, "m_nTracts..............: %i\n", m_nTracts);
  fprintf(fp, "m_nGridTracts..........: %i\n", m_nGridTracts);
  fprintf(fp, "m_nTimeIntervals.......: %i\n", m_nTimeIntervals);
  fprintf(fp, "m_nIntervalCut.........: %i\n", m_nIntervalCut);
  fprintf(fp, "m_nTotalCasesAtStart...: %ld\n", m_nTotalCasesAtStart);
  fprintf(fp, "m_nTotalCases..........: %ld\n", m_nTotalCases);
  fprintf(fp, "m_nTotalPop............: %.0f\n", m_nTotalPop);
  fprintf(fp, "m_nTotalMeasureAtStart.: %.0f\n", m_nTotalMeasureAtStart);
  fprintf(fp, "m_nTotalMeasure........: %.0f\n", m_nTotalMeasure);
  fprintf(fp, "m_nMaxCircleSize.......: %.2f\n", m_nMaxCircleSize);
  fprintf(fp, "________________________________________________________________\n");
}

void CSaTScanData::DisplayRelativeRisksForEachTract(FILE* pFile)
{
  char* szTID;
 // int   nTIDLen;

  //fprintf(pFile, "_____________________________________________________________________\n\n");
  //fprintf(pFile, "RELATIVE RISK FOR EACH INDIVIDUAL CENSUS AREA\n\n");
  //fprintf(pFile, "Census                        Observed    Expected    Relative\n");
  //fprintf(pFile, "Area                          Cases       Cases       Risk    \n\n");
  for (int i = 0; i < m_nTracts; i++)
  {
    szTID = gpTInfo->tiGetTid(i);
  //  nTIDLen = strlen(szTID);

    fprintf(pFile, "%-29s", szTID);

    fprintf(pFile, "%8i", m_pCases[0][i]);
    fprintf(pFile, "%12.2f   ", GetMeasureAdjustment()*m_pMeasure[0][i]);
    if (GetMeasureAdjustment()*m_pMeasure[0][i] != 0)
      fprintf(pFile, "%9.3f", ((double)(m_pCases[0][i]))/(GetMeasureAdjustment()*m_pMeasure[0][i]));
    else
      fprintf(pFile, "      n/a");
    fprintf(pFile, "\n");
  }
  fprintf(pFile, "\n");
}


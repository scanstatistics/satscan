#pragma hdrstop
#include "SaTScanData.h"

#define MAXWORDBUFFER 256
#define MAX_POP_DATES 200
#define POP_MONTH     7
#define POP_DAY       1

void DeleteCVEC(char **cvec, int nCats);

bool CSaTScanData::ReadCounts(const char* szCountFilename,
                              const char* szDescription,
                              count_t***  pCounts)
{
   int   i, j;
   int   nRec;
   bool  bValid;
   bool  bEmpty;
   FILE* fp;
   char  szData[MAX_LINESIZE];

   try
      {
      count_t nCount;
      Julian  nDate;
      tract_t tid;
    
     gpPrintDirection->SatScanPrintf("Reading the %s file\n", szDescription);
      
     if ((fp = fopen(szCountFilename, "r")) == NULL)
      {
        gpPrintDirection->SatScanPrintWarning("  Error: Could not open %s.\n", szCountFilename);
        return false;
      }

      // Allocate for counts
      *pCounts = (count_t**)Smalloc(m_nTimeIntervals * sizeof(count_t *), gpPrintDirection);
      for(i=0; i<m_nTimeIntervals; i++)
        (*pCounts)[i] = (count_t*)Smalloc(m_nTracts * sizeof(count_t), gpPrintDirection);
    
      // Initialize counts
      for(i=0; i<m_nTimeIntervals; i++)
        memset((*pCounts)[i], 0, m_nTracts);
    
      //Read and validate records in counts file
      nRec   = 0;
      bValid = true;
      bEmpty = true;
    
      while (fgets(szData, MAX_LINESIZE, fp))
      {
        nRec++;
        if (GetWord(szData, 0, gpPrintDirection) != 0)
        {
          bEmpty = false;
          if (!ParseCountLine(szDescription, nRec, szData, tid, nCount, nDate))
            bValid = false;
          else
            IncrementCount(tid, nCount, nDate, *pCounts);
        }
    
      }
    
      if (bEmpty)
      {
        gpPrintDirection->SatScanPrintWarning("  Error: File with %s data is empty.\n", szDescription);
        bValid = false;
      }
    
      fclose(fp);
      }
   catch (SSException & x)
      {
      x.AddCallpath("ReadCounts()", "CSaTScanData");
      throw;
      }
  return bValid;
};

bool CSaTScanData::ParseCountLine(const char*  szDescription, int nRec, char* szData,
                                  tract_t& tid, count_t& nCount, Julian& nDate)
{
   char         szTid[MAX_LINESIZE];
   unsigned int nYear, nMonth, nDay;
   int          nScanResult;
   int          nDataElements;
   int          nCats = gpCats->catGetNumEls();
   int    i;
   int    cat;
   bool   bCatsMissing;
   long   count;
   char** cvec;// = (char**)Smalloc(nCats * sizeof(char *), gpPrintDirection);

   try
      {
      cvec = (char**)Smalloc(nCats * sizeof(char *), gpPrintDirection);
      //set all pointers to zero, so you can clean up appropriately later
      memset(cvec, 0, nCats * sizeof(char *));
    
      nMonth = 1;
      nDay   = 1;
    
      //Parse tract id, count, & date from record
      nDataElements = 3;
      switch (m_pParameters->m_nPrecision)
      {
        case 0: nScanResult=sscanf(szData, "%s %ld", szTid, &nCount);
                JulianToMDY(&nMonth, &nDay, &nYear, m_nStartDate);
                nDataElements--;
                break;
        case 1: nScanResult=sscanf(szData, "%s %ld %d", szTid, &nCount, &nYear); break;
        case 2: nScanResult=sscanf(szData, "%s %ld %d/%d", szTid, &nCount, &nYear, &nMonth); break;
        case 3: nScanResult=sscanf(szData, "%s %ld %d/%d/%d", szTid, &nCount, &nYear, &nMonth, &nDay); break;
      }
    
      //Check: tract, count, & date exist
      if (nScanResult < m_pParameters->m_nPrecision+2)
      {
        free(cvec);
        gpPrintDirection->SatScanPrintWarning("  Error: Invalid record in %s file, line %d.\n", szDescription, nRec);
        gpPrintDirection->SatScanPrintWarning("         Please see '%s file format' in the help file.\n", szDescription);
        return false;
      }
    
      //Check: tract id valid
      tid = gpTInfo->tiGetTractNum(szTid);
    
      if (tid == -1)
      {
        free(cvec);
        gpPrintDirection->SatScanPrintWarning("  Error: Invalid tract ID in %s file, line %d.\n", szDescription, nRec);
        gpPrintDirection->SatScanPrintWarning("         This ID is not found in the coordinates file.\n");
        return false;
      }
    
      //Check: count >= 0
      if (nCount < 0)
      {
        free(cvec);
        gpPrintDirection->SatScanPrintWarning("  Error: Negative (or very large) count found in %s file, line %d.\n",szDescription, nRec);
        return false;
      }
    
      //Ensure four digit years
      int nYear4 = Ensure4DigitYear(nYear, m_pParameters->m_szStartDate, m_pParameters->m_szEndDate);
      switch (nYear4)
      {
          case -1:gpPrintDirection->SatScanPrintWarning("  Error: Due to study period greater than 100 years, unable\n");
                  gpPrintDirection->SatScanPrintWarning("         to convert two digit year in %s file, line %d.\n", szDescription, nRec);
                  gpPrintDirection->SatScanPrintWarning("         Please use four digit years.\n");
                  free(cvec);
                  return false;
          case -2:fprintf(stderr,"  Error: Invalid year in %s file, line %d.\n", szDescription, nRec);
                  gpPrintDirection->SatScanPrintWarning("  Error: Invalid year in %s file, line %d.\n", szDescription, nRec);
                  free(cvec);
                  return false;
      }
    
      //Check: startdate <= date <= enddate
      nDate = MDYToJulian(nMonth, nDay, nYear4);
    
      if (!(m_nStartDate <= nDate && nDate <= m_nEndDate))
      {
        free(cvec);
        gpPrintDirection->SatScanPrintWarning("  Error: Out of range time in %s file, line %d.\n",szDescription, nRec);
        gpPrintDirection->SatScanPrintWarning("         Times must correspond to study period specified on the Analysis Tab.\n");
        return false;
      }
    
      //Parse line for categories
      if ((m_pParameters->m_nModel==POISSON) ||
          (m_pParameters->m_nModel==BERNOULLI && strcmp(szDescription,"control")==0))
      {
        i            = 0;
        bCatsMissing = 0;
    
        while (i < nCats && !bCatsMissing)
        {
          char* p = GetWord(szData, i + nDataElements, gpPrintDirection);
          if (p == 0)
            bCatsMissing = 1;
          else
            Sstrcpy(&cvec[i],p, gpPrintDirection);
          i++;
        }
    
        //Check: category missing?
        if (bCatsMissing)
        {
          DeleteCVEC(cvec, nCats);
          gpPrintDirection->SatScanPrintWarning("  Error: Missing category variable in %s file, line %d.\n",szDescription,nRec);
          gpPrintDirection->SatScanPrintWarning("         Category combinations must correspond to those specified\n");
          gpPrintDirection->SatScanPrintWarning("         in the population file.\n");
          return false;
        }
    
        //Check: extra categories?
        if (GetWord(szData, nCats + nDataElements, gpPrintDirection))
        {
          DeleteCVEC(cvec, nCats);
          gpPrintDirection->SatScanPrintWarning("  Error: Extra data in %s file, line %d.\n", szDescription, nRec);
          gpPrintDirection->SatScanPrintWarning("         Please see '%s file format' in the help file.\n", szDescription);
          return false;
        }
    
        if (m_pParameters->m_nModel == POISSON)
        {
          //Check: categories correct?
          cat = gpCats->catGetCat(cvec);
          if (cat == -1)
          {
            DeleteCVEC(cvec, nCats);
            gpPrintDirection->SatScanPrintWarning("  Error: Invalid category in %s file, line %d.\n",szDescription, nRec);
            gpPrintDirection->SatScanPrintWarning("         Category combinations must correspond to those specified\n");
            gpPrintDirection->SatScanPrintWarning("         in the population file.\n");
            return false;
          }
    
          count = gpTInfo->tiGetCount(tid, cat);
          if (!gpTInfo->tiSetCount(tid, cat, count + nCount))
          {
            // KR (980916) : Aren't all these errors trapped above?
            DeleteCVEC(cvec, nCats);
            gpPrintDirection->SatScanPrintWarning("  Error: Record in %s file with no matching population record, line %d.\n", szDescription, nRec);
            return false;
          }
        }
      }
    
      DeleteCVEC(cvec, nCats);
      //for (i = 0; i < nCats; i++)
      //  free(cvec[i]);
      //free(cvec);
      }
   catch (SSException & x)
      {
      DeleteCVEC(cvec, nCats);
      x.AddCallpath("ParseCountLine()", "CSaTScanData");
      throw;
      }
  return true;
}

void DeleteCVEC(char **cvec, int nCats)
{
   int i;
   for (i = 0; i < nCats; i++)
    free(cvec[i]);
  free(cvec);
}

void CSaTScanData::IncrementCount(tract_t nTID, int nCount, Julian nDate,
                                  count_t** pCounts)
{
  pCounts[0][nTID] += nCount;
  for (int i=1; nDate >= m_pIntervalStartTimes[i]; i++)
        pCounts[i][nTID] += nCount;
}

/**********************************************************************
 Read the population file
 The number of category variables is determined by the first record.
 Any records deviating from this number will cause an error.
 Also, if the program is unable to read the tract-id, year, or population
 for a record, the program will issue an error.  The tract-id MUST match
 one read in ReadGeo().
 Return value:
   1 = success
   0 = errors encountered
 **********************************************************************/
bool CSaTScanData::ReadPops()
{
   bool    bValid;                            // File is valid, no errors found?
   bool    bEmpty;                            // File is empty?
   char    szData[MAX_LINESIZE];              // Line input from file
   char    szTid[MAX_LINESIZE];               // Tract ID
   tract_t nRec;                              // File record number
   tract_t nNonBlankLines;
   tract_t tract;                             // tract number
   int     cat;                               // category number
   int     year;                              // pop year
   long    pop;
   //  double  pop;                               // population
   int     ncats;                             // number of category vars
   char**  cvec = 0;                              // category-variable vector
   bool    bDateFound;
   int     nYear4;
   int     nDates = 0;
   Julian  nPopDate;
   Julian  tempPopDateArray[MAX_POP_DATES];
   int     i;
   FILE*   fp;                                // Ptr to population file

   try
      {
      gpPrintDirection->SatScanPrintf("Reading the population file\n");
     
      if ((fp = fopen(m_pParameters->m_szPopFilename, "r")) == NULL)
      {
        gpPrintDirection->SatScanPrintWarning("  Error: Could not open population file.\n", "ReadPops()");
        return false;
      }
    
      nRec   = 0;
      bValid = true;
      bEmpty = true;
    
      // 1st Pass
      while (fgets(szData, MAX_LINESIZE, fp))
      {
        nRec++;
    
        if (GetWord(szData, 0, gpPrintDirection) == 0)                 // Skip blank lines
          continue;
        else
          bEmpty=false;

        // Check for a tract id, year, and population count
        if (sscanf(szData, "%s %d %ld", szTid, &year, &pop) < 3)
        {
          gpPrintDirection->SatScanPrintWarning("  Error: Invalid record in population file, line %d.\n", nRec);
          gpPrintDirection->SatScanPrintWarning("         Please see 'population file format' in the help file.\n");
          bValid = false;
          continue;
        }
    
        if (pop < 0)
        {
          gpPrintDirection->SatScanPrintWarning("  Error: Negative (or very large) value in population file, line %d.\n", nRec);
          bValid = false;
          continue;
        }
    
        if (year < 0)
        {
          gpPrintDirection->SatScanPrintWarning("  Error: Invalid year in population file, line %d.\n", nRec);
          bValid = false;
          continue;
        }

        //Ensure four digit years
        nYear4 = Ensure4DigitYear(year, m_pParameters->m_szStartDate, m_pParameters->m_szEndDate);
        switch (nYear4)
        {
          case -1: gpPrintDirection->SatScanPrintWarning("  Error: Due to study period greater than 100 years, unable\n");
                   gpPrintDirection->SatScanPrintWarning("         to convert two digit year in population file, line %d.\n", nRec);
                   gpPrintDirection->SatScanPrintWarning("         Please use four digit years.\n");
                   bValid = false; continue;
          case -2: fprintf(stderr,"  Error: Invalid year in population file, line %d.\n", nRec);
                   gpPrintDirection->SatScanPrintWarning("  Error: Invalid year in population file, line %d.\n", nRec);
                   bValid = false; continue;
        }
    
        // Add Year to list of dates
        i = 0;
        bDateFound = false;
        nPopDate = MDYToJulian(POP_MONTH, POP_DAY, nYear4);
    
        // Use search algorithm !!!!!!
        while (i < nDates && !bDateFound)
        {
          bDateFound = (nPopDate == tempPopDateArray[i]);
          i++;
        }
    
        if (!bDateFound)
        {
          if (i>=MAX_POP_DATES)
            {
            bValid = false;
            gpPrintDirection->SatScanPrintWarning("  Error: Maximum number of population years (%i) has been exceeded.\n", MAX_POP_DATES);
            }
          else
          {
            tempPopDateArray[i] = nPopDate;
            nDates++;
          }
        }
    
      } // while - 1st Pass
    
      if (bEmpty)
      {
        gpPrintDirection->SatScanPrintWarning("  Error: File with population data is empty.\n", "ReadPops()");
        bValid = false;
      }
    
      // 2nd Pass
      if (bValid)
      {
    
        // Initialize population years list
        gpTInfo->tiSetupPopDates(tempPopDateArray, nDates, m_nStartDate, m_nEndDate);
    
        fseek(fp, 0L, SEEK_SET);
        nRec = 0;
        nNonBlankLines = 0;
    
        while (fgets(szData, MAX_LINESIZE, fp))
        {
    
          nRec++;
    
          // Skip Blank Lines
          if (GetWord(szData, 0, gpPrintDirection) == 0)
            continue;
          else
            nNonBlankLines++;
    
          // Get tract id, year, and population count
          sscanf(szData, "%s %d %ld", szTid, &year, &pop);

          //Ensure four digit years
          nYear4 = Ensure4DigitYear(year, m_pParameters->m_szStartDate, m_pParameters->m_szEndDate);
          switch (nYear4)
          {
            case -1: gpPrintDirection->SatScanPrintWarning("  Error: Due to study period greater than 100 years, unable\n");
                     gpPrintDirection->SatScanPrintWarning("         to convert two digit year in population file, line %d.\n", nRec);
                     gpPrintDirection->SatScanPrintWarning("         Please use four digit years.\n");
                     bValid = false; continue;
            case -2: fprintf(stderr,"  Error: Invalid year in population file, line %d.\n", nRec);
                     gpPrintDirection->SatScanPrintWarning("  Error: Invalid year in population file, line %d.\n", nRec);
                     bValid = false; continue;
          }
    
          // Determine number of categories from first record
          if (nNonBlankLines == 1)
          {
            ncats = 0;
            while (GetWord(szData, ncats + 3, gpPrintDirection))
              ncats++;
            cvec = (char**)Smalloc(ncats * sizeof(char *), gpPrintDirection);
            memset(cvec, 0, ncats * sizeof(char *));
            gpCats->catSetNumEls(ncats);
          }
    
          // Read categories into cvec
          for (i = 0; i < ncats; i++)
          {
            char *p = GetWord(szData, i + 3, gpPrintDirection);
            if (p == 0)
            {
              gpPrintDirection->SatScanPrintWarning("  Error: Too few categories in population file, line %d.\n",nRec);
              bValid = false;
              break;
            }
            Sstrcpy(&cvec[i],p, gpPrintDirection);
          }
    
          if (bValid == 0)
            continue;
    
          // Check for extraneous characters after the expected number of cats
          if (GetWord(szData, ncats + 3, gpPrintDirection))
          {
            gpPrintDirection->SatScanPrintWarning("  Error: Extra data in population file, line %d.\n", nRec);
            gpPrintDirection->SatScanPrintWarning("         Please see 'population file format' in the help file.\n");
            bValid = false;
            continue;
          }
    
          // Assign / Get category number
          cat = gpCats->catMakeCat(cvec);
          for (i = 0; i < ncats; i++)
            free(cvec[i]);
    
          // Check to see if tract is valid
          tract = gpTInfo->tiGetTractNum(szTid);
          if (tract == -1)
          {
            gpPrintDirection->SatScanPrintWarning("  Error: Invalid tract ID in population file, line line %d.\n", nRec);
            gpPrintDirection->SatScanPrintWarning("         This ID is not found in the coordinates file.\n");
            bValid = false;
            continue;
          }
    
          // Add Category to the tract
          // Add population count for this tract/category/year
          gpTInfo->tiAddCat(tract, cat, MDYToJulian(POP_MONTH, POP_DAY, nYear4), pop);
    
        } // while - 2nd pass
      } // if
    
    
      if (bValid && ncats > 0)
       free(cvec);
    
      fclose(fp);
      }
   catch (SSException & x)
      {
      if (cvec)
         DeleteCVEC(cvec, ncats);
      x.AddCallpath("ReadPops()", "CSaTScanData");
      throw;
      }
  return bValid;
}

/**********************************************************************
  Read the geographic data file.  This function now just calls either
  ReadGeoLatLong() or ReadGeoCoords depending on the coordinate data
**********************************************************************/
bool CSaTScanData::ReadGeo()
{
   bool bValid;

   try
      {
      if (m_pParameters->m_nCoordType == CARTESIAN)
         bValid = ReadGeoCoords();
      else
         bValid = ReadGeoLatLong();
      }
   catch (SSException & x)
      {
      x.AddCallpath("ReadGeo()", "CSaTScanData");
      throw;
      }
  return(bValid);
}

/**********************************************************************
 Read the latitude/longitude geographic data file.  Allocate & fill the
 "Tinfo" array. If invalid data is found in the file, an error message
 is printed, that record is ignored, and reading continues.
 Return value:
   1 = success
   0 = errors encountered
 **********************************************************************/
bool CSaTScanData::ReadGeoLatLong()
{
   bool    bValid;                            // File is valid, no errors found?
   bool    bEmpty;                            // File is empty?
   char    szData[MAX_LINESIZE];              // Line input from file
   char    szTid[MAX_LINESIZE];               // Tract ID
   int     nScanCount;                        // Num of items on input line
   tract_t nRec;                              // File record number
   double   Latitude, Longitude;               // Lat/Long coords
   double		pCoords[3];													 // 3-dimensional coords
   FILE*   fp;                                // Ptr to coordinates file

   try
      {
      gpPrintDirection->SatScanPrintf("Reading the geographic coordinates file (lat/lon).\n");

      if ((fp = fopen(m_pParameters->m_szCoordFilename, "r")) == NULL)
      {
        gpPrintDirection->SatScanPrintWarning("  Error: Could not open coordinates file (lat/long).\n");
        return false;
      }
    
      nRec   = 0;
      bValid = true;
      bEmpty = true;
    
      m_pParameters->m_nDimension = 3;
      gpTInfo->tiSetDimensions(m_pParameters->m_nDimension);
      gpGInfo->giSetDimensions(m_pParameters->m_nDimension);
    
      while (fgets(szData, MAX_LINESIZE, fp))
      {
        nRec++;
        if (GetWord(szData, 0, gpPrintDirection) == 0)
          continue;
        else
          bEmpty=false;

        nScanCount = sscanf(szData, "%s %lf %lf", szTid, &Latitude, &Longitude);
        if (nScanCount - 2 /* m_pParameters->m_nDimension */ < 1)
        {
          gpPrintDirection->SatScanPrintWarning("  Error: Invalid record in coordinates file (lat/lon), line %ld.\n", (long) nRec);
          gpPrintDirection->SatScanPrintWarning("         Please see 'coordinates file format' in the help file.\n");
          bValid = false;
          continue;
        }
    
        // Check for extra coordinates
        if (GetWord(szData, 3, gpPrintDirection))
        {
          gpPrintDirection->SatScanPrintWarning("  Error: Extra data in coordinate file (lat/lon), line %ld.\n", (long) nRec);
          gpPrintDirection->SatScanPrintWarning("         Please see 'coordinates file format' in the help file.\n");
          bValid = false;
        }
    
        //Check for correct ranges of latitude and longitude values
        if ((fabs(Latitude) > 90.0))
        {
          gpPrintDirection->SatScanPrintWarning("  Error: Latitude range error in coordinates file (2nd column), line %ld.\n", (long) nRec);
          gpPrintDirection->SatScanPrintWarning("         Latitude must be between -90 and 90.\n");
          bValid = false;
          continue;
        }

        if ((fabs(Longitude) > 180.0))
        {
          gpPrintDirection->SatScanPrintWarning("  Error: Longitude range error in coordinates file (3rd column), line %ld.\n", (long) nRec);
          gpPrintDirection->SatScanPrintWarning("         Longitude must be between -180 and 180.\n");
          bValid = false;
          continue;
        }

        // Convert to 3 dimensions then can add tracts
        ConvertFromLatLong(Latitude, Longitude, pCoords);
    
        // Add the tract
        if (!gpTInfo->tiInsertTnode(szTid, pCoords))
        {
          gpPrintDirection->SatScanPrintWarning("  Error: Duplicate tract ID in coordinates file (lat/lon), line %d.\n",nRec);
          bValid = false;
          continue;
        }
    
        // Store tracts as grid if no special grid file specified.
        if (!m_pParameters->m_bSpecialGridFile)
        {
          if (!gpGInfo->giInsertGnode(szTid, pCoords))
          {
            gpPrintDirection->SatScanPrintWarning("  Error: Duplicate tract ID in coordinates file (lat/lon), line %d.\n",nRec);
            bValid = false;
          }
        }
    
      } // while fgets()
    
      if (bEmpty)
      {
        gpPrintDirection->SatScanPrintWarning("  Error: Coordinates file is empty (lat/lon).\n", "ReadGeoLatLong()");
        bValid = false;
      }
      else if (gpTInfo->tiGetNumTracts()==1)
      {
        gpPrintDirection->SatScanPrintWarning("  Error: Coordinates file has only one record (lat/lon).\n", "ReadGeoLatLong()");
        bValid = false;
      }
    
      m_nTracts = gpTInfo->tiGetNumTracts();
      if (!m_pParameters->m_bSpecialGridFile)
        m_nGridTracts = gpGInfo->giGetNumTracts();
    
      fclose(fp); 
      }
   catch (SSException & x)
      {
      x.AddCallpath("ReadGeoLatLong()", "CSaTScanData");
      throw;
      }
   return bValid;
}

/**********************************************************************
 Read the geographic data file.  Allocate & fill the "Tinfo" array.
 If invalid data is found in the file, an error message is printed,
 that record is ignored, and reading continues.
 Return value:
   1 = success
   0 = errors encountered
 **********************************************************************/
bool CSaTScanData::ReadGeoCoords()
{
   bool    bValid;                            // File is valid, no errors found?
   bool    bEmpty;                            // File is empty?
   char    szData[MAX_LINESIZE];              // Line input from file
   char    szFirstLine[MAX_LINESIZE];         // First Line input from file
   char    szTid[MAX_LINESIZE];               // Tract ID
   int     nScanCount;                        // Num of items on input line
   tract_t nRec;                              // File record number
   double*  pCoords = NULL;                    // Ptr to Tract coords
   FILE*   fp;                             // Ptr to coordinates file
   int     i;

   try
      {
      gpPrintDirection->SatScanPrintf("Reading the geographic coordinates file\n");

      if ((fp = fopen(m_pParameters->m_szCoordFilename, "r")) == NULL)
      {
        gpPrintDirection->SatScanPrintWarning("  Error: Could not open coordinates file.\n", "ReadGeoCoords()");
        return false;
      }
    
      //nRec   = 0;
      bValid = true;
      bEmpty = true;
    
      // This section added 5/27/98 G. Gherman to take into account an arbitrary
      // number of dimensions WITHOUT specifying them.
      // Handle first non-blank line to determine number of dimensions.  This
      // record must have correct data.
      // Modified 6/23/98
      while (bEmpty && fgets(szFirstLine, MAX_LINESIZE, fp))
      {
        if (GetWord(szFirstLine, 0, gpPrintDirection) == 0)
        	continue;
        else
        {
        	bEmpty     = false;
     	 	nRec       = 1;
     	 	nScanCount = 0;

          // Initial pass to determine # of dimensions
          while (GetWord(szFirstLine, nScanCount, gpPrintDirection) != NULL)
            nScanCount++;
    
          if (nScanCount < 3) // Indicates no coords or only 1
          {
            gpPrintDirection->SatScanPrintWarning("  Error: Invalid data (< 3 items) in first record of coordinates file.\n");
            gpPrintDirection->SatScanPrintWarning("         Please see 'coordinates file format' in the help file.\n");
            return false;
          }
    
          #if (DEBUG)
         gpPrintDirection->SatScanPrintf("Number of dimensions in coordinates file = %d.\n",nScanCount-1);
          #endif
    
          m_pParameters->m_nDimension = nScanCount - 1;
          gpTInfo->tiSetDimensions(m_pParameters->m_nDimension);
          gpGInfo->giSetDimensions(m_pParameters->m_nDimension);
          pCoords = (double*)Smalloc(m_pParameters->m_nDimension * sizeof(double), gpPrintDirection);
    
          sscanf(GetWord(szFirstLine, 0, gpPrintDirection), "%s", szTid);
    
          // Re-use nScanCount to determine if data is valid
          nScanCount = 1;

          for (i=0; i<m_pParameters->m_nDimension; i++)
    		{
            nScanCount += sscanf(GetWord(szFirstLine,i+1, gpPrintDirection),"%lf",&pCoords[i]);
    		}
    
       	if (nScanCount - m_pParameters->m_nDimension < 1)
           {
           gpPrintDirection->SatScanPrintWarning("  Error: Invalid data in first line of coordinates file.\n");
           gpPrintDirection->SatScanPrintWarning("         Please see 'coordinates file format' in the help file.\n");
           return false;
          }
    
          // Add the tract
          gpTInfo->tiInsertTnode(szTid, pCoords);
    
          // Store tracts as grid if no special grid file specified.
          if (!m_pParameters->m_bSpecialGridFile)
            gpGInfo->giInsertGnode(szTid, pCoords);
    
          #if (DEBUG)
      	 	printf("First line of data from coordinates file:\n");
         gpPrintDirection->SatScanPrintf("Tract id = %s\n",szTid);
          for (i=0; i< (m_pParameters->m_nDimension); i++)
          {
           gpPrintDirection->SatScanPrintf("Coords[%d] = %f\n", i, pCoords[i]);
          }
          #endif
    
        } // else
      } // while(bEmpty && fgets(szFirstLine...))
    
      // Get rest of data
      while (fgets(szData, MAX_LINESIZE, fp) && !bEmpty)
      {
        nRec++;
        if (GetWord(szData, 0, gpPrintDirection) == 0)
          continue;
        else
          bEmpty=false;
    
        sscanf(GetWord(szData, 0, gpPrintDirection), "%s", szTid);
        nScanCount = 1;
    
        for (i=0; i<m_pParameters->m_nDimension; i++)
        {
      		nScanCount += sscanf(GetWord(szData,i+1, gpPrintDirection),"%lf",&pCoords[i]);
         	//nScanCount++;
        }
    
        #if(DEBUG)
       gpPrintDirection->SatScanPrintf("%d line of data from coordinates file:\n", nRec);
       gpPrintDirection->SatScanPrintf("Tract id = %s\n",szTid);
        for (i=0; i< (m_pParameters->m_nDimension); i++)
        {
         gpPrintDirection->SatScanPrintf("Coords[%d] = %f\n", i, pCoords[i]);
        }
        #endif

        if (nScanCount - m_pParameters->m_nDimension < 1)
        {
           gpPrintDirection->SatScanPrintWarning("  Error: Invalid record in coordinates file, line %ld.\n", (long) nRec);
           gpPrintDirection->SatScanPrintWarning("         Please see 'coordinates file format' in the help file.\n");
           bValid = false;
           continue;
        }
    
        // Add the tract
        if (!gpTInfo->tiInsertTnode(szTid, pCoords))
        {
          gpPrintDirection->SatScanPrintWarning("  Error: Duplicate tract ID in coordinates file, line %d.\n",nRec);
          bValid = false;
          continue;
        }
    
        // Store tracts as grid if no special grid file specified.
        if (!m_pParameters->m_bSpecialGridFile)
        {
           if (!gpGInfo->giInsertGnode(szTid, pCoords))
             {
             gpPrintDirection->SatScanPrintWarning("  Error: Duplicate tract ID in coordinates file, line %d.\n",nRec);
             bValid = false;
             }
        }
    
      } // while fgets(szData...)
    
      if (bEmpty)
      {
        gpPrintDirection->SatScanPrintWarning("  Error: Coordinates file is empty.\n", "ReadGeoCoords()");
        bValid = false;
      }
      else if (gpTInfo->tiGetNumTracts()==1 && m_pParameters->m_nAnalysisType != PURELYTEMPORAL)
      // This modified 6/25/98 so that one tract allowed for Purely Temporal analysis
      {
        gpPrintDirection->SatScanPrintWarning("  Error: Coordinates file has only one record.\n", "ReadGeoCoords()");
        bValid = false;
      }
    
      m_nTracts = gpTInfo->tiGetNumTracts();
      if (!m_pParameters->m_bSpecialGridFile)
        m_nGridTracts = gpGInfo->giGetNumTracts();
    
      fclose(fp);
    
      // Clean up pCoords
      if (pCoords != NULL)
        free(pCoords);
      }
   catch (SSException & x)
      {
      free(pCoords);
      x.AddCallpath("ReadGeoCoords()", "CSaTScanData");
      throw;
      }
   return bValid;
}

/**********************************************************************
  Read the special grid file.  This function now just calls either
  ReadGridLatLong() or ReadGridCoords depending on the coordinate data
**********************************************************************/
bool CSaTScanData::ReadGrid()
{
   bool bValid;

   try
      {
      if (m_pParameters->m_nCoordType == CARTESIAN)
         bValid = ReadGridCoords();
      else
         bValid = ReadGridLatLong();
      }
   catch (SSException & x)
      {
      x.AddCallpath("ReadGrid()", "CSaTScanData");
      throw;
      }
   return(bValid);
}

/**********************************************************************
 Read the special grid data file.  Allocate & fill the "Ginfo" array.
 If invalid data is found in the file, an error message is printed,
 that record is ignored, and reading continues.
 Return value:
   1 = success
   0 = errors encountered
 **********************************************************************/
bool CSaTScanData::ReadGridCoords()
{
   bool    bValid;                            // File is valid, no errors found?
   bool    bEmpty;                            // File is empty?
  // bool    bPrintDuplicateWarning = false;    // Did you print a warning for duplciate coordinates ?
   char    szData[MAX_LINESIZE];              // Line input from file
   char    szTid[MAX_LINESIZE];               // Grid tract ID
   int     nScanCount;                        // Num of items on input line
   tract_t nRec;                              // File record number
   double*   pCoords = 0;                           // Ptr to Grid tract coords
   FILE*   fp;                                // Ptr to grid file
   int i;
   
   try
      {
      gpPrintDirection->SatScanPrintf("Reading the grid file\n");

      if ((fp = fopen(m_pParameters->m_szGridFilename, "r")) == NULL)
      {
        gpPrintDirection->SatScanPrintWarning("  Error: Could not open special grid file.\n", "ReadGridCoords()");
        return false;
      }

      nRec   = 0;
      bValid = true;
      bEmpty = true;

      pCoords = (double*)Smalloc(m_pParameters->m_nDimension * sizeof(double), gpPrintDirection);

      while (fgets(szData, MAX_LINESIZE, fp))
      {
        nRec++;

        if (GetWord(szData, 0, gpPrintDirection) == 0)                     // ignore blank lines
          continue;
        else
          bEmpty=false;

        // Can't read too far so check for correct number of coordinates:
        if (!GetWord(szData, (m_pParameters->m_nDimension-1), gpPrintDirection) ||
             GetWord(szData, m_pParameters->m_nDimension, gpPrintDirection))
        {
          gpPrintDirection->SatScanPrintWarning("  Error: Invalid record in grid file, line %d.\n",nRec);
          gpPrintDirection->SatScanPrintWarning("         Please see 'grid file format' in the help file.\n");
          bValid = false;
          continue;
        }
        // What if too many coordinates in grid file???

        nScanCount = 0;
        for (i=0; i<m_pParameters->m_nDimension; i++)
        {
          // Modified 6/25/98 to make sure of valid (non-alpha) data
      		nScanCount += sscanf(GetWord(szData,i, gpPrintDirection),"%lf",&pCoords[i]);
         	//nScanCount++;
        }

        #if(DEBUG)
     	gpPrintDirection->SatScanPrintf("%d line of data from grid file:\n", nRec);
     	gpPrintDirection->SatScanPrintf("Tract id = %s\n",szTid);
        for (i=0; i< (m_pParameters->m_nDimension); i++)
        {
         gpPrintDirection->SatScanPrintf("Coords[%d] = %f\n", i, pCoords[i]);
        }
        #endif

        itoa(nRec, szTid, 10);

        if (nScanCount < m_pParameters->m_nDimension)
        {
          gpPrintDirection->SatScanPrintWarning("  Error: Invalid record in grid file, line %d.\n",nRec);
          gpPrintDirection->SatScanPrintWarning("         Please see 'grid file format' in the help file.\n");
          bValid = false;
          continue;
        }

        //*******************************************************************
        // v2.2 Ignor duplicate grid file coordinates
        //
        // just print a message in the warnings box and only print it once.
        // DO NOT END JOB....
        //*******************************************************************
        if (!gpGInfo->giInsertGnode(szTid, pCoords))
        {
          // if (! bPrintDuplicateWarning)
          //    gpPrintDirection->SatScanPrintWarning("Note: The grid file has multiple identical coordinates.\nThis does not effect the results, but the program will run faster if duplicates are removed.");
          // bPrintDuplicateWarning = true;
          char sMessage[100];
          sprintf(sMessage, "  Error: Duplicate tract ID in grid file, line %d.\n",nRec);
          SSGenerateWarning(sMessage, "ReadGridCoords()");
          bValid = false;
        }
      } // while fgets()

      if (bEmpty)
      {
        gpPrintDirection->SatScanPrintWarning("  Error: Grid file is empty.\n", "ReadGridCoords()");
        bValid = false;
      }

      m_nGridTracts = gpGInfo->giGetNumTracts();

      fclose(fp);
      free(pCoords);
      }
   catch (SSException & x)
      {
      free(pCoords);
      x.AddCallpath("ReadGridCoords()", "CSaTScanData");
      throw;
      }
   return bValid;
}


bool CSaTScanData::ReadGridLatLong()
{
   bool    bValid;                            // File is valid, no errors found?
   bool    bEmpty;                            // File is empty?
  // bool    bPrintDuplicateWarning = false;    // Did you print a warning for duplciate coordinates ?
   char    szData[MAX_LINESIZE];              // Line input from file
   char    szTid[MAX_LINESIZE];               // Grid tract ID
   int     nScanCount;                        // Num of items on input line
   tract_t nRec;                              // File record number
   float   Latitude, Longitude;
   double*	 pCoords = 0;                           // Ptr to Grid tract coords
   FILE*   fp;                                // Ptr to grid file

   try
      {
      gpPrintDirection->SatScanPrintf("Reading the grid file (lat/lon).\n");

      if ((fp = fopen(m_pParameters->m_szGridFilename, "r")) == NULL)
      {
        gpPrintDirection->SatScanPrintWarning("  Error: Could not open special grid file (lat/lon).\n", "ReadGridLatLong()");
        return false;
      }
    
      nRec   = 0;
      bValid = true;
      bEmpty = true;
    
      pCoords = (double*)Smalloc(3 * sizeof(double), gpPrintDirection);
    
      while (fgets(szData, MAX_LINESIZE, fp))
      {
        nRec++;
    
        if (GetWord(szData, 0, gpPrintDirection) == 0)                     // ignore blank lines
          continue;
        else
          bEmpty=false;

        nScanCount = sscanf(szData, "%lf %lf",&Latitude, &Longitude);
    
        itoa(nRec, szTid, 10);
    
        if (nScanCount < 2)
        {
          gpPrintDirection->SatScanPrintWarning("  Error: Invalid record in grid file (lat/lon), line %d.\n",nRec);
          gpPrintDirection->SatScanPrintWarning("         Please see 'grid file format' in the help file.\n");
          bValid = false;
          continue;
        }
    
        // Check for extra coordinates
        if (GetWord(szData, 2, gpPrintDirection))
        {
           gpPrintDirection->SatScanPrintWarning("  Error: Extra data in grid file (lat/lon), line %ld.\n", (long) nRec);
           gpPrintDirection->SatScanPrintWarning("         Please see 'grid file format' in the help file.\n");
           bValid = false;
        }

        //Check for correct ranges of latitude and longitude values
        if ((fabs(Latitude) > 90.0))
        {
           gpPrintDirection->SatScanPrintWarning("  Error: Latitude range error in grid file (1st column), line %ld.\n", (long) nRec);
           gpPrintDirection->SatScanPrintWarning("         Latitude must be between -90 and 90.\n");
           bValid = false;
           continue;
        }

        if ((fabs(Longitude) > 180.0))
        {
           gpPrintDirection->SatScanPrintWarning("  Error: Longitude range error in grid file (2nd column), line %ld.\n", (long) nRec);
           gpPrintDirection->SatScanPrintWarning("         Longitude must be between -180 and 180.\n");
           bValid = false;
           continue;
        }
    
        // Convert to 3 dimensions then can add tracts
        ConvertFromLatLong(Latitude, Longitude, pCoords);

        //*******************************************************************
        // v2.2 Ignor duplicate grid file coordinates
        //
        // just print a message in the warnings box and only print it once.
        // DO NOT END JOB....
        //*******************************************************************
        if (!gpGInfo->giInsertGnode(szTid, pCoords))
        {
          // if (! bPrintDuplicateWarning)
          //    gpPrintDirection->SatScanPrintWarning("Note: The grid file has multiple identical coordinates.\nThis does not effect the results, but the program will run faster if duplicates are removed.");
           //bPrintDuplicateWarning = true;
           char sMessage[100];
          sprintf(sMessage,"  Error: Duplicate tract ID in grid file (lat/lon), line %d.\n",nRec);
          SSGenerateWarning(sMessage, "ReadGridLatLong()");
          bValid = false;
        }
      } // while fgets()
    
      if (bEmpty)
      {
        gpPrintDirection->SatScanPrintWarning("  Error: Grid file is empty (lat/lon).\n", "ReadGridLatLong()");
        bValid = false;
      }

      m_nGridTracts = gpGInfo->giGetNumTracts();
    
      fclose(fp);
      free(pCoords);
      }
   catch (SSException & x)
      {
      free(pCoords);
      x.AddCallpath("ReadGridLatLong()", "CSaTScanData");
      throw;
      }
   return bValid;
}



#include "SaTScan.h"
#pragma hdrstop
#include "SaTScanData.h"

#define MAXWORDBUFFER 256
const int POPULATION_DATE_PRECISION_MONTH_DEFAULT_DAY   = 15;
const int POPULATION_DATE_PRECISION_YEAR_DEFAULT_DAY    = 1;
const int POPULATION_DATE_PRECISION_YEAR_DEFAULT_MONTH  = 7;

void DeleteCVEC(char **cvec, int nCats);

bool CSaTScanData::ReadCounts(const char* szCountFilename, const char* szDescription, count_t***  pCounts) {
   int   nRec = 0;
   bool  bValid = true, bEmpty = true;
   FILE* fp;
   char  szData[MAX_LINESIZE];
   count_t nCount;
   Julian  nDate;
   tract_t tid;

   try {
     gpPrintDirection->SatScanPrintf("Reading the %s file\n", szDescription);

     if ((fp = fopen(szCountFilename, "r")) == NULL) {
        gpPrintDirection->SatScanPrintWarning("  Error: Could not open %s.\n", szCountFilename);
        return false;
     }

      // Allocate for counts
      *pCounts = (count_t**)Smalloc(m_nTimeIntervals * sizeof(count_t *), gpPrintDirection);
      for(int i = 0; i < m_nTimeIntervals; ++i) {
        (*pCounts)[i] = (count_t*)Smalloc(m_nTracts * sizeof(count_t), gpPrintDirection);
        // Initialize counts
         for(int j = 0; j < m_nTracts; ++j)
            (*pCounts)[i][j]=0;
      }

      while (fgets(szData, MAX_LINESIZE, fp)) {
        ++nRec;
        if (GetWord(szData, 0, gpPrintDirection) != 0) {
          bEmpty = false;

          if (!ParseCountLine(szDescription, nRec, szData, tid, nCount, nDate))
            bValid = false;
          else
            IncrementCount(tid, nCount, nDate, *pCounts);
        }
      }

      if (bEmpty) {
        gpPrintDirection->SatScanPrintWarning("  Error: File with %s data is empty.\n", szDescription);
        bValid = false;
      }
    
      fclose(fp);
   }
   catch (SSException & x) {
      x.AddCallpath("ReadCounts()", "CSaTScanData");
      throw;
   }
  return bValid;
};

bool CSaTScanData::ParseCountLine(const char*  szDescription, int nRec, char* szData, tract_t& tid, count_t& nCount, Julian& nDate) {
   unsigned int 	nYear, nMonth = 1, nDay = 1;
   float        	fTempCount; 
   int    		i, cat, nScanResult, nDataElements=3, nCats=gpCats->catGetNumEls();
   bool   		bCatsMissing=false;
   long   		count;
   char	             ** cvec, szTid[MAX_LINESIZE];
   BasePrint::eInputFileType       eFileType;

   try {
      cvec = (char**)Smalloc(nCats * sizeof(char *), gpPrintDirection);
      //set all pointers to zero, so you can clean up appropriately later
      memset(cvec, 0, nCats * sizeof(char *));

      //Parse tract id, count, & date from record
      switch (m_pParameters->m_nPrecision) {
        case 0: nScanResult=sscanf(szData, "%s %f", szTid, &fTempCount);
                JulianToMDY(&nMonth, &nDay, &nYear, m_nStartDate);
                nDataElements--;
                break;
        case 1: nScanResult=sscanf(szData, "%s %f %d", szTid, &fTempCount, &nYear); break;
        case 2: nScanResult=sscanf(szData, "%s %f %d/%d", szTid, &fTempCount, &nYear, &nMonth); break;
        case 3: nScanResult=sscanf(szData, "%s %f %d/%d/%d", szTid, &fTempCount, &nYear, &nMonth, &nDay); break;
      }
      // Value read from sscanf for nCount might be a float, in which case, preventing sscanf
      // from reading remaining variables correctly. 
      nCount = static_cast<count_t>(fTempCount);

      eFileType = (!strcmp(szDescription, "case") ? BasePrint::CASEFILE : BasePrint::CONTROLFILE);

      //Check: tract, count, & date exist
      if (nScanResult < m_pParameters->m_nPrecision+2) {
        free(cvec);
        gpPrintDirection->SatScanPrintInputFileWarning(eFileType, "  Error: Invalid record in %s file, line %d.\n", szDescription, nRec);
        gpPrintDirection->SatScanPrintInputFileWarning(eFileType, "         Please see '%s file format' in the help file.\n", szDescription);
        return false;
      }
    
      //Check: tract id valid
      tid = gpTInfo->tiGetTractIndex(szTid);
    
      if (tid == -1) {
        free(cvec);
        gpPrintDirection->SatScanPrintInputFileWarning(eFileType, "  Error: Invalid tract ID in %s file, line %d.\n", szDescription, nRec);
        gpPrintDirection->SatScanPrintInputFileWarning(eFileType, "         This ID is not found in the coordinates file.\n");
        return false;
      }
    
      //Check: count >= 0
      if (nCount < 0) {
        free(cvec);
        gpPrintDirection->SatScanPrintInputFileWarning(eFileType, "  Error: Negative (or very large) count found in %s file, line %d.\n",szDescription, nRec);
        return false;
      }
    
      //Ensure four digit years
      int nYear4 = Ensure4DigitYear(nYear, m_pParameters->m_szStartDate, m_pParameters->m_szEndDate);
      switch (nYear4) {
          case -1:gpPrintDirection->SatScanPrintInputFileWarning(eFileType, "  Error: Due to study period greater than 100 years, unable\n"
                                                                            "         to convert two digit year in %s file, line %d.\n"
                                                                            "         Please use four digit years.\n", szDescription, nRec);
                  free(cvec);
                  return false;
          case -2:fprintf(stderr,"  Error: Invalid year in %s file, line %d.\n", szDescription, nRec);
                  gpPrintDirection->SatScanPrintInputFileWarning(eFileType, "  Error: Invalid year in %s file, line %d.\n", szDescription, nRec);
                  free(cvec);
                  return false;
      }
    
      //Check: startdate <= date <= enddate
      nDate = MDYToJulian(nMonth, nDay, nYear4);
    
      if (!(m_nStartDate <= nDate && nDate <= m_nEndDate)) {
        free(cvec);
        gpPrintDirection->SatScanPrintInputFileWarning(eFileType, "  Error: Out of range time in %s file, line %d.\n"
                                                                  "         Times must correspond to study period specified on the Analysis Tab.\n" ,szDescription, nRec);
        return false;
      }

      //Parse line for categories
      if ((m_pParameters->m_nModel==POISSON) ||
          (m_pParameters->m_nModel==SPACETIMEPERMUTATION && m_pParameters->m_nMaxSpatialClusterSizeType == PERCENTAGEOFMEASURETYPE) ||
          (m_pParameters->m_nModel==BERNOULLI && strcmp(szDescription,"control")==0)) {
        i            = 0;

        while (i < nCats && !bCatsMissing) {
          char* p = GetWord(szData, i + nDataElements, gpPrintDirection);
          if (p == 0)
            bCatsMissing = true;
          else
            Sstrcpy(&cvec[i],p, gpPrintDirection);
          ++i;
        }
    
        //Check: category missing?
        if (bCatsMissing) {
          DeleteCVEC(cvec, nCats);
          gpPrintDirection->SatScanPrintInputFileWarning(eFileType, "  Error: Missing category variable in %s file, line %d.\n"
                                                                    "         Category combinations must correspond to those specified\n"
                                                                    "         in the population file.\n",szDescription,nRec);
          return false;
        }
    
        //Check: extra categories?
        if (GetWord(szData, nCats + nDataElements, gpPrintDirection)) {
          DeleteCVEC(cvec, nCats);
          gpPrintDirection->SatScanPrintInputFileWarning(eFileType, "  Error: Extra data in %s file, line %d.\n"
                                                                    "         Please see '%s file format' in the help file.\n", szDescription, nRec, szDescription);
          return false;
        }

        if (m_pParameters->m_nModel == POISSON ||
            (m_pParameters->m_nModel==SPACETIMEPERMUTATION &&
             m_pParameters->m_nMaxSpatialClusterSizeType == PERCENTAGEOFMEASURETYPE)) {
          //Check: categories correct?
          cat = gpCats->catGetCat(cvec);
          if (cat == -1) {
            DeleteCVEC(cvec, nCats);
            gpPrintDirection->SatScanPrintInputFileWarning(eFileType, "  Error: Invalid category in %s file, line %d.\n"
                                                                      "         Category combinations must correspond to those specified\n"
                                                                      "         in the population file.\n" ,szDescription, nRec);
            return false;
          }

          if (!gpTInfo->tiAddCount(tid, cat, nCount)) {
            // KR (980916) : Aren't all these errors trapped above?
            DeleteCVEC(cvec, nCats);
            gpPrintDirection->SatScanPrintInputFileWarning(eFileType, "  Error: Record in %s file with no matching population record, line %d.\n", szDescription, nRec);
            return false;
          }
        }
      }
      DeleteCVEC(cvec, nCats);

  }
   catch (SSException & x) {
      DeleteCVEC(cvec, nCats);
      x.AddCallpath("ParseCountLine()", "CSaTScanData");
      throw;
  }
  return true;
}

void DeleteCVEC(char **cvec, int nCats) {
   for (int i = 0; i < nCats; ++i)
    free(cvec[i]);
  free(cvec);
}

void CSaTScanData::IncrementCount(tract_t nTID, int nCount, Julian nDate, count_t** pCounts) {
  pCounts[0][nTID] += nCount;
  for (int i=1; nDate >= m_pIntervalStartTimes[i]; ++i)
        pCounts[i][nTID] += nCount;
}

/** Converts passed string specifiying a population date to a julian date.
    Precision is determined by date formats( YYYY/MM/DD, YYYY/MM, YYYY, YY/MM/DD,
    YY/MM, YY ) which is the complete set of valid formats that SaTScan currently
    supports. Since we accumulate errors/warnings when reading input files,
    indication of a bad date is returned and any messages sent to print direction. */
bool CSaTScanData::ConvertPopulationDateToJulian(const char * sDateString, int iRecordNumber, Julian & JulianDate) {
  bool          bValidDate=true;
  int           iYear, iMonth, iDay, iPrecision=0;
  const char  * ptr;

  try {
    //determine precision
    ptr = strchr(sDateString, '/');
    while (ptr) {
         iPrecision++;
         ptr = strchr(++ptr, '/');
    }
    //scan string
    switch (iPrecision) {
      case 0  : iMonth = POPULATION_DATE_PRECISION_YEAR_DEFAULT_MONTH;
                iDay = POPULATION_DATE_PRECISION_YEAR_DEFAULT_DAY;
                bValidDate = (sscanf(sDateString, "%d", &iYear) == 1 && iYear > 0);
                break;
      case 1  : iDay = POPULATION_DATE_PRECISION_MONTH_DEFAULT_DAY;
                bValidDate = (sscanf(sDateString, "%d/%d", &iYear, &iMonth) == 2 && iYear > 0 && iMonth > 0);
                break;
      case 2  : bValidDate = (sscanf(sDateString, "%d/%d/%d", &iYear, &iMonth, &iDay) == 3 && iYear > 0 && iMonth > 0 && iDay > 0);
                break;
      default : bValidDate = false;
    }

    if (! bValidDate)
      gpPrintDirection->SatScanPrintInputFileWarning(BasePrint::POPFILE, "  Error: Invalid date \"%s\" in population file, line %d.\n", sDateString, iRecordNumber);
    else {
      iYear = Ensure4DigitYear(iYear, m_pParameters->m_szStartDate, m_pParameters->m_szEndDate);
      switch (iYear) {
        case -1 : gpPrintDirection->SatScanPrintInputFileWarning(BasePrint::POPFILE, "  Error: Due to the study period being greater than 100 years, unable\n");
                  gpPrintDirection->SatScanPrintInputFileWarning(BasePrint::POPFILE, "         to determine century for two digit year \"%d\" in population file, line %d.\n",
                                                                  iYear, iRecordNumber);
                  gpPrintDirection->SatScanPrintInputFileWarning(BasePrint::POPFILE, "         Please use four digit years.\n");
                  bValidDate = false;
        case -2 : gpPrintDirection->SatScanPrintInputFileWarning(BasePrint::POPFILE, "  Error: Invalid year \"%d\" in population file, line %d.\n", iYear, iRecordNumber);
                  bValidDate = false;
        default : JulianDate = MDYToJulian(iMonth, iDay, iYear);
      }
    }
  }
  catch (ZdException & x) {
    x.AddCallpath("ConvertPopulationDateToJulian()","CSaTScanData");
    throw;
  }
  return bValidDate;
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
bool CSaTScanData::ReadPops() {
//  std::vector<int>                 vFirstTractDateIndex;
  FILE                          * fp; // Ptr to population file
  std::vector<Julian>             vPopulationDates;
  std::vector<Julian>::iterator   itrdates;
  char                          * ptr, ** cvec = 0,
                                  sDateString[MAX_LINESIZE], szData[MAX_LINESIZE], szTid[MAX_LINESIZE];
  int                             i, iCategoryIndex, iNumCategories;
  bool                            bValid=true, bEmpty=true, InvalidForProspective=false;
  tract_t                         nRec=0, nNonBlankLines, tract;
  float                           fPopulation;
  Julian                          PopulationDate;

  try {
    gpPrintDirection->SatScanPrintf("Reading the population file\n");
    if ((fp = fopen(m_pParameters->m_szPopFilename, "r")) == NULL) {
      gpPrintDirection->SatScanPrintInputFileWarning(BasePrint::POPFILE, "  Error: Could not open population file.\n", "ReadPops()");
      return false;
    }

    //1st Pass -- determine all the population dates.
    while (fgets(szData, MAX_LINESIZE, fp)) {
        ++nRec;
        if (GetWord(szData, 0, gpPrintDirection) == 0)                 // Skip blank lines
          continue;
        else
          bEmpty=false;

        //Check for a tract id, year, and population count
        if (sscanf(szData, "%s %s %f", szTid, sDateString, &fPopulation) < 3) {
          /** WE MIGHT BE ABLE TO PRODUCE A BETTER ERROR MESSAGE HERE*/
          gpPrintDirection->SatScanPrintInputFileWarning(BasePrint::POPFILE, "  Error: Invalid record in population file, line %d.\n", nRec);
          gpPrintDirection->SatScanPrintInputFileWarning(BasePrint::POPFILE, "         Please see 'population file format' in the help file.\n");
          bValid = false;
          continue;
        }

        if (fPopulation < 0) {
          /** WE MIGHT BE ABLE TO PRODUCE A BETTER ERROR MESSAGE HERE - LIKE THE INVALID POPULATION */
          gpPrintDirection->SatScanPrintInputFileWarning(BasePrint::POPFILE, "  Error: Negative (or very large) value in population file, line %d.\n", nRec);
          bValid = false;
          continue;
        }

        bValid = ConvertPopulationDateToJulian(sDateString, nRec, PopulationDate);
        if (! bValid)
          continue;
        itrdates = lower_bound(vPopulationDates.begin(), vPopulationDates.end(), PopulationDate);
        if (! (itrdates != vPopulationDates.end() && (*itrdates) == PopulationDate))
          vPopulationDates.insert(itrdates, PopulationDate);
      }//  while - 1st Pass

    if (bEmpty) {
      gpPrintDirection->SatScanPrintInputFileWarning(BasePrint::POPFILE, "  Error: Population data is empty.\n");
      if (nRec)
        gpPrintDirection->SatScanPrintInputFileWarning(BasePrint::POPFILE, "         Population file contains %d invalid records.\n");
      else
        gpPrintDirection->SatScanPrintInputFileWarning(BasePrint::POPFILE, "         Population file is empty.\n");
      bValid = false;
    }

    // 2nd Pass -- read data in structures
    if (bValid) {
      gpTInfo->tiSetupPopDates(vPopulationDates, m_nStartDate, m_nEndDate);
      //if (m_pParameters->m_nAnalysisType == PROSPECTIVESPACETIME)
      //  vFirstTractDateIndex.resize(gpTInfo->tiGetNumTracts(), -1);

      fseek(fp, 0L, SEEK_SET);
      nRec = 0;
      nNonBlankLines = 0;

      while (fgets(szData, MAX_LINESIZE, fp)) {
          ++nRec;

          // Skip Blank Lines
          if (GetWord(szData, 0, gpPrintDirection) == 0)
            continue;
          else
            ++nNonBlankLines;

          // Get tract id, year, and population count
          sscanf(szData, "%s %s %f", szTid, sDateString, &fPopulation);
          bValid = ConvertPopulationDateToJulian(sDateString, nRec, PopulationDate);
          if (! bValid)
            continue;

          // Determine number of categories from first record
          if (nNonBlankLines == 1) {
            iNumCategories = 0;
            while (GetWord(szData, iNumCategories + 3, gpPrintDirection))
              ++iNumCategories;
            cvec = (char**)Smalloc(iNumCategories * sizeof(char *), gpPrintDirection);
            memset(cvec, 0, iNumCategories * sizeof(char *));
            gpCats->catSetNumEls(iNumCategories);
          }

          // Read categories into cvec
          for (i=0; i < iNumCategories; ++i) {
            char *p = GetWord(szData, i + 3, gpPrintDirection);
            if (p == 0) {
              gpPrintDirection->SatScanPrintInputFileWarning(BasePrint::POPFILE, "  Error: Too few categories in population file, line %d.\n",nRec);
              bValid = false;
              break;
            }
            Sstrcpy(&cvec[i],p, gpPrintDirection);
          }

          if (bValid == 0)
            continue;

          // Check for extraneous characters after the expected number of cats
          if (GetWord(szData, iNumCategories + 3, gpPrintDirection)) {
            gpPrintDirection->SatScanPrintInputFileWarning(BasePrint::POPFILE, "  Error: Extra data in population file, line %d.\n", nRec);
            gpPrintDirection->SatScanPrintInputFileWarning(BasePrint::POPFILE, "         Please see 'population file format' in the help file.\n");
            bValid = false;
            continue;
          }

          // Assign / Get category number
          iCategoryIndex = gpCats->catMakeCat(cvec);
          for (i = 0; i < iNumCategories; ++i)
            free(cvec[i]);

          // Check to see if tract is valid
          tract = gpTInfo->tiGetTractIndex(szTid);
          if (tract == -1) {
            gpPrintDirection->SatScanPrintInputFileWarning(BasePrint::POPFILE, "  Error: Invalid tract ID in population file, line line %d.\n", nRec);
            gpPrintDirection->SatScanPrintInputFileWarning(BasePrint::POPFILE, "         This ID is not found in the coordinates file.\n");
            bValid = false;
            continue;
          }

          // Perform Check: When a prospective analysis is conducted and if a population file is
          //                used, and if the population for a tract is defined at more than one
          //                time period, error message should be shown in the running window and
          //                the application terminated.
          if (m_pParameters->m_nAnalysisType == PROSPECTIVESPACETIME && m_pParameters->m_nMaxSpatialClusterSizeType == PERCENTAGEOFMEASURETYPE) {
            //iDateIndex = gpTInfo->tiGetPopDateIndex(nPopDate);
            //if (vFirstTractDateIndex[tract] == -1 || vFirstTractDateIndex[tract] == iDateIndex)
            //  vFirstTractDateIndex[tract] = iDateIndex;
            //else
              InvalidForProspective = true;
          }

          // Add population count for this tract/category/year
          gpTInfo->tiAddCategoryToTract(tract, iCategoryIndex, PopulationDate, fPopulation);

        } // while - 2nd pass
      } // if

      if (InvalidForProspective)  {
        bValid = false;
        gpPrintDirection->SatScanPrintInputFileWarning(BasePrint::POPFILE, "\n  ERROR: For the prospective space-time analysis to be correct,\n"
                                                                           "           it is critical that the scanning spatial window is the\n"
                                                                           "           same for each of the analysis performed over time. If \n"
                                                                           "           there are multiple years in the population file, so that\n"
                                                                           "           the population size changes over time, as it does in your\n"
                                                                           "           data, then you must define the maximum circle size in\n"
                                                                           "           terms of a specific geographical radius rather than as a\n"
                                                                           "           percent of the total population at risk.\n\n\n");
      }
      if (bValid && iNumCategories > 0)
       free(cvec);

      fclose(fp);
  }
  catch (SSException & x) {
    if (cvec) DeleteCVEC(cvec, iNumCategories);
    x.AddCallpath("ReadPops()", "CSaTScanData");
    throw;
  }
  return bValid;
}

/**********************************************************************
  Read the geographic data file.  This function now just calls either
  ReadGeoLatLong() or ReadGeoCoords depending on the coordinate data
**********************************************************************/
bool CSaTScanData::ReadGeo() {
   bool bValid = true;

   try {
      bValid = ((m_pParameters->m_nCoordType == CARTESIAN) ? ReadGeoCoords() : ReadGeoLatLong());
   }
   catch (SSException & x) {
      x.AddCallpath("ReadGeo()", "CSaTScanData");
      throw;
   }
  return bValid;
}

/**********************************************************************
 Read the latitude/longitude geographic data file.  Allocate & fill the
 "Tinfo" array. If invalid data is found in the file, an error message
 is printed, that record is ignored, and reading continues.
 Return value:
   1 = success
   0 = errors encountered
 **********************************************************************/
bool CSaTScanData::ReadGeoLatLong() {
   bool    bValid = true,  bEmpty = true;
   char    szData[MAX_LINESIZE], szTid[MAX_LINESIZE];
   int     nScanCount;                        // Num of items on input line
   tract_t nRec = 0;                              // File record number
   double   Latitude, Longitude, pCoords[3];													 // 3-dimensional coords
   FILE*   fp;                                // Ptr to coordinates file

   try {
      gpPrintDirection->SatScanPrintf("Reading the geographic coordinates file (lat/lon).\n");

      if ((fp = fopen(m_pParameters->m_szCoordFilename, "r")) == NULL) {
        gpPrintDirection->SatScanPrintInputFileWarning(BasePrint::COORDFILE, "  Error: Could not open coordinates file (lat/long).\n");
        return false;
      }
    
      m_pParameters->m_nDimension = 3;
      gpTInfo->tiSetDimensions(m_pParameters->m_nDimension);
      gpGInfo->giSetDimensions(m_pParameters->m_nDimension);
    
      while (fgets(szData, MAX_LINESIZE, fp)) {
        ++nRec;
        if (GetWord(szData, 0, gpPrintDirection) == 0)
          continue;
        else
          bEmpty=false;

        nScanCount = sscanf(szData, "%s %lf %lf", szTid, &Latitude, &Longitude);
        if (nScanCount - 2 /* m_pParameters->m_nDimension */ < 1) {
          gpPrintDirection->SatScanPrintInputFileWarning(BasePrint::COORDFILE, "  Error: Invalid record in coordinates file (lat/lon), line %ld.\n", (long) nRec);
          gpPrintDirection->SatScanPrintInputFileWarning(BasePrint::COORDFILE, "         Please see 'coordinates file format' in the help file.\n");
          bValid = false;
          continue;
        }
    
        // Check for extra coordinates
        if (GetWord(szData, 3, gpPrintDirection)) {
          gpPrintDirection->SatScanPrintInputFileWarning(BasePrint::COORDFILE, "  Error: Extra data in coordinate file (lat/lon), line %ld.\n", (long) nRec);
          gpPrintDirection->SatScanPrintInputFileWarning(BasePrint::COORDFILE, "         Please see 'coordinates file format' in the help file.\n");
          bValid = false;
        }
    
        //Check for correct ranges of latitude and longitude values
        if ((fabs(Latitude) > 90.0)) {
          gpPrintDirection->SatScanPrintInputFileWarning(BasePrint::COORDFILE, "  Error: Latitude range error in coordinates file (2nd column), line %ld.\n", (long) nRec);
          gpPrintDirection->SatScanPrintInputFileWarning(BasePrint::COORDFILE, "         Latitude must be between -90 and 90.\n");
          bValid = false;
          continue;
        }

        if ((fabs(Longitude) > 180.0)) {
          gpPrintDirection->SatScanPrintInputFileWarning(BasePrint::COORDFILE, "  Error: Longitude range error in coordinates file (3rd column), line %ld.\n", (long) nRec);
          gpPrintDirection->SatScanPrintInputFileWarning(BasePrint::COORDFILE, "         Longitude must be between -180 and 180.\n");
          bValid = false;
          continue;
        }

        // Convert to 3 dimensions then can add tracts
        ConvertFromLatLong(Latitude, Longitude, pCoords);
    
        // Add the tract
        if (!gpTInfo->tiInsertTnode(szTid, pCoords)) {
          gpPrintDirection->SatScanPrintInputFileWarning(BasePrint::COORDFILE, "  Error: Duplicate tract ID in coordinates file (lat/lon), line %d.\n",nRec);
          bValid = false;
          continue;
        }
    
        // Store tracts as grid if no special grid file specified.
        if (!m_pParameters->m_bSpecialGridFile) {
          if (!gpGInfo->giInsertGnode(szTid, pCoords)) {
            gpPrintDirection->SatScanPrintInputFileWarning(BasePrint::COORDFILE, "  Error: Duplicate tract ID in coordinates file (lat/lon), line %d.\n",nRec);
            bValid = false;
          }
        }
    
      } // while fgets()
    
      if (bEmpty) {
        gpPrintDirection->SatScanPrintInputFileWarning(BasePrint::COORDFILE, "  Error: Coordinates file is empty (lat/lon).\n", "ReadGeoLatLong()");
        bValid = false;
      }
      else if (gpTInfo->tiGetNumTracts() == 1) {
        gpPrintDirection->SatScanPrintInputFileWarning(BasePrint::COORDFILE, "  Error: Coordinates file has only one record (lat/lon).\n", "ReadGeoLatLong()");
        bValid = false;
      }
    
      m_nTracts = gpTInfo->tiGetNumTracts();
      if (!m_pParameters->m_bSpecialGridFile)
        m_nGridTracts = gpGInfo->giGetNumTracts();
    
      fclose(fp); 
   }
   catch (SSException & x) {
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
   1, true = success
   0, false = errors encountered
 **********************************************************************/
bool CSaTScanData::ReadGeoCoords() {
   bool    bValid = true, bEmpty = true;
   char    szData[MAX_LINESIZE], szFirstLine[MAX_LINESIZE], szTid[MAX_LINESIZE];
   int     i, nScanCount = 0;                        // Num of items on input line
   tract_t nRec = 0;                              // File record number
   double*  pCoords = NULL;                    // Ptr to Tract coords
   FILE*   fp;                             // Ptr to coordinates file

   try {
      gpPrintDirection->SatScanPrintf("Reading the geographic coordinates file\n");

      if ((fp = fopen(m_pParameters->m_szCoordFilename, "r")) == NULL) {
        gpPrintDirection->SatScanPrintInputFileWarning(BasePrint::COORDFILE, "  Error: Could not open coordinates file.\n", "ReadGeoCoords()");
        return false;
      }
    
      // This section added 5/27/98 G. Gherman to take into account an arbitrary
      // number of dimensions WITHOUT specifying them.
      // Handle first non-blank line to determine number of dimensions.  This
      // record must have correct data.
      // Modified 6/23/98
      while (bEmpty && fgets(szFirstLine, MAX_LINESIZE, fp)) {
        if (GetWord(szFirstLine, 0, gpPrintDirection) == 0)
        	continue;
        else {
        	bEmpty     = false;
     	 	nRec       = 1;
     	 	nScanCount = 0;

          // Initial pass to determine # of dimensions
          while (GetWord(szFirstLine, nScanCount, gpPrintDirection) != NULL)
            ++nScanCount;

          // Indicates no coords or only 1
          if (nScanCount < 3) {
            gpPrintDirection->SatScanPrintInputFileWarning(BasePrint::COORDFILE, "  Error: Invalid data (< 3 items) in first record of coordinates file.\n");
            gpPrintDirection->SatScanPrintInputFileWarning(BasePrint::COORDFILE, "         Please see 'coordinates file format' in the help file.\n");
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

          for (i=0; i<m_pParameters->m_nDimension; ++i) {
            nScanCount += sscanf(GetWord(szFirstLine,i+1, gpPrintDirection),"%lf",&pCoords[i]);
    	  }

       	  if (nScanCount - m_pParameters->m_nDimension < 1) {
           gpPrintDirection->SatScanPrintInputFileWarning(BasePrint::COORDFILE, "  Error: Invalid data in first line of coordinates file.\n");
           gpPrintDirection->SatScanPrintInputFileWarning(BasePrint::COORDFILE, "         Please see 'coordinates file format' in the help file.\n");
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
          for (i=0; i< (m_pParameters->m_nDimension); ++i)
            gpPrintDirection->SatScanPrintf("Coords[%d] = %f\n", i, pCoords[i]);
#endif
    
        } // else
      } // while(bEmpty && fgets(szFirstLine...))

      // Get rest of data
      while (fgets(szData, MAX_LINESIZE, fp) && !bEmpty) {
        ++nRec;
        if (GetWord(szData, 0, gpPrintDirection) == 0)
          continue;
        else
          bEmpty=false;
    
        sscanf(GetWord(szData, 0, gpPrintDirection), "%s", szTid);
        nScanCount = 1;
    
        for (i=0; i<m_pParameters->m_nDimension; ++i)
      		nScanCount += sscanf(GetWord(szData,i+1, gpPrintDirection),"%lf",&pCoords[i]);

#if(DEBUG)
       gpPrintDirection->SatScanPrintf("%d line of data from coordinates file:\n", nRec);
       gpPrintDirection->SatScanPrintf("Tract id = %s\n",szTid);
        for (i=0; i< (m_pParameters->m_nDimension); ++i)
         gpPrintDirection->SatScanPrintf("Coords[%d] = %f\n", i, pCoords[i]);
#endif

        if (nScanCount - m_pParameters->m_nDimension < 1) {
           gpPrintDirection->SatScanPrintInputFileWarning(BasePrint::COORDFILE, "  Error: Invalid record in coordinates file, line %ld.\n", (long) nRec);
           gpPrintDirection->SatScanPrintInputFileWarning(BasePrint::COORDFILE, "         Please see 'coordinates file format' in the help file.\n");
           bValid = false;
           continue;
        }
    
        // Add the tract
        if (!gpTInfo->tiInsertTnode(szTid, pCoords)) {
          gpPrintDirection->SatScanPrintInputFileWarning(BasePrint::COORDFILE, "  Error: Duplicate tract ID in coordinates file, line %d.\n",nRec);
          bValid = false;
          continue;
        }

        // Store tracts as grid if no special grid file specified.
        if (!m_pParameters->m_bSpecialGridFile) {
           if (!gpGInfo->giInsertGnode(szTid, pCoords)) {
             gpPrintDirection->SatScanPrintInputFileWarning(BasePrint::COORDFILE, "  Error: Duplicate tract ID in coordinates file, line %d.\n",nRec);
             bValid = false;
           }
        }

      } // while fgets(szData...)

      if (bEmpty) {
        gpPrintDirection->SatScanPrintInputFileWarning(BasePrint::COORDFILE, "  Error: Coordinates file is empty.\n", "ReadGeoCoords()");
        bValid = false;
      }
      else if (gpTInfo->tiGetNumTracts()==1 && m_pParameters->m_nAnalysisType != PURELYTEMPORAL) {
      // This modified 6/25/98 so that one tract allowed for Purely Temporal analysis
        gpPrintDirection->SatScanPrintInputFileWarning(BasePrint::COORDFILE, "  Error: Coordinates file has only one record.\n", "ReadGeoCoords()");
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
   catch (SSException & x) {
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
bool CSaTScanData::ReadGrid() {
   bool bValid = true;

   try {
      bValid = ( (m_pParameters->m_nCoordType == CARTESIAN) ? ReadGridCoords() : ReadGridLatLong() );
   }
   catch (SSException & x)
      {
      x.AddCallpath("ReadGrid()", "CSaTScanData");
      throw;
      }
   return bValid;
}

/**********************************************************************
 Read the special grid data file.  Allocate & fill the "Ginfo" array.
 If invalid data is found in the file, an error message is printed,
 that record is ignored, and reading continues.
 Return value:
   1 = success
   0 = errors encountered
 **********************************************************************/
bool CSaTScanData::ReadGridCoords() {
   bool    bValid = true, bEmpty = true;
   char    szData[MAX_LINESIZE], szTid[MAX_LINESIZE];
   int     i, nScanCount = 0;                        // Num of items on input line
   tract_t nRec = 0;                              // File record number
   double*   pCoords = 0;                           // Ptr to Grid tract coords
   FILE*   fp;                                // Ptr to grid file
   
   try {
      gpPrintDirection->SatScanPrintf("Reading the grid file\n");

      if ((fp = fopen(m_pParameters->m_szGridFilename, "r")) == NULL) {
        gpPrintDirection->SatScanPrintInputFileWarning(BasePrint::GRIDFILE, "  Error: Could not open special grid file.\n", "ReadGridCoords()");
        return false;
      }

      pCoords = (double*)Smalloc(m_pParameters->m_nDimension * sizeof(double), gpPrintDirection);

      while (fgets(szData, MAX_LINESIZE, fp)) {
        ++nRec;

        if (GetWord(szData, 0, gpPrintDirection) == 0)                     // ignore blank lines
          continue;
        else
          bEmpty = false;

        // Can't read too far so check for correct number of coordinates:
        if (!GetWord(szData, (m_pParameters->m_nDimension-1), gpPrintDirection) || GetWord(szData, m_pParameters->m_nDimension, gpPrintDirection)) {
          gpPrintDirection->SatScanPrintInputFileWarning(BasePrint::GRIDFILE, "  Error: Invalid record in grid file, line %d.\n",nRec);
          gpPrintDirection->SatScanPrintInputFileWarning(BasePrint::GRIDFILE, "         Please see 'grid file format' in the help file.\n");
          bValid = false;
          continue;
        }
        // What if too many coordinates in grid file???

        // Modified 6/25/98 to make sure of valid (non-alpha) data
        for (i = 0; i < m_pParameters->m_nDimension; ++i)
      	   nScanCount += sscanf(GetWord(szData,i, gpPrintDirection),"%lf",&pCoords[i]);

#if(DEBUG)
     	gpPrintDirection->SatScanPrintf("%d line of data from grid file:\n", nRec);
     	gpPrintDirection->SatScanPrintf("Tract id = %s\n",szTid);
        for (i = 0; i < (m_pParameters->m_nDimension); ++i)
         gpPrintDirection->SatScanPrintf("Coords[%d] = %f\n", i, pCoords[i]);
#endif

        itoa(nRec, szTid, 10);

        if (nScanCount < m_pParameters->m_nDimension) {
          gpPrintDirection->SatScanPrintInputFileWarning(BasePrint::GRIDFILE, "  Error: Invalid record in grid file, line %d.\n",nRec);
          gpPrintDirection->SatScanPrintInputFileWarning(BasePrint::GRIDFILE, "         Please see 'grid file format' in the help file.\n");
          bValid = false;
          continue;
        }

        //*******************************************************************
        // v2.2 Ignor duplicate grid file coordinates
        //
        // just print a message in the warnings box and only print it once.
        // DO NOT END JOB....
        //*******************************************************************
        if (!gpGInfo->giInsertGnode(szTid, pCoords)) {
          char sMessage[100];
          sprintf(sMessage, "  Error: Duplicate tract ID in grid file, line %d.\n",nRec);
          SSGenerateWarning(sMessage, "ReadGridCoords()");
          bValid = false;
        }
      } // while fgets()

      if (bEmpty) {
        gpPrintDirection->SatScanPrintInputFileWarning(BasePrint::GRIDFILE, "  Error: Grid file is empty.\n", "ReadGridCoords()");
        bValid = false;
      }

      m_nGridTracts = gpGInfo->giGetNumTracts();

      fclose(fp);
      free(pCoords);
   }
   catch (SSException & x) {
      free(pCoords);
      x.AddCallpath("ReadGridCoords()", "CSaTScanData");
      throw;
   }
   return bValid;
}


bool CSaTScanData::ReadGridLatLong() {
   bool    	bValid=true, bEmpty=true;
   char    	szData[MAX_LINESIZE], szTid[MAX_LINESIZE];
   int     	nScanCount;                                 // Num of items on input line
   tract_t 	nRec = 0;                                   // File record number
   double   	Latitude, Longitude, * pCoords=0;
   FILE       * fp;                                         // Ptr to grid file

   try {
      gpPrintDirection->SatScanPrintf("Reading the grid file (lat/lon).\n");

      if ((fp = fopen(m_pParameters->m_szGridFilename, "r")) == NULL) {
        gpPrintDirection->SatScanPrintInputFileWarning(BasePrint::GRIDFILE, "  Error: Could not open special grid file (lat/lon).\n", "ReadGridLatLong()");
        return false;
      }
    
      pCoords = (double*)Smalloc(3 * sizeof(double), gpPrintDirection);
    
      while (fgets(szData, MAX_LINESIZE, fp)) {
        ++nRec;
    
        if (GetWord(szData, 0, gpPrintDirection) == 0)                     // ignore blank lines
          continue;
        else
          bEmpty=false;

        nScanCount = sscanf(szData, "%lf %lf",&Latitude, &Longitude);
        itoa(nRec, szTid, 10);

        if (nScanCount < 2) {
          gpPrintDirection->SatScanPrintInputFileWarning(BasePrint::GRIDFILE, "  Error: Invalid record in grid file (lat/lon), line %d.\n",nRec);
          gpPrintDirection->SatScanPrintInputFileWarning(BasePrint::GRIDFILE, "         Please see 'grid file format' in the help file.\n");
          bValid = false;
          continue;
        }
    
        // Check for extra coordinates
        if (GetWord(szData, 2, gpPrintDirection)) {
           gpPrintDirection->SatScanPrintInputFileWarning(BasePrint::GRIDFILE, "  Error: Extra data in grid file (lat/lon), line %ld.\n", (long) nRec);
           gpPrintDirection->SatScanPrintInputFileWarning(BasePrint::GRIDFILE, "         Please see 'grid file format' in the help file.\n");
           bValid = false;
        }

        //Check for correct ranges of latitude and longitude values
        if ((fabs(Latitude) > 90.0)) {
           gpPrintDirection->SatScanPrintInputFileWarning(BasePrint::GRIDFILE, "  Error: Latitude range error in grid file (1st column), line %ld.\n", (long) nRec);
           gpPrintDirection->SatScanPrintInputFileWarning(BasePrint::GRIDFILE, "         Latitude must be between -90 and 90.\n");
           bValid = false;
           continue;
        }

        if ((fabs(Longitude) > 180.0)) {
           gpPrintDirection->SatScanPrintInputFileWarning(BasePrint::GRIDFILE, "  Error: Longitude range error in grid file (2nd column), line %ld.\n", (long) nRec);
           gpPrintDirection->SatScanPrintInputFileWarning(BasePrint::GRIDFILE, "         Longitude must be between -180 and 180.\n");
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
        if (!gpGInfo->giInsertGnode(szTid, pCoords)) {
          // if (! bPrintDuplicateWarning)
          //    gpPrintDirection->SatScanPrintWarning("Note: The grid file has multiple identical coordinates.\nThis does not effect the results, but the program will run faster if duplicates are removed.");
           //bPrintDuplicateWarning = true;
           char sMessage[100];
          sprintf(sMessage,"  Error: Duplicate tract ID in grid file (lat/lon), line %d.\n",nRec);
          SSGenerateWarning(sMessage, "ReadGridLatLong()");
          bValid = false;
        }
      } // while fgets()
    
      if (bEmpty) {
        gpPrintDirection->SatScanPrintInputFileWarning(BasePrint::GRIDFILE, "  Error: Grid file is empty (lat/lon).\n", "ReadGridLatLong()");
        bValid = false;
      }

      m_nGridTracts = gpGInfo->giGetNumTracts();
    
      fclose(fp);
      free(pCoords);
   }
   catch (SSException & x) {
      free(pCoords);
      x.AddCallpath("ReadGridLatLong()", "CSaTScanData");
      throw;
   }
   return bValid;
}



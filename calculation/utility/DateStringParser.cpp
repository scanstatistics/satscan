//***************************************************************************
#include "SaTScan.h"
#pragma hdrstop
//***************************************************************************
#include "DateStringParser.h"

const unsigned int DateStringParser::DEFAULT_DAY                       = 1;
const unsigned int DateStringParser::DEFAULT_MONTH                     = 1;
const unsigned int DateStringParser::POP_PRECISION_MONTH_DEFAULT_DAY   = 15;
const unsigned int DateStringParser::POP_PRECISION_YEAR_DEFAULT_DAY    = 1;
const unsigned int DateStringParser::POP_PRECISION_YEAR_DEFAULT_MONTH  = 7;

/** constructor */
DateStringParser::DateStringParser() {}

/** destructor */
DateStringParser::~DateStringParser() {}

/** Attempts to ensure that passed year is four digits. The function is an artifact
    of the deprecated ability to specify input data dates with two digits. Returns
    indication of parse status. */
DateStringParser::ParserStatus DateStringParser::Ensure4DigitYear(unsigned int& y, Julian LowerBoundDt, Julian UpperBoundDt) {
  unsigned int month, day, nLowerBound, nUpperBound, nLowerC, nUpperC, nLowerD, nUpperD;

  JulianToMDY(&month, &day, &nLowerBound, LowerBoundDt);
  JulianToMDY(&month, &day, &nUpperBound, UpperBoundDt);
  nLowerC = (unsigned int) floor(nLowerBound / 100);
  nUpperC = (unsigned int) floor(nUpperBound / 100);
  nLowerD = nLowerBound - nLowerC*100;
  nUpperD = nUpperBound - nUpperC*100;

  if (y > MIN_YEAR)                              // Return y if 4 digit
    return VALID_DATE;
  else if (y > 99)
    return INVALID_DATE;
  else if (nLowerBound == 0 && nUpperBound == 0) // Assum 1900's if no
    y += 1900;                                   // bound specified
  else if (nLowerC == nUpperC)                   // Bound in same century
    y += nLowerC * 100;
  else if (nUpperBound - nLowerBound >= 100)     // Time period > 99 years,
    return AMBIGUOUS_YEAR;                       // century can not be determined
  else if (y >= nLowerD)                         // Date in Lower century
    y += nLowerC * 100;
  else if (y <= nUpperD)                         // Date in Upper century
    y += nUpperC * 100;
  else
    return INVALID_DATE;

  return VALID_DATE;
}

/** Validates that passed date compoments comprises a valid date and assigns passed
    reference to julian date equivalent. */
DateStringParser::ParserStatus DateStringParser::GetAsJulian(unsigned int iMonth, unsigned int iDay,
                                                             unsigned int iYear, Julian& theDate) {

   if (!IsDateValid(iMonth, iDay, iYear))
     return INVALID_DATE;
     
   theDate = MDYToJulian(iMonth, iDay, iYear);
   return VALID_DATE;
}                                                            

/** Parses passed string into date components, noting the determined precision
    and format. This function supports dates of formats: yyyy/mm/dd or
    mm/dd/yyyy with all lesser precisions (i.e. yyyy/mm or yyyy). This funciton
    also permits the date string use separators '/', '-', '.', '*'.
    Note that support for two digit years is available for YMD type format only,
    so that existing data sets being used with SaTScan continue to be valid. */
DateStringParser::ParserStatus DateStringParser::GetInParts(const char * sDateString,
                                                           const Julian& PeriodStart, const Julian& PeriodEnd,
                                                           unsigned int& iOne, unsigned int& iTwo, unsigned int& iThree,
                                                           DatePrecisionType& ePrecision, DateFormat& eDateFormat) {
  unsigned int        iLength, iCount=1;
  const char        * ptr = sDateString;
  ZdString            sFormat("%u");

  //determine precision
  while ((iLength = strcspn(ptr, "/-.*")) < strlen(ptr)) {
      ptr += iLength;
      if (++iCount > DAY)
        return INVALID_DATE;
      sFormat << *ptr << "%u";
      ptr += 1;
  }
  ePrecision = (DatePrecisionType)iCount;
  //scan into parts - determined by precision
  switch (ePrecision) {
      case YEAR  : if (sscanf(sDateString, sFormat.GetCString(), &iOne) != 1)
                     return INVALID_DATE;
                   break;
      case MONTH : if (sscanf(sDateString, sFormat.GetCString(), &iOne, &iTwo) != 2)
                     return INVALID_DATE;
                   break;
      case DAY   : if (sscanf(sDateString, sFormat.GetCString(), &iOne, &iTwo, &iThree) != 3)
                     return INVALID_DATE;
                   break;
  };
  //determine format - Y/M/D or M/D/Y
  //Format Y/M/D supports 2 digit years (no longer encouraged, but still supported),
  //we'll use this to determine the format - ambiguity causing assumption of Y/M/D.
  //The format M/D/Y is required to have a four digit year, we'll use this to
  //determine format.
  switch (ePrecision) {
      case YEAR  : eDateFormat = YMD; break;
      case MONTH : eDateFormat = iTwo > 999 ? MDY : YMD; break;
      case DAY   : eDateFormat = iThree > 999 ? MDY : YMD; break;
      default    : return INVALID_DATE;
  };
  return (eDateFormat == YMD ? Ensure4DigitYear(iOne, PeriodStart, PeriodEnd) : VALID_DATE);
}

/** Parses passed date string as a date string from the adjustments file. */
DateStringParser::ParserStatus DateStringParser::ParseAdjustmentDateString(const char * sDateString,
                                                                           const Julian& PeriodStart,
                                                                           const Julian& PeriodEnd,
                                                                           Julian& theDate,
                                                                           bool bStartDate) {
  ParserStatus          eParserStatus;
  DatePrecisionType     ePrecision;
  unsigned int          iOne, iTwo, iThree;
  DateFormat            eDateFormat;

  eParserStatus = GetInParts(sDateString, PeriodStart, PeriodEnd, iOne, iTwo, iThree, ePrecision, eDateFormat);
  if (eParserStatus != VALID_DATE)
    return eParserStatus;
  if (eDateFormat == MDY) {
    switch (ePrecision) {
      case YEAR  : iTwo = (bStartDate ? 1 : 12);
                   iThree = (bStartDate ? 1 : 31);
                   return GetAsJulian(iTwo, iThree, iOne, theDate);
      case MONTH : iThree = (bStartDate ? 1 : DaysThisMonth(iTwo, iOne));
                   return GetAsJulian(iOne, iThree, iTwo, theDate);
      case DAY   : return GetAsJulian(iOne, iTwo, iThree, theDate);
      default    : return INVALID_DATE;
    };
  }
  else {
    switch (ePrecision) {
      case YEAR  : iTwo = (bStartDate ? 1 : 12);
                   iThree = (bStartDate ? 1 : 31);
                   return GetAsJulian(iTwo, iThree, iOne, theDate);
      case MONTH : iThree = (bStartDate ? 1 : DaysThisMonth(iOne, iTwo));
                   return GetAsJulian(iTwo, iThree, iOne, theDate);
      case DAY   : return GetAsJulian(iTwo, iThree, iOne, theDate);
      default    : return INVALID_DATE;
    };
  }
}

/** Parses passed date string as a date string from the case or control file. */
DateStringParser::ParserStatus DateStringParser::ParseCountDateString(const char * sDateString,
                                                                     DatePrecisionType eMinPrecision,
                                                                     const Julian& PeriodStart,
                                                                     const Julian& PeriodEnd,
                                                                     Julian& theDate) {
  ParserStatus          eParserStatus;
  DatePrecisionType     ePrecision;
  unsigned int          iOne, iTwo, iThree;
  DateFormat            eDateFormat;

  eParserStatus = GetInParts(sDateString, PeriodStart, PeriodEnd, iOne, iTwo, iThree, ePrecision, eDateFormat);
  if (eParserStatus != VALID_DATE)
    return eParserStatus;
  if (ePrecision < eMinPrecision)
    return LESSER_PRECISION;
  if (eDateFormat == MDY) {
    switch (ePrecision) {
      case YEAR  : return GetAsJulian(DEFAULT_MONTH, DEFAULT_DAY, iOne, theDate);
      case MONTH : return GetAsJulian(iOne, DEFAULT_DAY, iTwo, theDate);
      case DAY   : return GetAsJulian(iOne, iTwo, iThree, theDate);
      default    : return INVALID_DATE;
    };
  }
  else {
    switch (ePrecision) {
      case YEAR  : return GetAsJulian(DEFAULT_MONTH, DEFAULT_DAY, iOne, theDate);
      case MONTH : return GetAsJulian(iTwo, DEFAULT_DAY, iOne, theDate);
      case DAY   : return GetAsJulian(iTwo, iThree, iOne, theDate);
      default    : return INVALID_DATE;
    };
  }
}

/** Parses passed date string as a date string from the population file. */
DateStringParser::ParserStatus DateStringParser::ParsePopulationDateString(const char * sDateString,
                                                                          const Julian& PeriodStart,
                                                                          const Julian& PeriodEnd,
                                                                          Julian& theDate,
                                                                          DatePrecisionType& eReadPrecision) {
  ParserStatus          eParserStatus;
  unsigned int          iOne, iTwo, iThree;
  DateFormat            eDateFormat;

  eParserStatus = GetInParts(sDateString, PeriodStart, PeriodEnd, iOne, iTwo, iThree, eReadPrecision, eDateFormat);
  if (eParserStatus != VALID_DATE)
    return eParserStatus;
  if (eDateFormat == MDY) {
    switch (eReadPrecision) {
      case YEAR  : return GetAsJulian(POP_PRECISION_YEAR_DEFAULT_MONTH, POP_PRECISION_YEAR_DEFAULT_DAY, iOne, theDate);
      case MONTH : return GetAsJulian(iOne, POP_PRECISION_MONTH_DEFAULT_DAY, iTwo, theDate);
      case DAY   : return GetAsJulian(iOne, iTwo, iThree, theDate);
      default    : return INVALID_DATE;
    };
  }
  else {
    switch (eReadPrecision) {
      case YEAR  : return GetAsJulian(POP_PRECISION_YEAR_DEFAULT_MONTH, POP_PRECISION_YEAR_DEFAULT_DAY, iOne, theDate);
      case MONTH : return GetAsJulian(iTwo, POP_PRECISION_MONTH_DEFAULT_DAY, iOne, theDate);
      case DAY   : return GetAsJulian(iTwo, iThree, iOne, theDate);
      default    : return INVALID_DATE;
    };
  }
}


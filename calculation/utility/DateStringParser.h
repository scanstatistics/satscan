//***************************************************************************
#ifndef DateStringParserH
#define DateStringParserH
//***************************************************************************
#include "JulianDates.h"

/** This class provides functionality for parsing strings that represent dates
    that are formatted in ways SaTScan claims to accept and returns Julian date
    equivalence. Any errors parsing string into date are indicated by return
    type DateStringParser::ParserStatus. Prior to the creation of this class,
    the only formats accepted were: "95", "1995", "1995/02", "95/02/05",
    and "1995/2/5". This class adds the ability to have dates separated by
    any of the characters: '/', '-', '*' and '.'. This class also adds the
    ability to have dates that are also formatted like: "02/1995" and
    "02/05/1995" (note that two digit years are not supported in new format).
    The public interface makes particular functions available for parsing
    dates of input files which, beyond parsing, have particular requirements. */
class DateStringParser {
  public:
    enum                                ParserStatus {VALID_DATE=0, INVALID_DATE, AMBIGUOUS_YEAR, LESSER_PRECISION};
    enum                                DateFormat {MDY=0, YMD};

  protected:
    static const unsigned int           DEFAULT_DAY;
    static const unsigned int           DEFAULT_MONTH;
    static const unsigned int           POP_PRECISION_MONTH_DEFAULT_DAY;
    static const unsigned int           POP_PRECISION_YEAR_DEFAULT_DAY;
    static const unsigned int           POP_PRECISION_YEAR_DEFAULT_MONTH;

    DateStringParser::ParserStatus      Ensure4DigitYear(unsigned int& y, Julian LowerBoundDt, Julian UpperBoundDt);
    DateStringParser::ParserStatus      GetAsJulian(unsigned int iMonth, unsigned int iDay, unsigned int iYear, Julian& theDate);
    DateStringParser::ParserStatus      GetInParts(const char * sDateString,
                                                   const Julian& PeriodStart, const Julian& PeriodEnd,
                                                   unsigned int& iOne, unsigned int& iTwo, unsigned int& iThree,
                                                   DatePrecisionType& ePrecision, DateFormat& eDateFormat);

  public:
    DateStringParser();
    ~DateStringParser();

    DateStringParser::ParserStatus      ParseAdjustmentDateString(const char * sDateString, const Julian& PeriodStart,
                                                                  const Julian& PeriodEnd, Julian& theDate, bool bStartDate);
    DateStringParser::ParserStatus      ParseCountDateString(const char * sDateString, DatePrecisionType eMinPrecision,
                                                             const Julian& PeriodStart, const Julian& PeriodEnd,
                                                             Julian& theDate);
    DateStringParser::ParserStatus      ParsePopulationDateString(const char * sDateString, const Julian& PeriodStart,
                                                                  const Julian& PeriodEnd, Julian& theDate);
};
//***************************************************************************
#endif


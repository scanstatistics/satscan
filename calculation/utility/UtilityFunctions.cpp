#include "SaTScan.h"
#pragma hdrstop
#include "UtilityFunctions.h"

double CalcLogLikelihood(count_t n, measure_t u, count_t N, measure_t U) {
  double nLogLikelihood;

  if (n != N && n != 0)
    nLogLikelihood = n*log(n/u) + (N-n)*log((N-n)/(U-u));
  else if (n == 0)
    nLogLikelihood = (N-n) * log((N-n)/(U-u));
  else
    nLogLikelihood = n*log(n/u);

   return (nLogLikelihood);
}

// Conversion routines for Latitude/Longitude option for data input
// and output based on the following formulas:
//
//		d = latitude
//		l = longitude
//		a = radius (fixed = 6367 km)
//
//		x = a cos(l) cos(d)
//		y = a sin(l) cos(d)
//		z = a sin(d)
//
//		Latitude and Longitude values should be float, ie, real numbers
//		with ranges [-90,90] and [-180,180] (in degrees) respectively.
void ConvertFromLatLong(double Latitude, double Longitude, std::vector<double>& vCoordinates) {
  double RADIUS = 6367; // Constant; radius of earth in km)

  vCoordinates[0] = (RADIUS * cos(Longitude*PI/180.0) * cos(Latitude*PI/180.0)); // x coordinate
  vCoordinates[1] = (RADIUS * sin(Longitude*PI/180.0) * cos(Latitude*PI/180.0)); // y coordinate
  vCoordinates[2] = (RADIUS * sin(Latitude*PI/180.0));														// z coordinate
}

/** converts passed coordinates to latitude/longitude
    NOTE: This function takes doubles and converts to floats. To keep
          consistancy with output, this function is intentionally left this way.
          Currently, this function is used soley for reporting of cluster
          coordinates in results and cluster data output files. Switching to
          doubles causes different output results due to greater precision. */
void ConvertToLatLong(float* Latitude, float* Longitude, double* pCoords) {
  float RADIUS = 6367; // Constant; radius of earth in km)

  if (pCoords[0] != 0) {
    *Longitude = (float)(atan(pCoords[1] / pCoords[0]) * 180.0 / PI);
    if (pCoords[0] < 0 && pCoords[1] > 0)
      *Longitude += 180.0;
    else if (pCoords[0] < 0 && pCoords[1] < 0)
      *Longitude -= 180.0;
  }
  else if (pCoords[1] > 0)
    *Longitude = 90.0;
  else if (pCoords[1] < 0)
    *Longitude = -90.0;
  else if (pCoords[1] == 0)
    *Longitude = 0.0;

  float tmp = sqrt((pCoords[0]*pCoords[0] + pCoords[1]*pCoords[1])/(RADIUS*RADIUS));
  *Latitude = (float)((pCoords[2] >= 0 ? (1.0) : (-1.0)) * acos(tmp) * 180.0 / PI);
}

void DisplayVersion(FILE* fp=stdout, int nPos=0)
{
  if (nPos==1)
    fprintf(fp,"                        ");
  fprintf(fp, "SaTScan v");
  fprintf(fp, VERSION_NUMBER);
  fprintf(fp, "\n");
}

/** Return Duczmal Compactness Correction coefficient. */
double GetDuczmalCorrection(double dEllipseShape) {
  return ( 4*dEllipseShape/(pow(dEllipseShape + 1, 2)) );
}


/** constructor */
StringParser::StringParser(BasePrint::eInputFileType eFileType) {
  gwCurrentWordIndex = -1;
  glReadCount = 0;
  geFileType = eFileType; 
  giSizeOfWordBuffer = MAX_LINEITEMSIZE;
  gpWord = new char[MAX_LINEITEMSIZE];
  gpWord[0] = 0;
}

/** destructor */
StringParser::~StringParser(){
  try {
    delete[] gpWord;
  }
  catch (...){}  
}

void StringParser::ThrowAsciiException() {
  std::string s;

  SSGenerateException("Error: The %s file contains data that is not ASCII formatted.\n"
                      "       Please see 'ASCII Input File Format' in the user guide for help.\n",
                      "CheckIsASCII()", BasePrint::GetInputFileType(geFileType, s).c_str());
}

/** Returns whether string has words. */
bool StringParser::HasWords() {
  return GetWord(0) != 0;
}

/** Returns number of words in string -- this could be better  */
int StringParser::GetNumberWords() {
  int   iWords=0;

  while (GetWord(iWords) != 0)
       ++iWords;
  return iWords;
}

/** Returns wWordIndex + 1 'th word in associated string.
    If wWordIndex is greater than number of words, NULL pointer returned. */
const char * StringParser::GetWord(short wWordIndex) {
  int           inwd, wdlen;
  short         w = wWordIndex;
  const char  * cp = gsReadBuffer;
  const char  * cp2;

  //short cut if this word has already been read
  if (wWordIndex == gwCurrentWordIndex)
    return gpWord;

  /* ignore spaces at start of line */
  while(isspace(*cp)) ++cp;

  /* find start of word */
  inwd = !isspace(*cp);
  while (*cp != '\0' && (w > 0 || !inwd)) {
       if (inwd == !!(isspace(*cp))) { /* entered or exited a word */
         inwd = !inwd;
         if (inwd) /* if entered a word, count it */
           if (--w == 0)
             break;
       }
       ++cp; /* next character */
  }

  /* handle underflow */
  if (*cp == '\0')
     return 0;

  /* find end of word */
  cp2 = cp + 1;
  while (!isspace(*cp2)) ++cp2;
  wdlen = cp2 - cp;
  if (wdlen > giSizeOfWordBuffer) {
    giSizeOfWordBuffer = wdlen + 1;
    delete[] gpWord; gpWord=0;
    gpWord =  new char[giSizeOfWordBuffer];
    gpWord[0] = 0;
  }
  memcpy(gpWord, cp, wdlen);
  gpWord[wdlen] = '\0';
  cp = gpWord;
  //check that word is ascii characters
  while (*cp != '\0') {
       if (!isascii(*cp))
         ThrowAsciiException();
       ++cp;
  }
  gwCurrentWordIndex = wWordIndex;
  return gpWord;
}

/** Reads a string from file and resets class variables. */
const char * StringParser::ReadString(FILE * pSourceFile) {
  ClearWordIndex();
  ++glReadCount;
  return fgets(gsReadBuffer, MAX_LINESIZE, pSourceFile);
}


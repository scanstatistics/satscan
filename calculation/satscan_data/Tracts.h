//*****************************************************************************
#ifndef __TRACTS_H
#define __TRACTS_H
//*****************************************************************************
#include "SaTScan.h"
#include "JulianDates.h"
#include "PopulationCategories.h"

/**********************************************************************
 file: Tracts.h
 Header file for tinfo.c, which abstracts tract, population & case
 count information
 **********************************************************************/

class TractDescriptor;
class TractHandler;

/** record for one population category in a single tract */
class CategoryDescriptor {
  //TractHandler and TractDescriptor are friend classes and all members/functions
  //are private so that TractHandler is the interface for interacting
  //with tracts. Accessing this object directly might violate relationship
  //with friend classes.
  friend class TractHandler;
  friend class TractDescriptor;

  private:
    int				giCategoryIndex;       /* category index                */
    float                     * gpPopulationList;      /* population at date index      */
    count_t       		gtCaseCount;           /* total case count for category */
    CategoryDescriptor        * gpNextDescriptor;      /* next CategoryDescriptor class in link-list */

    //Restricted to CategoryDescriptor - only this class should call these methods.
    void			Init() {giCategoryIndex=-1; gtCaseCount=0; gpPopulationList=0; gpNextDescriptor=0;}
    void                        Setup(int iPopulationListSize, int iCategoryIndex);

    void			AddCaseCount(count_t tCaseCount);
    void                        AddPopulationAtDateIndex(float fPopluation, int iDateIndex, const TractHandler & theTractHandler);
    void                        Combine(const CategoryDescriptor * pCategoryDescriptor, const TractHandler & theTractHandler);
    count_t			GetCaseCount() const {return gtCaseCount;}
    int 			GetCategoryIndex() const {return giCategoryIndex;}
    CategoryDescriptor        * GetNextDescriptor() {return gpNextDescriptor;}
    const CategoryDescriptor  * GetNextDescriptor() const {return gpNextDescriptor;}
    float			GetPopulationAtDateIndex(int iDateIndex, const TractHandler & theTractHandler) const;
    void			SetCaseCount(count_t tCaseCount);
    void			SetCategoryIndex(int iCategoryIndex) {giCategoryIndex = iCategoryIndex;}
    CategoryDescriptor        * SetNextDescriptor(int iPopulationListSize, int iCategoryIndex);
    void                        SetPopulationAtDateIndex(float fPopluation, int iDateIndex, const TractHandler & theTractHandler);
    void                        SetPopulationListSize(int iPopulationListSize);

    CategoryDescriptor(int iPopulationListSize, int iCategoryIndex = -1);

  public:
    ~CategoryDescriptor();
};

class CompareTractDescriptorIdentifier;

/** tract record              */
class TractDescriptor {
  //TractHandler and CompareTractDescriptorIdentifier are friend classes and
  //all members/functions are private so that TractHandler is the only interface
  //for interacting with tracts. Accessing this object directly might violate
  //relationship with friend classes.
  friend class TractHandler;
  friend class CompareTractDescriptorIdentifier;

  private:
    char                              * gsTractIdentifiers;
    double                            * gpCoordinates;             /* coordinates                   */
    CategoryDescriptor                * gpCategoryDescriptorsList; /* categories of tract link-list */

    //Restricted to TractDescriptor - only this class should call these methods.
    void                                Init() {gsTractIdentifiers=0; gpCoordinates=0; gpCategoryDescriptorsList=0;}
    void                                Setup(const char * sTractIdentifier, const double* pCoordinates, int iDimensions);

    void                                AddTractIdentifier(const char * sTractIdentifier);
    void                                Combine(const TractDescriptor * pCategoryDescriptor, const TractHandler & theTractHandler);
    bool                                CompareCoordinates(const double * pCoordinates, int iDimensions) const;
    bool                                CompareCoordinates(const TractDescriptor & Descriptor, const TractHandler & theTractHandler) const;
    CategoryDescriptor                * GetCategoryDescriptor(int iCategoryIndex);
    const CategoryDescriptor          * GetCategoryDescriptor(int iCategoryIndex) const;
    CategoryDescriptor                & GetCategoryDescriptor(int iCategoryindex, int iPopulationListSize);
    CategoryDescriptor                * GetCategoryDescriptorList();
    const CategoryDescriptor          * GetCategoryDescriptorList() const;
    const double                      *	GetCoordinates() const {return gpCoordinates;}
    double                            *	GetCoordinates(double* pCoordinates, const TractHandler & theTractHandler) const;
    double                            	GetCoordinatesAtDimension(int iDimension, const TractHandler & theTractHandler) const;
    int                                 GetNumTractIdentifiers() const;
    const char                        * GetTractIdentifier() const {return gsTractIdentifiers;}
    const char 			      * GetTractIdentifier(int iTractIdentifierIndex, std::string & sIndentifier);
    void                                GetTractIdentifiers(std::vector<std::string>& vIdentifiers) const;
    void                                SetCoordinates(const double* pCoordinates, int iDimensions);
    void				SetTractIdentifier(const char * sTractIdentifier);

    TractDescriptor(const char * sTractIdentifier, const double* pCoordinates, int iDimensions);

  public:
    ~TractDescriptor();
};

/** Function object used to compare CompareTractDescriptor objects by identifier. */
class CompareTractDescriptorIdentifier {
  public:
    CompareTractDescriptorIdentifier() {}

    bool operator() (TractDescriptor * lhs, TractDescriptor * rhs)
                   {
                   return (strcmp(lhs->gsTractIdentifiers, rhs->gsTractIdentifiers) < 0);
                   }
};

class CSaTScanData;
class TractHandler {
  private:
    const PopulationCategories                 * gpPopulationCategories;
    BasePrint                                  * gpPrintDirection;
    ZdPointerVector<TractDescriptor>             gvTractDescriptors;
    std::vector<Julian>                          gvPopulationDates;
    bool                                         bStartAsPopDt;
    bool                                         bEndAsPopDt;
    int                                          nDimensions;
    TractDescriptor                            * gpSearchTractDescriptor;
    std::map<std::string,TractDescriptor*>       gmDuplicateTracts;

    void                                Init();
    void                                Setup(const PopulationCategories & thePopulationCategories, BasePrint & PrintDirection);

  public:
    TractHandler(const PopulationCategories & PopulationCategories, BasePrint & PrintDirection);
    ~TractHandler();

    void                                tiAddCategoryToTract(tract_t tTractIndex, int iCategoryIndex, Julian PopulationDate, float fPopulation);
    int                                 tiAddCount(tract_t t, int iCategoryIndex, count_t Count);
    void                                tiAssignPopulation(CategoryDescriptor & thisCategoryDescriptor, Julian PopulationDate, float fPopulation);
    void                                tiCalculateAlpha(double** pAlpha, Julian StartDate, Julian EndDate) const;
    void                                tiCheckCasesHavePopulations(CSaTScanData & Data) const;
    bool                                tiCheckZeroPopulations(FILE *pDisplay) const;
    tract_t                             tiCombineDuplicatesByCoordinates();
    void                                tiConcaticateDuplicateTractIdentifiers();
    void                                tiFindPopDatesToUse(std::vector<Julian>& PopulationDates, Julian StartDate, Julian EndDate,
                                                            int* pnSourceOffset, int* pnDestOffset, bool* pbAddStart, bool* pbAddEnd,
                                                            int* pnDatesUsed, int* pnTotalPopDates);
    double                              tiGetAlphaAdjustedPopulation(double & dPopulation, tract_t t, int iCategoryIndex,
                                                                     int iStartPopulationDateIndex, int iEndPopulationDateIndex,
                                                                     double Alpha[]) const;
    count_t                             tiGetCategoryCaseCount(int iCategoryIndex) const;
    void                                tiGetCoords(tract_t t, double** pCoords) const;
    void                                tiGetCoords2(tract_t t, double* pCoords) const;
    count_t                             tiGetCount(tract_t t, int iCategoryIndex) const;
    int                                 tiGetDimensions() const {return nDimensions;}
    double                              tiGetDistanceSq(double* pCoords, double* pCoords2) const;
    int                                 tiGetNumCategories() const {return gpPopulationCategories->GetNumPopulationCategories();}
    int                                 tiGetNumPopDates() const {return (int)gvPopulationDates.size();}
    tract_t                             tiGetNumTracts() const {return (int)gvTractDescriptors.size();}
    Julian                              tiGetPopDate(int iPopulationDateIndex) const;
    int                                 tiGetPopDateIndex(Julian Date);
    float                               tiGetPopulation(tract_t t, int iCategoryIndex, int iPopulationDateIndex) const;
    int                                 tiGetPopUpLowIndex(Julian* pDates, int nDateIndex, int nMaxDateIndex,
                                                           int* nUpIndex, int* nLowIndex) const;
    double                              tiGetRiskAdjustedPopulation(measure_t & dMeanPopulation, tract_t t,
                                                                    int iPopulationDateIndex, double Risk[]) const;
    const char                        * tiGetTid(tract_t t, std::string& sFirst) const;
//    const char                        * tiGetTid(tract_t t) const;
    double                              tiGetTractCoordinate(tract_t t, int iDimension) const;
    void                                tiGetTractIdentifiers(tract_t t, std::vector<std::string>& vIdentifiers) const;
    tract_t                             tiGetTractIndex(const char *tid) const;
    int                                 tiInsertTnode(const char *tid, std::vector<double>& vCoordinates);
    void                                tiReportDuplicateTracts(FILE * fDisplay) const;
    void                                tiReportZeroPops(FILE *pDisplay) const;
    int                                 tiSetCount(tract_t t, int iCategoryIndex, count_t Count);
    void                                tiSetDimensions(int iDimensions) {nDimensions = iDimensions;}
    void                                tiSetupPopDates(std::vector<Julian>& PopulationDates, Julian StartDate, Julian EndDate);
};
//*****************************************************************************
#endif

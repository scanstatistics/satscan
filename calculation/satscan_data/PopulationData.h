//*****************************************************************************
#ifndef __Population_H
#define __Population_H
//*****************************************************************************
#include "SaTScan.h"
#include "UtilityFunctions.h"
#include "JulianDates.h"

class PopulationData;
/** record for one population category in a single tract */
class PopulationCategory {
  friend class PopulationData;
  private:
    unsigned int		giCategoryIndex;       /* category index                */
    float                     * gpPopulationList;      /* population at date index      */
    PopulationCategory        * gpNextDescriptor;      /* next CategoryDescriptor class in link-list */

    //Restricted to CategoryDescriptor - only this class should call these methods.
    void			Init() {giCategoryIndex=0;gpPopulationList=0; gpNextDescriptor=0;}
    void                        Setup(int iPopulationListSize, int iCategoryIndex);

    void                        AddPopulationAtDateIndex(float fPopluation, unsigned int iDateIndex, const PopulationData & thePopulation);
    unsigned int		GetCategoryIndex() const {return giCategoryIndex;}
    PopulationCategory        * GetNextDescriptor() {return gpNextDescriptor;}
    const PopulationCategory  * GetNextDescriptor() const {return gpNextDescriptor;}
    float			GetPopulationAtDateIndex(unsigned int iDateIndex, const PopulationData & thePopulation) const;
    void			SetCategoryIndex(unsigned int iCategoryIndex) {giCategoryIndex = iCategoryIndex;}
    PopulationCategory        * SetNextDescriptor(int iPopulationListSize, int iCategoryIndex);
    void                        SetPopulationAtDateIndex(float fPopluation, unsigned int iDateIndex, const PopulationData & thePopulation);
    void                        SetPopulationListSize(int iPopulationListSize);

    PopulationCategory(int iPopulationListSize, int iCategoryIndex = -1);

  public:
    ~PopulationCategory();
};


class CSaTScanData;
/** This file abstracts "categories" of populations(.i.e covariate combinations).
    Each data and case record has a variable number of fields which specify
    arbitrary string values.  Each combination of these values defines a
    separate category, by which the data is stratified. */
class PopulationData {
  private:
    bool                                gbAggregateCategories;
    int                                 giNumberCovariates;            /** number covariates expected in each record
                                                                           - as defined by first non-blank record of population file */
    std::vector<std::string>            gvCovariateNames;              /** names of covariates */
    std::vector<std::vector<int> >      gvPopulationCategories;        /** vector of population categories
                                                                           - integers are indexes of covariate names */
    std::vector<count_t>                gvCategoryCasesCount;          /** number of cases for category */                                                                        
    std::vector<count_t>                gvCategoryControlsCount;       /** number of controls for category */
    
    std::vector<PopulationCategory*>    gTractCategories;          /** one for each tract */

    std::vector<Julian>                 gvPopulationDates;

    bool                                gbStartAsPopDt;
    bool                                gbEndAsPopDt;    

    void                                Init() {giNumberCovariates=0;gbAggregateCategories=false;}

  public:
    PopulationData();
    ~PopulationData();

    void                                AddCaseCount(int iCategoryIndex, count_t Count);
    void                                AddCategoryToTract(tract_t tTractIndex, unsigned int iCategoryIndex, Julian PopulationDate, float fPopulation);
    void                                AddControlCount(int iCategoryIndex, count_t Count);
    void                                AssignPopulation(PopulationCategory & thisPopulationCategory, Julian PopulationDate, float fPopulation);
    void                                CalculateAlpha(double** pAlpha, Julian StartDate, Julian EndDate) const;
    void                                CheckCasesHavePopulations(const count_t * pCases, CSaTScanData & Data) const;    
    bool                                CheckZeroPopulations(FILE *pDisplay, BasePrint * pPrintDirection) const;
    void                                Display(BasePrint & PrintDirection) const;
    void                                FindPopDatesToUse(std::vector<Julian>& PopulationDates, Julian StartDate,
                                                          Julian EndDate, int* pnSourceOffset, int* pnDestOffset,
                                                          bool* pbAddStart, bool* pbAddEnd, int* pnDatesUsed,
                                                          int* pnPopDates);
    double                              GetAlphaAdjustedPopulation(double & dPopulation, tract_t t,
                                                                   int iCategoryIndex, int iStartPopulationDateIndex,
                                                                   int iEndPopulationDateIndex, double Alpha[]);
    count_t                             GetNumCategoryCases(int iCategoryIndex) const;
    count_t                             GetNumCategoryControls(int iCategoryIndex) const;
    int                                 GetNumPopulationCategories() const {return (int)gvPopulationCategories.size();}
    int                                 GetNumPopulationCategoryCovariates() const {return giNumberCovariates;}
    unsigned int                        GetNumPopulationDates() const {return gvPopulationDates.size();}
    PopulationCategory                * GetCategoryDescriptor(tract_t tTractIndex, unsigned int iCategoryIndex);
    float                               GetPopulation(tract_t t, int iCategoryIndex, int iPopulationDateIndex);
    PopulationCategory                & GetPopulationCategory(tract_t tTractIndex, unsigned int iCategoryIndex, int iPopulationListSize);
    const char                        * GetPopulationCategoryAsString(int iCategoryIndex, std::string & sBuffer) const;
    int                                 GetPopulationCategoryIndex(const std::vector<std::string>& vCategoryCovariates) const;
    Julian                              GetPopulationDate(int iDateIndex) const;
    int                                 GetPopulationDateIndex(Julian Date) const;
    int                                 GetPopUpLowIndex(Julian* pDates, int nDateIndex,
                                                         int nMaxDateIndex, int* nUpIndex, int* nLowIndex) const;
    double                              GetRiskAdjustedPopulation(measure_t & dMeanPopulation, tract_t t,
                                                                  int iPopulationDateIndex, double Risk[]) const;
    int                                 LowerPopIndex(Julian Date) const;                                                                  
    int                                 MakePopulationCategory(StringParser & Parser, int iScanOffset, BasePrint & PrintDirection);
    void                                ReportZeroPops(CSaTScanData & Data, FILE *pDisplay, BasePrint * pPrintDirection) const;
    void                                SetAggregateCategories(bool b);
    void                                SetNumTracts(unsigned int iTracts) {gTractCategories.resize(iTracts, 0);}
    void                                SetupPopDates(std::vector<Julian>& PopulationDates, Julian StartDate,
                                                      Julian EndDate, BasePrint * pPrintDirection);
    int                                 UpperPopIndex(Julian Date) const;                                                   
};
//*****************************************************************************
#endif

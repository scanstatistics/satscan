//*****************************************************************************
#ifndef __Population_H
#define __Population_H
//*****************************************************************************
#include "SaTScan.h"
#include "UtilityFunctions.h"
#include "JulianDates.h"

class PopulationData; /** forward class declaration */

/** record for one population category in a single tract */
class PopulationCategory {
  friend class PopulationData;
  private:
    unsigned int		giCategoryIndex;       /* category index                */
    float                     * gpPopulationList;      /* population at date index      */
    PopulationCategory        * gpNextDescriptor;      /* next CategoryDescriptor class in link-list */

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

    //private constructor - accessible only to class PopulationData
    PopulationCategory(int iPopulationListSize, int iCategoryIndex = -1);

  public:
    ~PopulationCategory();
};


class CSaTScanData; /** forward class declaration */

/** Population data. Maintains:
    - list of population dates as read from population, if Poisson model.
    - functionality to aide in measure calculation for Poisson model
    - populations for each location in separate categories; as per input data.
    - list of total cases/controls for each population category */
class PopulationData {
  private:
    bool                                gbAggregateCategories;     /** indicates that category data should be aggregated
                                                                       together, not maintained separate - Bernoulli */
    int                                 giNumberCovariates;        /** number covariates expected in each record
                                                                       - as defined by first non-blank record of population file */
    std::vector<std::string>            gvCovariateNames;          /** names of covariates */
    std::vector<std::vector<int> >      gvPopulationCategories;    /** vector of population categories
                                                                       - integers are indexes of covariate names */
    std::vector<count_t>                gvCategoryCasesCount;      /** number of cases for category */
    std::vector<count_t>                gvCategoryControlsCount;   /** number of controls for category */

    std::vector<PopulationCategory*>    gTractCategories;          /** population categories for each tract */

    std::vector<Julian>                 gvPopulationDates;         /** collection of all population dates */

    bool                                gbStartAsPopDt;            /** indicates whether the study period start
                                                                       date was introduced into gvPopulationDates */
    bool                                gbEndAsPopDt;              /** indicates whether the study period end
                                                                       date was introduced into gvPopulationDates */

    void                                AssignPopulation(PopulationCategory& thisPopulationCategory, Julian PopulationDate, float fPopulation, bool bTrueDate);
    void                                Init() {giNumberCovariates=0;gbAggregateCategories=false;}

  public:
    PopulationData();
    ~PopulationData();

    void                                AddCaseCount(int iCategoryIndex, count_t Count);
    void                                AddCategoryToTract(tract_t tTractIndex, unsigned int iCategoryIndex,
                                                           const std::pair<Julian, DatePrecisionType>& prPopulationDate, float fPopulation);
    void                                AddControlCount(int iCategoryIndex, count_t Count);
    void                                CalculateAlpha(std::vector<double>& vAlpha, Julian StartDate, Julian EndDate) const;
    void                                CheckCasesHavePopulations(const count_t * pCases, const CSaTScanData& Data) const;    
    bool                                CheckZeroPopulations(FILE *pDisplay, BasePrint& PrintDirection) const;
    void                                Display(BasePrint & PrintDirection) const;
    double                              GetAlphaAdjustedPopulation(double& dPopulation, tract_t t,
                                                                   int iCategoryIndex, int iStartPopulationDateIndex,
                                                                   int iEndPopulationDateIndex,
                                                                   const std::vector<double>& vAlpha) const;
    count_t                             GetNumCategoryCases(int iCategoryIndex) const;
    count_t                             GetNumCategoryControls(int iCategoryIndex) const;
    int                                 GetNumPopulationCategories() const {return (int)gvPopulationCategories.size();}
    int                                 GetNumPopulationCategoryCovariates() const {return giNumberCovariates;}
    unsigned int                        GetNumPopulationDates() const {return gvPopulationDates.size();}
    PopulationCategory                * GetCategoryDescriptor(tract_t tTractIndex, unsigned int iCategoryIndex);
    const PopulationCategory          * GetCategoryDescriptor(tract_t tTractIndex, unsigned int iCategoryIndex) const;
    float                               GetPopulation(tract_t t, int iCategoryIndex, int iPopulationDateIndex);
    PopulationCategory                & GetPopulationCategory(tract_t tTractIndex, unsigned int iCategoryIndex, int iPopulationListSize);
    const char                        * GetPopulationCategoryAsString(int iCategoryIndex, std::string & sBuffer) const;
    int                                 GetPopulationCategoryIndex(const std::vector<std::string>& vCategoryCovariates) const;
    Julian                              GetPopulationDate(int iDateIndex) const;
    int                                 GetPopulationDateIndex(Julian Date, bool bTrueDate) const;
    void                                GetPopUpLowIndex(Julian* pDates, int nDateIndex,
                                                         int nMaxDateIndex, int& nUpIndex, int& nLowIndex) const;
    measure_t                           GetRiskAdjustedPopulation(measure_t& dMeanPopulation, tract_t t,
                                                                  int iPopulationDateIndex, const std::vector<double>& vRisk) const;
    int                                 LowerPopIndex(Julian Date) const;                                                                  
    int                                 MakePopulationCategory(StringParser& Parser, unsigned int iScanOffset, BasePrint& PrintDirection);
    void                                ReportZeroPops(const CSaTScanData& Data, FILE *pDisplay, BasePrint& PrintDirection) const;
    void                                SetAggregateCategories(bool b);
    void                                SetNumTracts(unsigned int iTracts) {gTractCategories.resize(iTracts, 0);}
    void                                SetPopulationDates(std::vector<std::pair<Julian, DatePrecisionType> >& PopulationDates,
                                                           Julian StartDate, Julian EndDate);
    int                                 UpperPopIndex(Julian Date) const;                                                   
};
//*****************************************************************************
#endif

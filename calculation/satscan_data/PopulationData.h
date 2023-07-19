//******************************************************************************
#ifndef __PopulationData_H
#define __PopulationData_H
//******************************************************************************
#include "SaTScan.h"
#include "UtilityFunctions.h"
#include "JulianDates.h"

class PopulationData; /** forward class declaration */
class DataSource;     /** forward class definition */

/** Stores data for covariate category for a single location. */
class CovariateCategory {
  friend class PopulationData;
  private:
    unsigned int		giCategoryIndex;       /* category index                */
    float                     * gpPopulationList;      /* population at date index      */
    CovariateCategory         * gpNextDescriptor;      /* next CovariateCategory class in link-list */

    void			Init() {giCategoryIndex=0;gpPopulationList=0; gpNextDescriptor=0;}
    void                        Setup(int iPopulationListSize, int iCategoryIndex);

    void                        AddPopulationAtDateIndex(float fPopluation, unsigned int iDateIndex, const PopulationData & thePopulation);
    unsigned int		GetCategoryIndex() const {return giCategoryIndex;}
    CovariateCategory         * GetNextDescriptor() {return gpNextDescriptor;}
    const CovariateCategory   * GetNextDescriptor() const {return gpNextDescriptor;}
    float			GetPopulationAtDateIndex(unsigned int iDateIndex, const PopulationData & thePopulation) const;
    void			SetCategoryIndex(unsigned int iCategoryIndex) {giCategoryIndex = iCategoryIndex;}
    CovariateCategory         * SetNextDescriptor(int iPopulationListSize, int iCategoryIndex);
    void                        SetPopulationAtDateIndex(float fPopluation, unsigned int iDateIndex, const PopulationData & thePopulation);
    void                        SetPopulationListSize(int iPopulationListSize);

    //private constructor - accessible only to class PopulationData
    CovariateCategory(int iPopulationListSize, int iCategoryIndex = -1);

  public:
    ~CovariateCategory();
};

/** Stores case count for category type. This class is used by the ordinal and multinomial models. */
class CategoryType {
  private:
    count_t     gtTotalCases;
    std::string _category_label;

  public:
    CategoryType(const std::string& category_label, count_t tInitialCount=0) : _category_label(category_label), gtTotalCases(tInitialCount) {}
    ~CategoryType() {}

    bool                    operator==(const CategoryType& rhs) const { return (_category_label == rhs._category_label); }
    void                    AddCaseCount(count_t tCount);
    void                    DecrementCaseCount(count_t tCount) { gtTotalCases = std::max(0L, gtTotalCases - tCount); }
    const std::string     & getCategoryLabel() const { return _category_label; }
    static double           getAsOrdinalNumber(const std::string& s);
    double                  getCategoryLabelAsOrdinalNumber() const;
    count_t                 GetTotalCases() const {return gtTotalCases;}
};

class CompareCategoryTypeByLabel {
public:
    bool operator() (const CategoryType& lhs, const CategoryType& rhs) {
        return lhs.getCategoryLabel() < rhs.getCategoryLabel();
    }
};

class CompareCategoryTypeByOrdinal {
public:
    bool operator() (const CategoryType& lhs, const CategoryType& rhs) {
        return lhs.getCategoryLabelAsOrdinalNumber() < rhs.getCategoryLabelAsOrdinalNumber();
    }
};


class CSaTScanData; /** forward class declaration */

/** Population data. Maintains:
    - list of population dates as read from population, if Poisson model.
    - functionality to aide in measure calculation for Poisson model
    - populations for each location in separate covariate categories; as per input data.
    - list of total cases/controls for each population covariate category
    - list of total cases for each ordinal number category */
class PopulationData {
  public:
      typedef std::pair<Julian, DatePrecisionType> PopulationDate_t;
      typedef std::vector<PopulationDate_t> PopulationDateContainer_t;
      typedef std::vector<std::string> CovariatesNames_t;
      typedef std::vector<size_t> AdditionalCovariates_t;

  private:
    bool                                gbAggregateCovariateCategories;   /** indicates that category data should be aggregated
                                                                              together, not maintained separate - Bernoulli */
    short                               giNumberCovariatesPerCategory;    /** number covariates expected in each record */
    CovariatesNames_t                   gvCovariateNames;                 /** covariates labels */
    std::vector<std::vector<int> >      gvCovariateCategories;            /** vector of covariate categories
                                                                              - integers are indexes of covariate names */
    std::vector<count_t>                gvCovariateCategoryCaseCount;     /** number of total cases for covariate categories */
    std::vector<count_t>                gvCovariateCategoryControlCount;  /** number of total controls for covariate categories */
    std::vector<CovariateCategory*>     gCovariateCategoriesPerLocation;  /** CovariateCategory objects for each location */
    std::vector<CategoryType>           _category_types;                  /** categories defined for population stratified by category type */
    std::vector<Julian>                 gvPopulationDates;                /** collection of all population dates */
    bool                                _introduced_start_as_pop;         /** indicates whether the study period start date was introduced into gvPopulationDates */
    bool                                _introduced_end_as_pop;           /** indicates whether the study period end date was introduced into gvPopulationDates */
    mutable AdditionalCovariates_t      _additionalCovariates;

    bool                                AssignPopulation(CovariateCategory& thisCovariateCategory, Julian PopulationDate, float fPopulation, bool bTrueDate);
    void                                Init();

  public:
    PopulationData();
    ~PopulationData();

    void                                AddCovariateCategoryCaseCount(int iCategoryIndex, count_t Count);
    void                                AddCovariateCategoryControlCount(int iCategoryIndex, count_t Count);
    void                                AddCovariateCategoryPopulation(tract_t tTractIndex, unsigned int iCategoryIndex,
                                                                       const PopulationDate_t& prPopulationDate,
                                                                       float fPopulation);
    size_t                              addCategoryTypeCaseCount(const std::string& categoryTypeLabel, count_t Count, bool asOrdinal);
    void                                CalculateAlpha(std::vector<double>& vAlpha, Julian StartDate, Julian EndDate) const;
    void                                CheckCasesHavePopulations(const count_t * pCases, const CSaTScanData& Data) const;
    bool                                CheckZeroPopulations(BasePrint& PrintDirection) const;
    int                                 CreateCovariateCategory(DataSource& Source, short iScanOffset, BasePrint& PrintDirection);
    void                                Display(FILE* pFile) const;
    double                              GetAlphaAdjustedPopulation(double& dPopulation, tract_t t, int iCategoryIndex,
                                                                   int iStartPopulationDateIndex, int iEndPopulationDateIndex,
                                                                   const std::vector<double>& vAlpha) const;
    int                                 GetNumCovariateCategories() const {return (int)gvCovariateCategories.size();}
    count_t                             GetNumCovariateCategoryCases(int iCategoryIndex) const;
    count_t                             GetNumCovariateCategoryControls(int iCategoryIndex) const;
    int                                 GetNumCovariatesPerCategory() const {return giNumberCovariatesPerCategory;}
    count_t                             GetNumCategoryTypeCases(int iCategoryIdx) const;
    size_t                              GetNumOrdinalCategories() const {return _category_types.size();}
    unsigned int                        GetNumPopulationDates() const {return gvPopulationDates.size();}
    CovariateCategory                 * GetCovariateCategory(tract_t tTractIndex, unsigned int iCategoryIndex);
    const CovariateCategory           * GetCovariateCategory(tract_t tTractIndex, unsigned int iCategoryIndex) const;
    CovariateCategory                 & GetCovariateCategory(tract_t tTractIndex, unsigned int iCategoryIndex, int iPopulationListSize);
    const char                        * GetCovariateCategoryAsString(int iCategoryIndex, std::string& sBuffer) const;
    int                                 GetCovariateCategoryIndex(const CovariatesNames_t& vCovariates) const;
    const std::string                 & GetCategoryTypeLabel(int iCategoryIndex) const;
    float                               GetPopulation(tract_t t, int iCategoryIndex, int iPopulationDateIndex);
    Julian                              GetPopulationDate(int iDateIndex) const;
    int                                 GetPopulationDateIndex(Julian Date, bool bTrueDate) const;
    void                                GetPopUpLowIndex(const std::vector<Julian>& vIntervalStartDates, int nDateIndex,
                                                         int nMaxDateIndex, int& nUpIndex, int& nLowIndex) const;
    measure_t                           GetRiskAdjustedPopulation(measure_t& dMeanPopulation, tract_t t,
                                                                  int iPopulationDateIndex, const std::vector<double>& vRisk) const;
    int                                 LowerPopIndex(Julian Date) const;
    void                                RemoveCategoryTypeCases(size_t iCategoryIndex, count_t tCount);
    void                                ReportZeroPops(const CSaTScanData& Data, FILE *pDisplay, BasePrint& PrintDirection) const;
    void                                setAdditionalCovariates(CovariatesNames_t& covariates);
    void                                SetAggregateCovariateCategories(bool b);
    void                                SetNumTracts(unsigned int iTracts) {gCovariateCategoriesPerLocation.resize(iTracts, 0);}
    void                                SetPopulationDates(PopulationDateContainer_t& readPopDates, Julian StartDate, Julian EndDate, bool dayPlus=true);
    int                                 UpperPopIndex(Julian Date) const;
};
//******************************************************************************
#endif


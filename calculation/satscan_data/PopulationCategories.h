//*****************************************************************************
#ifndef __POPULATIONCATEGORIES_H
#define __POPULATIONCATEGORIES_H
//*****************************************************************************
#include "SaTScan.h"
#include "UtilityFunctions.h"

/** This file abstracts "categories" of populations(.i.e covariate combinations).
    Each data and case record has a variable number of fields which specify
    arbitrary string values.  Each combination of these values defines a
    separate category, by which the data is stratified. */

class PopulationCategories {
  private:
    bool                                gbAggregateCategories;
    int                                 giNumberCovariates;            /** number covariates expected in each record
                                                                           - as defined by first non-blank record of population file */
    std::vector<std::string>            gvCovariateNames;              /** names of covariates */
    std::vector<std::vector<int> >      gvPopulationCategories;        /** vector of population categories
                                                                           - integers are indexes of covariate names */
    std::vector<count_t>                gvCategoryCasesCount;          /** number of cases for category */                                                                        
    std::vector<count_t>                gvCategoryControlsCount;       /** number of controls for category */

    void                                Init() {giNumberCovariates=0;gbAggregateCategories=false;}

  public:
    PopulationCategories();
    ~PopulationCategories();

    void                                AddCaseCount(int iCategoryIndex, count_t Count);
    void                                AddControlCount(int iCategoryIndex, count_t Count);
    void                                Display(BasePrint & PrintDirection) const;
    count_t                             GetNumCategoryCases(int iCategoryIndex) const;
    count_t                             GetNumCategoryControls(int iCategoryIndex) const;
    int                                 GetPopulationCategoryIndex(const std::vector<std::string>& vCategoryCovariates) const;
    const char                        * GetPopulationCategoryAsString(int iCategoryIndex, std::string & sBuffer) const;
    int                                 GetNumPopulationCategories() const {return (int)gvPopulationCategories.size();}
    int                                 GetNumPopulationCategoryCovariates() const {return giNumberCovariates;}
    int                                 MakePopulationCategory(const char* szDescription, StringParser & Parser, int iScanOffset, BasePrint & PrintDirection);
    void                                SetAggregateCategories(bool b);
};
//*****************************************************************************
#endif

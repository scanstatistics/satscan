//*****************************************************************************
#ifndef __POPULATIONCATEGORIES_H
#define __POPULATIONCATEGORIES_H
//*****************************************************************************
#include "SatScan.h"
#include "UtilityFunctions.h"

/** This file abstracts "categories" of populations(.i.e covariate combinations).
    Each data and case record has a variable number of fields which specify
    arbitrary string values.  Each combination of these values defines a
    separate category, by which the data is stratified. */

class PopulationCategories {
  private:
    int                                 giNumberCovariates;            /** number covariates expected in each record
                                                                           - as defined by first non-blank record of population file */
    std::vector<std::string>            gvCovariateNames;              /** names of covariates */
    std::vector<std::vector<int> >      gvPopulationCategories;        /** vector of population categories
                                                                           - integers are indexes of covariate names */

    void                                Init() {giNumberCovariates=0;}

  public:
    PopulationCategories();
    ~PopulationCategories();

    void                                Display(BasePrint & PrintDirection) const;
    int                                 GetPopulationCategoryIndex(const std::vector<std::string>& vCategoryCovariates) const;
    const char                        * GetPopulationCategoryAsString(int iCategoryIndex, std::string & sBuffer) const;
    int                                 GetNumPopulationCategories() const {return (int)gvPopulationCategories.size();}
    int                                 GetNumPopulationCategoryCovariates() const {return giNumberCovariates;}
    int                                 MakePopulationCategory(StringParser & Parser, int iLineNumber, BasePrint & PrintDirection);
};
//*****************************************************************************
#endif

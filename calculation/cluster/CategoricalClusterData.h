//******************************************************************************
#ifndef __CategoricalClusterData_H
#define __CategoricalClusterData_H
//******************************************************************************
#include "AbstractClusterData.h"

class OrdinalCombinedCategory;
class OrdinalLikelihoodCalculator;

/**Abstract base class for all categorical cluster data. */
class AbstractCategoricalClusterData {
  public:
    AbstractCategoricalClusterData() {}
    virtual ~AbstractCategoricalClusterData() {}

    virtual void GetOrdinalCombinedCategories(const OrdinalLikelihoodCalculator& Calculator,
                                              std::vector<OrdinalCombinedCategory>& vCategoryContainer,
                                              unsigned int tSetIndex=0) const = 0;
};

/** Class representing accumulated data of spatial clustering organized by category. */
class CategoricalSpatialData : public AbstractSpatialClusterData, public AbstractCategoricalClusterData {
  public:
    CategoricalSpatialData(const DataSetInterface& Interface);
    CategoricalSpatialData(const AbstractDataSetGateway& DataGateway);
    virtual CategoricalSpatialData * Clone() const;
    virtual ~CategoricalSpatialData();

    //public data member -- public for speed considerations
    std::vector<count_t>        gvCasesPerCategory;      /* accumulated cases, organized by category */

    virtual void                Assign(const AbstractSpatialClusterData& rhs);
    CategoricalSpatialData    & operator=(const CategoricalSpatialData& rhs);

    virtual void                AddNeighborData(tract_t tNeighborIndex, const AbstractDataSetGateway& DataGateway, size_t tSetIndex=0);
    virtual double              CalculateLoglikelihoodRatio(AbstractLikelihoodCalculator& Calculator);
    virtual count_t             GetCaseCount(unsigned int tSetIndex=0) const;
    virtual count_t             GetCategoryCaseCount(unsigned int iCategoryIndex, unsigned int tSetIndex=0) const;
    virtual void                GetOrdinalCombinedCategories(const OrdinalLikelihoodCalculator& Calculator,
                                                             std::vector<OrdinalCombinedCategory>& vCategoryContainer,
                                                             unsigned int tSetIndex=0) const;
    virtual measure_t           GetMeasure(unsigned int tSetIndex=0) const;
    inline virtual void         InitializeData() {std::fill(gvCasesPerCategory.begin(), gvCasesPerCategory.end(), 0);}
};

/** Class representing accumulated data of temporal clustering partitioned by category.
    If instantiated through public constructors, points to already calculated
    purely temporal arrays supplied by DataInterface. The protected constructor
    is intended to permit instantiation through a derived class, where perhaps
    pointers will be allocated and data supplied by some other process. */
class CategoricalTemporalData : public AbstractTemporalClusterData, public AbstractCategoricalClusterData {
  public:
    CategoricalTemporalData(const DataSetInterface& Interface);
    CategoricalTemporalData(const AbstractDataSetGateway& DataGateway);
    virtual CategoricalTemporalData * Clone() const;
    virtual ~CategoricalTemporalData();

    virtual void                Assign(const AbstractTemporalClusterData& rhs);
    CategoricalTemporalData   & operator=(const CategoricalTemporalData& rhs);

    //public data member -- public for speed considerations
    count_t                  ** gppCategoryCases;        /* pointers to temporal arrays of category case data */
    std::vector<count_t>        gvCasesPerCategory;      /* accumulated cases, organized by category */

    virtual void                AddNeighborData(tract_t tNeighborIndex, const AbstractDataSetGateway& DataGateway, size_t tSetIndex=0);
    virtual unsigned int        GetAllocationSize() const;
    virtual count_t             GetCaseCount(unsigned int tSetIndex=0) const;
    virtual count_t             GetCategoryCaseCount(unsigned int iCategoryIndex, unsigned int tSetIndex=0) const;
    virtual void                GetOrdinalCombinedCategories(const OrdinalLikelihoodCalculator& Calculator,
                                                             std::vector<OrdinalCombinedCategory>& vCategoryContainer,
                                                             unsigned int tSetIndex=0) const;
    virtual measure_t           GetMeasure(unsigned int tSetIndex=0) const;
    virtual void                InitializeData() {std::fill(gvCasesPerCategory.begin(), gvCasesPerCategory.end(), 0);}
    virtual void                Reassociate(const DataSetInterface& Interface);
    virtual void                Reassociate(const AbstractDataSetGateway& DataGateway);
};

/** Class representing accumulated data of prospective spatial clustering partitioned by category.
    For spatial cluster data, in a prospective analysis, the supposed study
    period does not necessarily remain fixed but changes with the prospective
    end date. This class represents that 'spatial' data clustering. */
class CategoricalProspectiveSpatialData : public CategoricalTemporalData {
  private:
    void                                Init() {gpCategoryCasesHandler=0;}
    void                                Setup(const CSaTScanData& Data, const DataSetInterface& Interface);

  protected:
    unsigned int                        giNumTimeIntervals;       /** number of time intervals in study period */
    unsigned int                        giProspectiveStart;       /** index of prospective start date in DataInterface case array */

    TwoDimensionArrayHandler<count_t> * gpCategoryCasesHandler;

  public:
    CategoricalProspectiveSpatialData(const CSaTScanData& Data, const DataSetInterface& Interface);
    CategoricalProspectiveSpatialData(const CSaTScanData& Data, const AbstractDataSetGateway& DataGateway);
    CategoricalProspectiveSpatialData(const CategoricalProspectiveSpatialData& rhs);
    virtual CategoricalProspectiveSpatialData * Clone() const;
    virtual ~CategoricalProspectiveSpatialData();

    virtual void                        Assign(const AbstractTemporalClusterData& rhs);
    CategoricalProspectiveSpatialData & operator=(const CategoricalProspectiveSpatialData& rhs);

    virtual void                        AddNeighborData(tract_t tNeighborIndex, const AbstractDataSetGateway & DataGateway, size_t tSetIndex=0);
    virtual double                      CalculateLoglikelihoodRatio(AbstractLikelihoodCalculator& Calculator);
    virtual unsigned int                GetAllocationSize() const {return gpCategoryCasesHandler->Get2ndDimension();}
    virtual void                        InitializeData() {std::fill(gvCasesPerCategory.begin(), gvCasesPerCategory.end(), 0);
                                                          gpCategoryCasesHandler->Set(0);}
    virtual void                        Reassociate(const DataSetInterface& Interface) {/*nop*/}
    virtual void                        Reassociate(const AbstractDataSetGateway& DataGateway) {/*nop*/}
};

/** Class representing accumulated data of space-time clustering partitioned by category.*/
class CategoricalSpaceTimeData : public CategoricalTemporalData {
  private:
     void                                Init() {gpCategoryCasesHandler=0;}
     void                                Setup(const DataSetInterface& Interface);

  protected:
     TwoDimensionArrayHandler<count_t> * gpCategoryCasesHandler;

  public:
    CategoricalSpaceTimeData(const DataSetInterface& Interface);
    CategoricalSpaceTimeData(const AbstractDataSetGateway& DataGateway);
    CategoricalSpaceTimeData(const CategoricalSpaceTimeData& rhs);
    virtual CategoricalSpaceTimeData   * Clone() const;
    virtual ~CategoricalSpaceTimeData();

    virtual void                        Assign(const AbstractTemporalClusterData& rhs);
    CategoricalSpaceTimeData          & operator=(const CategoricalSpaceTimeData& rhs);

    virtual void                        AddNeighborData(tract_t tNeighborIndex, const AbstractDataSetGateway& DataGateway, size_t tSetIndex=0);
    virtual void                        InitializeData() {std::fill(gvCasesPerCategory.begin(), gvCasesPerCategory.end(), 0);
                                                          gpCategoryCasesHandler->Set(0);}
    virtual void                        Reassociate(const DataSetInterface& Interface) {/*nop*/}
    virtual void                        Reassociate(const AbstractDataSetGateway& DataGateway) {/*nop*/}
};
//******************************************************************************
#endif


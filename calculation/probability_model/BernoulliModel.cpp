//******************************************************************************
#include "SaTScan.h"
#pragma hdrstop
//******************************************************************************
#include "BernoulliModel.h"
#include "SSException.h"
#include "ClosedLoopData.h"

/** Constructor */
CBernoulliModel::CBernoulliModel() : CModel() {}

/** Destructor */                
CBernoulliModel::~CBernoulliModel() {}

/** calculates expected number of cases */
void CBernoulliModel::CalculateMeasure(RealDataSet& DataSet, const CSaTScanData& DataHub) {
    count_t               tTotalCases(0), tTotalControls(0), ** ppCases = DataSet.getCaseData().GetArray(), ** ppControls = DataSet.getControlData().GetArray();
    measure_t          ** ppMeasure, tTotalMeasure(0);

    /* pass const CSaTScanData& DataHub to get access to Julian intervalIndexToJulian(unsigned int intervalIdx) const;*/
    RealDataSet::CountsByWeekDay_t totalCasesByWeekDay, totalControlsByWeekDay;

    try {
        unsigned int j, jmax=DataSet.getLocationDimension(), i, imax=DataSet.getIntervalDimension();
        ppMeasure = DataSet.allocateMeasureData().GetArray();
        for (j=0; j < jmax; ++j) {
            tTotalCases    += ppCases[0][j];
            tTotalControls += ppControls[0][j];
            for (i=0; i < imax; ++i) {
                boost::gregorian::greg_weekday weekday = getWeekDay(DataHub.intervalIndexToJulian(i));
                totalCasesByWeekDay[weekday] += (i == imax-1 ?  ppCases[i][j] : ppCases[i][j] - ppCases[i+1][j]);
                totalControlsByWeekDay[weekday] += (i == imax-1 ?  ppControls[i][j] : ppControls[i][j] - ppControls[i+1][j]);
                ppMeasure[i][j]  = ppCases[i][j] + ppControls[i][j];
            }
            tTotalMeasure += ppMeasure[0][j];

            // Check to see if total case or control values have wrapped
            if (tTotalCases < 0)
                throw resolvable_error("Error: : The total number of cases in dataset %u is greater than the maximum allowed of %ld.\n",
                                       DataSet.getSetIndex(), std::numeric_limits<count_t>::max());
            if (tTotalControls < 0)
                throw resolvable_error("Error: The total number of controls in dataset %u is greater than the maximum allowed of %ld.\n",
                                       DataSet.getSetIndex(), std::numeric_limits<count_t>::max());
        }

        if (tTotalControls == 0)
            throw resolvable_error("Error: No controls found in data set %u.\n", DataSet.getSetIndex());

        DataSet.setTotalCases(tTotalCases);
        DataSet.setTotalControls(tTotalControls);
        DataSet.setTotalMeasure(tTotalMeasure);
        DataSet.setTotalPopulation(tTotalMeasure);

        count_t totalWeekCases = 0;
        for (RealDataSet::CountsByWeekDay_t::iterator itr=totalCasesByWeekDay.begin(); itr != totalCasesByWeekDay.end(); ++itr) {
            totalWeekCases += itr->second;
        }
        if (totalWeekCases != tTotalCases)
            throw resolvable_error("Error: Total week day cases does not equal total cases (%d != %d).\n", totalWeekCases, tTotalCases);
        count_t totalWeekControls = 0;
        for (RealDataSet::CountsByWeekDay_t::iterator itr=totalControlsByWeekDay.begin(); itr != totalControlsByWeekDay.end(); ++itr) {
            totalWeekControls += itr->second;
        }
        if (totalWeekControls != tTotalControls)
            throw resolvable_error("Error: Total week day controls does not equal total controls (%d != %d).\n", totalWeekControls, tTotalControls);
        DataSet.setTotalCasesByWeekDay(totalCasesByWeekDay);
        DataSet.setTotalControlByWeekDay(totalControlsByWeekDay);
    } catch (prg_exception &x) {
        x.addTrace("CalculateMeasure()","CBernoulliModel");
        throw;
    }
}

/** Returns population as defined in CCluster object. */
double CBernoulliModel::GetPopulation(size_t tSetIndex, const CCluster& Cluster, const CSaTScanData& DataHub) const {
  double                dPopulation=0.0;
  count_t               nNeighbor;
  measure_t           * pPTMeasure, ** ppMeasure;

  try {
    const ClosedLoopData * seasonalhub = dynamic_cast<const ClosedLoopData*>(&DataHub);
	if (!seasonalhub && DataHub.GetParameters().GetAnalysisType() == SEASONALTEMPORAL) throw prg_error("Unable to dynamic cast CSaTScanData to ClosedLoopData.", "GetPopulation()");

    switch (Cluster.GetClusterType()) {
     case PURELYTEMPORALCLUSTER            :
          pPTMeasure = DataHub.GetDataSetHandler().GetDataSet(tSetIndex).getMeasureData_PT();
		  if (seasonalhub) {
              int end_interval = std::min(Cluster.m_nLastInterval, seasonalhub->getExtendedPeriodStart());
			  dPopulation = pPTMeasure[Cluster.m_nFirstInterval] - (end_interval == seasonalhub->getExtendedPeriodStart() ? 0 : pPTMeasure[end_interval]);
			  dPopulation += pPTMeasure[0] - pPTMeasure[std::max(0, Cluster.m_nLastInterval - seasonalhub->getExtendedPeriodStart())];
		  } else {
			  dPopulation = pPTMeasure[Cluster.m_nFirstInterval] - pPTMeasure[Cluster.m_nLastInterval];
		  }
        break;
     case SPACETIMECLUSTER                 :
        if (Cluster.m_nLastInterval != DataHub.GetNumTimeIntervals()) {
          ppMeasure = DataHub.GetDataSetHandler().GetDataSet(tSetIndex).getMeasureData().GetArray();
          for (int i=1; i <= Cluster.GetNumTractsInCluster(); ++i) {
            nNeighbor = DataHub.GetNeighbor(Cluster.GetEllipseOffset(), Cluster.GetCentroidIndex(), i, Cluster.GetCartesianRadius());
            dPopulation += ppMeasure[Cluster.m_nFirstInterval][nNeighbor] - ppMeasure[Cluster.m_nLastInterval][nNeighbor];
          }
          break;
        }
     case PURELYSPATIALCLUSTER             :
     case PURELYSPATIALMONOTONECLUSTER     :
        ppMeasure = DataHub.GetDataSetHandler().GetDataSet(tSetIndex).getMeasureData().GetArray();
        for (int i=1; i <= Cluster.GetNumTractsInCluster(); ++i) {
          nNeighbor = DataHub.GetNeighbor(Cluster.GetEllipseOffset(), Cluster.GetCentroidIndex(), i, Cluster.GetCartesianRadius());
          dPopulation += ppMeasure[Cluster.m_nFirstInterval][nNeighbor];
        }
        break;
     case SPATIALVARTEMPTRENDCLUSTER       :
     case PURELYSPATIALPROSPECTIVECLUSTER  :
     default                               :
       throw prg_error("Unknown cluster type '%d'.","GetPopulation()", Cluster.GetClusterType());
    }
  }
  catch (prg_exception &x) {
    x.addTrace("GetPopulation()","CBernoulliModel");
    throw;
  }

  return dPopulation;
}


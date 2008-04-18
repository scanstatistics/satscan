//******************************************************************************
#include "SaTScan.h"
#pragma hdrstop
//******************************************************************************
#include "DataSetWriter.h"
#include "SSException.h"

/** Returns new AbstractDataSetWriter object. */
AbstractDataSetWriter * AbstractDataSetWriter::getNewDataSetWriter(const CParameters& Parameters) {
  switch(Parameters.GetProbabilityModelType()) {
    case POISSON              :
    case BERNOULLI            :
    case SPACETIMEPERMUTATION : return new DefaultDataSetWriter();
    case CATEGORICAL          :
    case ORDINAL              : return new OrdinalDataSetWriter();
    case EXPONENTIAL          : return new ExponentialDataSetWriter();
    case WEIGHTEDNORMAL       :
       throw prg_error("getNewDataSetReader() not implemented for Weighted Normal model.","getNewDataSetReader()");
    case NORMAL               :
       throw prg_error("getNewDataSetReader() not implemented for Normal model.","getNewDataSetReader()");
    case RANK                 :
       throw prg_error("getNewDataSetReader() not implemented for Exponential model.","getNewDataSetReader()");
    default : throw prg_error("Unknown model type '%d'.","getNewDataSetReader()", Parameters.GetProbabilityModelType());
  }
}

/** Writes case data to file stream based upon analysis type. */
void DefaultDataSetWriter::write(const DataSet& Set, const CParameters& Parameters) const {
  std::ofstream stream;

  //open output file
  stream.open(Parameters.GetSimulationDataOutputFilename().c_str(), std::ios::ate);
  if (!stream) throw resolvable_error("Error: Could not open the simulated data output file '%s'.\n",
                                      Parameters.GetSimulationDataOutputFilename().c_str());

  switch (Parameters.GetAnalysisType()) {
    case PURELYSPATIAL             :
     {count_t  * pSimCases(Set.getCaseData().GetArray()[0]);
      for (unsigned int t=0; t < Set.getLocationDimension(); ++t)
           stream << pSimCases[t] << " ";
      stream << std::endl;
     }
     break;
    case PURELYTEMPORAL            :
    case PROSPECTIVEPURELYTEMPORAL :
     {count_t  * pSimCases(Set.getCaseData_PT());
      for (unsigned int i=0; i < Set.getIntervalDimension(); ++i)
         stream << pSimCases[i] << " ";
      stream << std::endl;
     }
     break;
    case SPACETIME                 :
    case PROSPECTIVESPACETIME      :
    case SPATIALVARTEMPTREND       :
     {count_t  ** ppSimCases = Set.getCaseData().GetArray();
      for (unsigned int t=0; t < Set.getLocationDimension(); ++t) {
         for (unsigned int i=0; i < Set.getIntervalDimension(); ++i)
           stream << ppSimCases[i][t] << " ";
         stream << std::endl;
      }
     }
     break;
    default : throw prg_error("Unknown analysis type '%d'.","write()", Parameters.GetAnalysisType());
  }
  stream.close();
}

/** Writes category case data to file stream based upon analysis type. */
void OrdinalDataSetWriter::write(const DataSet& Set, const CParameters& Parameters) const {
  std::ofstream stream;

  //open output file
  stream.open(Parameters.GetSimulationDataOutputFilename().c_str(), std::ios::ate);
  if (!stream) throw resolvable_error("Error: Could not open the simulated data output file '%s'.\n",
                                      Parameters.GetSimulationDataOutputFilename().c_str());
  switch (Parameters.GetAnalysisType()) {
    case PURELYSPATIAL             :
     {for (CasesByCategory_t::const_iterator itr=Set.getCaseData_Cat().begin(); itr != Set.getCaseData_Cat().end(); ++itr) {
       count_t * pSimCases = (*itr)->GetArray()[0];
       for (unsigned int t=0; t < Set.getLocationDimension(); ++t)
         stream << pSimCases[t] << " ";
       stream << std::endl;
      }
     }
     break;
    case PURELYTEMPORAL            :
    case PROSPECTIVEPURELYTEMPORAL :
     {for (unsigned int c=0; c < Set.getCaseData_PT_Cat().Get1stDimension(); ++c) {
       count_t * pSimCases = Set.getCaseData_PT_Cat().GetArray()[c];
       for (unsigned int i=0; i < Set.getIntervalDimension(); ++i)
         stream << pSimCases[i] << " ";
       stream << std::endl;
      }
     }
     break;
    case SPACETIME                 :
    case PROSPECTIVESPACETIME      :
    case SPATIALVARTEMPTREND       :
     {for (CasesByCategory_t::const_iterator itr=Set.getCaseData_Cat().begin(); itr != Set.getCaseData_Cat().end(); ++itr) {
       count_t ** ppSimCases = (*itr)->GetArray();
       for (unsigned int t=0; t < Set.getLocationDimension(); ++t) {
         for (unsigned int i=0; i < Set.getIntervalDimension(); ++i)
           stream << ppSimCases[i][t] << " ";
         stream << std::endl;
       }
      }
     }
     break;
    default : throw prg_error("Unknown analysis type '%d'.","write()", Parameters.GetAnalysisType());
  }
  stream.close();
}

/** Writes case and measure data to file stream based upon analysis type. */
void ExponentialDataSetWriter::write(const DataSet& Set, const CParameters& Parameters) const {
  std::ofstream stream;

  //open output file
  stream.open(Parameters.GetSimulationDataOutputFilename().c_str(), std::ios::ate);
  if (!stream) throw resolvable_error("Error: Could not open the simulated data output file '%s'.\n",
                                      Parameters.GetSimulationDataOutputFilename().c_str());

  switch (Parameters.GetAnalysisType()) {
    case PURELYSPATIAL             :
      {count_t * pSimCases = Set.getCaseData().GetArray()[0];
       for (unsigned int t=0; t < Set.getLocationDimension(); ++t)
            stream << pSimCases[t] << " ";
       stream << std::endl;

       measure_t * pSimMeasure(Set.getMeasureData().GetArray()[0]);
       for (unsigned int t=0; t < Set.getLocationDimension(); ++t)
           stream << pSimMeasure[t] << " ";
       stream << std::endl;
      }
     break;
    case PURELYTEMPORAL            :
    case PROSPECTIVEPURELYTEMPORAL :
     {count_t * pSimCases = Set.getCaseData_PT();
      for (unsigned int i=0; i < Set.getIntervalDimension(); ++i)
         stream << pSimCases[i] << " ";
      stream << std::endl;

      measure_t * pSimMeasure = Set.getMeasureData_PT();
      for (unsigned int i=0; i < Set.getIntervalDimension(); ++i)
         stream << pSimMeasure[i] << " ";
      stream << std::endl;
     }
     break;
    case SPACETIME                 :
    case PROSPECTIVESPACETIME      :
    case SPATIALVARTEMPTREND       :
     {count_t ** ppSimCases = Set.getCaseData().GetArray();
      for (unsigned int t=0; t < Set.getLocationDimension(); ++t) {
         for (unsigned int i=0; i < Set.getIntervalDimension(); ++i)
            stream << ppSimCases[i][t] << " ";
         stream << std::endl;
      }
      measure_t ** ppSimMeasure = Set.getMeasureData().GetArray();
      for (unsigned int t=0; t < Set.getLocationDimension(); ++t) {
        for (unsigned int i=0; i < Set.getIntervalDimension(); ++i)
           stream << ppSimMeasure[i][t] << " ";
        stream << std::endl;
      }
     } 
     break;
    default : throw prg_error("Unknown analysis type '%d'.","write()", Parameters.GetAnalysisType());
  }
  stream.close();
}


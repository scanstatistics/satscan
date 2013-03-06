//***************************************************************************
#ifndef __IniParameterSpecification_H
#define __IniParameterSpecification_H
//***************************************************************************
#include "Parameters.h"
#include "Ini.h"

/** Defines interface for retrieving ini section and key name for parameters. */
class IniParameterSpecification {
  public:
    typedef std::vector<std::pair<const char*, const char*> > ParameterInfo_t;
    typedef std::map<ParameterType, std::pair<const char*, const char*> > MultipleParameterInfo_t;

  public:
    static const char         * NotUsed;
    static const char         * Input;
    static const char         * MultipleDataSets;
    static const char         * DataChecking;
    static const char         * NeighborsFile;
    static const char         * SpatialNeighbors;
    static const char         * Analysis;
    static const char         * SpatialWindow;
    static const char         * TemporalWindow;
    static const char         * Polygons;
    static const char         * SpaceAndTimeAdjustments;
    static const char         * Inference;
    static const char         * Output;
    static const char         * ClustersReported;
    static const char         * AdditionalOutput;

    static const char         * EllipticScan;
    static const char         * SequentialScan;
    static const char         * IsotonicScan;
    static const char         * PowerSimulations;
    static const char         * PowerEvaluations;
    static const char         * RunOptions;
    static const char         * BatchModeFeatures;
    static const char         * System;

    static const char         * InputFiles;
    static const char         * ScanningWindow;
    static const char         * TimeParameters;
    static const char         * OutputFiles;
    static const char         * AdvancedFeatures;

  protected:
    ParameterInfo_t             gvParameterInfo;
    MultipleParameterInfo_t     gvMultipleParameterInfo;

    void                        BuildPrimaryParameterList();
    void                        Build_4_0_x_ParameterList();
    void                        Build_5_0_x_ParameterList();
    void                        Build_5_1_x_ParameterList();
    void                        Build_6_0_x_ParameterList();
    void                        Build_6_1_x_ParameterList();
    void                        Build_7_0_x_ParameterList();
    void                        Build_8_0_x_ParameterList();
    void                        Build_8_2_x_ParameterList();
    void                        Build_9_0_x_ParameterList();
    void                        Build_9_2_x_ParameterList();

   public:
     IniParameterSpecification();
     IniParameterSpecification(const IniFile& SourceFile, CParameters& Parameters);
     virtual ~IniParameterSpecification();

    bool                GetParameterIniInfo(ParameterType eParameterType, const char ** sSectionName, const char ** sKey) const;
    bool                GetMultipleParameterIniInfo(ParameterType eParameterType, const char ** sSectionName, const char ** sKey) const;
};
//***************************************************************************
#endif

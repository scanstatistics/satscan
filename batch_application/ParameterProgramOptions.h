//***************************************************************************
#ifndef __ParameterProgramOptions_H
#define __ParameterProgramOptions_H
//***************************************************************************
#include "Parameters.h"
#include "ParameterFileAccess.h"
#include "IniParameterSpecification.h"
#include <boost/program_options.hpp>
#include "boost/tuple/tuple.hpp"
namespace po = boost::program_options;
#include <sstream>

class ParameterProgramOptions: public AbtractParameterFileAccess {
    public:
        typedef boost::tuple<po::options_description,bool,std::string>  ParamOpt_t; // (po::options_description, visible, extra text)
        typedef std::vector<ParamOpt_t> ParamOptContainer_t;

    protected:
        const IniParameterSpecification   _specifications;
        static int ADDITIONAL_DATASETS;
        static int POLYGON_REGIONS;

        const char * getOption(ParameterType e, bool withShortName=false) const;
        const char * GetParameterLabel(ParameterType e) const {return getOption(e);}

    public:
        ParameterProgramOptions(CParameters& Parameters, BasePrint& print): AbtractParameterFileAccess(Parameters, print) {}

        ParamOptContainer_t& getOptions(ParamOptContainer_t& opt_descriptions);
        void setParameterOverrides(const po::variables_map& vm);

        void listOptions(FILE * fp=stdout);

        bool Read(const char* szFilename) {return false;}
        void Write(const char * szFilename) {}
};
//***************************************************************************
#endif

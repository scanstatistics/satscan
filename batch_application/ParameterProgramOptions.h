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
        typedef boost::shared_ptr<ParamOpt_t> ParamOptItem_t;
        typedef std::vector<ParamOptItem_t> ParamOptContainer_t;

        const char * getOption(ParameterType e, bool withShortName=false) const;

    protected:
        const IniParameterSpecification   _specifications;
        static unsigned int ADDITIONAL_DATASETS;
        static unsigned int POLYGON_REGIONS;

        const char * GetParameterLabel(ParameterType e) const {return getOption(e);}

    public:
        ParameterProgramOptions(CParameters& Parameters, CParameters::CreationVersion version, BasePrint& Print);

        ParamOptContainer_t& getOptions(ParamOptContainer_t& opt_descriptions);
        void setParameterOverrides(const po::variables_map& vm);

        void listOptions(FILE * fp=stdout);

        bool Read(const char* szFilename);
        void Write(const char * szFilename);
};
//***************************************************************************
#endif

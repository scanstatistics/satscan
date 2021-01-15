//******************************************************************************
#ifndef __DataSetReader_H
#define __DataSetReader_H
//******************************************************************************
#include "DataSet.h"
#include <iostream>
#include <fstream>

/** Abstract data set reader */
class AbstractDataSetReader {
    public:
        AbstractDataSetReader() {}
        virtual ~AbstractDataSetReader() {}

        virtual void read(DataSet& Set, const CParameters& Parameters, const std::string& sourcename, unsigned int iSimulation) = 0;
        static AbstractDataSetReader * getNewDataSetReader(const CParameters& Parameters);
};

/** Default data set reader. Currently implemented for Poission, Bernoulli and STP models. */
class DefaultDataSetReader : public AbstractDataSetReader {
    public:
        DefaultDataSetReader() : AbstractDataSetReader() {}
        virtual ~DefaultDataSetReader() {}

        virtual void read(DataSet& Set, const CParameters& Parameters, const std::string& sourcename, unsigned int iSimulation);
};

/** Ordinal data set writer. */
class OrdinalDataSetReader : public AbstractDataSetReader {
    public:
        OrdinalDataSetReader() : AbstractDataSetReader() {}
        virtual ~OrdinalDataSetReader() {}

        virtual void read(DataSet& Set, const CParameters& Parameters, const std::string& sourcename, unsigned int iSimulation);
};
//******************************************************************************
#endif


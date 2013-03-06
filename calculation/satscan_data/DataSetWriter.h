//******************************************************************************
#ifndef DataSetWriterH
#define DataSetWriterH
//******************************************************************************
#include "DataSet.h"
#include <iostream>
#include <fstream>

/** Abstract data set writer. */
class AbstractDataSetWriter {
  public:
    virtual void        write(const DataSet& Set, const CParameters& Parameters, const std::string& filename) const = 0;
    static AbstractDataSetWriter * getNewDataSetWriter(const CParameters& Parameters);
};

/** Default data set writer, currently implemented for Poission, Bernoulli and STP models. */
class DefaultDataSetWriter : public AbstractDataSetWriter {
  public:
    virtual void        write(const DataSet& Set, const CParameters& Parameters, const std::string& filename) const;
};

/** Ordinal data set writer. */
class OrdinalDataSetWriter : public AbstractDataSetWriter {
  public:
    virtual void        write(const DataSet& Set, const CParameters& Parameters, const std::string& filename) const;
};

/** Exponential data set writer. */
class ExponentialDataSetWriter : public AbstractDataSetWriter {
  public:
    virtual void        write(const DataSet& Set, const CParameters& Parameters, const std::string& filename) const;
};
//******************************************************************************
#endif

//******************************************************************************
#ifndef __ShapeDataFileWriter_H
#define __ShapeDataFileWriter_H
//******************************************************************************
#include "Parameters.h"
#include "FileName.h"
#include "ShapeFile.h"

/** Shape file data writer. */
class ShapeDataFileWriter {
    private:
        void                        _setup(const CParameters& Parameters, const std::string& sFileExtension,  bool bAppend);

    protected:
        static const char         * SHAPE_FILE_EXT;
        std::auto_ptr<ShapeFile>    _file;

public:
        FileName                    _fileName;

   public :
        ShapeDataFileWriter(const CParameters& Parameters, const std::string& sFileExtension, bool bAppend = false);
        ~ShapeDataFileWriter();

        void	                    writeCoordinates(double x, double y);
};
//******************************************************************************
#endif

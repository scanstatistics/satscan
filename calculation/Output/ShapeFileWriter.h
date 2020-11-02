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
        void                        _setup(const CParameters& Parameters, const std::string& sFileExtension, int shapetype,  bool bAppend);

    protected:
        static const char         * SHAPE_FILE_EXT;
        std::auto_ptr<ShapeFile>    _file;

public:
        FileName                    _fileName;

   public :
        ShapeDataFileWriter(const CParameters& Parameters, const std::string& sFileExtension, int shapetype=SHPT_POINT, bool bAppend = false);
        ~ShapeDataFileWriter();

        int                         getShapeType() const {return _file->getType();}
        void                        writeCoordinates(double x, double y);
        void                        writePolygon(const std::vector<double>& polygonX, const std::vector<double>& polygonY);
		void                        writePolyline(const std::vector<double>& polylineX, const std::vector<double>& polylineY);
};
//******************************************************************************
#endif

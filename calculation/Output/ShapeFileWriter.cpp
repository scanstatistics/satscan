//******************************************************************************
#include "SaTScan.h"
#pragma hdrstop
//******************************************************************************
#include "ShapeFileWriter.h"
#include "AbstractDataFileWriter.h"


// TODO: It appears that shape filenames can not be like: NYCfever.col.dbf, NYCfever.col.shp, NYCfever.col.shx, NYCfever.col.prj
//       The names would need to be like:  NYCfever_col.dbf, NYCfever_col.shp, NYCfever_col.shx, NYCfever_col.prj
//       It would be really nice to be able to use .col output with other shp files. Perhaps we message can be given to user
//       indicating that file name is wrong. -- would need to change extensions from .col to _col ?


const char * ShapeDataFileWriter::SHAPE_FILE_EXT    = ".shp";

/** constructor */
ShapeDataFileWriter::ShapeDataFileWriter(const CParameters& Parameters, const std::string& sFileExtension, int shapetype, bool bAppend) {
    try {
        _setup(Parameters, sFileExtension, shapetype, bAppend);
    } catch (prg_exception& x) {
        x.addTrace("constructor()","ShapeDataFileWriter");
        throw;
    }
}

/** destructor */
ShapeDataFileWriter::~ShapeDataFileWriter() {}

/** internal setup - opens file stream */
void ShapeDataFileWriter::_setup(const CParameters& Parameters, const std::string& sFileExtension, int shapetype, bool bAppend) {
    std::string   buffer, ext(sFileExtension);

    try {
        ext += SHAPE_FILE_EXT;
        _fileName.setFullPath(Parameters.GetOutputFileName().c_str());
        _fileName.setExtension(ext.c_str());

        // open file stream for writing
        if (bAppend) {
            _file.reset(new ShapeFile(_fileName.getFullPath(buffer).c_str(), "r+b"));
            if (_file->getType() != shapetype)
                throw prg_error("Shape file has type %d but expecting type %d.","_setup()", _file->getType(), shapetype);
        } else {
            _file.reset(new ShapeFile(_fileName.getFullPath(buffer).c_str(), shapetype));
        }
    } catch (prg_exception& x) {
        x.addTrace("_setup()","ShapeDataFileWriter");
        throw;
    }
}

/** Write data to file stream. Note that caller is responsible for ensuring that
    record buffer structure agrees with file structure. */
void ShapeDataFileWriter::writeCoordinates(double x, double y) {
    try {
        _file->writePoint(x, y);
    } catch (prg_exception& x) {
        x.addTrace("writeCoordinates()","ShapeDataFileWriter");
        throw;
    }
}

/** Write data to file stream. Note that caller is responsible for ensuring that
    record buffer structure agrees with file structure. */
void ShapeDataFileWriter::writePolygon(const std::vector<double>& polygonX, const std::vector<double>& polygonY) {
    try {
        _file->writePolygon(polygonX, polygonY);
    } catch (prg_exception& x) {
        x.addTrace("writePolygon()","ShapeDataFileWriter");
        throw;
    }
}

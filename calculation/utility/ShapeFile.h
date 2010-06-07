//******************************************************************************
#ifndef __ShapeFile_H
#define __ShapeFile_H
//******************************************************************************
#include "shapefil.h"
//#include "contrib/shpgeo.h"
#include "ObservableRegion.h"

class ShapeFile {
    private:
        std::string     _filename;
        std::string     _mode;

        void            _createDefaultProjectionFile();

    public:
        ShapeFile(const char * fileName, const char * mode = "rb");
        ShapeFile(const char * fileName, int SHPType=SHPT_POINT);
        ~ShapeFile();

        int             getType() const;
        void            writePoint(double x, double y);
        void            writePolygon(const ConvexPolygonObservableRegion& polygon);

        static void     dump(const char * fileName);
        static void     test(const char * fileName);
};
//******************************************************************************
#endif

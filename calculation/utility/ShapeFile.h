//******************************************************************************
#ifndef __ShapeFile_H
#define __ShapeFile_H
//******************************************************************************
#include <locale> 
#include "shapefil.h"
#include "ObservableRegion.h"
#include <boost/algorithm/string/find.hpp>
#include <boost/algorithm/string/case_conv.hpp>
#include <boost/regex.hpp>

/* Parses the ESRI Projection Engine (PE) string.
 http://support.esri.com/en/knowledgebase/techarticles/detail/14056

 This class does not completely parse the PE string, just key elements to add in determining:
    PROJCS : projected coordinate system
    GEOGCS : geographic coordinate system
    DATUM : provides a frame of reference for measuring locations on the surface of the earth
    SPHEROID : approximation the shape of the earth

*/
class Projection {
    public:
        typedef std::pair<std::string, std::string> keyword_info_t;
        typedef boost::tuple<bool,std::string,std::string> utm_info_t; // <is UTM, Zone, Hemisphere>
        static const char * WGS_1984;

    protected:
        std::string _pe_string; // Projection Engine Well-Known Text String
        keyword_info_t _PROJCS; // Projected Coordinate System
        keyword_info_t _GEOGCS; // Geographic Coordinate System
        keyword_info_t _DATUM;  // Geographic Coordinate System Datum
        keyword_info_t _SPHEROID;
        keyword_info_t _PROJECTION;
        keyword_info_t _PARAMETER_NORTHING;
        keyword_info_t _PARAMETER_EASTING;

        void parsePE(const std::string& pe_string);
        keyword_info_t & readKeyWordInfo(const std::string& source, const std::string& keyword, keyword_info_t& info);
        keyword_info_t & readKeyWordInfoWithIdentifier(const std::string& source, const std::string& keyword, const std::string& identifier, keyword_info_t& info);

    public:
        Projection() {}
        Projection(const std::string& pe_string) : _pe_string(pe_string) { parse(_pe_string); }

        utm_info_t guessUTM() const;
        bool isProjectedCoordinateSystem() const {
            /* Returns true if 'PROJCS' keyword is defined. */
            return (_PROJCS.first.size() > 0);
        }
        void parse(const std::string& pe_string) {
            _pe_string = pe_string;
            readKeyWordInfo(_pe_string, "PROJCS", _PROJCS);
            readKeyWordInfo(_pe_string, "GEOGCS", _GEOGCS);
            readKeyWordInfo(_GEOGCS.second, "DATUM", _DATUM);
            readKeyWordInfo(_DATUM.second, "SPHEROID", _SPHEROID);
            readKeyWordInfo(_pe_string, "PROJECTION", _PROJECTION);
            readKeyWordInfoWithIdentifier(_pe_string, "PARAMETER", "false_northing", _PARAMETER_NORTHING);
            readKeyWordInfoWithIdentifier(_pe_string, "PARAMETER", "false_easting", _PARAMETER_EASTING);
        }
};

class ShapeFile {
    public:
        // modified structs/functions from shapelib contrib shpgeo.c
        typedef struct {double x; double y;} PT;

        static int SHPD_POINT;
        static int SHPD_LINE;
        static int SHPD_AREA;
        static int SHPD_Z;
        static int SHPD_MEASURE;

        static int SHPDimension(int SHPType);
        static PT SHPCentrd_2d(SHPObject *psCShape);
        static int RingCentroid_2d(int nVertices, double *a, double *b, ShapeFile::PT *C, double *Area);

    private:
        std::string     _filename;
        std::string     _mode;
        std::auto_ptr<Projection> _projection;

        void            _createDefaultProjectionFile();

    public:
        ShapeFile(const char * fileName, const char * mode = "rb", bool createprj=true);
        ShapeFile(const char * fileName, int SHPType=SHPT_POINT);

        int             getEntityCount() const;
        void            getShapeAsXY(int entityIdx, double& x, double& y);
        int             getType() const;
        bool            hasProjection() const {return _projection.get() != 0;}
        const Projection & getProjection() const {return *_projection;}
        void            writePoint(double x, double y);
        void            writePolygon(const ConvexPolygonObservableRegion& polygon);
        void            writePolygon(const std::vector<double>& polygonX, const std::vector<double>& polygonY);

        static std::string& getProjectionFilename(const std::string& shapefile, std::string& projectionfile);
        static const char * getTypeAsString(int type);
        static void     dump(const char * fileName);
        static void     test(const char * fileName);
};
//******************************************************************************
#endif

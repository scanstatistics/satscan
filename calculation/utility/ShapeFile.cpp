//******************************************************************************
#include "SaTScan.h"
#pragma hdrstop
//******************************************************************************
#include "ShapeFile.h"
#include "SSException.h"
#include "FileName.h"
#include "UtilityFunctions.h"
#include <iostream>
#include <fstream>

/* World Geodetic System OF 1984 - http://en.wikipedia.org/wiki/World_Geodetic_System */
const char * Projection::WGS_1984 = "GEOGCS[\"GCS_WGS_1984\",DATUM[\"D_WGS_1984\",SPHEROID[\"WGS_1984\",6378137, 298.257223563]],PRIMEM[\"Greenwich\",0],UNIT[\"Degree\",0.0174532925199433]]";

Projection::utm_info_t Projection::guessUTM() const {
    /* Try guessing a hemisphere and zone from projection name -- format is typically like: PROJCS["NAD27(76) / UTM zone 15N" ... */
    boost::regex utm_info("zone[\\s_]*(\\d{1,2})[\\s_]*([NS])", boost::regex_constants::icase);
    boost::cmatch matches;
    if (boost::regex_search(_PROJCS.first.c_str(), matches, utm_info)) {
        return utm_info_t(true, std::string(matches[1].first, matches[1].second),std::string(matches[2].first, matches[2].second));
    }
    return utm_info_t(false, std::string(""),std::string(""));
}

Projection::keyword_info_t& Projection::readKeyWordInfo(const std::string& source, const std::string& keyword, keyword_info_t& info) {
    info.first.clear();
    info.second.clear();
    std::string::size_type pos = source.find(keyword.c_str());
    std::string::size_type end = source.size();
    bool isSingular=false;
    if (pos != std::string::npos) {
        pos += keyword.size() + 1; //  move beyond keyword and open bracket
        while (source.at(pos) != ',' && pos < end) {
                ++pos;
                if (source.at(pos) != '"' && source.at(pos) != ']') {
                    info.first += source.at(pos);
                } 
                isSingular |= (source.at(pos) == ']'); // found closing bracket, so don't expect meta data
        }
        //info.first.pop_back();
        info.first.erase(info.first.size() - 1);
        if (!isSingular) {
            // try to read the meta data for this keyword
            int leftBracket=1/* opening bracket of keyword */, rightBracket=0;
            // find the next open bracket
            while ((++pos) < end && (leftBracket == 1 || leftBracket != rightBracket)) {
                info.second += source.at(pos);
                if (source.at(pos) == '[') ++leftBracket;
                if (source.at(pos) == ']') ++rightBracket;
            }
            //if (info.second.size() > 0 && info.second.back() == ']') {
            if (info.second.size() > 0 && *(info.second.end() - 1) == ']') {
                // remove last bracket -- it belongs to the keyword
                //info.second.pop_back();
                info.second.erase(info.second.size() - 1);
            }
        }
    }
    return info;
}

Projection::keyword_info_t & Projection::readKeyWordInfoWithIdentifier(const std::string& source, const std::string& keyword, const std::string& identifier, keyword_info_t& info) {
    info.first.clear();
    info.second.clear();
    std::string searchIdentifier;
    std::string::size_type end = source.size();

    // search for a PARAMETER with 'keyword' as label
    std::string::size_type pos = 0;
    if (pos != std::string::npos) {
        bool isSingular=false;
        bool found=false;
        while (!found && pos < end && !isSingular) {
            searchIdentifier.clear();
            pos = source.find(keyword.c_str(), pos);
            if (pos == source.npos) break;
            pos += keyword.size() + 1; //  move beyond keyword and open bracket
            while (source.at(pos) != ',' && pos < end && !isSingular) {
                ++pos;
                if (source.at(pos) != '"' && source.at(pos) != ']') {
                    searchIdentifier += source.at(pos);
                } 
                isSingular |= (source.at(pos) == ']'); // found closing bracket, so don't expect value
            }
            //searchIdentifier.pop_back();
            searchIdentifier.erase(searchIdentifier.size() - 1);
            found = (searchIdentifier == identifier);
        }
        if (found && !isSingular) {
            info.first = identifier;
            // try to read value
            int leftBracket=1/* opening bracket of keyword */, rightBracket=0;
            // find the next open bracket
            while ((++pos) < end && (leftBracket != rightBracket)) {
                info.second += source.at(pos);
                if (source.at(pos) == '[') ++leftBracket;
                if (source.at(pos) == ']') ++rightBracket;
            }
            if (info.second.size() > 0 && *(info.second.end() - 1) == ']') {
            //if (info.second.size() > 0 && info.second.back() == ']') {
                // remove last bracket -- it belongs to the keyword
                info.second.erase(info.second.size() - 1);
                //info.second.pop_back();
            }
        }
    }
    return info;
}

ShapeFile::ShapeFile(const char * fileName, const char * mode, bool createprj) : _filename(fileName), _mode(mode) {
    if (createprj)
        _createDefaultProjectionFile();
    else {
        // try to read a associated projection file
        std::string projection;
        if (ValidateFileAccess(getProjectionFilename(_filename, projection))) {
            std::ifstream filestream(projection.c_str());
            if (filestream) {
                std::stringstream contents;
                contents << filestream.rdbuf();
                _projection.reset(new Projection((contents.str())));
            }
        }
    }
}

ShapeFile::ShapeFile(const char * fileName, int SHPType) : _filename(fileName), _mode("r+b") {
    SHPClose(SHPCreate(_filename.c_str(), SHPType));
    _createDefaultProjectionFile();
}

/* Returns projection filename for current shapefile. */
std::string& ShapeFile::getProjectionFilename(const std::string& shapefile, std::string& projectionfile) {
    FileName projFilename(shapefile.c_str());
    projFilename.setExtension(".prj");
    projFilename.getFullPath(projectionfile);
    return projectionfile;
}

/** Create default projection file.
    TODO: Can we better guess based upon range of points? */
void ShapeFile::_createDefaultProjectionFile() {
    std::string projection;
    getProjectionFilename(_filename, projection);
    // create default projection file if none exists
    if (!ValidateFileAccess(projection)) {
        FILE * pFile = fopen(projection.c_str(), "w");
        if (pFile == NULL)
            throw resolvable_error("Error: Unable to create projection file '%s'.\n", projection.c_str());
        fprintf(pFile, Projection::WGS_1984);
        fclose(pFile);
        _projection.reset(new Projection((Projection::WGS_1984)));
    }
}

/* Returns shape type as human-readable string. */
const char * ShapeFile::getTypeAsString(int type) {
    switch (type) {
        case  SHPT_NULL        :  return "Null shape";
        case  SHPT_POINT       :  return "Point";
        case  SHPT_ARC         :  return "Polyline";
        case  SHPT_POLYGON     :  return "Polygon";
        case  SHPT_MULTIPOINT  :  return "MultiPoint";
        case  SHPT_POINTZ      :  return "PointZ";
        case  SHPT_ARCZ        :  return "PolylineZ";
        case  SHPT_POLYGONZ    :  return "PolygonZ";
        case  SHPT_MULTIPOINTZ :  return "MultiPointZ";
        case  SHPT_POINTM      :  return "PointM";
        case  SHPT_ARCM        :  return "PolylineM";
        case  SHPT_POLYGONM    :  return "PolygonM";
        case  SHPT_MULTIPOINTM :  return "MultiPointM";
        case  SHPT_MULTIPATCH  :  return "MultiPatch";
        default : return "";
    }
}

/* Returns the number of entities defined in shapefile. */
int ShapeFile::getEntityCount() const {
    SHPHandle shp = SHPOpen(_filename.c_str(), _mode.c_str());
    if (!shp) throw prg_error("Unable to open file.","getEntityCount()");
    int entities;
    SHPGetInfo(shp, &entities, NULL, NULL, NULL);
    SHPClose(shp);
    return entities;
}

/* Attempts to read entity as X/Y values, performing any necessary calculations to get geometric centroid. */
void ShapeFile::getShapeAsXY(int entityIdx, double& x, double& y) {
    // The contrib c files will likley be useful here.
    SHPHandle shp = SHPOpen(_filename.c_str(), _mode.c_str());
    if (!shp) throw prg_error("Unable to open file.","getShapeAsXY()");
    SHPObject * pShape = SHPReadObject(shp, entityIdx);
    switch (getType()) {
        case SHPT_POINT : 
            assert(pShape->nVertices == 1);
            x = pShape->padfX[0];
            y = pShape->padfY[0];
            break;
        case SHPT_POLYGON : 
        default :
            PT pt = SHPCentrd_2d(pShape);
            x = pt.x;
            y = pt.y;
    }
    SHPDestroyObject(pShape);
    SHPClose(shp);
}

int ShapeFile::getType() const {
    int shapeType;

    SHPHandle shp = SHPOpen(_filename.c_str(), _mode.c_str());
    if (!shp) throw prg_error("Unable to open file.","getType()");
    SHPGetInfo(shp, NULL, &shapeType, NULL, NULL);
    SHPClose(shp);

    return shapeType;
}

void ShapeFile::writePoint(double x, double y) {
    if (getType() != SHPT_POINT)
        throw prg_error("File type (%d) does not match SHPT_POINT!","writePoint()",getType());

    SHPHandle shp = SHPOpen(_filename.c_str(), _mode.c_str());
    if (!shp) throw prg_error("Unable to open file.","writePoint()");
    SHPObject * pShape = SHPCreateSimpleObject(SHPT_POINT, 1, (double*)&(x), (double*)&(y), NULL);
    SHPWriteObject(shp, -1, pShape);
    SHPDestroyObject(pShape);
    SHPClose(shp);
}

void ShapeFile::writePolygon(const ConvexPolygonObservableRegion& polygon) {
    throw prg_error("ShapeFile::writePolygon(const ConvexPolygonObservableRegion&) not implemented yet!","writePolygon()");
}

void ShapeFile::writePolygon(const std::vector<double>& polygonX, const std::vector<double>& polygonY) {
    if (polygonX.size() != polygonY.size())
        throw prg_error("Number of X coordinates (%u) does not equal number of Y coordinates (%u)!","writePolygon()",polygonX.size(),polygonY.size());
    if (getType() != SHPT_POLYGON)
        throw prg_error("File type (%d) does not match SHPT_POLYGON!","writePolygon()",getType());

    SHPHandle shp = SHPOpen(_filename.c_str(), _mode.c_str());
    if (!shp) throw prg_error("Unable to open file.","writePolygon()");
    SHPObject * pShape = SHPCreateSimpleObject(SHPT_POLYGON, polygonX.size(), const_cast<double*>(&(polygonX[0])), const_cast<double*>(&(polygonY[0])), NULL);
    SHPWriteObject(shp, -1, pShape);
    SHPDestroyObject(pShape);
    SHPClose(shp);
}

/////////////////////////////////////////


void ShapeFile::dump(const char * fileName) {
    int   nShapeType, nEntities, i, iPart, bValidate = 0,nInvalidCount=0;
    const char  *pszPlus;
    double  adfMinBound[4], adfMaxBound[4];

    std::string s(fileName); s += ".dump";
    FILE* pFile;
    if ((pFile = fopen(s.c_str(), "w")) == NULL)
        throw resolvable_error("Error: Unable to open dump file '%s'.\n", s.c_str());

    /* -------------------------------------------------------------------- */
    /*      Open the passed shapefile.                                      */
    /* -------------------------------------------------------------------- */
    SHPHandle hSHP = SHPOpen( fileName, "rb" );
    if( hSHP == NULL ) {
        fprintf(pFile, "Unable to open:%s\n", fileName );
      return;
    }

    /* -------------------------------------------------------------------- */
    /*      Print out the file bounds.                                      */
    /* -------------------------------------------------------------------- */
    SHPGetInfo( hSHP, &nEntities, &nShapeType, adfMinBound, adfMaxBound );

    fprintf(pFile, "Shapefile Type: %s   # of Shapes: %d\n\n", SHPTypeName( nShapeType ), nEntities );

    fprintf(pFile, "File Bounds: (%12.3f,%12.3f,%g,%g)\n"
            "         to  (%12.3f,%12.3f,%g,%g)\n",
            adfMinBound[0],
            adfMinBound[1],
            adfMinBound[2],
            adfMinBound[3],
            adfMaxBound[0],
            adfMaxBound[1],
            adfMaxBound[2],
            adfMaxBound[3] );

    /* -------------------------------------------------------------------- */
    /*  Skim over the list of shapes, printing all the vertices.  */
    /* -------------------------------------------------------------------- */
    for( i = 0; i < nEntities; i++ ) {
      int   j;
        SHPObject *psShape;

      psShape = SHPReadObject( hSHP, i );

      fprintf(pFile, "\nShape:%d (%s)  nVertices=%d, nParts=%d\n"
                "  Bounds:(%12.3f,%12.3f, %g, %g)\n"
                "      to (%12.3f,%12.3f, %g, %g)\n",
              i, SHPTypeName(psShape->nSHPType),
                psShape->nVertices, psShape->nParts,
                psShape->dfXMin, psShape->dfYMin,
                psShape->dfZMin, psShape->dfMMin,
                psShape->dfXMax, psShape->dfYMax,
                psShape->dfZMax, psShape->dfMMax );

      for( j = 0, iPart = 1; j < psShape->nVertices; j++ ){
            const char  *pszPartType = "";

            if( j == 0 && psShape->nParts > 0 )
                pszPartType = SHPPartTypeName( psShape->panPartType[0] );

          if( iPart < psShape->nParts && psShape->panPartStart[iPart] == j ) {
                pszPartType = SHPPartTypeName( psShape->panPartType[iPart] );
            iPart++;
            pszPlus = "+";
          }
          else
              pszPlus = " ";

          fprintf(pFile, "   %s (%12.3f,%12.3f, %g, %g) %s \n",
                   pszPlus,
                   psShape->padfX[j],
                   psShape->padfY[j],
                   psShape->padfZ[j],
                   psShape->padfM[j],
                   pszPartType );
      }

        if( bValidate )
        {
            int nAltered = SHPRewindObject( hSHP, psShape );

            if( nAltered > 0 )
            {
                fprintf(pFile, "  %d rings wound in the wrong direction.\n",
                        nAltered );
                nInvalidCount++;
            }
        }

        SHPDestroyObject( psShape );
    }

    SHPClose( hSHP );

    if( bValidate )
    {
        fprintf(pFile, "%d object has invalid ring orderings.\n", nInvalidCount );
    }
    fclose(pFile);
}

void ShapeFile::test(const char * fileName) {
    SHPHandle shp = SHPCreate ( fileName, SHPT_POINT );

    double * padfX = (double *) malloc(sizeof(double) * 1);
    double * padfY = (double *) malloc(sizeof(double) * 1);

    for (int i=1; i <= 10; ++i) {
        padfX[0] = 12.0 * i;
        padfY[0] = 24.0 * i;
        SHPObject * pShape = SHPCreateSimpleObject(SHPT_POINT, 1, padfX, padfY, NULL);
        SHPWriteObject(shp, -1, pShape);
        SHPDestroyObject(pShape);
    }

    free( padfX );
    free( padfY );
    SHPClose( shp );
}

int ShapeFile::SHPD_POINT = 1;
int ShapeFile::SHPD_LINE = 2;
int ShapeFile::SHPD_AREA = 4;
int ShapeFile::SHPD_Z = 8;
int ShapeFile::SHPD_MEASURE = 16;

/* **************************************************************************
 * SHPDimension
 *
 * Return the Dimensionality of the SHPObject
 * a handy utility function
 *
 * **************************************************************************/
int ShapeFile::SHPDimension(int SHPType) {
    int dimension=0;
    switch ( SHPType ) {
      case  SHPT_POINT       :  dimension = SHPD_POINT; break;
      case  SHPT_ARC         :  dimension = SHPD_LINE; break;
      case  SHPT_POLYGON     :  dimension = SHPD_AREA; break;
      case  SHPT_MULTIPOINT  :  dimension = SHPD_POINT; break;
      case  SHPT_POINTZ      :  dimension = SHPD_POINT | SHPD_Z; break;
      case  SHPT_ARCZ        :  dimension = SHPD_LINE | SHPD_Z; break;
      case  SHPT_POLYGONZ    :  dimension = SHPD_AREA | SHPD_Z; break;
      case  SHPT_MULTIPOINTZ :  dimension = SHPD_POINT | SHPD_Z; break;
      case  SHPT_POINTM      :  dimension = SHPD_POINT | SHPD_MEASURE; break;
      case  SHPT_ARCM        :  dimension = SHPD_LINE | SHPD_MEASURE; break;
      case  SHPT_POLYGONM    :  dimension = SHPD_AREA | SHPD_MEASURE; break;
      case  SHPT_MULTIPOINTM :  dimension = SHPD_POINT | SHPD_MEASURE; break;
      case  SHPT_MULTIPATCH  :  dimension = SHPD_AREA; break;
    }
   return ( dimension );
}

/* **************************************************************************
 * SHPCentrd_2d
 *
 * Return the single mathematical / geometric centroid of a potentially
 * complex/compound SHPObject
 *
 * reject non area SHP Types
 *
 * **************************************************************************/
ShapeFile::PT ShapeFile::SHPCentrd_2d(SHPObject *psCShape) {
    int ring, ringPrev, ring_nVertices, rStart;
    double Area=0, ringArea;
    PT ringCentrd, C={0.0,0.0};

    if ( !(SHPDimension (psCShape->nSHPType) & SHPD_AREA) ) {
        C.x = std::numeric_limits<double>::quiet_NaN();
        C.y = std::numeric_limits<double>::quiet_NaN();
        return C;
    }
    #ifdef DEBUG
    printf ("for Object with %d vtx, %d parts [ %d, %d] \n", psCShape->nVertices, psCShape->nParts, psCShape->panPartStart[0],psCShape->panPartStart[1]);
    #endif
    /* for each ring in compound / complex object calc the ring cntrd    */
    ringPrev = psCShape->nVertices;
    for (ring = (psCShape->nParts - 1); ring >= 0; ring--) {
        rStart = psCShape->panPartStart[ring];
        ring_nVertices = ringPrev - rStart;
        RingCentroid_2d(ring_nVertices, (double*) &(psCShape->padfX [rStart]), (double*) &(psCShape->padfY [rStart]), &ringCentrd, &ringArea);
        #ifdef DEBUG
        printf ("(SHPCentrd_2d)  Ring %d, vtxs %d, area: %f, ring centrd %f, %f \n", ring, ring_nVertices, ringArea, ringCentrd.x, ringCentrd.y);
        #endif
        /* use Superposition of these rings to build a composite Centroid    */
        /* sum the ring centrds * ringAreas,  at the end divide by total area  */
        C.x +=  ringCentrd.x * ringArea;
        C.y +=  ringCentrd.y * ringArea;
        Area += ringArea;
        ringPrev = rStart;
    }
    /* hold on the division by AREA until were at the end */
    C.x = C.x / Area;
    C.y = C.y / Area;
    #ifdef DEBUG
    printf ("SHPCentrd_2d) Overall Area: %f, Centrd %f, %f \n", Area, C.x, C.y);
    #endif
    return C;
}

/* **************************************************************************
 * RingCentroid_2d
 *
 * Return the mathematical / geometric centroid of a single closed ring
 *
 * **************************************************************************/
int ShapeFile::RingCentroid_2d(int nVertices, double *a, double *b, ShapeFile::PT *C, double *Area) {
    int   iv,jv;
    int   sign_x, sign_y;
    double  dy_Area, dx_Area, Cx_accum, Cy_accum, ppx, ppy;
    double  x_base, y_base, x, y;

    /* the centroid of a closed Ring is defined as
     *
     *      Cx = sum (cx * dArea ) / Total Area
     *  and
     *      Cy = sum (cy * dArea ) / Total Area
    */
    x_base = a[0];
    y_base = b[0];

    Cy_accum = 0.0;
    Cx_accum = 0.0;

    ppx = a[1] - x_base;
    ppy = b[1] - y_base;
    *Area = 0;

    /* Skip the closing vector */
    for ( iv = 2; iv <= nVertices - 2; iv++ ) {
        x = a[iv] - x_base;
        y = b[iv] - y_base;

        /* calc the area and centroid of triangle built out of an arbitrary   */
        /* base_point on the ring and each successive pair on the ring      */

        /* Area of a triangle is the cross product of its defining vectors    */
        /* Centroid of a triangle is the average of its vertices        */

        dx_Area =  ((x * ppy) - (y * ppx)) * 0.5;
        *Area += dx_Area;

        Cx_accum += ( ppx + x ) * dx_Area;
        Cy_accum += ( ppy + y ) * dx_Area;
        #ifdef DEBUG2
        printf("(ringcentrd_2d)  Pp( %f, %f), P(%f, %f)\n", ppx, ppy, x, y);
        printf("(ringcentrd_2d)    dA: %f, sA: %f, Cx: %f, Cy: %f \n", dx_Area, *Area, Cx_accum, Cy_accum);
        #endif
        ppx = x;
        ppy = y;
    }

    #ifdef DEBUG2
    printf("(ringcentrd_2d)  Cx: %f, Cy: %f \n", ( Cx_accum / ( *Area * 3) ), ( Cy_accum / (*Area * 3) ));
    #endif

    /* adjust back to world coords */
    C->x = ( Cx_accum / ( *Area * 3)) + x_base;
    C->y = ( Cy_accum / ( *Area * 3)) + y_base;

    return 1;
}

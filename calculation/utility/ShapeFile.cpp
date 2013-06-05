//******************************************************************************
#include "SaTScan.h"
#pragma hdrstop
//******************************************************************************
#include "ShapeFile.h"
#include "SSException.h"
#include "FileName.h"
#include "UtilityFunctions.h"


ShapeFile::ShapeFile(const char * fileName, const char * mode) : _filename(fileName), _mode(mode) {
    _createDefaultProjectionFile();
}

ShapeFile::ShapeFile(const char * fileName, int SHPType) : _filename(fileName), _mode("r+b") {
    SHPClose(SHPCreate(_filename.c_str(), SHPType));
    _createDefaultProjectionFile();
}

ShapeFile::~ShapeFile() {}

/** Create default projection file. 
    TODO: Can we better guess based upon range of points? */
void ShapeFile::_createDefaultProjectionFile() {
    std::string     buffer;
    FileName    projFilename(_filename.c_str());

    projFilename.setExtension(".prj");
    projFilename.getFullPath(buffer);

    // create default projection file if none exists
    if (!ValidateFileAccess(buffer)) {
        FILE * pFile = fopen(buffer.c_str(), "w");
        if (pFile == NULL)
            throw resolvable_error("Error: Unable to create projection file '%s'.\n", buffer.c_str());
        fprintf(pFile, "GEOGCS[\"GCS_WGS_1984\",DATUM[\"D_WGS_1984\",SPHEROID[\"WGS_1984\",6378137, 298.257223563]],PRIMEM[\"Greenwich\",0],UNIT[\"Degree\",0.0174532925199433]]");
        fclose(pFile);
    }
}

int ShapeFile::getType() const {
    int shapeType; 

    SHPHandle shp = SHPOpen(_filename.c_str(), _mode.c_str());
    SHPGetInfo(shp, NULL, &shapeType, NULL, NULL);
    SHPClose(shp);

    return shapeType;
}

void ShapeFile::writePoint(double x, double y) {
    if (getType() != SHPT_POINT) 
        throw prg_error("File type (%d) does not match SHPT_POINT!","writePoint()",getType());

    SHPHandle shp = SHPOpen(_filename.c_str(), _mode.c_str());
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
    SHPObject * pShape = SHPCreateSimpleObject(SHPT_POLYGON, polygonX.size(), const_cast<double*>(&(polygonX[0])), const_cast<double*>(&(polygonY[0])), NULL); 
    SHPWriteObject(shp, -1, pShape);
    SHPDestroyObject(pShape);
    SHPClose(shp);
}

/////////////////////////////////////////


void ShapeFile::dump(const char * fileName) {
    int		nShapeType, nEntities, i, iPart, bValidate = 0,nInvalidCount=0;
    const char 	*pszPlus;
    double 	adfMinBound[4], adfMaxBound[4];

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
    /*	Skim over the list of shapes, printing all the vertices.	*/
    /* -------------------------------------------------------------------- */
    for( i = 0; i < nEntities; i++ ) {
	    int		j;
        SHPObject	*psShape;

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
            const char	*pszPartType = "";

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

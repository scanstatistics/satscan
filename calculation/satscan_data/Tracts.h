//*****************************************************************************
#ifndef __TRACTS_H
#define __TRACTS_H
//*****************************************************************************
#include "SaTScan.h"
#include "JulianDates.h"

/**********************************************************************
 file: Tracts.h
 Header file for tinfo.c, which abstracts tract, population & case
 count information
 **********************************************************************/

class TractHandler;
class CompareTractDescriptorIdentifier;

/** tract record              */
class TractDescriptor {
  //TractHandler and CompareTractDescriptorIdentifier are friend classes and
  //all members/functions are private so that TractHandler is the only interface
  //for interacting with tracts. Accessing this object directly might violate
  //relationship with friend classes.
  friend class TractHandler;
  friend class CompareTractDescriptorIdentifier;

  private:
    char                              * gsTractIdentifiers;
    double                            * gpCoordinates;             /* coordinates                   */

    //Restricted to TractDescriptor - only this class should call these methods.
    void                                Init() {gsTractIdentifiers=0; gpCoordinates=0;}
    void                                Setup(const char * sTractIdentifier, const double* pCoordinates, int iDimensions);

    void                                AddTractIdentifier(const char * sTractIdentifier);
    void                                Combine(const TractDescriptor * pCategoryDescriptor, const TractHandler & theTractHandler);
    bool                                CompareCoordinates(const double * pCoordinates, int iDimensions) const;
    bool                                CompareCoordinates(const TractDescriptor & Descriptor, const TractHandler & theTractHandler) const;
    const double                      *	GetCoordinates() const {return gpCoordinates;}
    double                            	GetCoordinatesAtDimension(int iDimension, const TractHandler & theTractHandler) const;
    int                                 GetNumTractIdentifiers() const;
    const char                        * GetTractIdentifier() const {return gsTractIdentifiers;}
    const char 			      * GetTractIdentifier(int iTractIdentifierIndex, std::string & sIndentifier);
    void                                GetTractIdentifiers(std::vector<std::string>& vIdentifiers) const;
    void                                RetrieveCoordinates(TractHandler const & theTractHandler, std::vector<double> & vRepository) const;
    void                                SetCoordinates(const double* pCoordinates, int iDimensions);
    void				SetTractIdentifier(const char * sTractIdentifier);

    TractDescriptor(const char * sTractIdentifier, const double* pCoordinates, int iDimensions);

  public:
    ~TractDescriptor();
};

/** Function object used to compare CompareTractDescriptor objects by identifier. */
class CompareTractDescriptorIdentifier {
  public:
    CompareTractDescriptorIdentifier() {}

    bool operator() (TractDescriptor * lhs, TractDescriptor * rhs)
                   {
                   return (strcmp(lhs->gsTractIdentifiers, rhs->gsTractIdentifiers) < 0);
                   }
};

class CSaTScanData;
class TractHandler {
  private:
    ZdPointerVector<TractDescriptor>             gvTractDescriptors;
    int                                          nDimensions;
    TractDescriptor                            * gpSearchTractDescriptor;
    std::map<std::string,TractDescriptor*>       gmDuplicateTracts;

    void                                Init();
    void                                Setup();

  public:
    TractHandler();
    ~TractHandler();

    tract_t                             tiCombineDuplicatesByCoordinates();
    void                                tiConcaticateDuplicateTractIdentifiers();
    int                                 tiGetDimensions() const {return nDimensions;}
    double                              tiGetDistanceSq(const std::vector<double>& vFirstPoint, const std::vector<double>& vSecondPoint) const;
    tract_t                             tiGetNumTracts() const {return (int)gvTractDescriptors.size();}
    const char                        * tiGetTid(tract_t t, std::string& sFirst) const;
    double                              tiGetTractCoordinate(tract_t t, int iDimension) const;
    void                                tiGetTractIdentifiers(tract_t t, std::vector<std::string>& vIdentifiers) const;
    tract_t                             tiGetTractIndex(const char *tid) const;
    int                                 tiInsertTnode(const char *tid, std::vector<double>& vCoordinates);
    void                                tiReportDuplicateTracts(FILE * fDisplay) const;
    void                                tiRetrieveCoords(tract_t t, std::vector<double> & vRepository) const;
    void                                tiSetDimensions(int iDimensions) {nDimensions = iDimensions;}
};
//*****************************************************************************
#endif

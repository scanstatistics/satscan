//---------------------------------------------------------------------------
#ifndef __MULTIDIMARRAYHANDLER_H
#define __MULTIDIMARRAYHANDLER_H
//---------------------------------------------------------------------------
template <class T>
class MultipleDimesionArrayHandler {
  protected:
    static unsigned int giGrowthSize;

    virtual void        Allocate() = 0;
    virtual void        Deallocate() = 0;

  public:
    MultipleDimesionArrayHandler() {}
    virtual ~MultipleDimesionArrayHandler() {}
};

template <class T>
class TwoDimensionArrayHandler : public MultipleDimesionArrayHandler<T> {
  protected:
    T                ** gppData;
    unsigned int        g1stDimension;
    unsigned int        g2ndDimension;

    virtual void        Allocate();
    virtual void        Deallocate();
    void                Init() {gppData=0;}

  public:
    TwoDimensionArrayHandler(unsigned int FirstDimension, unsigned int SecondDimension);
    TwoDimensionArrayHandler(unsigned int FirstDimension, unsigned int SecondDimension, const T& t);
    virtual ~TwoDimensionArrayHandler();

    TwoDimensionArrayHandler & operator=(const TwoDimensionArrayHandler<T> & rhs);

    void                ExpandSecondDimension(const T& t);
    inline T         ** GetArray() const {return gppData;}
    inline T         ** GetArray() {return gppData;}
    inline unsigned int Get1stDimension() const {return g1stDimension;}
    inline unsigned int Get2ndDimension() const {return g2ndDimension;}
    void                ResizeSecondDimension(unsigned int uiSize, const T& t);
    void                Set(const T& t);
    static void         SynchronizeSecondDimension(TwoDimensionArrayHandler<T>& lhs, TwoDimensionArrayHandler<T>& rhs, const T& t);
};

template <class T>
class ThreeDimensionArrayHandler : public MultipleDimesionArrayHandler<T> {
  protected:
    T               *** gpppData;
    unsigned int        g1stDimension;
    unsigned int        g2ndDimension;
    unsigned int        g3rdDimension;

    virtual void        Allocate();
    virtual void        Deallocate();
    void                Init() {gpppData=0;}

  public:
    ThreeDimensionArrayHandler(unsigned int FirstDimension, unsigned int SecondDimension, unsigned int ThirdDimension);
    ThreeDimensionArrayHandler(unsigned int FirstDimension, unsigned int SecondDimension, unsigned int ThirdDimension, const T& t);
    virtual ~ThreeDimensionArrayHandler();

    ThreeDimensionArrayHandler & operator=(const ThreeDimensionArrayHandler<T> & rhs);

    void                ExpandThirdDimension(const T& t);
    void                FreeThirdDimension();
    inline T        *** GetArray() const {return gpppData;}
    inline T        *** GetArray() {return gpppData;}
    inline unsigned int Get1stDimension() const {return g1stDimension;}
    inline unsigned int Get2ndDimension() const {return g2ndDimension;}
    inline unsigned int Get3rdDimension() const {return g3rdDimension;}
    void                ResizeThirdDimension(unsigned int uiSize, const T& t);
    void                Set(const T& t);
    static void         SynchronizeThirdDimension(ThreeDimensionArrayHandler<T>& lhs, ThreeDimensionArrayHandler<T>& rhs, const T& t);
};

#include "MultipleDimensionArrayHandler.hxx"
#endif

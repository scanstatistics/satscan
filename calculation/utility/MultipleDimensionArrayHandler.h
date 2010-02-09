//---------------------------------------------------------------------------
#ifndef __MULTIDIMARRAYHANDLER_H
#define __MULTIDIMARRAYHANDLER_H
//---------------------------------------------------------------------------
#include <vector>

//specialized templated C array class -- used instead of standard template library containers
//Reasoning: Standard library containers do not provide a public means for which to define growth,
//           but instead commonly grow in chunks (Borland ~256b). In the context of the TractHandler
//           class, we really want certain arrays to growth in an 'as needed' manner; otherwise
//           we could be wasting considerable amounts of memory.
template <class T>
class MinimalGrowthArray {
   private:
     T             * gpArray;
     unsigned int    giSize;

   public:
     MinimalGrowthArray(const std::vector<T>& v);
     MinimalGrowthArray(const MinimalGrowthArray& a);
     MinimalGrowthArray() : giSize(0), gpArray(0) {}
     ~MinimalGrowthArray();

     const T       & operator[](const unsigned int i) const {return gpArray[i];}
     bool            operator!=(const MinimalGrowthArray<T> & rhs) const;
     void            add(const T& x, bool bSort);
     void            clear() {delete[] gpArray; gpArray=0; giSize=0;}
     bool            exists(const T& x) const;
     std::vector<T>& get(std::vector<T>& v) const;
     unsigned int    size() const {return giSize;}
};

template <class T>
MinimalGrowthArray<T>::MinimalGrowthArray(const std::vector<T>& v) : giSize(v.size()) {
   gpArray = new T[giSize];
   for (unsigned int i=0; i < giSize; ++i) gpArray[i] = v[i];
}

template <class T>
MinimalGrowthArray<T>::MinimalGrowthArray(const MinimalGrowthArray<T>& a) : giSize(a.giSize) {
   gpArray = new T[giSize];
   for (unsigned int i=0; i < giSize; ++i) gpArray[i] = a.gpArray[i];
}

template <class T>
MinimalGrowthArray<T>::~MinimalGrowthArray() {try {delete[] gpArray;}catch(...){}}

template <class T>
bool MinimalGrowthArray<T>::operator!=(const MinimalGrowthArray<T> & rhs) const {
  if (giSize != rhs.giSize) return true;
  for (unsigned int i=0; i < giSize; ++i) if (gpArray[i]!=rhs.gpArray[i]) return true;
  return false;
}

template <class T>
void MinimalGrowthArray<T>::add(const T& x, bool bSort) {
  T * p = new T[giSize + 1];
  for (unsigned int i=0; i < giSize; ++i) p[i] = gpArray[i];
  std::swap(p, gpArray); delete[] p;
  gpArray[giSize] = x; ++giSize; if (bSort) std::sort(gpArray, gpArray + giSize);
}

template <class T>
std::vector<T>& MinimalGrowthArray<T>::get(std::vector<T>& v) const {
  v.resize(giSize);
  for (unsigned int i=0; i < giSize; ++i) 
      v[i] = gpArray[i];
  return v;
}

template <class T>
bool MinimalGrowthArray<T>::exists(const T& x) const {
  for (unsigned int i=0; i < giSize; ++i) if (gpArray[i] == x) return true;
  return false;
}

/** Templated multi-dimensional array handler classes. These classes are intended
    to help manage allocation and deallocation of multiple dimensional arrays. The
    arrays themselves are public to whom ever wants them, inorder to keep access
    times as it was below creation of these classes. When used properly, there
    should be less risk for mishap then previous allocating/deallocating methods.
    QUESTION: Why not us vectors of vectors?
              The size of the multi-dimensional arrays used in SaTScan can be
              indeterminably large. Compounded with the number of arrays created
              for each analysis, can cause memory problems. Compare the amount of
              memory for a vector vs an array:
              array  - pointer plus allocation size as asked
              vector<> - vector class ~32 bytes plus initial allocation of
                         256 bytes(in Builder, you can change this but it is
                         a #define, so you effect all vectors), though subsequent
                         allocations can be defined by creating own allocator class.
    QUESTION: Why not use the boost library, or similiar, multi-dimensional array?
              The boost library implements a multi-dimensional array class that
              has to drawbacks for use with SaTScan, both memory problems.
              - The multi-dimensional array is not arrays of pointers to arrays
                but one array, allocated to the total size needed for entire structure.
                Since we could be asking for large multi-dimensional arrays, we are
                compounding are memory demands by asking for a contiguous block.
              - Accessing elements of the array is done through overloaded [] operator.
                Each time we want to access an element of array, we are adding the
                overhead of a function call.                                            */

template <class T>
class MultipleDimesionArrayHandler {
  protected:
    static const unsigned int giGrowthSize = 3;

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

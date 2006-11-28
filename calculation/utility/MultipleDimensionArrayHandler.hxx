
#include "SSException.h"

template<class T>
TwoDimensionArrayHandler<T>::TwoDimensionArrayHandler(unsigned int FirstDimension, unsigned int SecondDimension)
                            :MultipleDimesionArrayHandler<T>(), g1stDimension(FirstDimension), g2ndDimension(SecondDimension) {
  try {
    Init();
    Allocate();
  }
  catch (prg_exception& x) {
    x.addTrace("constructor()","TwoDimensionArrayHandler");
    throw;
  }
}

template<class T>
TwoDimensionArrayHandler<T>::TwoDimensionArrayHandler(unsigned int FirstDimension, unsigned int SecondDimension, const T& t)
                            :MultipleDimesionArrayHandler<T>(), g1stDimension(FirstDimension), g2ndDimension(SecondDimension) {
  try {
    Init();
    Allocate();
    Set(t);
  }
  catch (prg_exception& x) {
    x.addTrace("constructor()","TwoDimensionArrayHandler");
    throw;
  }
}

template<class T>
TwoDimensionArrayHandler<T>::~TwoDimensionArrayHandler() {
  try {
    Deallocate();
  }
  catch (...){}
}

template<class T>
TwoDimensionArrayHandler<T> & TwoDimensionArrayHandler<T>::operator=(const TwoDimensionArrayHandler<T> & rhs) {
  size_t   i, j;

  try {
    if (g1stDimension != rhs.g1stDimension)
      throw prg_error("First dimension of arrays are: %i  and %i. Assigment can not be performed.",
                          "operator=()", g1stDimension, rhs.g1stDimension);
    if (g2ndDimension != rhs.g2ndDimension)
      throw prg_error("Second dimension of arrays are: %i  and %i. Assigment can not be performed.",
                          "operator=()", g2ndDimension, rhs.g2ndDimension);

    for (i=0; i < g1stDimension; ++i)
       for (j=0; j < g2ndDimension; ++j)
          gppData[i][j] = rhs.gppData[i][j];
  }
  catch (prg_exception& x) {
    x.addTrace("operator=()","TwoDimensionArrayHandler");
    throw;
  }
  return *this;
}

template<class T>
void TwoDimensionArrayHandler<T>::Allocate() {
  size_t   i;

  try {
    gppData = new T*[g1stDimension];
    memset(gppData, 0, sizeof(T*) * g1stDimension);
    for (i=0; i < g1stDimension; ++i)
       gppData[i] = new T[g2ndDimension];
  }
  catch (prg_exception& x) {
     Deallocate();
     throw;
   }
}

template<class T>
void TwoDimensionArrayHandler<T>::Deallocate() {
  size_t   i;

  try {
    if (gppData) {
      for (i=0; i < g1stDimension; ++i)
         delete[] gppData[i];
      delete[] gppData; gppData=0;
    }  
  }
  catch (...){}
}

template<class T>
void TwoDimensionArrayHandler<T>::ExpandSecondDimension(const T& t) {
  ResizeSecondDimension(g2ndDimension + TwoDimensionArrayHandler<T>::giGrowthSize, t);
}

template<class T>
void TwoDimensionArrayHandler<T>::ResizeSecondDimension(unsigned int uiSize, const T& t) {
  size_t                   i, k, uiNumCopy;
  T                      * p=0;

  try {
    uiNumCopy = std::min(g2ndDimension, uiSize);
    for (i=0; i < g1stDimension; ++i) {
       p = new T[uiSize];
       for (k=0; k < uiNumCopy; ++k)
          p[k] = gppData[i][k];
       for (k=uiNumCopy; k < uiSize; ++k)
          p[k] = t;
       delete[] gppData[i];
       gppData[i] = p;
    }
    g2ndDimension = uiSize;
  }
  catch (prg_exception& x) {
    throw;
  }
}

template<class T>
void TwoDimensionArrayHandler<T>::Set(const T& t) {
  size_t   i, j;

  try {
    for (i=0; i < g1stDimension; ++i)
       for (j=0; j < g2ndDimension; ++j)
          gppData[i][j] = t;
  }
  catch (prg_exception& x) {
    Deallocate();
    throw;
  }
}

template<class T>
void TwoDimensionArrayHandler<T>::SynchronizeSecondDimension(TwoDimensionArrayHandler<T>& lhs, TwoDimensionArrayHandler<T>& rhs, const T& t) {
  if (lhs.Get2ndDimension() < rhs.Get2ndDimension())
    lhs.ResizeSecondDimension(rhs.Get2ndDimension(), t);
  else if (lhs.Get2ndDimension() > rhs.Get2ndDimension())
    rhs.ResizeSecondDimension(lhs.Get2ndDimension(), t);
}

template<class T>
ThreeDimensionArrayHandler<T>::ThreeDimensionArrayHandler(unsigned int FirstDimension, unsigned int SecondDimension, unsigned int ThirdDimension)
                              :MultipleDimesionArrayHandler<T>(), g1stDimension(FirstDimension), g2ndDimension(SecondDimension), g3rdDimension(ThirdDimension) {
  try {
    Init();
    Allocate();
  }
  catch (prg_exception& x) {
    x.addTrace("constructor()","ThreeDimensionArrayHandler");
    throw;
  }
}

template<class T>
ThreeDimensionArrayHandler<T>::ThreeDimensionArrayHandler(unsigned int FirstDimension, unsigned int SecondDimension, unsigned int ThirdDimension, const T& t)
                              :MultipleDimesionArrayHandler<T>(), g1stDimension(FirstDimension), g2ndDimension(SecondDimension), g3rdDimension(ThirdDimension) {
  try {
    Init();
    Allocate();
    Set(t);
  }
  catch (prg_exception& x) {
    x.addTrace("constructor()","ThreeDimensionArrayHandler");
    throw;
  }
}

template<class T>
ThreeDimensionArrayHandler<T>::~ThreeDimensionArrayHandler() {
  try {
    Deallocate();
  }
  catch (...){}
}

template<class T>
ThreeDimensionArrayHandler<T> & ThreeDimensionArrayHandler<T>::operator=(const ThreeDimensionArrayHandler<T> & rhs) {
  size_t   i, j, k;

  try {
    if (g1stDimension != rhs.g1stDimension)
      throw prg_error("First dimension of arrays are: %i  and %i. Assigment can not be performed.",
                          "operator=()", g1stDimension, rhs.g1stDimension);
    if (g2ndDimension != rhs.g2ndDimension)
      throw prg_error("Second dimension of arrays are: %i  and %i. Assigment can not be performed.",
                          "operator=()", g2ndDimension, rhs.g2ndDimension);

    if (g3rdDimension != rhs.g3rdDimension)
      throw prg_error("Third dimension of arrays are: %i  and %i. Assigment can not be performed.",
                          "operator=()", g3rdDimension, rhs.g3rdDimension);

    for (i=0; i < g1stDimension; ++i)
       for (j=0; j < g2ndDimension; ++j)
          for (k=0; k < g3rdDimension; ++k)
             gpppData[i][j][k] = rhs.gpppData[i][j][k];
  }
  catch (prg_exception& x) {
    x.addTrace("operator=()","ThreeDimensionArrayHandler");
    throw;
  }
  return *this;
}

template<class T>
void ThreeDimensionArrayHandler<T>::Allocate() {
  size_t                   i, j;

  try {
    gpppData = new T**[g1stDimension];
    memset(gpppData, 0, sizeof(T**) * g1stDimension);
    for (i=0; i < g1stDimension; ++i) {
      //allocate 2nd dimension at i
      gpppData[i] = new T*[g2ndDimension];
      memset(gpppData[i], 0, sizeof(T*) * g2ndDimension);
      //for (j=0; j < g2ndDimension; ++j)
      //   gpppData[i][j] = 0;
      //allocate 3rd dimension at i for each j, if not zero
      if (g3rdDimension)//with specific intention meant for sorted array
        for (j=0; j < g2ndDimension; ++j)
           gpppData[i][j] = new T[g3rdDimension];

    }
  }
  catch (prg_exception& x) {
    Deallocate();
    throw;
  }
}

template<class T>
void ThreeDimensionArrayHandler<T>::Deallocate() {
  size_t   i, j;

  try {
    if (gpppData) {
      for (i=0; i < g1stDimension; ++i) {
         for (j=0; gpppData[i] && j < g2ndDimension; ++j)
            delete[] gpppData[i][j];
         delete[] gpppData[i];
      }
      delete[] gpppData; gpppData=0;
    }
  }
  catch (...){}
}

template<class T>
void ThreeDimensionArrayHandler<T>::ExpandThirdDimension(const T& t) {
  ResizeThirdDimension(g3rdDimension + ThreeDimensionArrayHandler<T>::giGrowthSize, t);
}

template<class T>
void ThreeDimensionArrayHandler<T>::FreeThirdDimension() {
  size_t   i, j;

  try {
    if (gpppData) {
      for (i=0; i < g1stDimension; ++i) {
         for (j=0; gpppData[i] && j < g2ndDimension; ++j) {
            delete[] gpppData[i][j]; gpppData[i][j]=0;
         }
      }
    }
  }
  catch (...){}
}

template<class T>
void ThreeDimensionArrayHandler<T>::ResizeThirdDimension(unsigned int uiSize, const T& t) {
  size_t                   i, j, k, uiNumCopy;
  T                      * p=0;

  try {
    uiNumCopy = std::min(g3rdDimension, uiSize);
    for (i=0; i < g1stDimension; ++i)
      for (j=0; j < g2ndDimension; ++j) {
         p = new T[uiSize];
         for (k=0; k < uiNumCopy; ++k)
            p[k] = gpppData[i][j][k];
         for (k=uiNumCopy; k < uiSize; ++k)
            p[k] = t;
         delete[] gpppData[i][j];
         gpppData[i][j] = p;
      }
    g3rdDimension = uiSize;
  }
  catch (prg_exception& x) {
    throw;
  }
}

template<class T>
void ThreeDimensionArrayHandler<T>::Set(const T& t) {
  size_t   i, j, k;

  try {
    for (i=0; i < g1stDimension; ++i)
       for (j=0; j < g2ndDimension; ++j)
          for (k=0; k < g3rdDimension; ++k)
             gpppData[i][j][k] = t;
  }
  catch (prg_exception& x) {
    Deallocate();
    throw;
  }
}

template<class T>
void ThreeDimensionArrayHandler<T>::SynchronizeThirdDimension(ThreeDimensionArrayHandler<T>& lhs, ThreeDimensionArrayHandler<T>& rhs, const T& t) {
  if (lhs.Get3rdDimension() < rhs.Get3rdDimension())
    lhs.ResizeThirdDimension(rhs.Get3rdDimension(), t);
  else if (lhs.Get3rdDimension() > rhs.Get3rdDimension())
    rhs.ResizeThirdDimension(lhs.Get3rdDimension(), t);
}

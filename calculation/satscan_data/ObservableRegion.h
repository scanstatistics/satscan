//*****************************************************************************
#ifndef __SCAN_REGION_H
#define __SCAN_REGION_H
//*****************************************************************************
#pragma once

#include <vector>
#include "RandomDistribution.h"

/* Abstract observable region. */
class ObservableRegion {
	public:
        ObservableRegion() {}
        virtual ~ObservableRegion() {}

        virtual double       getArea() const = 0;
        virtual std::string  toString() const = 0;
};

/* Defines a linear inequality. */
class LinearInequality {
    public:
        enum InequalitySign {lessThan=0, lessThanOrEqual, greaterThan, greaterThanOrEqual};

    private:
        double               _a;
        double               _b;
        double               _c;
        InequalitySign       _sign;
        InequalitySign       _origSign;

    public:
		LinearInequality(double a, double b, double c, InequalitySign op);
		~LinearInequality() {}

        bool                 operator==(const LinearInequality& rhs) const {
                                 if (_a != rhs._a || _b != rhs._b || _c != rhs._c || _sign != rhs._sign) return false;
                                 return true;
                             }
        bool                 operator!=(const LinearInequality& rhs) const {
                                 return !(*this == rhs);
                             }
        bool                 operator<(const LinearInequality& rhs) const {
                                 if (_a >= rhs._a || _b >= rhs._b || _c >= rhs._c || _sign >= rhs._sign) return false;
                                 return true;
                             }

        bool                 evaluate(double px, double py) const;
        bool                 evaluateForceNonInclusive(double px, double py) const;
        bool                 evaluateExplicit(double px, double py, InequalitySign op) const;
        std::string          toString() const;
        void                 retrievePointOfIntersection(const LinearInequality& other, double& x, double& y) const;
};

class Vertex {
    friend class ConvexPolygonBuilder;
    friend class ConvexPolygonObservableRegion;

    private:
        double     _x;
        double     _y;

    public:
        Vertex() : _x(0), _y(0) {}
        Vertex(double x, double y) : _x(x), _y(y) {}
        ~Vertex() {}  

        Vertex& operator=(const Vertex& rhs) {
                                 _x = rhs._x;
                                 _y = rhs._y;
                                 return *this;
                             }
        bool  operator==(const Vertex& rhs) const {
                                 if (_x == rhs._x && _y == rhs._y) 
                                     return true; 
                                 return false;
                             }
        bool  operator!=(const Vertex& rhs) const {
                                 return !(*this == rhs);
                             }
        bool  operator<(const Vertex& rhs) const {
                                 if (_x == rhs._x) return _y < rhs._y;
                                 return _x < rhs._x;
                             }

        virtual std::string    toString() const;
};

typedef std::vector<LinearInequality>                         InequalityContainer_t; 
typedef std::vector<Vertex>                                   VertexContainer_t; 
typedef std::map<const LinearInequality*, VertexContainer_t > InequalityVertexContainer_t;

/* Convex polygon observable region. */
class ConvexPolygonObservableRegion : public ObservableRegion {
    friend class ConvexPolygonBuilder;

	private:
		double                       _x_left;
	    double                       _x_right;
		double                       _y_bottom;
	    double                       _y_top;
        InequalityContainer_t        _linearInequalities;
        VertexContainer_t            _vertices;
        double                       _areaOfRegion;

        ConvexPolygonObservableRegion(const InequalityContainer_t& linearInequalities,
                                      const VertexContainer_t& vertices,
                                      double areaOfRegion);

    public:
        virtual ~ConvexPolygonObservableRegion() {}

        ConvexPolygonObservableRegion & operator=(const ConvexPolygonObservableRegion& rhs) {
                                           _x_left = rhs._x_left;
                                           _x_right = rhs._x_right;
                                           _y_bottom = rhs._y_bottom;
                                           _y_top = rhs._y_top;
                                           _linearInequalities = rhs._linearInequalities;
                                           _vertices = rhs._vertices;
                                           _areaOfRegion = rhs._areaOfRegion;
                                           return *this;
                                        }

        virtual double         getArea() const {return _areaOfRegion;}
        bool                   intersectsPoint(double px, double py, bool forceNonInclusive) const;
        bool                   intersectsRegion(const ConvexPolygonObservableRegion& other) const;
        virtual std::string    toString() const;
        void                   retrieveRandomPointInRegion(double& x, double& y, RandomNumberGenerator& rng) const;
};

class ConvexPolygonBuilder {
    private:
        static const size_t MIN_INEQUALITIES = 3;

        static double                         convertToDouble(const std::string& s, double defaultValue);
        static double                         calculateArea(const InequalityVertexContainer_t& inequalityVertices);
        static void                           removeVerticesOutsideRegion(InequalityVertexContainer_t& inequalityVertices);    
        static InequalityVertexContainer_t  & retrieveAllVertices(const InequalityContainer_t& inequalities, InequalityVertexContainer_t& inequalityVertices);

    public:
        ConvexPolygonBuilder() {}
        ~ConvexPolygonBuilder() {}

        static ConvexPolygonObservableRegion  buildConvexPolygon(const InequalityContainer_t& linearInequalities);
        static void                           generateRandomPoints(const ConvexPolygonObservableRegion& polygon, std::string& filename, size_t tNum);
        static InequalityContainer_t          parse(const std::string& linearInequalities);
        static void test();
};

//*****************************************************************************
#endif

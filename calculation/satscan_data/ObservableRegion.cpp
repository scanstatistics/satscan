//******************************************************************************
#include "SaTScan.h"
#pragma hdrstop
//******************************************************************************
#include "ObservableRegion.h"
#include "SSException.h"
#include<boost/tokenizer.hpp>
#include <boost/regex.hpp>
#include "UtilityFunctions.h"
#include <iostream>
#include <fstream>

////////////// class Vertex ////////////////////////////////////////////////////////////

std::string Vertex::toString() const {
    std::stringstream vertex;
    vertex << "x=" << _x << ";y=" << _y << ";"; 
    return vertex.str().c_str();
}

////////////// class LinearInequality //////////////////////////////////////////////////

/* constructor - ay + bx + c <= 0 */
LinearInequality::LinearInequality(double a, double b, double c, InequalitySign sign)
                 :_a(a), _b(b), _c(c), _sign(sign), _origSign(sign) {}

/* */
bool LinearInequality::evaluate(double px, double py) const {
   return evaluateExplicit(px, py, _sign);
}

bool LinearInequality::evaluateExplicit(double px, double py, InequalitySign sign) const {
  double eval = py * _a + px * _b + _c;
  switch (sign) {
      case lessThan            : return eval < 0;
      case lessThanOrEqual     : return eval <= 0;
      case greaterThan         : return eval > 0;
      case greaterThanOrEqual  : return eval >= 0;
      default : throw prg_error("Unknown enumeration '%d'","evaluateExplicit()",sign);
  };
}

bool LinearInequality::evaluateForceNonInclusive(double px, double py) const {
  if (_sign == lessThanOrEqual) return evaluateExplicit(px, py, lessThan);
  if (_sign == greaterThanOrEqual) return evaluateExplicit(px, py, greaterThan);
  return evaluateExplicit(px, py, _sign);
}

/* Returns the line equation. */
std::string LinearInequality::toString() const {
    std::stringstream equation;
    equation << _a << "y"  << (_b < 0 ? " - ": " + ") << fabs(_b) << "x" << (_c < 0 ? " - ": " + ") << fabs(_c);
    switch (_origSign) {
        case lessThan            : equation << " < 0"; break;
        case lessThanOrEqual     : equation << " <= 0"; break;
        case greaterThan         : equation << " > 0"; break;
        case greaterThanOrEqual  : equation << " >= 0"; break;
        default : throw prg_error("Unknown enumeration '%d'","toString()",_origSign);
    };
    return equation.str().c_str();
}

/* Retrieves the point of intersection between lines defined by this linear inequality and other. */
void LinearInequality::retrievePointOfIntersection(const LinearInequality& other, double& x, double& y) const {
    if (_a == 0 && other._a == 0) {
      // both are of the form x >= const -- do not intersect
      x = std::numeric_limits<double>::infinity(); 
      y = std::numeric_limits<double>::infinity(); 
    }
    else if (_a == 0) {
      // first equation is line parallel to y-axis -- intersects at x
      x = -_c/_b;
      // evaluate second equation with resulting x value
      y = (-other._b * x - other._c)/other._a;
    }
    else if (other._a == 0) {
      // second equation is line parallel to y-axis -- intersects at x
      x = -other._c/other._b;
      // evaluate first equation with resulting x value
      y = (-_b * x - _c)/_a;
    }
    else if (_b - other._b == 0 /* also check for NaN */) {
      x = std::numeric_limits<double>::infinity(); 
      y = std::numeric_limits<double>::infinity(); 
    }
    else {
      // solve for x by settings the two equations equal
      x = (other._c/other._a - _c/_a)/(_b/_a - other._b/other._a);
      // evaluate first equation with resulting x value
      y = (-_b * x - _c)/_a;
    }
}

////////////// class ConvexPolygonObservableRegion ///////////////////////////////

/* constructor */
ConvexPolygonObservableRegion::ConvexPolygonObservableRegion(const InequalityContainer_t& linearInequalities, 
                                                             const VertexContainer_t& vertices,
                                                             double areaOfRegion) 
        :ObservableRegion(), _linearInequalities(linearInequalities), _vertices(vertices), _areaOfRegion(areaOfRegion),
        _x_left(std::numeric_limits<double>::max()), _x_right(-std::numeric_limits<double>::max()), 
        _y_bottom(std::numeric_limits<double>::max()), _y_top(-std::numeric_limits<double>::max()) {

   // Determine the rectangular region that bounds polygon region -- we'll need this for randomization.
   // Note: Depending on the shape and orientation of polygon, this rectangular region may contain
   //       a great deal of empty space. We might want to consider tilting the rectangular to better
   //       better fit polygon. This will of course mean that the method to generate a random point
   //       will also have to be updated.
   for (VertexContainer_t::const_iterator itr=_vertices.begin(); itr != _vertices.end(); ++itr) {
       _x_left = std::min(_x_left, itr->_x);
       _x_right = std::max(_x_right, itr->_x);
       _y_bottom = std::min(_y_bottom, itr->_y);
       _y_top = std::max(_y_top, itr->_y);
   }
}

/* Returns indication of whether point intersects region by evaluating point against all inequalities */
bool ConvexPolygonObservableRegion::intersectsPoint(double px, double py, bool forceNonInclusive) const {
    bool intersects=true; 
    InequalityContainer_t::const_iterator itr = _linearInequalities.begin();
    for (;itr != _linearInequalities.end() && intersects; ++itr) {
        intersects = forceNonInclusive ? itr->evaluateForceNonInclusive(px, py) : itr->evaluate(px, py);
    }
    return intersects;
}

/* Returns indication of whether regions intersect. */
bool ConvexPolygonObservableRegion::intersectsRegion(const ConvexPolygonObservableRegion& other) const {
    bool intersects=false;

    // Determine if any vertices in other polygon intersect this polygon.
    for (VertexContainer_t::const_iterator itr=other._vertices.begin(); itr != other._vertices.end() && !intersects; ++itr)
        intersects = intersectsPoint(itr->_x, itr->_y, true);
    // Determine if any vertices in this polygon intersect other polygon.
    for (VertexContainer_t::const_iterator itr=_vertices.begin(); itr != _vertices.end() && !intersects; ++itr)
        intersects = other.intersectsPoint(itr->_x, itr->_y, true);
    return intersects;
}

/* Returns region definition as string. */
std::string ConvexPolygonObservableRegion::toString() const {
    std::stringstream polygon;
    polygon << "Rectangular Bound Region(_x_left=" << _x_left << ";_x_right=" 
            << _x_right << ";_y_bottom=" << _y_bottom << ";_y_top=" << _y_top << ")" << std::endl;
    polygon << "Area of Region: "  << _areaOfRegion << std::endl;
    for (size_t i=0; i < _linearInequalities.size(); ++i) {
        polygon << "Linear Inequality: "  << _linearInequalities[i].toString() << std::endl;
    }
    for (size_t i=0; i < _vertices.size(); ++i) {
        polygon << "Vertex: "  << _vertices[i].toString() << std::endl;
    }
    return polygon.str().c_str();
}

/* Retrieves random x,y point from region. */
void ConvexPolygonObservableRegion::retrieveRandomPointInRegion(double& x, double& y, RandomNumberGenerator& rng) const {
    int count=0;
    do {
        count++;
        x = Equilikely(_x_left, _x_right, rng); 
        y = Equilikely(_y_bottom, _y_top, rng);
    } while (!intersectsPoint(x, y, false));
}

////////////// class ConvexPolygonBuilder ///////////////////////////////


/* Calcuates the area of convex polygon defined by remaining inequalities. 
   If the inequalities are correctly bounding a close region, we should be able to
   find the connecting vertices and therefore calculate area. */
double ConvexPolygonBuilder::calculateArea(const InequalityVertexContainer_t& inequalityVertices) {
   // Pick an arbitrary point to start from ... we'll just use the first iterator.
   Vertex start = inequalityVertices.begin()->second.front();
   InequalityVertexContainer_t::const_iterator itrCurrent=inequalityVertices.begin();
   double determinantPart1 = itrCurrent->second.at(0)._x * itrCurrent->second.at(1)._y, 
          determinantPart2 = itrCurrent->second.at(1)._x * itrCurrent->second.at(0)._y;
   size_t matchIndex=1;

   for (;;) {
       // find the other equation that passed through vertex: itrCurrent->second.at(matchIndex)
       InequalityVertexContainer_t::const_iterator itrSearch=inequalityVertices.begin();
       VertexContainer_t::const_iterator itrFind;
       for (;itrSearch != inequalityVertices.end();) {            
           if (itrSearch != itrCurrent) {
              itrFind = std::find(itrSearch->second.begin(), itrSearch->second.end(), itrCurrent->second.at(matchIndex));
              if (itrFind != itrSearch->second.end()) break; // found other inequality that passes through current vertex
           }
           ++itrSearch;
       }
       // If search did not find other inequality that passes through vertex, throw error ...
       if (itrSearch == inequalityVertices.end())
         throw prg_error("Unable to find other equation that passes through vertex(%lf,%lf).", "calculateArea()",
                         itrCurrent->second.at(matchIndex)._x, itrCurrent->second.at(matchIndex)._y);
       // Update determinant parts given current vertex and the other vertex of found inequality.
       size_t searchMatchIndex = (std::distance(itrSearch->second.begin(), itrFind) == 0 ? 1 : 0);
       determinantPart1 += itrCurrent->second.at(matchIndex)._x * itrSearch->second.at(searchMatchIndex)._y;
       determinantPart2 += itrSearch->second.at(searchMatchIndex)._x * itrCurrent->second.at(matchIndex)._y;
       // Update variables for next iteration.
       matchIndex = searchMatchIndex;
       itrCurrent = itrSearch;
       // If we are back at the starting vertex, we have completed the bound region and can break out of loop.
       if (start == itrCurrent->second.at(matchIndex))
           break;
   }
   return fabs(0.5 * (determinantPart1 - determinantPart2)); 
}

/* Remove vertices that are outside the region being defined 
    -- those that do not fulfill one or more of the inequalities */
void ConvexPolygonBuilder::removeVerticesOutsideRegion(InequalityVertexContainer_t& inequalityVertices) {
    size_t remainingVertices=0;
   // eliminate those vertices that are outside the region (those that do not fulfill one or more of the inequalities) -- #4
   InequalityVertexContainer_t::iterator itrTop=inequalityVertices.begin();
   for (;itrTop != inequalityVertices.end();) {
       // get the vertices that we are evaluating
       VertexContainer_t& topVertices = itrTop->second;
       VertexContainer_t::iterator itrVertex = topVertices.begin(); 
       for (;itrVertex != topVertices.end();) {
           // remove infinity/undefined vertices
           if (itrVertex->_x == std::numeric_limits<double>::infinity() || itrVertex->_y == std::numeric_limits<double>::infinity())
              itrVertex = topVertices.erase(itrVertex);
           else {
              // evaluate this vertex against all inequalities
              bool pass=true; 
              InequalityVertexContainer_t::iterator itrEvaluation=inequalityVertices.begin();
              for (;itrEvaluation != inequalityVertices.end() && pass; ++itrEvaluation) {
                  pass = itrEvaluation->first->evaluate(itrVertex->_x, itrVertex->_y);
              }
              itrVertex = (pass ? itrVertex + 1 : topVertices.erase(itrVertex));
           }
       }
       remainingVertices += topVertices.size();
       // remove inequalities that no longer have vertices
       if (topVertices.size() == 0) {
           InequalityVertexContainer_t::iterator temp = itrTop;
           ++itrTop;
           inequalityVertices.erase(temp);
       } else ++itrTop;
   }
   // If all vertices are eliminated, the area is zero (warning).
   if (remainingVertices == 0) 
       throw region_exception("The area of polygon is zero.");
}

/* Finds all vertices(intersection points) of all pairs of equations. Returns a std::map
   of LinearInequality objects to collection of vertices. */
InequalityVertexContainer_t & ConvexPolygonBuilder::retrieveAllVertices(const InequalityContainer_t& inequalities,
                                                                        InequalityVertexContainer_t& inequalityVertices) {
   InequalityContainer_t::const_iterator itrOuter(inequalities.begin()), itr_end(inequalities.end());
   size_t tNumVertices=0;
   // iterate through collection of inequalities, determining intersection point with all others
   for (;itrOuter != itr_end; ++itrOuter) {
      InequalityContainer_t::const_iterator itrInner(itrOuter + 1);
      for (;itrInner != itr_end; ++itrInner) {
          // find point of intersection between equations
          Vertex vertex;
          itrOuter->retrievePointOfIntersection(*itrInner, vertex._x, vertex._y);
          // notes that each inequality has this vertex
          inequalityVertices[&(*itrOuter)].push_back(vertex);
          inequalityVertices[&(*itrInner)].push_back(vertex);
          ++tNumVertices;
      }
   }
   // check that the number of vertices is: (# of inequalities) * ((# of inequalities) - 1)/2
   if (tNumVertices != (inequalities.size() * (inequalities.size() - 1) /2 )) {
       throw prg_error("Number vertices is %d, expected %d.", "retrieveAllVertices()", tNumVertices, (inequalities.size() * (inequalities.size() - 1) /2 ));
   }

   return inequalityVertices;
}

double ConvexPolygonBuilder::convertToDouble(const std::string& s, double defaultValue) {
   if (s.size() < 1) return defaultValue;
   std::istringstream i(s);
   double x;
   if (!(i >> x)) throw prg_error("Conversion failed.","convertToDouble()");
   return x;
} 

/** Attempts to parse string into a collection of LinearInequality objects */
InequalityContainer_t ConvexPolygonBuilder::parse(const std::string& linearInequalitiesString) {
   InequalityContainer_t linearInequalities;

   try {
     boost::tokenizer<boost::escaped_list_separator<char> > inequalities(linearInequalitiesString);
     boost::tokenizer<boost::escaped_list_separator<char> >::const_iterator itr=inequalities.begin();
     // regex to match inequalities of the form: ay <= bx + c
     boost::regex reYFull("^(-?\\d+\\.\\d+|-?\\d+)?y\\s*(<=|>=)\\s*(-?\\d+\\.\\d+|-?\\d+)?x\\s*(\\+|\\-)?\\s*(\\d+\\.\\d+|\\d+)*$", boost::regex_constants::icase);
     // regex to match inequalities of the form: ay <= c
     boost::regex reYConst("^(-?\\d+\\.\\d+|-?\\d+)?y\\s*(<=|>=)\\s*(-?\\d+\\.\\d+|-?\\d+)+$", boost::regex_constants::icase);
     // regex to match inequalities of the form: bx <= c
     boost::regex reXConst("^(-?\\d+\\.\\d+|-?\\d+)?x\\s*(<=|>=)\\s*(-?\\d+\\.\\d+|-?\\d+)+$", boost::regex_constants::icase);
     for (;itr != inequalities.end(); ++itr) {
        std::string value = (*itr);
        trimString(value);        
        bool matched = false;
        double a, b, c;
        LinearInequality::InequalitySign op=LinearInequality::lessThanOrEqual;
        boost::cmatch matches;
        std::istringstream convert;
        if (boost::regex_match(value.c_str(), matches, reYFull)) {
            // get Y coefficient
            a = convertToDouble(std::string(matches[1].first, matches[1].second), 1.0);
            // get equality sign
            convert.str(std::string(matches[2].first, matches[2].second));
            op = (!convert.str().compare("<=") ? LinearInequality::lessThanOrEqual : LinearInequality::greaterThanOrEqual);
            // get X coefficient
            b = convertToDouble(std::string(matches[3].first, matches[3].second), 1.0);
            // get constant
            c = convertToDouble(std::string(matches[5].first, matches[5].second), 0.0);
            // get binary operator if constant defined
            if (c != 0.0) {
              convert.str(std::string(matches[4].first, matches[4].second));
              c = (!convert.str().compare("+") ? c : -c);
            }
            // add inequality to collection
            linearInequalities.push_back(LinearInequality(a, (b == 0.0 ? b : -b), (c == 0.0 ? c : -c), op));
            matched=true;
        }
        else if (boost::regex_match(value.c_str(), matches, reYConst)) {
            // get Y coefficient
            a = convertToDouble(std::string(matches[1].first, matches[1].second), 1.0);
            // get equality sign
            convert.str(std::string(matches[2].first, matches[2].second));
            op = (!convert.str().compare("<=") ? LinearInequality::lessThanOrEqual : LinearInequality::greaterThanOrEqual);
            // get constant
            c = convertToDouble(std::string(matches[3].first, matches[3].second), 0.0);
            // add inequality to collection
            linearInequalities.push_back(LinearInequality(a, 0.0, (c == 0.0 ? c : -c), op));
            matched=true;
        }
        else if (boost::regex_match(value.c_str(), matches, reXConst)) {
            // get X coefficient
            b = convertToDouble(std::string(matches[1].first, matches[1].second), 1.0);
            // get equality sign
            convert.str(std::string(matches[2].first, matches[2].second));
            op = (!convert.str().compare("<=") ? LinearInequality::lessThanOrEqual : LinearInequality::greaterThanOrEqual);
            // get constant
            c = convertToDouble(std::string(matches[3].first, matches[3].second), 0.0);
            linearInequalities.push_back(LinearInequality(0.0, b, (c == 0.0 ? c : -c), op));
            matched=true;
        }
        else 
            throw prg_exception("Inequality does not match known format (%s).", value.c_str());
       
        // matches[0] contains the original string.  matches[n]
        // contains a sub_match object for each matching
        // subexpression
        //for (int i = 0; i < matches.size(); i++) {
        //    // sub_match::first and sub_match::second are iterators that
        //    // refer to the first and one past the last chars of the
        //    // matching subexpression
        //     std::string match(matches[i].first, matches[i].second);
        //     std::cout << "\tmatches[" << i << "] = " << match << std::endl;
        // }
        // std::cout << std::endl;
     }     
   }
   catch (boost::regex_error& e) {
     throw prg_error(e.what(), "parse()");
   }
   catch (prg_exception& x) {
     x.addTrace("parse()","ConvexPolygonBuilder");
     throw;
   }
   return linearInequalities;
}

/**  */
ConvexPolygonObservableRegion ConvexPolygonBuilder::buildConvexPolygon(const InequalityContainer_t& linearInequalities) {
   // ensure that at least minimum number of inequalities exist
   if (linearInequalities.size() < MIN_INEQUALITIES) 
       throw region_exception("A minimum of %d linear inequalities are required for polygon specification.", MIN_INEQUALITIES);

   // make local copy and remove duplicate inequalities -- #2
   InequalityContainer_t inequalities(linearInequalities);
   std::sort(inequalities.begin(), inequalities.end());
   inequalities.erase(std::unique(inequalities.begin(), inequalities.end()), inequalities.end());

   // find the vertices (intersection points) of all pairs of equations -- #3
   InequalityVertexContainer_t inequalityVertices;
   retrieveAllVertices(inequalities, inequalityVertices);

   // eliminate those vertices that are outside the region (those that do not fulfill one or more of the inequalities) -- #4
   removeVerticesOutsideRegion(inequalityVertices);

   // eliminate those inequalities that do not have 2 different unique vertices -- #5
   InequalityVertexContainer_t::iterator itrMap=inequalityVertices.begin();
   for (;itrMap != inequalityVertices.end();) {
       // remove duplicate vertices associated with this inequality
       std::sort(itrMap->second.begin(), itrMap->second.end());
       itrMap->second.erase(std::unique(itrMap->second.begin(), itrMap->second.end()), itrMap->second.end());
       // if now of unique vertices is less than 2, get ignore that inequality
       if (itrMap->second.size() < 2) {
          InequalityVertexContainer_t::iterator temp = itrMap;
          ++itrMap;
          inequalityVertices.erase(temp);
       } else ++itrMap;
   }

   // create a collection of remaining unique vertices, eliminating duplicates vertices -- #6
   VertexContainer_t remainingVertices;
   for (itrMap=inequalityVertices.begin(); itrMap != inequalityVertices.end(); ++itrMap) {
       remainingVertices.resize(remainingVertices.size() + itrMap->second.size());
       std::copy(itrMap->second.begin(), itrMap->second.end(), remainingVertices.begin() + (remainingVertices.size() - itrMap->second.size()));
   }
   std::sort(remainingVertices.begin(), remainingVertices.end());
   remainingVertices.erase(std::unique(remainingVertices.begin(), remainingVertices.end()), remainingVertices.end());

   // if there is only one vertex left, the area is 0 (warning) -- #7
   if (remainingVertices.size() < 2) 
       throw region_exception("The area of polygon is zero.");

   // if there is only one vertex left, the area is 0 (warning) -- #8
   if (remainingVertices.size() > inequalityVertices.size())
       throw region_exception("Polygon is not properly defined.");
       //throw region_exception("Area is infinite -- %u vertices and %u inequalities.", 
       //                       remainingVertices.size(), inequalityVertices.size());
    
   // calculate the area of convex polygon -- #9, 10, 11
   double areaOfRegion = calculateArea(inequalityVertices);
   std::vector<LinearInequality> finalLinearInequalities;
   for (itrMap=inequalityVertices.begin(); itrMap != inequalityVertices.end(); ++itrMap)
       finalLinearInequalities.push_back(*(itrMap->first));

   ConvexPolygonObservableRegion polygon(finalLinearInequalities, remainingVertices, areaOfRegion);
   return polygon;
}

void ConvexPolygonBuilder::generateRandomPoints(const ConvexPolygonObservableRegion& polygon, std::string& filename, size_t tNum) {
  std::ofstream stream;
  stream.open(filename.c_str(), std::ios::app);
  if (!stream) throw resolvable_error("Error: Could not open file '%s'.\n", filename.c_str());

  double x, y;
  RandomNumberGenerator rng;

  for (size_t t=0; t < tNum; ++t) {
     polygon.retrieveRandomPointInRegion(x, y, rng);
     stream << x << " " << y << std::endl;
  }
  stream.close();
}

void ConvexPolygonBuilder::test() {
   std::vector<LinearInequality> lines;

   try {
     std::string list("y>=-7,y<=5,x>=-3,x<=4");
     ConvexPolygonObservableRegion Polygon_parsed = ConvexPolygonBuilder::buildConvexPolygon(parse(list));
     printf("Polygon_parsed:\n%s", Polygon_parsed.toString().c_str());

     //std::string list2("y >= x, y <= -1x + 5, x >= -5");
     std::string list2("y >= 10x - 50, y >= x, y <= -1x + 5, y >= x + 9, y <= -1x - 9, y <= -1x - 5, y <= 2x + 5, x >= -5");
     ConvexPolygonObservableRegion Polygon_parsed2 = ConvexPolygonBuilder::buildConvexPolygon(parse(list2));
     printf("Polygon_parsed:\n%s", Polygon_parsed2.toString().c_str());

     double x, y;
     RandomNumberGenerator rng;
     bool intersects=false;

     lines.clear();
     lines.push_back(LinearInequality(1, 0, 7, LinearInequality::greaterThanOrEqual)); // y >= -7
     lines.push_back(LinearInequality(1, 0, -5, LinearInequality::lessThanOrEqual)); // y <= 5
	 lines.push_back(LinearInequality(0, 1, 3, LinearInequality::greaterThanOrEqual)); // x >= -3
     //lines.push_back(LinearInequality(0, -2, -6, LinearInequality::lessThanOrEqual)); // x >= -3
     lines.push_back(LinearInequality(0, 1, -4, LinearInequality::lessThanOrEqual)); // x <= 4
     //lines.push_back(LinearInequality(0, 2, -8, LinearInequality::lessThanOrEqual)); // -2x <= -8
     ConvexPolygonObservableRegion poylgon = ConvexPolygonBuilder::buildConvexPolygon(lines);
     printf("Polygon:\n%s", poylgon.toString().c_str());

     poylgon.intersectsPoint(5, -5, false);
     poylgon.retrieveRandomPointInRegion(x,y,rng);

     /* square region */
     lines.clear();
     //lines.push_back(LinearInequality(1, -10, 50, LinearInequality::greaterThanOrEqual));
     lines.push_back(LinearInequality(1, -1, 0, LinearInequality::greaterThanOrEqual)); // y => x
     lines.push_back(LinearInequality(1, 1, -5, LinearInequality::lessThanOrEqual)); // y <= -x + 5 
     //lines.push_back(LinearInequality(1, -1, -9, LinearInequality::greaterThanOrEqual));
     //lines.push_back(LinearInequality(1, 1, 9, LinearInequality::lessThanOrEqual));//
     //lines.push_back(LinearInequality(1, 1, 5, LinearInequality::lessThanOrEqual));
     //lines.push_back(LinearInequality(1, -2, -5, LinearInequality::lessThanOrEqual));  // y <= 2x + 5
     lines.push_back(LinearInequality(0, 1, 5, LinearInequality::greaterThanOrEqual));   // x >= -5
     ConvexPolygonObservableRegion poylgon1 = ConvexPolygonBuilder::buildConvexPolygon(lines);
     printf("Polygon1:\n%s", poylgon1.toString().c_str());

     /* polygon*/
     lines.clear();
     lines.push_back(LinearInequality(1, 0, 5, LinearInequality::greaterThanOrEqual));
     lines.push_back(LinearInequality(1, 0, -5, LinearInequality::lessThanOrEqual));
     lines.push_back(LinearInequality(1, -8, -50, LinearInequality::lessThanOrEqual));
     lines.push_back(LinearInequality(1, 8, -50, LinearInequality::lessThanOrEqual));
     lines.push_back(LinearInequality(1, -8, 50, LinearInequality::greaterThanOrEqual));
     lines.push_back(LinearInequality(1, 8, 50, LinearInequality::greaterThanOrEqual));      
     ConvexPolygonObservableRegion poylgon2 = ConvexPolygonBuilder::buildConvexPolygon(lines);
     printf("Polygon2:\n%s", poylgon2.toString().c_str());

     /* triangle */
     lines.clear();
     lines.push_back(LinearInequality(1, -1, 0, LinearInequality::greaterThanOrEqual));
     lines.push_back(LinearInequality(1, -2, 0, LinearInequality::greaterThanOrEqual));
     lines.push_back(LinearInequality(1, -4, 0, LinearInequality::greaterThanOrEqual));
     lines.push_back(LinearInequality(1, -6, 0, LinearInequality::greaterThanOrEqual));
     lines.push_back(LinearInequality(1, -8, 0, LinearInequality::greaterThanOrEqual));
     lines.push_back(LinearInequality(1, -10, 0, LinearInequality::greaterThanOrEqual));
     lines.push_back(LinearInequality(1, 1, 0, LinearInequality::greaterThanOrEqual));
     lines.push_back(LinearInequality(1, 2, 0, LinearInequality::greaterThanOrEqual));
     lines.push_back(LinearInequality(1, 4, 0, LinearInequality::greaterThanOrEqual));
     lines.push_back(LinearInequality(1, 6, 0, LinearInequality::greaterThanOrEqual));
     lines.push_back(LinearInequality(1, 8, 0, LinearInequality::greaterThanOrEqual));
     lines.push_back(LinearInequality(1, 10, 0, LinearInequality::greaterThanOrEqual));
     lines.push_back(LinearInequality(1, 0, -20, LinearInequality::lessThanOrEqual));
     ConvexPolygonObservableRegion poylgon3 = ConvexPolygonBuilder::buildConvexPolygon(lines);
     printf("Polygon3:\n%s", poylgon3.toString().c_str());
     poylgon3.retrieveRandomPointInRegion(x,y,rng);

     intersects = poylgon3.intersectsRegion(poylgon2);
     intersects = poylgon3.intersectsPoint(5, -5, false);

     /* triangle */
     lines.clear();
     lines.push_back(LinearInequality(1, -0.8, -10, LinearInequality::lessThanOrEqual));
     lines.push_back(LinearInequality(1, -0.5, -10, LinearInequality::greaterThanOrEqual));
     lines.push_back(LinearInequality(1, 0, -25, LinearInequality::lessThanOrEqual));
     ConvexPolygonObservableRegion poylgon4 = ConvexPolygonBuilder::buildConvexPolygon(lines);
     printf("Polygon4:\n%s", poylgon4.toString().c_str());

     /* overlapping test */
     lines.clear();
     lines.push_back(LinearInequality(1, -1, 0, LinearInequality::greaterThanOrEqual));
     lines.push_back(LinearInequality(1, 0, -5, LinearInequality::lessThanOrEqual));
     lines.push_back(LinearInequality(1, -3, -5, LinearInequality::lessThanOrEqual));
     ConvexPolygonObservableRegion poylgon5 = ConvexPolygonBuilder::buildConvexPolygon(lines);
     printf("Polygon5:\n%s", poylgon5.toString().c_str());

     lines.clear();
     lines.push_back(LinearInequality(1, -1, 0, LinearInequality::lessThanOrEqual));
     lines.push_back(LinearInequality(1, 0, 5, LinearInequality::greaterThanOrEqual));
     lines.push_back(LinearInequality(1, -2, 5, LinearInequality::greaterThanOrEqual));
     ConvexPolygonObservableRegion poylgon6 = ConvexPolygonBuilder::buildConvexPolygon(lines);
     printf("Polygon6:\n%s", poylgon5.toString().c_str());
     intersects = poylgon5.intersectsRegion(poylgon6);
   } catch (region_exception& e) {
       printf("Polygon Error: %s\n", e.what());
   }
}

// Adam J Vaughn
// December 2002
//
// Class stsOutputFileRegister is a class designed to prevent an output file to be written to by
// two concurrent running threads. The idea is that this class maintains a list(vector) of filenames
// which are currently running and the calling class asks the Register(this class) whether or not an 
// output file is currently running with a given filename before starting that thread. All filenames are 
// released upon destruction of the registry class.

#include "stsSaTScan.h"
#pragma hdrstop

#include "stsOutputFileRegistry.h"

// constructor
stsOutputFileRegister::stsOutputFileRegister() {
}

// destructor
stsOutputFileRegister::~stsOutputFileRegister() {
   gvOutputFileNames.erase(gvOutputFileNames.begin(), gvOutputFileNames.end());
}

// returns whether or not the filename is already registered
// pre: none
// post: returns true if found in register, false otherwise
bool stsOutputFileRegister::IsAlreadyRegistered(const std::string& sFileName) {
   return (find(gvOutputFileNames.begin(), gvOutputFileNames.end(), sFileName) != gvOutputFileNames.end());
}

// adds the filename to the global registry
// pre: filename does not already exist in the registry
// post: pushes the filename back onto the vector
void stsOutputFileRegister::Register(const std::string& sFileName) {
   gvOutputFileNames.push_back(sFileName);
}

// releases the filename from the registry so that it can potentially be used again
// pre: none
// post: removes the filename from the registry (i.e. global vector)
void stsOutputFileRegister::Release(const std::string& sFileName) {
   std::vector<std::string>::iterator	iter = find(gvOutputFileNames.begin(), gvOutputFileNames.end(), sFileName);
   if(iter != gvOutputFileNames.end())
      gvOutputFileNames.erase(iter);
}

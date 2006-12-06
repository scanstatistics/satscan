//---------------------------------------------------------------------------
#include "SaTScan.h"
#pragma hdrstop
//---------------------------------------------------------------------------
#include "SSException.h"
#include "UtilityFunctions.h"
#include "FileName.h" 

////////////////////////////////////////////////////////////////////////////////

prg_exception::prg_exception() : std::exception() {}

prg_exception::prg_exception(const std::string& what_arg) : std::exception() {
  _what=what_arg;
}

prg_exception::prg_exception(const char * format, const char * method, ...) : std::exception() {
  try {
    std::vector<char> temp(1);
    va_list varArgs;
    va_start (varArgs, method);
    size_t iStringLength = vsnprintf(&temp[0], temp.size(), format, varArgs);
    va_end(varArgs);
    temp.resize(iStringLength + 1);
    va_start (varArgs, method);
    vsnprintf(&temp[0], iStringLength + 1, format, varArgs);
    va_end(varArgs);
    _what = &temp[0];
  }
  catch (...) {}
  addTrace(method);
}

void prg_exception::addWhat(const char * message) {
  _what += message;
}

void prg_exception::addTrace(const char * method) {
  std::string temp;
  printString(temp, "%s\n", method);
  _trace.append(temp);
}

void prg_exception::addTrace(const char * method, const char * clazz) {
  std::string temp;
  printString(temp, "%s  of  %s\n", method, clazz);
  _trace.append(temp);
}

/** ::addTrace(__FILE__, __LINE__); */
void prg_exception::addTrace(const char * file, int line) {
  std::string temp;
  printString(temp, "line %d  of %s\n", line, file);
  _trace.append(temp);
}

////////////////////////////////////////////////////////////////////////////////

prg_error::prg_error(const char * format, const char * method, ...) : prg_exception() {
  try {
    std::vector<char> temp(1);
    va_list varArgs;
    va_start (varArgs, method);
    size_t iStringLength = vsnprintf(&temp[0], temp.size(), format, varArgs);
    va_end(varArgs);
    temp.resize(iStringLength + 1);
    va_start (varArgs, method);
    vsnprintf(&temp[0], iStringLength + 1, format, varArgs);
    va_end(varArgs);
    _what = &temp[0];
  }
  catch (...) {}
  addTrace(method);
}

////////////////////////////////////////////////////////////////////////////////

memory_exception::memory_exception(const char * message) : std::bad_alloc() {
  _what = message;
}

////////////////////////////////////////////////////////////////////////////////

resolvable_error::resolvable_error() : prg_exception() {}

resolvable_error::resolvable_error(const char * format, ...) : prg_exception() {
  try {
    std::vector<char> temp(1);
    va_list varArgs;
    va_start(varArgs, format);
    size_t iStringLength = vsnprintf(&temp[0], temp.size(), format, varArgs);
    va_end(varArgs);
    temp.resize(iStringLength + 1);
    va_start(varArgs, format);
    vsnprintf(&temp[0], iStringLength + 1, format, varArgs);
    va_end(varArgs);
    _what = &temp[0];
  }
  catch (...) {}
}

////////////////////////////////////////////////////////////////////////////////

/**  Construct. This is an alternate constructor for when the varArgs list for sMessage
     has already been prepared. Primarily, this will be used by derived classes.        */
usage_error::usage_error(const char * sExecutableFullpathName) : prg_exception() {
   FileName     exe(sExecutableFullpathName);

   printString(_what, "usage: %s%s [parameters file] [options]\n\n"
                  "options: -o     -- ignore parameter result filename setting, use next parameter\n"
                  "                   ex. c:\\>satscan.exe c:\\parameters.prm -o c:\\alternate.out.txt\n"
                  "         -c     -- confirm parameter file correctness only (does not perform analysis)\n"
                  "         -p     -- print parameter settings to screen (does not perform analysis)\n",
               exe.getFileName().c_str(), exe.getExtension().c_str());
}

////////////////////////////////////////////////////////////////////////////////

char * out_of_memory_cache = 0;

void reserve_memory_cache() {
  if (!out_of_memory_cache) out_of_memory_cache = new char[32000];
}

void release_memory_cache() {
   delete [] out_of_memory_cache; out_of_memory_cache=0;
}

void prg_new_handler() {
   release_memory_cache();
   throw std::bad_alloc();
}

////////////////////////////////////////////////////////////////////////////////


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
  __what=what_arg;
}

prg_exception::prg_exception(const char * format, const char * method, ...) : std::exception() {
  va_list varArgs;
  va_start (varArgs, method);
  printStringArgs(__what, varArgs, format);
  va_end(varArgs);
  addTrace(method);
}

prg_exception::prg_exception(va_list varArgs, const char * format, const char * method) {
  printStringArgs(__what, varArgs, format);
  va_end(varArgs);
  addTrace(method);
}

void prg_exception::addWhat(const char * message) {
 __what += message;
}

void prg_exception::addTrace(const char * method) {
  std::string temp;
  printString(temp, "%s\n", method);
  __trace.append(temp);
}

void prg_exception::addTrace(const char * method, const char * clazz) {
  std::string temp;
  printString(temp, "%s  of  %s\n", method, clazz);
  __trace.append(temp);
}

/** ::addTrace(__FILE__, __LINE__); */
void prg_exception::addTrace(const char * file, int line) {
  std::string temp;
  printString(temp, "line %d  of %s\n", line, file);
  __trace.append(temp);
}

////////////////////////////////////////////////////////////////////////////////

prg_error::prg_error(const char * format, const char * method, ...) : prg_exception() {
  va_list varArgs;
  va_start (varArgs, method);
  printStringArgs(__what, varArgs, format);
  va_end(varArgs);
  addTrace(method);
}

////////////////////////////////////////////////////////////////////////////////

memory_exception::memory_exception(const char * message) : std::bad_alloc() {
  __what = message;
}

////////////////////////////////////////////////////////////////////////////////

resolvable_error::resolvable_error() : prg_exception() {}

resolvable_error::resolvable_error(const char * format, ...) : prg_exception() {
  va_list varArgs;
  va_start (varArgs, format);
  printStringArgs(__what, varArgs, format);
  va_end(varArgs);
}

////////////////////////////////////////////////////////////////////////////////

/**  Construct. This is an alternate constructor for when the varArgs list for sMessage
     has already been prepared. Primarily, this will be used by derived classes.        */
usage_error::usage_error(const char * sExecutableFullpathName) : prg_exception() {
   FileName     exe(sExecutableFullpathName);

   printString(__what, "usage: %s%s [parameters file] [options]\n\n"
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


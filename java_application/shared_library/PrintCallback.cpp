#include "SaTScan.h"
#pragma hdrstop
#include "PrintCallback.h"

#ifdef _WINDOWS_
#include <comutil.h>
#pragma comment(lib, "comsuppw.lib")
#endif

/** constructor */
C_PrintCallback::C_PrintCallback(C_Callback * callback, bool bSuppressWarnings)
                :CallbackPrint(bSuppressWarnings) {
  _callback = *callback;
}

void C_PrintCallback::print(const char * sMessage) {
  _cancelled = (_callback(sMessage) != 0);
}

#ifdef _PYTHON_CALLBACK_
/** constructor */
PY_PrintCallback::PY_PrintCallback(PY_Callback * callback, bool bSuppressWarnings)
                :CallbackPrint(bSuppressWarnings) {
  //_callback = *callback;
  _callback = callback;//(C_Callback(callback));
}

void PY_PrintCallback::print(const char * sMessage) {
  _cancelled = (PY_Callback(_callback)(Py_BuildValue("s", sMessage)) != 0);
}
#endif

#ifdef _WINDOWS_
VB_PrintCallback::VB_PrintCallback(long callBackAddress, bool bSuppressWarnings)
                 :CallbackPrint(bSuppressWarnings) {
  // Point the function pointer at the passed-in address.
  _vb_callback = (VB_Callback)callBackAddress;
}

void VB_PrintCallback::print(const char * sMessage) {
   // _bstr_t bstrString;
   // bstrString = sMessage;
   _cancelled = (_vb_callback(sMessage) != 0);
   //_vb_callback(BSTR(SysAllocString(bstrString)));
}
#endif

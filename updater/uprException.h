//---------------------------------------------------------------------------
#ifndef uprExceptionH
#define uprExceptionH
//---------------------------------------------------------------------------
#include <stdexcept>

class update_cancelled : public std::runtime_error {
  public:
    explicit update_cancelled(const std::string& what_arg) : std::runtime_error(what_arg){}
};
class archive_error : public update_cancelled {
  public:
    explicit archive_error(const std::string& what_arg) : update_cancelled(what_arg){}
};
class file_access_error : public update_cancelled {
  public:
    explicit file_access_error(const std::string& what_arg) : update_cancelled(what_arg) {}
};
#endif
 
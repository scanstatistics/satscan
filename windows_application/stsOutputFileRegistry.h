#ifndef STSOUTPUTFILEREGISTER_H
#define STSOUTPUTFILEREGISTER_H

class stsOutputFileRegister {
   public:
      stsOutputFileRegister();
      virtual ~stsOutputFileRegister();
      
      bool 	IsAlreadyRegistered(const std::string& sFileName);
      void 	Register(const std::string& sFileName);
      void	Release(const std::string& sFileName);
   private:
      std::vector<std::string>  gvOutputFileNames;
};

#endif
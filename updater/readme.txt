
Documentation	: SaTScan Update Feature
Date		: September 17, 2004


Overview: 
   The graphical Windows version of SaTScan contains a feature which, in response to
   user actions, does the following:  
       - checks for newer versions
       - downloads updates from 'www.satscan.org'
       - installs new executables and supporting files onto client machine

   The version update comes in 2 files: an updater application and the bundled version
   updates. The updater application is a C++Builder project which extracts files from
   the bundled version updates contained in a Winzip archive.

Archive File: 
   The update application makes use of the path storing ability of Winzip to dictate
   installation/overwriting of files relative to the directory for which archive file was
   dowloaded into. So it is important that the update archive is created with the proper
   relative paths.
      ex. Currently, the target download directory is where the executables are located.
          Below that is a directory named 'Sample Data'. The archive file should be created
          such that, should you want to install a new sample data set, the file paths in the
          archive would be: 'Sample Data/newfile.prm'
                            'Sample Data/newfile.cas', etc.
                           
Archive Library: 
   The C library, ZZipLib, maintained at SourceForge is used to read and extract files from 
   archive, version 0.10.82 (http://zziplib.sourceforge.net/). Two files required
   modification in this version to compile w/ C++Builder 5:
      file 'zzip-conf.h'  - inserted text at line 149
                            #if defined __BORLANDC__                   
                            #  ifndef strcasecmp
                            #  define strcasecmp stricmp
                            #  endif
                            #endif
                         
      file 'zzip-io.c'    - inserted text at line 17
                            #include <io.h>                          

User Licence Agreement:
   A user license agreement is presented to the users who are updating from a previous
   version of SaTScan, for which there wasn't an agreement in the installer (v4.0.2 and
   prior). The text of this ULA is compiled into the executable; see in file 'dlgULA.cpp'
   for documentation on how to update the text of the agreement.

SaTScan Source Code Particular Updates:
  The SaTScan source code contains a version id that is used to identify one version from
  the next, utilized when checking for updates. This id is a sequential number that is
  incremented for version of SaTScan released, major and minor. Each new release of SaTScan
  must increment this id inorder for the update process to function properly.
  
Uploading to SaTScan website:
  Once the archive file has been created, and a new updater application created as needed,
  both the updater executable and the archive file must be uploaded to www.satscan.org.
    - navigate to the download area of the website
    - in the password edit box, type a special administrator password then click download
      * please see Martin Kulldorff for this password
    - scan to section labeled 'Version Update System Admin Page' and click corresponding link
    - in the top most area, you'll see information for the most current release
    - specify the same updated version information as in SaTScan source code
    - specfiy the local updater application and archive file to upload
    - select 'Add Entry'
    
Misc:
  At the time, the updater application seemed appropriate. We were creating the installers with
  InstallShield Express 2.12; which did not provide a feature to create an application update installer.
  Since then two shortcomings are known:
    - should version updates install additionals, other than those originally installed; uninstalling
      will not remove these files from the users system
    - InstallShield Express 2.12 creates a 16-bit installer. On 32-bit Windows this is not a problem but
      64-bit Windows does not support 16-bit applications by default. Though, at this time, there are not
      many users likely to experience this problem with the installer; it is prudent to switch a 32-bit
      installer now (Note that we did actually get a few users that did come across this issue). 
      * The v4.0.3 installer was re-created with InstallShield Professional; which does have the ability to
        create update installers (though we aren't using it currently); which makes our updater application/
        archive method not needed. 
      * Note that switching to the professional version of InstallShield also alleviated an issue with 2000/XP
        users where attempting to install to the 'Program Files' directory without administrator privileges was
        causing the install to be denied. A much more meaningful message is presented to the professional
        version. 
      

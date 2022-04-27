

REM "C:\Program Files (x86)\Java\jdk1.7.0_25\bin\javah.exe" -classpath C:\prj\satscan.development\satscan\java_application\jni_application\build\classes org.satscan.app.Parameters
REM "C:\Program Files (x86)\Java\jdk1.7.0_25\bin\javah.exe" -classpath C:\prj\satscan.development\satscan\java_application\jni_application\build\classes org.satscan.app.CalculationThread
REM "C:\Program Files (x86)\Java\jdk1.7.0_25\bin\javah.exe" -classpath C:\prj\satscan.development\satscan\java_application\jni_application\build\classes org.satscan.app.AppConstants
REM "C:\Program Files (x86)\Java\jdk1.7.0_25\bin\javah.exe" -classpath C:\prj\satscan.development\satscan\java_application\jni_application\build\classes org.satscan.gui.ParameterSettingsFrame
REM "C:\Program Files (x86)\Java\jdk1.7.0_25\bin\javah.exe" -classpath C:\prj\satscan.development\satscan\java_application\jni_application\build\classes org.satscan.gui.OberservableRegionsFrame
REM "C:\Program Files (x86)\Java\jdk1.7.0_25\bin\javah.exe" -classpath C:\prj\satscan.development\satscan\java_application\jni_application\build\classes org.satscan.importer.ShapefileDataSource

javac.exe C:\prj\satscan.development\satscan\java_application\jni_application\build\classes org.satscan.app.Parameters.java -h . -classpath .\build\classes
javac.exe C:\prj\satscan.development\satscan\java_application\jni_application\build\classes org.satscan.app.CalculationThread.java -h . -classpath .\build\classes
javac.exe C:\prj\satscan.development\satscan\java_application\jni_application\build\classes org.satscan.app.AppConstants.java -h . -classpath .\build\classes
javac.exe C:\prj\satscan.development\satscan\java_application\jni_application\build\classes org.satscan.gui.ParameterSettingsFrame.java -h . -classpath .\build\classes
javac.exe C:\prj\satscan.development\satscan\java_application\jni_application\build\classes org.satscan.gui.OberservableRegionsFrame.java -h . -classpath .\build\classes
javac.exe C:\prj\satscan.development\satscan\java_application\jni_application\build\classes org.satscan.gui.ApplicationPreferences.java -h . -classpath .\build\classes

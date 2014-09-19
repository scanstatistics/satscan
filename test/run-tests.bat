

set app=%~dp0Win32\Release\test.exe
set log=%~dp0test-log.txt
set report=%~dp0test-results.xml
set files_path=%~dp0..\installers\sample_data

REM All Unit Tests
%app% --show_progress=yes --catch_system_errors=yes --build_info=yes --log_level=all --log_format=hrf --log_sink=%log% --report_format=xml --report_level=detailed --report_sink=%report% --fixture-datafiles-path=%files_path%

REM Julian Date Unit Tests
REM %app% --show_progress=yes --catch_system_errors=yes --log_level=all --log_sink=%log% --report_format=xml --report_level=detailed --report_sink=%report% --run_test=julian* --fixture-datafiles-path=%files_path%

REM Loglikelihood Unit Tests
REM %app% --show_progress=yes --catch_system_errors=yes --log_level=all --log_sink=%log% --report_format=xml --report_level=detailed --report_sink=%report% --run_test=*loglikelihood*  --fixture-datafiles-path=%files_path%

REM sample data Unit Tests
REM %app% --show_progress=yes --catch_system_errors=yes --log_level=all --log_sink=%log% --report_format=xml --report_level=detailed --report_sink=%report% --run_test=*sampledata*,parameter*  --fixture-datafiles-path=%files_path%

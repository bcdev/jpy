SET SDK_SETENV=C:\Program Files\Microsoft SDKs\Windows\v7.1\bin\setenv
:: Python up to 3.4 is build with Visual Studio 9, we use 10
SET VS90COMNTOOLS=%VS100COMNTOOLS%
SET DISTUTILS_USE_SDK=1

SET JDK32_HOME=C:\Program Files (x86)\Java\jdk1.7.0_45
SET JDK64_HOME=C:\Program Files\Java\jdk1.8.0_05

SET PY27_64=C:\Python27-amd64
SET PY33_64=C:\Python33-amd64
SET PY34_64=C:\Python34-amd64
SET PY27_32=C:\Python27
SET PY33_32=C:\Python33
SET PY34_32=C:\Python34



IF NOT EXIST "%JDK64_HOME%" GOTO Build_32
SET JAVA_HOME=%JDK64_HOME%
CALL "%SDK_SETENV%" /x64 /release

SET PYTHONHOME=%PY27_64%
IF NOT EXIST "%PYTHONHOME%" GOTO Build_PY33_64
ECHO Starting build using "%PYTHONHOME%" and "%JAVA_HOME%"
"%PYTHONHOME%\python.exe" setup.py --maven build

:Build_PY33_64
SET PYTHONHOME=%PY33_64%
IF NOT EXIST "%PYTHONHOME%" GOTO Build_PY34_64
ECHO Starting build using "%PYTHONHOME%" and "%JAVA_HOME%"
"%PYTHONHOME%\python.exe" setup.py --maven build

:Build_PY34_64
SET PYTHONHOME=%PY34_64%
IF NOT EXIST "%PYTHONHOME%" GOTO Build_32
ECHO Starting build using "%PYTHONHOME%" and "%JAVA_HOME%"
"%PYTHONHOME%\python.exe" setup.py --maven build


:Build_32

IF NOT EXIST "%JDK32_HOME%" GOTO Build_End
SET JAVA_HOME=%JDK32_HOME%
CALL "%SDK_SETENV%" /x86 /release

SET PYTHONHOME=%PY27_32%
IF NOT EXIST "%PYTHONHOME%" GOTO Build_PY33_32
ECHO Starting build using "%PYTHONHOME%" and "%JAVA_HOME%"
"%PYTHONHOME%\python.exe" setup.py --maven build

:Build_PY33_32
SET PYTHONHOME=%PY33_32%
IF NOT EXIST "%PYTHONHOME%" GOTO Build_PY34_32
ECHO Starting build using "%PYTHONHOME%" and "%JAVA_HOME%"
"%PYTHONHOME%\python.exe" setup.py --maven build

:Build_PY34_32
SET PYTHONHOME=%PY34_32%
IF NOT EXIST "%PYTHONHOME%" GOTO Build_End
ECHO Starting build using "%PYTHONHOME%" and "%JAVA_HOME%"
"%PYTHONHOME%\python.exe" setup.py --maven build


:Build_End

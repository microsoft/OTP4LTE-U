@REM Copyright (c) Microsoft Corporation
@REM All rights reserved. 
@REM 
@REM Licensed under the Apache License, Version 2.0 (the ""License""); you
@REM may not use this file except in compliance with the License. You may
@REM obtain a copy of the License at
@REM 
@REM http://www.apache.org/licenses/LICENSE-2.0
@REM 
@REM THIS CODE IS PROVIDED ON AN *AS IS* BASIS, WITHOUT WARRANTIES OR
@REM CONDITIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING WITHOUT
@REM LIMITATION ANY IMPLIED WARRANTIES OR CONDITIONS OF TITLE, FITNESS FOR
@REM A PARTICULAR PURPOSE, MERCHANTABLITY OR NON-INFRINGEMENT.
@REM 
@REM See the Apache Version 2.0 License for specific language governing
@REM permissions and limitations under the License.

@SET tempcurdir=%cd%
@ECHO Set ZIRIA_VS to the version of VS you want to use
@ECHO e.g. ZIRIA_VS="C:\Program Files (x86)\Microsoft Visual Studio 12.0\VC"
call %ZIRIA_VS%\vcvarsall.bat x86
@cd %tempcurdir%

REM We need to clear MAKEFLAGS from make (http://www.cygwin.com/ml/cygwin/1999-10/msg00354.html)
set MAKEFLAGS=

@REM msbuild LTE-MAC.vcxproj /target:build /p:Configuration=Release /p:Platform=x64 /m /verbosity:minimal
@REM The actions of %MODE% (e.g. /p:mode=TEST) are defined in .vcxproj file
@REM msbuild CompilerVS.vcxproj /target:build /p:Configuration=Debug /p:Platform=Win32 /m /verbosity:minimal %MODE%

@REM NOTE: It has to be Win32 here and not x64 as x64 does not support _asm, which we still use in TurboDecoder
@REM NOTE: Seems that build does not always rebuilds properly so we force rebuild here (since it has to rebuild everything anyway)
msbuild CompilerVS-inline.vcxproj /target:rebuild /p:Configuration=Release /p:Platform=Win32 /m /verbosity:minimal %MODE%

if %errorlevel% NEQ 0 (
set errorlev=1
del CompilerVS-inline\Win32\Release\CompilerVS-inline.exe
) else (
set errorlev=0
)
exit /B %errorlev%


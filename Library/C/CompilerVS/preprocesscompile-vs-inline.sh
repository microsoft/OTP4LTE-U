# 
# Copyright (c) Microsoft Corporation
# All rights reserved. 
#
# Licensed under the Apache License, Version 2.0 (the ""License""); you
# may not use this file except in compliance with the License. You may
# obtain a copy of the License at
#
# http://www.apache.org/licenses/LICENSE-2.0
#
# THIS CODE IS PROVIDED ON AN *AS IS* BASIS, WITHOUT WARRANTIES OR
# CONDITIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING WITHOUT
# LIMITATION ANY IMPLIED WARRANTIES OR CONDITIONS OF TITLE, FITNESS FOR
# A PARTICULAR PURPOSE, MERCHANTABLITY OR NON-INFRINGEMENT.
#
# See the Apache Version 2.0 License for specific language governing
# permissions and limitations under the License.
#
#

#!/bin/bash

set -e

export TOP=$(cd $(dirname $0)/.. && pwd -P)
export INC=$(cd $(dirname $0)/../../Ziria && pwd -P)
export ZR=`cygpath $ZIRIA_ROOT`
source $ZR/scripts/common.sh

echo $1
#echo "Preprocessing..."
#gcc -x c -P -E $1 >$1.expanded
gcc $DEFINES -I $ZR/lib -I $INC -I $EXT_INC -w -x c -E $1 >$1.expanded


#echo "Running WPL compiler..."
$ZR/wplc $WPLCFLAGS $EXTRAOPTS -i $1.expanded -o $1.c
cp $1.c $TOP/test.c


#echo "Compiling C code (VS) ..."
pushd . && cd $TOP/CompilerVS/ && ./build-inline.bat && popd


if [[ $# -ge 2 ]]
then
   # cp -f is sometimes not sufficient on cygwin
   rm -f $2
   cp -f $TOP/CompilerVS/CompilerVS-inline/Win32/Release/CompilerVS-inline.exe $2
fi



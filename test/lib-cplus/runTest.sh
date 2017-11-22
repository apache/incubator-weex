#!/usr/bin/env bash

cd "$(dirname "$0")"
rm -rf build/
mkdir build
cd build
../lcov/bin/lcov -d ./CMakeFiles/WXCoreLayout_lib.dir/ -z -b . --initial -o tmp.info
cmake -DTEST=ON -DCOVERAGE=ON ..
make
./runUnitTests
../lcov/bin/lcov -q -d ./CMakeFiles/WXCoreLayout_lib.dir/ -t 'Layout Coverage Detail' -o 'tmp.info' --rc lcov_branch_coverage=1 -b . -c
../lcov/bin/lcov -q --remove tmp.info '/usr/include/*' '/usr/lib/*' '/Applications/Xcode.app/*' '/Library/Developer/CommandLineTools/*' -o last.info --rc lcov_branch_coverage=1
../lcov/bin/genhtml last.info --rc lcov_branch_coverage=1 --output-directory lcov --title 'Layout Coverage Detail'
open ./lcov/index.html

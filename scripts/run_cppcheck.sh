#!/bin/bash

src_files=$(find src -regex '.*\.\(cc\|hh\)')
test_files=$(find test -regex '.*\.\(cc\|hh\)')
cppcheck --std=c++11 --enable=all --error-exitcode=1 -x c++ -q -f --suppress=*:include/* -Isrc -Iinclude $src_files


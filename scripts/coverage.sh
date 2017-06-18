#!/bin/bash

if [ ! -d build-coverage ];
then
        meson build-coverage --buildtype debug --werror -Db_coverage=true
fi

cd build-coverage
ninja test

case "$1" in
        -codecov)
                bash <(curl -s https://codecov.io/bash) -p .. -t $2
                ;;
        -html)
                lcov --directory . --capture --output-file ./code-coverage.info
                lcov --remove code-coverage.info "/usr/*" "test/*" -o code-coverage.info
                genhtml code-coverage.info --output-directory ./coverage-report/
                ;;
esac

cd ..


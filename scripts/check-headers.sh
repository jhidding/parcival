#!/bin/bash

header_header=$(cat << EOF
/* Copyright 2017 Netherlands eScience Center
 *
 *   Licensed under the Apache License, Version 2.0 (the "License");
 *   you may not use this file except in compliance with the License.
 *   You may obtain a copy of the License at
 *
 *       http://www.apache.org/licenses/LICENSE-2.0
 *
 *   Unless required by applicable law or agreed to in writing, software
 *   distributed under the License is distributed on an "AS IS" BASIS,
 *   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *   See the License for the specific language governing permissions and
 *   limitations under the License.
 */
#pragma once
EOF
)

cd src
header_files=$(find -regex '.*\.hh')
return_value=0

check_begin() {
        content=${2:0:${#header_header}}
        if [ "${content}" != "${header_header}" ]
        then
                echo "header error in ${1}"
                diff <(echo "${content}") <(echo "${header_header}")
                return_value=1
        fi
}


check_doxygenated() {
        # remove the dot-slash, backslash the dots and slashes
        filename_re=$(echo $1 | sed -e 's/\.\/\(.*\)/\1/;s/\//\\\//;s/\./\\\./')
        doxy_re="\\/\\*!\\ \\\\file\\ ${filename_re}"
        if [[ ! "${2}" =~ ${doxy_re} ]]
        then
                echo "Missing Doxygen \`\\file\` statement: $1"
                echo "failed to match the expression: ${doxy_re}"
                return_value=1
        fi
}

for f in ${header_files}
do
        file_content=$(cat ${f})
        check_begin $f "${file_content}"
        check_doxygenated $f "${file_content}"
done

cd ..
exit ${return_value}


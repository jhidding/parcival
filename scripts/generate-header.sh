#!/bin/bash

license_text=$(cat << EOF
/* Copyright 2017 Johan Hidding
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
EOF
)

echo "Adding the following header to all source files that don't start with a comment."
echo "---"
echo "${license_text}"
echo "---"

files=$(find src test -regex '.*\.\(cc\|hh\)' -not -path 'test/gtest/*' -not -path 'test/gmock/*')
for f in ${files}; do
        content=$(cat ${f})
        if ! [[ "${content}" =~ ^\/\*\ Copyright.* ]]; then
                echo "${f}"
                echo "${license_text}" > ${f}
                echo "${content}" >> ${f}
        fi
done

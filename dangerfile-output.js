/*
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 */

import { fail, warn } from 'danger'
const shell = require('shelljs')
const title = process.env.TITLE
const fs = require('fs')

// code come from: https://stackoverflow.com/a/12745196
// get the index of nth char in string
function nth_occurrence (string, char, nth) {
  var first_index = string.indexOf(char)
  var length_up_to_first_index = first_index + 1
  if (nth === 1) {
      return first_index;
  } else {
      var string_after_first_occurrence = string.slice(length_up_to_first_index);
      var next_occurrence = nth_occurrence(string_after_first_occurrence, char, nth - 1);
      if (next_occurrence === -1) {
          return -1;
      } else {
          return length_up_to_first_index + next_occurrence;  
      }
  }
}

function check_android_lint(content){
  // in xml report,Overview Section,Disabled Checks Section and Suppressing Warnings and Errors Section is not reported.
  // in html report, those Section are included,
  // but Overview Section can't work in markdown,
  // on the other hand,Disabled Checks Section and Suppressing Warnings and Errors Section are not needed.
  // the first section is Overview section,
  // the last two section are  Disabled Checks Section and Suppressing Warnings and Errors Section.
  // so we should grep the str from the second <section to the third </section> from last"
  const occurance = content.split('</section>').length - 1
  if (occurance > 3) {
    content = content.substr(nth_occurrence(content, '<section ', 2))
    content = content.substr(0, nth_occurrence(content, '</section>', occurance - 3))
    fail(content)
    fail(title)
  }
}

if (title === 'OCLint Result') {
  const command = 'cat ios/sdk/oclint.log | grep -v "Summary: TotalFiles=" | grep -i "P[1|2]"'
  const child = shell.exec(command)
  if (child.stdout !== '') {
    fail(child.stdout)
    fail(title)
  }
  if (child.stderr !== '') {
    fail(child.stderr)
    fail(title)
  }
}
else if (title === 'AndroidLint Result') {
  var apacheRelease_content = fs.readFileSync('android/sdk/build/reports/lint-results-apacheRelease.html', 'utf8')
  var legacyRelease_content = fs.readFileSync('android/sdk/build/reports/lint-results-apacheRelease.html', 'utf8')
  check_android_lint(apacheRelease_content)
  check_android_lint(legacyRelease_content)
}
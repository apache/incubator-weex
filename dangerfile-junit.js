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
import { danger, fail, warn } from "danger";
import { DOMParser } from "xmldom";
import fs from "fs";
import path from "path";

const TEST_RESULT_FOLDER = "android/sdk/build/test-results/debug";

const processReports = folder => {
  var failureCases = [];
  fs.readdirSync(folder).forEach(file => {
    const content = fs.readFileSync(path.resolve(folder, file)).toString();
    let doc = new DOMParser().parseFromString(content, "text/xml");
    let length = doc.childNodes.length;
    for (let i = 0; i < length; i++) {
      let node = doc.childNodes.item(i);
      if (node.tagName == "testsuite") {
        let suiteData = parseSuite(node);
        if (Number.parseInt(suiteData.failures) > 0) {
          suiteData.testcases.forEach(testcase => {
            fail(
              `${suiteData.name}/${testcase.name}:\n\n\n ${testcase.failure}`
            );
          });
        }
      }
    }
  });
};

const parseCase = testcase => {
  let data = {
    name: testcase.getAttribute("name"),
    classname: testcase.getAttribute("classname")
  };
  let failureElems = testcase.getElementsByTagName("failure");
  if (failureElems != undefined && failureElems.length > 0) {
    data.failure = failureElems[0].getAttribute("message");
  }
  return data;
};

const parseSuite = testsuite => {
  let data = {
    name: testsuite.getAttribute("name"),
    tests: testsuite.getAttribute("tests"),
    failures: testsuite.getAttribute("failures"),
    errors: testsuite.getAttribute("errors"),
    testcases: []
  };
  if (testsuite.hasChildNodes) {
    let length = testsuite.childNodes.length;
    for (let i = 0; i < length; i++) {
      let testcase = testsuite.childNodes.item(i);
      if (testcase.tagName == "testcase") {
        data.testcases.push(parseCase(testcase));
      }
    }
  }
  return data;
};

processReports(TEST_RESULT_FOLDER);

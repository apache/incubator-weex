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
import { schedule, danger, fail, warn, message, markdown } from "danger";
import fs from "fs";
import path from 'path';
import GitHubApi from 'github';
import parseDiff from 'parse-diff';

// Make sure there are changelog entries
const hasChangelog = danger.git.modified_files.includes("changelog.md")
if (!hasChangelog) { warn("No Changelog changes!") }

const jsFiles = danger.git.created_files.filter(path => path.endsWith("js"));

function absolute (relPath) {
  return path.resolve(__dirname, relPath)
}

const flowIgnorePaths = [
  'node_modules',
  'test',
  'build',
  'examples',
  'doc',
  'android',
  'ios',
  'bin',
  'dist',
  'flow-typed'
].map(function (rel) {
  return absolute(rel)
});

// new js files should have `@flow` at the top
const unFlowedFiles = jsFiles.filter(filepath => {
  let i = 0
  const len = flowIgnorePaths.length
  while (i < len) {
    const p = flowIgnorePaths[i]
    if (absolute(filepath).indexOf(p) > -1) {
      // ignore this file because it's in the flow-ignore-paths.
      return false;
    }
    i++
  }
  const content = fs.readFileSync(filepath);
  return !content.includes("@flow");
});

if (unFlowedFiles.length > 0) {
  warn(
    `These new JS files do not have Flow enabled: ${unFlowedFiles.join(", ")}`
  );
}

// Error or Warn when delete public interface
var methion_break_change = false;
for (let c of danger.git.commits) {
  // console.log("msg:" + c.message);
  if (c.message && c.message.match(/break\s+change/i)) {
    methion_break_change = true;
    break;
  }
}

// File name match any of these patterns will be ignored.
function is_ignored_public_check(file) {
  var ignored_break_change_pattern = [
    /^android\/sdk\/src\/test\/.+/,
     /^android\/playground\/.+/
  ];
  for (let p of ignored_break_change_pattern) {
    if (file.match(p)) {
      return true;
    }
  }
  return false;
}

var has_app_changes = false;
var has_test_changes = false;
var codefiles = [];
for (let file of danger.git.modified_files) {
  if (file.match(/WeexSDK\/Source/)) {
    has_app_changes = true;
  } else if (file.match(/WeexSDKTests/)) {
    has_test_changes = true;
  }

  if (!is_ignored_public_check(file) && file.endsWith(".java")) {
    var diff = danger.git.diffForFile(file);
    // console.log("diff：" + diff+ typeof diff);
    if (diff && diff.match(/^-\s*?public\s+[\s\S]+$/gm)) {
      if (methion_break_change) {
        warn("Potential BREAK CHANGE. Modify public in   " + file);
      } else {
        warn(
          "Potential BREAK CHANGE. Modify public in " +
            file +
            " without metion it in commit message. You'd better add 'break change' in your commit log. "
        );
      }
    }
  }

  if (
    file.endsWith(".h") ||
    file.endsWith(".m") ||
    file.endsWith(".mm") ||
    file.endsWith(".java") ||
    file.endsWith(".js")
  ) {
    codefiles.push(file);
  }
}
if(danger.git.added_files){
  for (let file of danger.git.added_files) {
    if (
      file.endsWith(".h") ||
      file.endsWith(".m") ||
      file.endsWith(".mm") ||
      file.endsWith(".java") ||
      file.endsWith(".js")
    ) {
      codefiles.push(file);
    }
  }
}

if (danger.git.lines_of_code > 500) {
  warn("Big PR");
}

if (danger.git.lines_of_code > 100 && has_app_changes && !has_test_changes) {
  warn("This PR may need tests.");
}

//check ios copyright
//see scripts/rh/header.template
const copyright_header_components = [
  "Licensed to the Apache Software Foundation \\(ASF\\) under one",
  "or more contributor license agreements.  See the NOTICE file",
  "distributed with this work for additional information",
  "regarding copyright ownership\\.  The ASF licenses this file",
  'to you under the Apache License, Version 2\\.0 \\(the',
  '"License"\\); you may not use this file except in compliance',
  'with the License\\.  You may obtain a copy of the License at'
];

//path prefix
const ignoreCopyrightVerifyPath = [
  'test',
  'pre-build',
  'android/playground/app/src/main/assets',
  'android/sdk/assets',
  'ios/playground/bundlejs',
  'ios/sdk/WeexSDK/Resources',
  'ios/sdk/WeexSDK/Sources/Layout'
]

codefiles.forEach(filepath => {
  for(var i=ignoreCopyrightVerifyPath.length-1;i>=0;i--){
    if(filepath.startsWith(ignoreCopyrightVerifyPath[i])){
      return
    }
  }
  const content = fs.readFileSync(filepath).toString();
  for (const line of copyright_header_components) {
    if (!content.match(new RegExp(line))) {
      fail("Code file "+ filepath +" does not have the copyright header.");
      return;
    }
  }
});


/*
 * try to find the appropriate reviewer according to the blame info
 * will be seperated to a danger plugin
 */

schedule(new Promise((resolve, reject) => {
  try {
    findReviewer(resolve, reject)
  } catch (e) {
    console.log(e)
    resolve()
  }
}));

function findReviewer(resolve, reject) {
  var github = new GitHubApi({
    protocol: "https",
    host: "api.github.com",
  });

  var fileToDeletedLinesMap = {}
  var fileToNormalLinesMap = {}
  var fileToBlamesMap = {}
  github.pullRequests.get({
    owner: danger.github.pr.base.user.login,
    repo: danger.github.pr.base.repo.name,
    number: danger.github.pr.number,
    headers: {Accept: 'application/vnd.github.diff'}
  }, function (err, result) {
    if ("undefined" === typeof result || "undefined" === typeof result.data || err) {
      reject()
      return
    }
    parseDeleteAndNormalLines(result.data, fileToDeletedLinesMap, fileToNormalLinesMap)
    var promises = danger.git.modified_files.map(function(file) {
      let repoURL = danger.github.pr.base.repo.html_url
      let fileName = file.replace(/^.*[\\\/]/, '')
      let blameURL = repoURL + '/blame/' + danger.github.pr.base.ref + '/' + file
      // console.log("Getting blame html: " + blameURL)
      return getContent(blameURL)
    });

    Promise.all(promises).then(datas => {
      datas.forEach(function(data, index) {
        fileToBlamesMap[danger.git.modified_files[index]] = parseBlame(data);
      });
      findBlameReviewers(fileToDeletedLinesMap, fileToNormalLinesMap, fileToBlamesMap)
      resolve()
    })
  });
}

function getContent(url) {
  // return new pending promise
  return new Promise((resolve, reject) => {
    // select http or https module, depending on reqested url
    const lib = url.startsWith('https') ? require('https') : require('http');
    const request = lib.get(url, (response) => {
      // handle http errors
      if (response.statusCode < 200 || response.statusCode > 299) {
         reject(new Error('Failed to load page, status code: ' + response.statusCode));
       }
      // temporary data holder
      const body = [];
      // on every content chunk, push it to the data array
      response.on('data', (chunk) => body.push(chunk));
      // we are done, resolve promise with those joined chunks
      response.on('end', () => resolve(body.join('')));
    });
    // handle connection errors of the request
    request.on('error', (err) => reject(err))
    })
}

function parseDeleteAndNormalLines(diffData, fileToDeletedLinesMap, fileToNormalLinesMap) {
  var diffs = parseDiff(diffData)
  diffs.forEach(diff => {
    fileToDeletedLinesMap[diff.from] = [];
    fileToNormalLinesMap[diff.from] = [];
    diff.chunks.forEach(chunk => {
      chunk.changes.forEach(change => {
        if (change.del) {
          fileToDeletedLinesMap[diff.from].push(change.ln)
        }
        if (change.normal) {
          fileToNormalLinesMap[diff.from].push(change.ln1)
        }
      })
    })
  })
}


function parseBlame(html) {
  let regular = /(<img alt="@([^"]+)" class="avatar blame-commit-avatar"|<tr class="blame-line")/g
  var match
  var currentUser
  var lines = []
  while ((match = regular.exec(html)) !== null) {
    let user = match[2]
    if (user) {
      currentUser = user
    } else {
      lines.push(currentUser)
    }
  }

  return lines
}

function findBlameReviewers(fileToDeletedLinesMap, fileToNormalLinesMap, fileToBlamesMap) {
  var reviewers = {};

  // deleted lines get 3 points, normal line get 1 point
  Object.keys(fileToDeletedLinesMap).forEach(function (file) {
    let deletedLines = fileToDeletedLinesMap[file]
    var blames = fileToBlamesMap[file]
    deletedLines.forEach(lineNumber => {
      var name = blames[lineNumber]
      reviewers[name] = (reviewers[name] || 0) + 3
    })
  });

  Object.keys(fileToNormalLinesMap).forEach(function (file) {
    let normalLines = fileToNormalLinesMap[file];
    var blames = fileToBlamesMap[file]
    normalLines.forEach(lineNumber => {
      var name = blames[lineNumber]
      reviewers[name] = (reviewers[name] || 0) + 1
    })
  });

  console.log('blame point:', reviewers)
  var names = Object.keys(reviewers)
  names.sort((name1, name2) => {
    return reviewers[name1] > reviewers[name2] ? -1 : 1 
  })

  var prUser = danger.github.pr.user.login
  names.splice(names.findIndex(el => {
    return el === prUser
  }), 1)

  if (names.length > 0) {
    if (names.length > 2) {
      names = names.slice(0, 2)
    }
    names = names.map(name => {
      return '@' + name
    })

    message("According to the blame info, we recommended " + names.join(' , ') + " to be the reviewers.")
  }
}

/*
 * find reviewer end
 */
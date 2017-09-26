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
var isNotDanger = false;
console.log('pr.title:'+danger.github.pr.title)
if(!isNotDanger && danger.github.pr.title 
  && danger.github.pr.title.match(/@notdanger/i)){
  isNotDanger = true;
}
console.log('pr.body:'+danger.github.pr.body)
if(!isNotDanger && danger.github.pr.body 
  && danger.github.pr.body.match(/@notdanger/i)){
  isNotDanger = true;
}
if(!isNotDanger){
  for (let c of danger.git.commits) {
    // console.log("msg:" + c.message);
    if (c.message && c.message.match(/@notdanger/i)) {
      isNotDanger = true;
      break;
    }
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

async function checkBreakChange(file){
  var diff = await danger.git.diffForFile(file);
  if (diff && diff.removed && diff.removed.match(/^-\s*?public\s+[\s\S]+$/gm)) {
    if (isNotDanger) {
      warn("Potential BREAK CHANGE. Modify public in   " + file);
    } else {
      warn(
        "Potential BREAK CHANGE. Modify public in " +
          file +
          " without metion it in commit message. You'd better add '@notdanger' in your commit log. "
      );
    }
  }
}

var has_sdk_changes = false;
var has_test_changes = false;
var has_doc_changes = false;
var filesToVerifySrcHeader = [];
var fileCount = 0;

const type_unknown = 0;
const type_ios_sdk = 1;
const type_android_sdk = 2;
const type_ios_test = 3;
const type_android_test = 4;
const type_jsfm = 5;
const type_jsfm_test = 6;
const type_doc = 7;
const type_ui_test = 8;

const getFileType = file => {
  if (file.match(/WeexSDK\/Sources\/.+\.(m|h|mm)/)) {
    return type_ios_sdk;
  } else if (file.match(/WeexSDKTests\//)) {
    return type_ios_test;
  } else if (file.match(/android\/sdk\/src\/test\/.+\.java/)) {
    return type_android_test;
  } else if (file.match(/android\/sdk\/src\/main\/java\/.+\.java/)) {
    return type_android_sdk;
  } else if (
    file.match(/html5\/(shared|frameworks|render|runtime|services)\/.+\.js/)
  ) {
    return type_jsfm;
  } else if (file.match(/html5\/test\/.+\.js/)) {
    return type_jsfm_test;
  } else if (file.match(/doc\/\.+\.md/)) {
    return type_doc;
  } else if(file.match(/test\/scripts\/.+\.js/) || file.match(/test\/pages\/.+\.vue/)){
    return type_ui_test
  }else{
    return type_unknown
  }
}

const checkChangedFile = file => {
  fileCount++;
  console.log("check changed file:"+file)
  let fileType = getFileType(file)

  has_sdk_changes =
    has_sdk_changes ||
    fileType == type_android_sdk ||
    fileType == type_ios_sdk ||
    fileType == type_jsfm;
  has_test_changes =
    has_test_changes ||
    fileType == type_android_test ||
    fileType == type_ios_test ||
    fileType == type_jsfm_test ||
    fileType == type_ui_test
  has_doc_changes = has_doc_changes || fileType == type_doc;

};

const checkAndroidBreakChange = file => {
  if (getFileType(file) == type_android_sdk && !is_ignored_public_check(file)) {
    schedule(async () => {
      await checkBreakChange(file);
    });
  }
}

const checkFileToVerifySrcHeader = file => {
  if (
    file.endsWith(".h") ||
    file.endsWith(".m") ||
    file.endsWith(".mm") ||
    file.endsWith(".java") ||
    file.endsWith(".js")
  ) {
    filesToVerifySrcHeader.push(file);
  }
};

if (danger.git.modified_files) {
  danger.git.modified_files.forEach(file => {
    checkChangedFile(file);
    checkAndroidBreakChange(file);
    checkFileToVerifySrcHeader(file);
  });
}

console.log('checkFileToVerifySrcHeader')
if (danger.git.created_files) {
  danger.git.created_files.forEach(file => {
    checkChangedFile(file);
    checkFileToVerifySrcHeader(file);
  });
}
console.log('checkAndroidBreakChange')
if (danger.git.deleted_files) {
  danger.git.deleted_files.forEach(file => {
    checkChangedFile(file);
    checkAndroidBreakChange(file);
  });
}

if (has_sdk_changes && !has_test_changes) {
  if(isNotDanger) warn("This PR modify SDK code without add/modify testcases.")
  // else fail("This PR modify SDK code. Please add/modify corresponding testcases. If it is ok, please comment about it. Or put '@notdanger' in you commit message.");
}

if (has_sdk_changes && !has_doc_changes) {
  warn("This PR should update related documents as well. ");
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
  'packages',
  'pre-build',
  'html5/test/case',
  'android/playground/app/src/main/assets',
  'android/sdk/assets',
  'ios/playground/bundlejs',
  'ios/sdk/WeexSDK/Resources',
  'ios/sdk/WeexSDK/Sources/Layout',
  'ios/sdk/WeexSDK/dependency/SRWebSocket'
]

console.log('copyright_header_components')
filesToVerifySrcHeader.forEach(filepath => {
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

console.log('findReviewer')
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
  var repoName = danger.github.pr.base.repo && danger.github.pr.base.repo.name
  github.pullRequests.get({
    owner: danger.github.pr.base.user.login,
    repo: repoName,
    number: danger.github.pr.number,
    headers: {Accept: 'application/vnd.github.diff',"user-agent": "node.js"}
  }, function (err, result) {
    console.log('parseDeleteAndNormalLines')
    if ("undefined" === typeof result || "undefined" === typeof result.data || err) {
      resolve()
      return
    }
    parseDeleteAndNormalLines(result.data, fileToDeletedLinesMap, fileToNormalLinesMap)
    console.log('getContent')
    var promises = danger.git.modified_files.map(function(file) {
      let repoURL = danger.github.pr.base.repo.html_url
      let fileName = file.replace(/^.*[\\\/]/, '')
      let blameURL = repoURL + '/blame/' + danger.github.pr.base.ref + '/' + file
      // console.log("Getting blame html: " + blameURL)
      console.log('getContent2')
      return getContent(blameURL)
    });

    console.log('findBlameReviewers')
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
    const request = lib.get(url, (function (url) {
      return (response) => {
        // handle http errors
        console.log('response:', response.statusCode)
        if (response.statusCode < 200 || response.statusCode > 299) {
          if (response.statusCode === 404  || response.statusCode === 502) {
            // ignore this, probably a renamed file,or .so that can't blame 
            return resolve('')
          }
          reject(new Error('Failed to load page, status code: ' + response.statusCode + ', '
            + ' url: ' + url));
        }
        // temporary data holder
        const body = [];
        // on every content chunk, push it to the data array
        response.on('data', (chunk) => body.push(chunk));
        // we are done, resolve promise with those joined chunks
        response.on('end', () => resolve(body.join('')));
      }
    })(url));
    // handle connection errors of the request
    request.on('error', (err) => reject(err))
    })
}

function parseDeleteAndNormalLines(diffData, fileToDeletedLinesMap, fileToNormalLinesMap) {
  try {
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
  } catch (error) {
    console.log(error)
  }

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
    if (!blames) {
      console.error(`failed to find blame info for (${file})`)
      return;
    }
    deletedLines.forEach(lineNumber => {
      var name = blames[lineNumber]
      if (!!reviewers) {
        reviewers[name] = (reviewers[name] || 0) + 3
      }
    })
  });

  Object.keys(fileToNormalLinesMap).forEach(function (file) {
    let normalLines = fileToNormalLinesMap[file];
    var blames = fileToBlamesMap[file]
    if (!blames) {
      console.error(`failed to find blame info for (${file})`)
      return;
    }
    normalLines.forEach(lineNumber => {
      var name = blames[lineNumber]
      if (!!reviewers) {
        reviewers[name] = (reviewers[name] || 0) + 1
      }
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

import { danger, fail, warn } from "danger";
import fs from "fs";
import path from 'path';

// Make sure there are changelog entries
// const hasChangelog = danger.git.modified_files.includes("changelog.md")
// if (!hasChangelog) { fail("No Changelog changes!") }

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
  return absolute('html5/' + rel)
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
  console.log("check file:" + file);
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
    file.endsWith(".mm")
  ) {
    codefiles.push(file);
  }
}
if(danger.git.added_files){
  for (let file of danger.git.added_files) {
    if (
      file.endsWith(".h") ||
      file.endsWith(".m") ||
      file.endsWith(".mm")
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
const copyright_header_components = [
  "Created by Weex.",
  "Copyright \\(c\\) .*, Alibaba, Inc. All rights reserved.",
  "This source code is licensed under the Apache Licence 2.0.",
  "For the full copyright and license information,please view the LICENSE file in the root directory of this source tree."
];

codefiles.forEach(filepath => {
  const content = fs.readFileSync(filepath).toString();
  for (const line of copyright_header_components) {
    if (!content.match(new RegExp(line))) {
      fail("Code file "+ filepath +" does not have the copyright header.");
      return;
    }
  }
});

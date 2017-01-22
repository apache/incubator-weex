# Warn when there is a big PR
warn("Big PR") if git.lines_of_code > 500

has_app_changes = !git.modified_files.grep(/WeexSDK\/Source/).empty?
has_test_changes = !git.modified_files.grep(/WeexSDKTests/).empty?

# Non-trivial amounts of app changes without tests
if git.lines_of_code > 100 && has_app_changes && !has_test_changes
  warn "This PR may need tests."
end

# Shows all build errors, warnings and unit tests results generated from `xcodebuild`
# xcode_summary.ignored_files = '**/Pods/**'
# xcode_summary.report 'ios/sdk/xcodebuild.json'

# Fails build when Copyright header is not included
oc_files = (git.modified_files + git.added_files).select do |line|
  line.end_with?(".h") || line.end_with?(".m") || line.end_with?(".mm")
end

copyright_header_components = Array.[](
  /Created by Weex./,
  /Copyright \\(c\\) .*, Alibaba, Inc. All rights reserved./,
  /This source code is licensed under the Apache Licence 2.0./,
  /For the full copyright and license information,please view the LICENSE file in the root directory of this source tree./
)

no_copyright_files = oc_files.select do |file_path|
  contents = File.read(file_path)
  for line in copyright_header_components
    if not contents =~ line
      return true
    end
  end

  return false
end

message(no_copyright_files)



# prose.check_spelling oc_files

# Find reviewers without parsing blame information
# from files matching to 'Pods/*'
# mention.run(2, ["Pods/*"], [])

message("Test Message")

# warn("Please add your name", file: "CHANGELOG.md", line: 4)

# fail("Our linter has failed.")

# markdown("## xxxxx")







# Warn when there is a big PR
warn("Big PR") if git.lines_of_code > 500

has_app_changes = !git.modified_files.grep(/WeexSDK\/Source/).empty?
has_test_changes = !git.modified_files.grep(/WeexSDKTests/).empty?

# Non-trivial amounts of app changes without tests
if git.lines_of_code > 100 && has_app_changes && !has_test_changes
  warn "This PR may need tests."
end

# Shows all build errors, warnings and unit tests results generated from `xcodebuild`
xcode_summary.ignored_files = '**/Pods/**'
xcode_summary.report 'ios/sdk/xcodebuild.json'


# Checks the presence of Xcode file headers.
clorox.level = "failure"
clorox.check ["ios/sdk"]

# prose.check_spelling oc_files

# Find reviewers without parsing blame information
# from files matching to 'Pods/*'
# mention.run(2, ["Pods/*"], [])

# message("Test Message")

# warn("Please add your name", file: "CHANGELOG.md", line: 4)

# fail("Our linter has failed.")

# markdown("## xxxxx")







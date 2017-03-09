# Error or Warn when delete public interface
metion_break_change = git.commits.any? { |c| c.message =~ /'breack change'/ }

# File name match any of these patterns will be ignored.
def is_ignored_public_check file
  ignored_break_change_pattern = Array.[](
    /^android\/sdk\/src\/test\/.+/,
    /^android\/playground\/.+/,
  )
  for pattern in ignored_break_change_pattern do
    if file =~ pattern
      return true
    end
  end
  return false
end


for file in git.modified_files do
  if not is_ignored_public_check(file) && file.end_with?("java")
    diff = git.diff_for_file(file)
    if diff && diff.patch =~ /^-\s*?public\s+[\s\S]+$/ 
      if metion_break_change
        warn("Potential BREAK CHANGE. Modify public in #{file}")
      else
        fail("Potential BREAK CHANGE. Modify public in #{file} without metion it in commit message. ")
      end
    end
  end
end

# Warn when there is a big PR
warn("Big PR") if git.lines_of_code > 500

has_app_changes = !git.modified_files.grep(/WeexSDK\/Source/).empty?
has_test_changes = !git.modified_files.grep(/WeexSDKTests/).empty?

# Non-trivial amounts of app changes without tests
if git.lines_of_code > 100 && has_app_changes && !has_test_changes
  warn "This PR may need tests."
end


# Fails build when Copyright header is not included
oc_files = (git.modified_files + git.added_files).uniq.select do |file_path|
  file_path.end_with?(".h") || file_path.end_with?(".m") || file_path.end_with?(".mm")
end

copyright_header_components = Array.[](
  /Created by Weex./,
  /Copyright \(c\) .*, Alibaba, Inc. All rights reserved./,
  /This source code is licensed under the Apache Licence 2.0./,
  /For the full copyright and license information,please view the LICENSE file in the root directory of this source tree./
)

def has_copyright_header contents, copyright
  for line in copyright do
    if not contents =~ line
      return false
    end
  end
  return true
end

for file_path in oc_files do
  contents = File.read(file_path)
  if not has_copyright_header(contents, copyright_header_components)
    fail("Objective-C file '#{file_path}' does not have the copyright header.")
  end
end

# prose.check_spelling oc_files

# Find reviewers without parsing blame information
# from files matching to 'Pods/*'
# mention.run(2, ["Pods/*"], [])


# warn("Please add your name", file: "CHANGELOG.md", line: 4)

# fail("Our linter has failed.")

# markdown("## xxxxx")
# frozen_string_literal: true
if !ENV['CIRCLECI']
  source "https://rubygems.org" do
    gem "xcpretty"
    gem "xcpretty-json-formatter"
    gem "danger"
    gem "danger-xcode_summary"
    gem "cocoapods"
    # fork by 'https://github.com/flix-tech/danger-code_style_validation.git'
    # update the message level from fail to warn
    gem 'danger-code_style_validation', :git => 'https://github.com/wrmswindmill/danger-code_style_validation.git'
  end
end
# gem "danger-prose"
# gem "danger-clorox"
# gem "danger-mention"

# gem "rails"

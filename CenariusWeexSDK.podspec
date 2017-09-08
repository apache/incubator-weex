# coding: utf-8
Pod::Spec.new do |s|

  s.name         = "CenariusWeexSDK"

  s.version      = "0.15.0.2"

  s.summary      = "WeexSDK Source ."

  s.description  = <<-DESC
                   A framework for building Mobile cross-platform UI
                   DESC

  s.homepage     = "https://github.com/alibaba/weex"
  s.license      = "MIT"
  s.authors      = { "M" => "myeveryheart@qq.com" }
  s.platform     = :ios
  s.ios.deployment_target = '10.0'
  s.source       = { :git => "https://github.com/macula-projects/cenarius-weex.git", :tag => "#{s.version}" }
  s.source_files = 'ios/sdk/WeexSDK/Sources/**/*.{h,m,mm,c}'
  s.resources = 'pre-build/native-bundle-main.js', 'ios/sdk/WeexSDK/Resources/wx_load_error@3x.png'

  s.user_target_xcconfig  = { 'FRAMEWORK_SEARCH_PATHS' => "'$(PODS_ROOT)/WeexSDK'" }
  s.requires_arc = true
  s.prefix_header_file = 'ios/sdk/WeexSDK/Sources/Supporting Files/WeexSDK-Prefix.pch'

  s.xcconfig = { "OTHER_LINK_FLAG" => '$(inherited) -ObjC'}

  s.frameworks = 'CoreMedia','MediaPlayer','AVFoundation','AVKit','JavaScriptCore', 'GLKit', 'OpenGLES', 'CoreText', 'QuartzCore', 'CoreGraphics'

  s.dependency 'SocketRocket'
  s.libraries = "stdc++"

end

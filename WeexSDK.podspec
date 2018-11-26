# coding: utf-8
Pod::Spec.new do |s|

  s.name         = "WeexSDK"

  s.version      = "0.19.0"

  s.summary      = "WeexSDK Source ."

  s.description  = <<-DESC
                   A framework for building Mobile cross-platform UI
                   DESC

  s.homepage     = "https://github.com/alibaba/weex"
  s.license = {
    :type => 'Copyright',
    :text => <<-LICENSE
           Alibaba-INC copyright
    LICENSE
  }
  s.authors      = { "cxfeng1"      => "cxfeng1@gmail.com",
                     "yangshengtao" => "yangshengtao1314@163.com",
                     "kfeagle"      => "sunjjbobo@163.com"
                   }
  s.platform     = :ios
  s.ios.deployment_target = '8.0'
  s.source =  { :path => '.' }
  s.source_files = 'ios/sdk/WeexSDK/Sources/**/*.{h,m,mm,c,cpp,cc}'
  s.resources = 'pre-build/*.js','ios/sdk/WeexSDK/Resources/wx_load_error@3x.png'

  s.user_target_xcconfig  = { 'FRAMEWORK_SEARCH_PATHS' => "'$(PODS_ROOT)/WeexSDK'" }
  s.requires_arc = true
  s.prefix_header_file = 'ios/sdk/WeexSDK/Sources/Supporting Files/WeexSDK-Prefix.pch'

  s.private_header_files = 'ios/sdk/WeexSDK/Sources/Component/RecycleList/WXJSASTParser.h',
                           'ios/sdk/WeexSDK/Sources/Layout/WXScrollerComponent+Layout.h',
                           'weex_core/Source/**/*.{h,hpp}'

  s.xcconfig = { "OTHER_LINK_FLAG" => '$(inherited) -ObjC', 'GCC_PREPROCESSOR_DEFINITIONS' => 'OS_IOS=1' }

  s.frameworks = 'CoreMedia','MediaPlayer','AVFoundation','AVKit','JavaScriptCore','GLKit','OpenGLES','CoreText','QuartzCore','CoreGraphics'
  
  s.default_subspec='WeexCore'

  s.subspec 'WeexCore' do |w|
    w.source_files = 'weex_core/Source/base/**/*.{h,hpp,m,mm,c,cpp,cc}',
                    'weex_core/Source/core/**/*.{h,hpp,m,mm,c,cpp,cc}',
                    'weex_core/Source/wson/**/*.{h,hpp,m,mm,c,cpp,cc}',
                    'weex_core/Source/third_party/**/*.{h,hpp,m,mm,c,cpp,cc}',
                    'weex_core/Source/include/**/*.{h,hpp,m,mm,c,cpp,cc}'
    w.exclude_files = 'weex_core/Source/**/*android.{h,hpp,m,mm,c,cpp,cc}'

    w.xcconfig = { 'USER_HEADER_SEARCH_PATHS' => ['${PODS_ROOT}/Headers/Public/WeexSDK/core/**'] }
    w.header_mappings_dir = 'weex_core/Source'

    w.libraries = "c++"
  end

end

# coding: utf-8

Pod::Spec.new do |s|

  s.name         = "WeexSDK"

  s.version      = "0.24.0"

  s.summary      = "WeexSDK Source."

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
  s.authors      = {
                    "cxfeng1"      => "cxfeng1@gmail.com",
                    "boboning"     => "ningli928@163.com",
                    "yangshengtao" => "yangshengtao1314@163.com",
                    "kfeagle"      => "sunjjbobo@163.com",
                    "acton393"     => "zhangxing610321@gmail.com",
                    "wqyfavor"     => "wqyfavor88@gmail.com",
                    "doumafang "   => "doumafang@gmail.com"
                   }
  s.platform     = :ios
  s.ios.deployment_target = '8.0'

  # use for public
  # s.source =  {
  #  :git => 'https://github.com/apache/incubator-weex.git',
  #  :tag => #{s.version}
  # }

  # use for playground
  s.source =  { :path => '.' }

  s.source_files = 'ios/sdk/WeexSDK/Sources/**/*.{h,m,mm,c,cpp,cc}',
                    'weex_core/Source/base/**/*.{h,hpp,m,mm,c,cpp,cc}',
                    'weex_core/Source/core/**/*.{h,hpp,m,mm,c,cpp,cc}',
                    'weex_core/Source/wson/**/*.{h,hpp,m,mm,c,cpp,cc}',
                    'weex_core/Source/third_party/**/*.{h,hpp,m,mm,c,cpp,cc}',
                    'weex_core/Source/include/**/*.{h,hpp,m,mm,c,cpp,cc}'
  s.exclude_files = 'weex_core/Source/**/*android.{h,hpp,m,mm,c,cpp,cc}',
                    'weex_core/Source/base/android',
                    'weex_core/Source/base/base64',
                    'weex_core/Source/base/crash',
                    'weex_core/Source/base/utils',
                    'weex_core/Source/base/thread',
                    'weex_core/Source/base/third_party',
                    'weex_core/Source/base/message_loop',
                    'weex_core/Source/base/time_point.*',
                    'weex_core/Source/base/time_utils.*',
                    'weex_core/Source/base/time_unit.*',
                    'weex_core/Source/third_party/IPC',
                    'weex_core/Source/core/network/android/',
                    'weex_core/Source/include/JavaScriptCore/',
                    'weex_core/Source/include/wtf'

  s.private_header_files = 'ios/sdk/WeexSDK/Sources/Component/RecycleList/WXJSASTParser.h'
  s.public_header_files = 'ios/sdk/WeexSDK/Sources/WeexSDK.h',
                          'ios/sdk/WeexSDK/Sources/Layout/WXComponent+Layout.h',
                          'ios/sdk/WeexSDK/Sources/Debug/WXDebugTool.h',
                          'ios/sdk/WeexSDK/Sources/Loader/WXResourceLoader.h',
                          'ios/sdk/WeexSDK/Sources/WebSocket/WXWebSocketHandler.h',
                          'ios/sdk/WeexSDK/Sources/Module/WXVoiceOverModule.h',
                          'ios/sdk/WeexSDK/Sources/Module/WXPrerenderManager.h',
                          'ios/sdk/WeexSDK/Sources/Module/WXModalUIModule.h',
                          'ios/sdk/WeexSDK/Sources/Module/WXStreamModule.h',
                          'ios/sdk/WeexSDK/Sources/Component/WXListComponent.h',
                          'ios/sdk/WeexSDK/Sources/Component/WXScrollerComponent.h',
                          'ios/sdk/WeexSDK/Sources/Component/WXRichText.h',
                          'ios/sdk/WeexSDK/Sources/Component/WXIndicatorComponent.h',
                          'ios/sdk/WeexSDK/Sources/Component/WXAComponent.h',
                          'ios/sdk/WeexSDK/Sources/Component/WXRefreshComponent.h',
                          'ios/sdk/WeexSDK/Sources/Component/Recycler/WXRecyclerComponent.h',
                          'ios/sdk/WeexSDK/Sources/Controller/WXBaseViewController.h',
                          'ios/sdk/WeexSDK/Sources/Controller/WXRootViewController.h',
			  'ios/sdk/WeexSDK/Sources/Handler/WXNavigationDefaultImpl.h',
                          'ios/sdk/WeexSDK/Sources/View/WXView.h',
                          'ios/sdk/WeexSDK/Sources/View/WXErrorView.h',
                          'ios/sdk/WeexSDK/Sources/Protocol/*.h',
                          'ios/sdk/WeexSDK/Sources/Network/WXResourceRequestHandler.h',
                          'ios/sdk/WeexSDK/Sources/Network/WXResourceRequest.h',
                          'ios/sdk/WeexSDK/Sources/Network/WXResourceResponse.h',
                          'ios/sdk/WeexSDK/Sources/Model/WXSDKInstance.h',
                          'ios/sdk/WeexSDK/Sources/Model/WXJSExceptionInfo.h',
                          'ios/sdk/WeexSDK/Sources/Model/WXComponent.h',
                          'ios/sdk/WeexSDK/Sources/Monitor/WXMonitor.h',
                          'ios/sdk/WeexSDK/Sources/Monitor/WXExceptionUtils.h',
                          'ios/sdk/WeexSDK/Sources/Monitor/WXAnalyzerCenter.h',
                          'ios/sdk/WeexSDK/Sources/Manager/WXSDKManager.h',
                          'ios/sdk/WeexSDK/Sources/Manager/WXBridgeManager.h',
                          'ios/sdk/WeexSDK/Sources/Manager/WXComponentManager.h',
                          'ios/sdk/WeexSDK/Sources/Manager/WXComponentFactory.h',
                          'ios/sdk/WeexSDK/Sources/Manager/WXInvocationConfig.h',
                          'ios/sdk/WeexSDK/Sources/Engine/WXSDKEngine.h',
                          'ios/sdk/WeexSDK/Sources/Engine/WXSDKError.h',
                          'ios/sdk/WeexSDK/Sources/Eagle/WXDataRenderHandler.h',
                          'ios/sdk/WeexSDK/Sources/Utility/WXConvert.h',
                          'ios/sdk/WeexSDK/Sources/Utility/WXUtility.h',
                          'ios/sdk/WeexSDK/Sources/Utility/WXConvertUtility.h',
                          'ios/sdk/WeexSDK/Sources/Utility/WXLog.h',
                          'ios/sdk/WeexSDK/Sources/Utility/WXDefine.h',
                          'ios/sdk/WeexSDK/Sources/Utility/WXType.h',
                          'ios/sdk/WeexSDK/Sources/Utility/NSObject+WXSwizzle.h',
                          'ios/sdk/WeexSDK/Sources/Utility/WXAppConfiguration.h',
                          'ios/sdk/WeexSDK/Sources/Performance/WXApmForInstance.h',
                          'ios/sdk/WeexSDK/Sources/Bridge/JSContext+Weex.h',
                          'ios/sdk/WeexSDK/Sources/Bridge/WXBridgeMethod.h',
                          'weex_core/Source/core/layout/flex_enum.h',
                          'weex_core/Source/core/layout/layout.h',
                          'weex_core/Source/core/layout/style.h',
                          'weex_core/Source/core/bridge/eagle_bridge.h'

  s.module_map = 'WeexSDK.modulemap'

  # 0.21.0 版本开始不再需要 native-bundle-main.js
  s.resources = 'pre-build/*.js','ios/sdk/WeexSDK/Resources/wx_load_error@3x.png'

  s.user_target_xcconfig  = { 'FRAMEWORK_SEARCH_PATHS' => "'$(PODS_ROOT)/WeexSDK'" }
  s.requires_arc = true
  s.prefix_header_file = 'ios/sdk/WeexSDK/Sources/Supporting Files/WeexSDK-Prefix.pch'

  s.xcconfig = { "OTHER_LINK_FLAG" => '$(inherited) -ObjC' }
  s.pod_target_xcconfig = { 'USER_HEADER_SEARCH_PATHS' => '${PODS_TARGET_SRCROOT}/weex_core/Source/ ${PROJECT_DIR}/../../../weex_core/Source',
    'GCC_PREPROCESSOR_DEFINITIONS' => 'OS_IOS=1' }

  s.frameworks = 'CoreMedia','MediaPlayer','AVFoundation','AVKit','JavaScriptCore','GLKit','OpenGLES','CoreText','QuartzCore','CoreGraphics'
  
  s.libraries = 'c++'

end

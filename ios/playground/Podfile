source 'https://github.com/CocoaPods/Specs.git'
platform :ios, '8.0'
#inhibit_all_warnings!

def common
    pod 'WeexSDK', :path=>'../../'
    pod 'WXDevtool', '0.15.3'
    pod 'SDWebImage', '3.7.5'
    pod 'SocketRocket', '0.4.2'
    pod 'ATSDK-Weex', '0.0.1',:configurations => ['Debug']
    
    # WeexGcanvas is added by Weex Plugin, more info at https://market.dotwe.org/ext/list.htm
    #pod 'WeexGcanvas'
end

target 'WeexDemo' do
    common
end

target 'WeexUITestDemo' do
    common
end

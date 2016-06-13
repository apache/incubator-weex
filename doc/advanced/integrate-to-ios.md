# import Weex iOS to your project

You will need to build Weex from source if you want to work on a new feature/bug fix, try out the latest features not released yet, or maintain your own fork with patches that cannot be merged to the core.

Assuming you have installed [iOS Develop Environment](https://developer.apple.com/library/ios/documentation/IDEs/Conceptual/AppStoreDistributionTutorial/Setup/Setup.html) and [CocoaPods](https://guides.cocoapods.org/using/getting-started.html). 

#### 1. Clone source from github

First, you need to git clone `weex` from github:

```
git clone https://github.com/alibaba/weex.git
```
#### 2. Import WeexSDK to project

Copy the whole folder `/ios/sdk` to your project directory.

Before adding the dependencies, please confirm that the project directory already exists the Podfile. If not, create a new one. Then, edit this file, adding some necessary dependecis for the target.

```
	platform :ios, '7.0'
	pod 'WeexSDK', :path=>'./sdk/'
```
Run pod install in current directory, for a while, .xcworkspace will be created.  At this point, the dependencies have been established.

#### 3. Init Weex Environment
We are used to doing some initial tasks in appDelegate. Of course, there is no exception. You can do this in `didFinishLaunchingWithOptions` as follows.

```
//business configuration
[WXAppConfiguration setAppGroup:@"AliApp"];
[WXAppConfiguration setAppName:@"WeexDemo"];
[WXAppConfiguration setAppVersion:@"1.0.0"];

//init sdk enviroment   
[WXSDKEngine initSDKEnviroment];
 
//register custom module and component，optional
[WXSDKEngine registerComponent:@"MyView" withClass:[MyViewComponent class]];
[WXSDKEngine registerModule:@"event" withClass:[WXEventModule class]];

//register the implementation of protocol, optional
[WXSDKEngine registerHandler:[WXNavigationDefaultImpl new] withProtocol:@protocol(WXNavigationProtocol)];

//set the log level    
[WXLog setLogLevel:WXLogLevelVerbose];

```

#### 4. Render Weex Instance
Weex supports two different modes, the full page rendering and part of page rendering. 
Something you have to do is to render weex view with specific URL, then add it to the parent container, which may be the viewController.

```
#import <WeexSDK/WXSDKInstance.h>

- (void)viewDidLoad 
{
	[super viewDidLoad];
	
	_instance = [[WXSDKInstance alloc] init];
	_instance.viewController = self;
    _instance.frame = self.view.frame; 
    [_instance renderWithURL:self.url options:@{@"bundleUrl":[self.url absoluteString]} data:nil];
    
    __weak typeof(self) weakSelf = self;
    _instance.onCreate = ^(UIView *view) {
        [weakSelf.weexView removeFromSuperview];
        [weakSelf.view addSubview:weakSelf.weexView];
    };
    
    _instance.onFailed = ^(NSError *error) {
    	//process failure
    };
    
    _instance.renderFinish = ^ (UIView *view) {
    	//process renderFinish
    };
}
```
WXSDKInstance is a very imporent class, which provides you with some basic methods and callbacks, such as renderWithURL、onCreate、onFailed and etc. You can understand their usage by reading WXSDKInstance.h.


#### 5. Destroy Weex Instance
Please release weex instance in dealloc stage of viewContoller, or it will lead to memory leak.

```
- (void)dealloc
{
    [_instance destroyInstance];
}
```
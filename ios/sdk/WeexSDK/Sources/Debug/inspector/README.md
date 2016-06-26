# WeeX debugger

## weex-debug启动：

0. 启动weex-debug

	0. $:npm install -g weex-debugger

	0. $:weex-debug   

		执行完会显示native需要链接的ws地址,之后自动打开chrome 。

## native 调用 
0. +(void)launchInspectorWithSocketUrl:(NSURL *)url，
 
   url为terminal显示的ws地址。

	 eg：[WXDebugTool launchInspectorWithSocketUrl:[NSURL URLWithString:@"ws://30.30.29.242:8088/debugProxy/native"]];

	启动app，此时chrome页会显示你的设备与app name，选择启动inspector和debugger开启调试模式

0. 如果你选择debugger模式，点击debugger按钮，此时app会提示您重启app，重启之后可以断点调试。



￼
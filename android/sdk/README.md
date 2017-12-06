## Unit Test

Unit test code is under 'src/test'. You can run unit test use command:   
```bash
./gradlew clean testDebugUnitTest jacocoTestReportDebug
```
This task will run all unit test and produce both unit test report and jacoco test coverage report in 'build/reports'.   

## clone by 0.17.0

### 已fix问题list:
* weex原先只有armeabi和x86版本，电竞只支持armeabi-v7a的so，需要将armeabi拷出一份放在v7a中，且修改WXSoInstallMgrSdk.initSo()方法，允许加载v7a的so
* fix WXScroller.addChild()中child参数为空导致的NPE问题
* fix 切换屏幕时，WXSDKInstance在页面pause之后调用setSize修改页面的宽高比例，在resume后修改回来，页面没有重新触发layout导致页面错误的问题（pause之后不再允许setSize方法调用）

### todo list:
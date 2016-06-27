### weex inspector

weex inspector is a customed devtools inspired by [Stetho](https://github.com/facebook/stetho). Work well both on emulator and real device.

##### How to use

- gradle dependency on inspector.
````
dependencies {
        compile(:inspector)
}
````

- initialize in your XXXApplication file.
````
    public class MyApplication extends Application {
      public void onCreate() {
      super.onCreate();
      initDebugEnv(true, CURRENT_IP);
      }
      private void initDebugEnv(boolean enable, String host) {
        WXEnvironment.sDebugMode = enable;
        WXEnvironment.sDebugWsUrl = "ws://" + host + ":8088/debugProxy/native";
      }
}
````

- enable network inspection.
````
OkHttpClient client = new OkHttpClient();
client.networkInterceptors().add(new OkHttpInterceptor());
````

- notice
  the network inspection only support OKHttpClient right now!!!
  if you want to use the network inspection to catch your bundle request, you must change your bundle ip to the real server ip. for example, in the playground you need to change the value of CURRENT_IP in WXApplication.java from "your_current_IP" to a server ip like "30.30.30.150";
 
- ship it
  1.you must launch your bundle server firstly. in your weex dir, run command "./start";
  2.then launch your remote debug server. run command "weex-debug", chrome will open a web show noting until you launch your app;
  3.if your app launched, you will see a device list in the chrome web view opened by last step, each device item have to button, "open debugger" and "open inspector";
  4.click the button "open inspector" chrome will open a tab show the inspector view, on the other side, click the button "open debugger" the chrome will open a new tab to show the debug view;

 
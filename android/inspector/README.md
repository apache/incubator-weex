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
        WeexInspector.initializeWithDefaults(this);
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
  if you want to use the network inspection to catch your bundle request, you must change your bundle ip to the real server ip. for example, in the playground you need to change the value of CURRENT_IP in WXMainActivity.java from "your_current_IP" to a server ip like "30.30.30.150";
 
- ship it
  1.launch your app and line to PC with USB;
  2.type "chrome://inspect" in your chrome browser and enter, you can enjoy it now!

 
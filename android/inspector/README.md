### weex inspector

weex inspector is a customed devtools inspired by [Stetho](https://github.com/facebook/stetho).

##### How to use?

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
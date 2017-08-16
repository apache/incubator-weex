cp ./libweexjsc.so ../sdk/libs/armeabi/
cp ./libweexjsb.so ../sdk/libs/armeabi/
cp ./libweexjss.so ../sdk/libs/armeabi/
cp ./libgnustl_shared.so ../sdk/libs/armeabi/
cp ./libyogacore.so ../sdk/libs/armeabi/

echo "cp libweexjsc.so libgnustl_shared.so libyogacore.so"
cd ..
./gradlew assembleDebug

echo "build playground apk success"

echo "install apk:"
echo "adb install -r ../playground/app/build/outputs/apk/playground-debug.apk"

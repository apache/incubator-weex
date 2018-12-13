cd ../android
rm -rf sdk/build/intermediates/bundles/default/jni/*
./gradlew clean assembleRelease
cp sdk/build/intermediates/bundles/default/jni/armeabi/libweexcore.so sdk/libs/armeabi/libweexcore.so
cp sdk/build/intermediates/bundles/default/jni/armeabi-v7a/libweexcore.so sdk/libs/armeabi-v7a/libweexcore.so
cp sdk/build/intermediates/bundles/default/jni/x86/libweexcore.so sdk/libs/x86/libweexcore.so

cd ../android
rm -rf sdk/build/intermediates/bundles/release/jni/*
./gradlew clean assemble
cp sdk/build/intermediates/bundles/release/jni/armeabi/libweexcore.so sdk/libs/armeabi/libweexcore.so
cp sdk/build/intermediates/bundles/release/jni/armeabi-v7a/libweexcore.so sdk/libs/armeabi-v7a/libweexcore.so
cp sdk/build/intermediates/bundles/release/jni/x86/libweexcore.so sdk/libs/x86/libweexcore.so

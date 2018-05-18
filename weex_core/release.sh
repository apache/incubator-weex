cd ../android
rm -rf sdk/build/intermediates/bundles/release/jni/*
./gradlew clean assemble
cp sdk/build/intermediates/bundles/release/jni/armeabi/libweexjsc.so sdk/libs/armeabi/libweexjsc.so
cp sdk/build/intermediates/bundles/release/jni/armeabi-v7a/libweexjsc.so sdk/libs/armeabi-v7a/libweexjsc.so
cp sdk/build/intermediates/bundles/release/jni/x86/libweexjsc.so sdk/libs/x86/libweexjsc.so

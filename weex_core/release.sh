cd ../android
rm -rf sdk/build/intermediates/bundles/default/jni/*
./gradlew clean assembleRelease
cp sdk/build/intermediates/bundles/default/jni/armeabi/libweexcore.so sdk/libs/armeabi/libweexcore.so
cp sdk/build/intermediates/bundles/default/jni/armeabi-v7a/libweexcore.so sdk/libs/armeabi-v7a/libweexcore.so
cp sdk/build/intermediates/bundles/default/jni/x86/libweexcore.so sdk/libs/x86/libweexcore.so

# cp jss so

cp sdk/build/intermediates/bundles/default/jni/armeabi/libweexjss.so sdk/libs/armeabi/libweexjss.so
cp sdk/build/intermediates/bundles/default/jni/armeabi-v7a/libweexjss.so sdk/libs/armeabi-v7a/libweexjss.so
cp sdk/build/intermediates/bundles/default/jni/x86/libweexjss.so sdk/libs/x86/libweexjss.so

# back up obj

#rm -rf ~/Desktop/weex_so_armeabi
#mkdir ~/Desktop/weex_so_armeabi
#cp sdk/build/intermediates/cmake/release/obj/armeabi/libweexjss.so ~/Desktop/weex_so_armeabi/
#cp sdk/build/intermediates/cmake/release/obj/armeabi/libweexcore.so ~/Desktop/weex_so_armeabi/
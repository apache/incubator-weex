cd ../android
#rm -rf sdk/build/intermediates/bundles/default/jni/*
./gradlew  assembleRelease clean  -PbuildRuntimeApi=true

#cp sdk/build/intermediates/bundles/default/jni/armeabi/libweexcore.so sdk/libs/armeabi/libweexcore.so
#cp sdk/build/intermediates/bundles/default/jni/armeabi-v7a/libweexcore.so sdk/libs/armeabi-v7a/libweexcore.so
#cp sdk/build/intermediates/bundles/default/jni/x86/libweexcore.so sdk/libs/x86/libweexcore.so

# cp jss so

cp sdk/build/intermediates/intermediate-jars/release/jni/arm64-v8a/libweexjss.so sdk/libs/arm64-v8a/libweexjssr.so
cp sdk/build/intermediates/intermediate-jars/release/jni/armeabi-v7a/libweexjss.so sdk/libs/armeabi-v7a/libweexjssr.so
cp sdk/build/intermediates/intermediate-jars/release/jni/x86/libweexjss.so sdk/libs/x86/libweexjssr.so




# back up obj

rm -rf ~/Desktop/weex_so_armeabi
mkdir ~/Desktop/weex_so_armeabi
cp sdk/build/intermediates/cmake/release/obj/arm64-v8a/libweexjss.so ~/Desktop/weex_so_armeabi/libweexjssr_64_v8a.so
cp sdk/build/intermediates/cmake/release/obj/armeabi-v7a/libweexjss.so ~/Desktop/weex_so_armeabi/libweexjssr_v7a.so
cp sdk/build/intermediates/cmake/release/obj/x86/libweexjss.so ~/Desktop/weex_so_armeabi/libweexjssr_x86.so



./gradlew  assembleRelease -PbuildRuntimeApi=false
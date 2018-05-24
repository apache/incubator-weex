cd ../android/sdk
gradle compileDebugSources
cd ..
cp sdk/build/intermediates/cmake/debug/obj/armeabi/libweexjsc.so sdk/libs/armeabi/libweexjsc.so
cp sdk/build/intermediates/cmake/debug/obj/armeabi-v7a/libweexjsc.so sdk/libs/armeabi-v7a/libweexjsc.so
cp sdk/build/intermediates/cmake/debug/obj/x86/libweexjsc.so sdk/libs/x86/libweexjsc.so

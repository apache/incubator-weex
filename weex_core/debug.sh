cd ../android/sdk
gradle compileDebugSources
cd ..
cp sdk/build/intermediates/cmake/debug/obj/armeabi/libweexcore.so sdk/libs/armeabi/libweexcore.so
cp sdk/build/intermediates/cmake/debug/obj/armeabi-v7a/libweexcore.so sdk/libs/armeabi-v7a/libweexcore.so
cp sdk/build/intermediates/cmake/debug/obj/x86/libweexcore.so sdk/libs/x86/libweexcore.so

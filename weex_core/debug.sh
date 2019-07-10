cd ../android/sdk
../gradlew assembleDebug
cd ..
cp sdk/build/intermediates/cmake/debug/obj/armeabi/libweexcore.so sdk/libs/armeabi/libweexcore.so
##cp sdk/build/intermediates/cmake/debug/obj/armeabi-v7a/libweexcore.so sdk/libs/armeabi-v7a/libweexcore.so
##cp sdk/build/intermediates/cmake/debug/obj/x86/libweexcore.so sdk/libs/x86/libweexcore.so

cp sdk/build/intermediates/cmake/debug/obj/armeabi/libweexjss.so sdk/libs/armeabi/libweexjss.so
##cp sdk/build/intermediates/cmake/debug/obj/armeabi-v7a/libweexjss.so sdk/libs/armeabi-v7a/libweexjss.so
##cp sdk/build/intermediates/cmake/debug/obj/x86/libweexjss.so sdk/libs/x86/libweexjss.so
export ANDROID_NDK='/Users/furture/Library/Android/sdk/android-ndk-r16b'
${ANDROID_NDK}/toolchains/arm-linux-androideabi-4.9/prebuilt/darwin-x86_64/arm-linux-androideabi/bin/strip -s sdk/libs/armeabi/libweexcore.so
${ANDROID_NDK}/toolchains/arm-linux-androideabi-4.9/prebuilt/darwin-x86_64/arm-linux-androideabi/bin/strip -s sdk/libs/armeabi/libweexjss.so

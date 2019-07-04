echo "--> # buils jsc_runtime so"


../gradlew assembleRelease  -PbuildRuntimeApi=true 


echo "--> ## cp jss_runtime so"

unzip -o build/outputs/aar/weex_sdk-release.aar -d build/outputs/aar/weex_sdk-release


cp build/outputs/aar/weex_sdk-release/jni/armeabi-v7a/libweexjss.so libs/armeabi-v7a/libweexjssr.so
cp build/outputs/aar/weex_sdk-release/jni/arm64-v8a/libweexjss.so libs/arm64-v8a/libweexjssr.so
cp build/outputs/aar/weex_sdk-release/jni/x86/libweexjss.so libs/x86/libweexjssr.so


echo "--> ## cp jss_runtime Symbol so"

rm -rf unstrippedSo
mkdir unstrippedSo

cp build/unstrippedSo/armeabi-v7a_libweexjss.so unstrippedSo/armeabi-v7a_libweexjssr.so
cp build/unstrippedSo/arm64-v8a_libweexjss.so unstrippedSo/arm64-v8a_libweexjssr.so
cp build/unstrippedSo/x86_libweexjss.so unstrippedSo/x86_libweexjssr.so
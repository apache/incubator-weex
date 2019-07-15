# Build from source
## Install gradlew   
  See https://gradle.org/install#with-a-package-manager
## Build Weex   
run `gradle build`   
Playground app artifact will be under 'playground/build'
## The submodule weex-playground
[Weex Playground](https://github.com/apache/incubator-weex-playground) can be used to test and preview weex pages on Android and it has been added as a submodule.
### Relationship between weex and playground
* The playground submodule will be cloned and updated automatically when developers clone the ‘incubator-weex’ repo and build it.In this case, playground project will implement the source code of weex-sdk.
* When developer directly clone the 'weex-playground' repo and run it separately, the playground project will implement a corresponding aar version of weex-sdk.
### When to publish SNAPSHOT
When some commits of  playground rely on unpublished features of weex-sdk,a SNAPSHOT of weex-sdk will be needed to support it.
### How to publish SNAPSHOT
 `cd android `  
`./gradlew clean install ArtifactoryPublish -PweexVersion=$PUBLISH_VERSION -PbintrayUser=alibabaweex -PbintrayApiKey=$JCENTER_TOKEN `  

* Explanation for variable:  
$PUBLISH_VERSION The version of snapshot, like 0.26.1.3-SNAPSHOT  
$JCENTER_TOKEN The private key for JCenter (https://bintray.com/alibabaweex/maven/weex_sdk/), which is the distribution channel for Android




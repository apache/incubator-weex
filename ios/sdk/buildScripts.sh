#!/bin/sh

# generateFileHeader
# param 1：headerFilePath
function generateFileHeader() {
	headerFilePath=$1
	headerFileName=`basename $1`
	echo '/*' > $headerFilePath
echo '* Licensed to the Apache Software Foundation (ASF) under one 
* or more contributor license agreements.  See the NOTICE file
* distributed with this work for additional information
* regarding copyright ownership.  The ASF licenses this file
* to you under the Apache License, Version 2.0 (the
* "License"); you may not use this file except in compliance
* with the License.  You may obtain a copy of the License at
*
*   http://www.apache.org/licenses/LICENSE-2.0
*
* Unless required by applicable law or agreed to in writing,
* software distributed under the License is distributed on an
* "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
* KIND, either express or implied.  See the License for the
* specific language governing permissions and limitations
* under the License.' >> $headerFilePath
	echo ' */' >> $headerFilePath
	echo '' >> $headerFilePath
	echo '#ifdef __OBJC__' >> $headerFilePath
	echo '' >> $headerFilePath
	return 0;
}

# generateFileFooter
# param 1：headerFilePath
function generateFileFooter() {
	headerFilePath=$1
	echo '' >> $headerFilePath
	echo '#endif /* __OBJC__ */' >> $headerFilePath
	return 0;
}

# findHeaders
# param 1：projectFilePath
# param 2：searchPattern
function findHeaders() {
	projectFilePath=$1
	searchPattern=$2
	grep -o "[0-9A-F]\{23\}.*[0-9a-zA-Z+_-]*\.h.*PBXBuildFile.*ATTRIBUTES.*=.*(${searchPattern}" "$projectFilePath/project.pbxproj" | grep -o '[0-9A-Za-z+_-]*\.h' | grep -v 'Weex-.*\.h' | uniq | sort -r
	return 0;
}

# generateImport
# param 1：projectFilePath
# param 2：searchPattern
# param 3：headerFilePath
# param 4：sdkName
# param 5：excludeHeaders
# param 6：externalHeader
function generateImport() {
	projectFilePath=$1
	searchPattern=$2
	headerFilePath=$3
	sdkName=$4
	excludeHeaders=$5
	externalHeader=$6

	if [[ $externalHeader ]]; then
		if [ "$searchPattern" = 'Private' ]; then
			echo "#import <${PRODUCT_NAME}/$externalHeader>" >> $headerFilePath
		else
			echo "#import \"$externalHeader\"" >> $headerFilePath
		fi
	fi
	headers=$(findHeaders $projectFilePath $searchPattern)
	headers=$(awk -v RS=' ' '!headers[$1]++' <<< ${headers[@]})
	echo $headers

	for header in $headers; do
		if [ "$searchPattern" = 'Private' ]; then
			echo "#import <${PRODUCT_NAME}/$header>" >> $headerFilePath
		else
			if [ "$header" = "${sdkName}.h" ];then
				continue
			fi

			exclude="no"
			for excludeHeader in ${excludeHeaders[@]}; do
				if [ "$excludeHeader" = "$header" ]; then
					exclude='yes'
					break
				fi
			done
			if [ "$exclude" = "yes" ]; then
				continue
			fi
			echo "#import \"$header\"" >> $headerFilePath
		fi
	done
	return 0;
}

# generateHeader
# param 1：projectPath
# param 2：headerFilePath
# param 3：searchPattern
# param 4：sdkName
# param 5：excludeHeaders
# param 6：externalHeader
function generateHeader() {
	projectPath=$1
	headerFilePath=$2
	searchPattern=$3
	sdkName=$4
	excludeHeaders=$5
	externalHeader=$6
	generateFileHeader $headerFilePath
	generateImport $projectPath $searchPattern $headerFilePath $sdkName "${excludeHeaders}" $externalHeader
	generateFileFooter $headerFilePath
	return 0
}

# generateSDKHeader
# param 1: sdkName
# param 2：excludeHeaders
# param 3：supportPrivate
function generateSDKHeader() {
	sdkName=$1
	excludeHeaders=$2
	supportPrivate=$3
	headerFilePath="${PROJECT_DIR}/${sdkName}/Sources"
	publicHeaderFilePath="${headerFilePath}/${sdkName}.h"

	if [ -f "$publicHeaderFilePath" ]; then
		rm $publicHeaderFilePath
	fi
	generateHeader "${PROJECT_DIR}/${PROJECT_NAME}.xcodeproj" "${publicHeaderFilePath}" 'Public' $sdkName "${excludeHeaders}"
}

# generateBuildTime
# param 1: file
function generateBuildTime() {
	file=$1
	if [ -f "$file" ]; then
		now="`date -u "+%Y-%m-%d %H:%M:%S"`";
		nowStamp="`date +%s`";

		echo '/*' > $file
		echo '* Licensed to the Apache Software Foundation (ASF) under one 
* or more contributor license agreements.  See the NOTICE file
* distributed with this work for additional information
* regarding copyright ownership.  The ASF licenses this file
* to you under the Apache License, Version 2.0 (the
* "License"); you may not use this file except in compliance
* with the License.  You may obtain a copy of the License at
*
*   http://www.apache.org/licenses/LICENSE-2.0
*
* Unless required by applicable law or agreed to in writing,
* software distributed under the License is distributed on an
* "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
* KIND, either express or implied.  See the License for the
* specific language governing permissions and limitations
* under the License.' >> $file
		echo ' */' >> $file
		echo '' >> $file
		echo "#import \"WXVersion.h\"
#import \"WXDefine.h\"

static const char* WeexSDKBuildTime = \"$now UTC\";
static const unsigned long WeexSDKBuildTimestamp = $nowStamp;

NSString* GetWeexSDKVersion(void)
{
    return WX_SDK_VERSION;
}

NSString* GetWeexSDKBuildTime(void)
{
    return [NSString stringWithUTF8String:WeexSDKBuildTime];
}

unsigned long GetWeexSDKBuildTimestamp(void)
{
    return WeexSDKBuildTimestamp;
}
" >> $file
	fi
}

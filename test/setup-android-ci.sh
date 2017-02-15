function installAndroidSDK {
    brew install android-sdk
    export ANDROID_HOME=/usr/local/opt/android-sdk
    export PATH=$PATH:$ANDROID_HOME/tools:$ANDROID_HOME/platform-tools
    echo yes | android update sdk --all --no-ui --force -t 2 #platform tools
    echo yes | android update sdk --all --no-ui --force -t 1 #tools
    echo yes | android update sdk --all --no-ui --force -t 11 #build-tool
    echo yes | android update sdk --all --no-ui --force -t 39 #sdk android-19
    echo yes | android update sdk --all --no-ui --force -t 35 #sdk android-23
    echo yes | android update sdk --all --no-ui --force -t 96 #sys-img
    echo yes | android update sdk --all --no-ui --force -t 160 #support
}

function createAVD {
    echo no | android create avd --force -n test -t android-19 --abi default/armeabi-v7a
}

function waitForEmulator {
  local bootanim=""
  export PATH=$(dirname $(dirname $(which android)))/platform-tools:$PATH
  until [[ "$bootanim" =~ "stopped" ]]; do
    sleep 5
    bootanim=$(adb -e shell getprop init.svc.bootanim 2>&1)
    echo "emulator status=$bootanim"
  done
  adb shell input keyevent 82 &
}

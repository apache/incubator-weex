#!/bin/bash -eu

function installAndroidSDK {
    # brew install android-sdk
    # export ANDROID_HOME=/usr/local/opt/android-sdk
    # export PATH=$PATH:$ANDROID_HOME/tools:$ANDROID_HOME/platform-tools
    echo yes | android update sdk --all --no-ui --force -t platform-tools #platform tools
    echo yes | android update sdk --all --no-ui --force -t tools #tools
    echo yes | android update sdk --all --no-ui --force -t build-tools-23.0.2 #build-tool
    echo yes | android update sdk --all --no-ui --force -t android-19 #sdk android-19
    echo yes | android update sdk --all --no-ui --force -t android-23 #sdk android-23
    echo yes | android update sdk --all --no-ui --force -t sys-img-armeabi-v7a-android-21 #sys-img
    echo yes | android update sdk --all --no-ui --force -t extra-android-m2repository #support
}

function createAVD {
    echo no | android create avd --force -n weexavd -t android-22 --abi default/armeabi-v7a
}

function startAVD {
  emulator -avd weexavd -no-audio -no-window
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

function installNode {
    curl -o- https://raw.githubusercontent.com/creationix/nvm/v0.33.1/install.sh | bash
    export NVM_DIR="$HOME/.nvm"
    bash "$NVM_DIR/nvm.sh"
    export CHROME_BIN=chromium-browser
    nvm install 7.0.0
    nvm use 7.0.0
}

function printEnvInfo {
    node -v
    nvm ls
    printenv
}



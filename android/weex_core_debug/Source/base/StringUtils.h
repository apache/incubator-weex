#ifndef _STRING_UTILS_H_
#define _STRING_UTILS_H_

#include <jni.h>
#include <IPC/IPCArguments.h>
#include <IPC/Serializing/IPCSerializer.h>
#include <IPC/IPCString.h>
#include <IPC/IPCByteArray.h>
#include <malloc.h>
#include "ScopedJString.h"
#include <iostream>
#include <sstream>
#include <string>

namespace WeexCore {

  static std::string jString2Str(JNIEnv *env, jstring jstr) {
    char *rtn = NULL;
    jclass clsstring = env->FindClass("java/lang/String");
    jstring strencode = env->NewStringUTF("GB2312");
    jmethodID mid = env->GetMethodID(clsstring, "getBytes", "(Ljava/lang/String;)[B");
    jbyteArray barr = (jbyteArray) env->CallObjectMethod(jstr, mid, strencode);
    jsize alen = env->GetArrayLength(barr);
    jbyte *ba = env->GetByteArrayElements(barr, JNI_FALSE);
    if (alen > 0) {
      rtn = (char *) malloc(alen + 1);
      memcpy(rtn, ba, alen);
      rtn[alen] = 0;
    }
    env->ReleaseByteArrayElements(barr, ba, 0);

    env->DeleteLocalRef(clsstring);
    env->DeleteLocalRef(strencode);
    env->DeleteLocalRef(barr);

    std::string stemp(rtn);
    free(rtn);
    return stemp;
  }

  static std::string jByteArray2Str(JNIEnv *env, jbyteArray barr) {
    char *rtn = NULL;
    jsize alen = env->GetArrayLength(barr);
    jbyte *ba = env->GetByteArrayElements(barr, JNI_FALSE);
    if (alen > 0) {
      rtn = (char *) malloc(alen + 1);
      memcpy(rtn, ba, alen);
      rtn[alen] = 0;
    }
    env->ReleaseByteArrayElements(barr, ba, 0);

    std::string stemp(rtn);
    free(rtn);
    return stemp;
  }

  static const char *GetUTFChars(JNIEnv *env, jstring str) {
    const char *c_str = NULL;
    if (env) {
      c_str = env->GetStringUTFChars(str, NULL);
    }
    env->ReleaseStringUTFChars(str, c_str);
    return c_str;
  }

//static jstring Char2JString(JNIEnv* env, const char* pat) {
//    //定义java String类 strClass
//    jclass strClass = (env)->FindClass("Ljava/lang/String;");
//    //获取String(byte[],String)的构造器,用于将本地byte[]数组转换为一个新String
//    jmethodID ctorID = (env)->GetMethodID(strClass, "<init>", "([BLjava/lang/String;)V");
//    //建立byte数组
//    jbyteArray bytes = (env)->NewByteArray(strlen(pat));
//    //将char* 转换为byte数组
//    (env)->SetByteArrayRegion(bytes, 0, strlen(pat), (jbyte*) pat);
//    // 设置String, 保存语言类型,用于byte数组转换至String时的参数
//    jstring encoding = (env)->NewStringUTF("GB2312");
//    //将byte数组转换为java String,并输出
//
//    jstring ret = (jstring) (env)->NewObject(strClass, ctorID, bytes, encoding);
//
//    env->DeleteLocalRef(strClass);
//    env->DeleteLocalRef(bytes);
//    env->DeleteLocalRef(encoding);
//
//    return ret;
//}



  static jstring &Str2JString(JNIEnv *env, const std::string &nativeString) {
    jstring str = env->NewStringUTF(nativeString.c_str());
    return str;
  }

  static jbyteArray String2JByteArray(JNIEnv *env, std::string &str) {
    jbyteArray ba = nullptr;
    // CString strData = str.utf8();
    // int strLen = str.size();
    // ba = env->NewByteArray(strLen);
    // env->SetByteArrayRegion(ba, 0, strLen, reinterpret_cast<const jbyte *>(strData.data()));
    return ba;
  }

  static jstring getArgumentAsJString(JNIEnv *env, IPCArguments *arguments, int argument) {
    jstring ret = nullptr;
    if (arguments->getType(argument) == IPCType::STRING) {
      const IPCString *s = arguments->getString(argument);
      ret = env->NewString(s->content, s->length);
    }
    return ret;
  }

  static jbyteArray getArgumentAsJByteArray(JNIEnv *env, IPCArguments *arguments, size_t argument) {
    jbyteArray ba = nullptr;
    if (argument >= arguments->getCount())
      return nullptr;
    if (arguments->getType(argument) == IPCType::BYTEARRAY) {
      const IPCByteArray *ipcBA = arguments->getByteArray(argument);
      int strLen = ipcBA->length;
      ba = env->NewByteArray(strLen);
      env->SetByteArrayRegion(ba, 0, strLen,
                              reinterpret_cast<const jbyte *>(ipcBA->content));
    }
    return ba;
  }

  static void addString(JNIEnv *env, IPCSerializer *serializer, jstring str) {
    ScopedJString scopedString(env, str);
    const uint16_t *chars = scopedString.getChars();
    size_t charsLength = scopedString.getCharsLength();
    serializer->add(chars, charsLength);
  }

  static void addJSONString(JNIEnv *env, IPCSerializer *serializer, jstring str) {
    ScopedJString scopedString(env, str);
    const uint16_t *chars = scopedString.getChars();
    size_t charsLength = scopedString.getCharsLength();
    serializer->addJSON(chars, charsLength);
  }

  template<class Type>
  static Type stringToNum(const std::string &str) {
    std::istringstream iss(str);
    Type num;
    iss >> num;
    return num;
  }

}
#endif //_STRING_UTILS_H_

#ifndef _STRING_UTILS_H_
#define _STRING_UTILS_H_

#include <jni.h>
#include "ScopedJString.h"
#include "../IPC/IPCArguments.h"
#include "wtf/text/WTFString.h"
#include "Serializing/IPCSerializer.h"
#include "Buffering/IPCBuffer.h"
#include "IPCArguments.h"
#include "IPCByteArray.h"
#include "IPCException.h"
#include "IPCHandler.h"
#include "IPCMessageJS.h"
#include "IPCResult.h"
#include "IPCSender.h"
#include "IPCString.h"

namespace WeexCore
{

static std::string jString2Str(JNIEnv *env, jstring jstr)
{
    char *rtn = NULL;
    jclass clsstring = env->FindClass("java/lang/String");
    jstring strencode = env->NewStringUTF("GB2312");
    jmethodID mid = env->GetMethodID(clsstring, "getBytes", "(Ljava/lang/String;)[B");
    jbyteArray barr = (jbyteArray)env->CallObjectMethod(jstr, mid, strencode);
    jsize alen = env->GetArrayLength(barr);
    jbyte *ba = env->GetByteArrayElements(barr, JNI_FALSE);
    if (alen > 0)
    {
        rtn = (char *)malloc(alen + 1);
        memcpy(rtn, ba, alen);
        rtn[alen] = 0;
    }
    env->ReleaseByteArrayElements(barr, ba, 0);
    std::string stemp(rtn);
    free(rtn);
    return stemp;
}

static void jString2Log(JNIEnv *env, jstring instance, jstring str)
{
    if (str != NULL)
    {
        const char *c_instance = env->GetStringUTFChars(instance, NULL);
        const char *c_str = env->GetStringUTFChars(str, NULL);
        if (c_str)
        {
            LOGA("jsLog>>> instance :%s,c_str:%s", c_instance, c_str);
        }
        env->ReleaseStringUTFChars(instance, c_instance);
        env->ReleaseStringUTFChars(str, c_str);
    }
}

static String jString2String(JNIEnv *env, jstring str)
{
    if (str != NULL)
    {
        ScopedJString scopedstr(env, str);
        size_t length = scopedstr.getCharsLength();
        const jchar *str = scopedstr.getChars();
        UChar *dst;
        String s = String::createUninitialized(length, dst);
        memcpy(dst, str, length * sizeof(UChar));
        return s;
    }
    return String("");
}

static jstring String2JString(JNIEnv *env, String &s)
{
    jstring ret = env->NewStringUTF(s.utf8().data());
    return ret;
}

static jbyteArray String2JByteArray(JNIEnv *env, String &str)
{
    jbyteArray ba = nullptr;
    CString strData = str.utf8();
    int strLen = strData.length();
    ba = env->NewByteArray(strLen);
    env->SetByteArrayRegion(ba, 0, strLen, reinterpret_cast<const jbyte *>(strData.data()));
    return ba;
}

/**
 * WTFString to char*
 */
static char* wtfString2cstr(const String str_temp)
{
    char* result = (char*)malloc(str_temp.length()+1);
    strcpy(result,str_temp.utf8().data());
    return result;
}

static jstring getArgumentAsJString(JNIEnv* env, IPCArguments* arguments, int argument)
{
    jstring ret = nullptr;
    if (arguments->getType(argument) == IPCType::STRING) {
        const IPCString* s = arguments->getString(argument);
        ret = env->NewString(s->content, s->length);
    }
    return ret;
}

static jbyteArray getArgumentAsJByteArray(JNIEnv* env, IPCArguments* arguments, size_t argument)
{
    jbyteArray ba = nullptr;
    if (argument >= arguments->getCount())
        return nullptr;
    if (arguments->getType(argument) == IPCType::BYTEARRAY) {
        const IPCByteArray* ipcBA = arguments->getByteArray(argument);
        int strLen = ipcBA->length;
        ba = env->NewByteArray(strLen);
        env->SetByteArrayRegion(ba, 0, strLen,
            reinterpret_cast<const jbyte*>(ipcBA->content));
    }
    return ba;
}

static void addString(JNIEnv* env, IPCSerializer* serializer, jstring str)
{
    ScopedJString scopedString(env, str);
    const uint16_t* chars = scopedString.getChars();
    size_t charsLength = scopedString.getCharsLength();
    serializer->add(chars, charsLength);
}

static void addJSONString(JNIEnv* env, IPCSerializer* serializer, jstring str)
{
    ScopedJString scopedString(env, str);
    const uint16_t* chars = scopedString.getChars();
    size_t charsLength = scopedString.getCharsLength();
    serializer->addJSON(chars, charsLength);
}

}
#endif //_STRING_UTILS_H_

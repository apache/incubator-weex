#ifndef _SCOPED_JSTRING_UTF_8_H_
#define _SCOPED_JSTRING_UTF_8_H_

#include <locale.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <thread>
#include <type_traits>
#include <wtf/CommaPrinter.h>
#include <wtf/CurrentTime.h>
#include <wtf/MainThread.h>
#include <wtf/NeverDestroyed.h>
#include <wtf/StringPrintStream.h>
#include <wtf/text/StringBuilder.h>
#include "./base64/base64.h"
#include <jni.h>

namespace WeexCore {

class ScopedJStringUTF8 {
public:
    ScopedJStringUTF8(JNIEnv *env, jstring);

    ~ScopedJStringUTF8();

    const char *getChars();

private:
    JNIEnv *m_env;
    jstring m_jstring;
    const char *m_chars;
};

}
#endif //_SCOPED_JSTRING_UTF_8_H_

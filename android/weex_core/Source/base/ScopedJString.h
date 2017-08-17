#ifndef _SCOPED_JSTRING_H_
#define _SCOPED_JSTRING_H_

#include "./base64/base64.h"
#include <jni.h>

namespace WeexCore {

class ScopedJString {
public:
    ScopedJString(JNIEnv *env, jstring);

    ~ScopedJString();

    const jchar *getChars();

    size_t getCharsLength();

private:
    JNIEnv *m_env;
    jstring m_jstring;
    const uint16_t *m_chars;
    size_t m_len;
};

}
#endif //_SCOPED_JSTRING_H_

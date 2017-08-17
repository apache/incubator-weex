#include "ScopedJStringUTF8.h"

namespace WeexCore {

ScopedJStringUTF8::ScopedJStringUTF8(JNIEnv *env, jstring _jstring)
        : m_env(env), m_jstring(_jstring), m_chars(nullptr) {
}

ScopedJStringUTF8::~ScopedJStringUTF8() {
  if (m_chars)
    m_env->ReleaseStringUTFChars(m_jstring, m_chars);
}

const char *ScopedJStringUTF8::getChars() {
  if (m_chars)
    return m_chars;
  m_chars = m_env->GetStringUTFChars(m_jstring, nullptr);
  return m_chars;
}

} //WeexCore
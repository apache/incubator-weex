#include "ScopedJString.h"

namespace WeexCore {

ScopedJString::ScopedJString(JNIEnv *env, jstring _jstring)
        : m_env(env), m_jstring(_jstring), m_chars(nullptr), m_len(0) {
}

ScopedJString::~ScopedJString() {
  if (m_chars)
    m_env->ReleaseStringChars(m_jstring, m_chars);
}

const jchar *ScopedJString::getChars() {
  if (m_chars)
    return m_chars;
  m_chars = m_env->GetStringChars(m_jstring, nullptr);
  m_len = m_env->GetStringLength(m_jstring);
  return m_chars;
}

size_t ScopedJString::getCharsLength() {
  if (m_chars)
    return m_len;
  m_len = m_env->GetStringLength(m_jstring);
  return m_len;
}

} //WeexCore
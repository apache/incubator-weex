#ifndef TRACE_H
#define TRACE_H

#include "jni.h"

enum Phase {
  TRACE_EVENT_PHASE_BEGIN,
  TRACE_EVENT_PHASE_END,
  TRACE_EVENT_PHASE_INSTANT,
};

#define TRACE_EVENT_BEGIN0(category_group, name)               \
  {                                                            \
    using base::debug::TraceEvent;                             \
    TraceEvent te(category_group, name,                        \
                  TRACE_EVENT_PHASE_BEGIN);                    \
    te.SendToATrace();                                         \
  }

#define TRACE_EVENT_END0(category_group, name)               \
  {                                                          \
    using base::debug::TraceEvent;                           \
    TraceEvent te(category_group, name,                      \
                  TRACE_EVENT_PHASE_END);                    \
    te.SendToATrace();                                       \
  }

#define TRACE_EVENT_BEGIN1(category_group, name, key, value)   \
  {                                                            \
    using base::debug::TraceEvent;                             \
    TraceEvent te(category_group, name,                        \
                  TRACE_EVENT_PHASE_BEGIN);                    \
    te.SetArgs(key, value);                                    \
    te.SendToATrace();                                         \
  }

namespace base {
namespace debug {
class TraceEvent {
 public:
  TraceEvent(const char* category_group, const char* name, Phase phase);
  static void StartATrace(JNIEnv*);
  static void StopATrace(JNIEnv*);
  static bool isEnable();

  void SendToATrace();
  inline void SetArgs(const char* key, const char* value) {
    key_ = key;
    value_ = value;
  }

 private:
  Phase phase_;
  const char* category_group_;
  const char* name_;
  const char* key_;
  const char* value_;
};

class TraceScope {
 public:
  inline TraceScope(const char* category_group,
                    const char* name,
                    const char* key = nullptr,
                    const char* value = nullptr);
  inline ~TraceScope();

 private:
  const char* category_group_;
  const char* name_;
};

TraceScope::TraceScope(const char* category_group,
                       const char* name,
                       const char* key,
                       const char* value)
    : category_group_(category_group), name_(name) {
  TRACE_EVENT_BEGIN1(category_group, name, key, value);
}
TraceScope::~TraceScope() {
  TRACE_EVENT_END0(category_group_, name_);
}
}
}
#endif /* TRACE_H */

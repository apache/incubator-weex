#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#include <stdlib.h>
#include <stdarg.h>
#include <android/log.h>

#include <string>
#include "Trace.h"
#define TAG "LINZJ"
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, TAG, __VA_ARGS__)
namespace {

int g_atrace_fd = -1;
const char kATraceMarkerFile[] = "/sys/kernel/debug/tracing/trace_marker";

std::string StringPrintf(const char* fmt, ...) {
  va_list ap;
  char buf[256];
  va_start(ap, fmt);
  vsnprintf(buf, 256, fmt, ap);
  va_end(ap);
  return std::string(buf);
}

void WriteEvent(char phase,
                const char* category_group,
                const char* name,
                const char* key = NULL,
                const char* value = NULL) {
  std::string out = StringPrintf("%c|%d|%s", phase, getpid(), name);

  out += '|';
  // args
  if (key) {
    out += StringPrintf("%s=%s", key, value);
  }
  out += '|';
  out += category_group;
  write(g_atrace_fd, out.c_str(), out.size());
}

}  // namespace

namespace base {
namespace debug {

static jclass jTraceClass;
static bool isTraceEnabled(JNIEnv* env) {
  if (!env)
    return true;
  jboolean enabled = false;
  jTraceClass = (env)->FindClass("com/taobao/weex/utils/Trace");
  jmethodID mid = (env)->GetStaticMethodID(jTraceClass, "getTraceEnabled", "()Z");
  enabled = (env)->CallStaticBooleanMethod(jTraceClass, mid);
  return enabled;
}

// These functions support Android systrace.py when 'webview' category is
// traced. With the new adb_profile_chrome, we may have two phases:
// - before WebView is ready for combined tracing, we can use adb_profile_chrome
//   to trace android categories other than 'webview' and chromium categories.
//   In this way we can avoid the conflict between StartATrace/StopATrace and
//   the intents.
// - TODO(wangxianzhu): after WebView is ready for combined tracing, remove
//   StartATrace, StopATrace and SendToATrace, and perhaps send Java traces
//   directly to atrace in trace_event_binding.cc.

void TraceEvent::StartATrace(JNIEnv* env) {
  if (!isTraceEnabled(env))
    return;

  if (g_atrace_fd != -1)
    return;

  g_atrace_fd = open(kATraceMarkerFile, O_WRONLY);
  if (g_atrace_fd == -1) {
    LOGE("Couldn't open %s", kATraceMarkerFile);
    return;
  }
}

void TraceEvent::StopATrace(JNIEnv* env) {
  if (jTraceClass != NULL)
    (env)->DeleteLocalRef(jTraceClass);

  if (g_atrace_fd == -1)
    return;

  close(g_atrace_fd);
  g_atrace_fd = -1;
}

void TraceEvent::SendToATrace() {
  if (g_atrace_fd == -1)
    return;

  const char* category_group = category_group_;

  switch (phase_) {
    case TRACE_EVENT_PHASE_BEGIN:
      WriteEvent('B', category_group, name_, key_, value_);
      break;

    case TRACE_EVENT_PHASE_END:
      // Though a single 'E' is enough, here append pid, name and
      // category_group etc. So that unpaired events can be found easily.
      WriteEvent('E', category_group, name_);
      break;

    case TRACE_EVENT_PHASE_INSTANT:
      // Simulate an instance event with a pair of begin/end events.
      WriteEvent('B', category_group, name_);
      write(g_atrace_fd, "E", 1);
      break;

    default:
      // Do nothing.
      break;
  }
}

bool TraceEvent::isEnable()
{
    return g_atrace_fd != -1;
}

TraceEvent::TraceEvent(const char* category_group,
                       const char* name,
                       Phase phase)
    : category_group_(category_group),
      name_(name),
      phase_(phase),
      key_(NULL),
      value_(NULL) {}
}  // namespace debug
}  // namespace base

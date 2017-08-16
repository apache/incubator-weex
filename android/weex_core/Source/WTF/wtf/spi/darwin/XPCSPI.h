/*
 * Copyright (C) 2014-2016 Apple Inc. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY APPLE INC. ``AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL APPLE INC. OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
 * OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE. 
 */

#pragma once

#include <dispatch/dispatch.h>
#include <os/object.h>

#if PLATFORM(MAC) || USE(APPLE_INTERNAL_SDK)
#include <xpc/xpc.h>
#else

#if OS_OBJECT_USE_OBJC
OS_OBJECT_DECL(xpc_object);
typedef xpc_object_t xpc_connection_t;

static ALWAYS_INLINE void _xpc_object_validate(xpc_object_t object)
{
    void *isa = *(void * volatile *)(OS_OBJECT_BRIDGE void *)object;
    (void)isa;
}

#define XPC_GLOBAL_OBJECT(object) ((OS_OBJECT_BRIDGE xpc_object_t)&(object))

#else // OS_OBJECT_USE_OBJC

typedef void* xpc_object_t;
typedef void* xpc_connection_t;

#define XPC_GLOBAL_OBJECT(object) (&(object))

#endif // OS_OBJECT_USE_OBJC

typedef const struct _xpc_type_s* xpc_type_t;

#if COMPILER_SUPPORTS(BLOCKS)
typedef bool (^xpc_array_applier_t)(size_t index, xpc_object_t);
typedef bool (^xpc_dictionary_applier_t)(const char *key, xpc_object_t value);
typedef void (^xpc_handler_t)(xpc_object_t);
#endif

typedef void (*xpc_connection_handler_t)(xpc_connection_t connection);

#define XPC_ARRAY_APPEND ((size_t)(-1))
#define XPC_ERROR_CONNECTION_INVALID XPC_GLOBAL_OBJECT(_xpc_error_connection_invalid)
#define XPC_ERROR_TERMINATION_IMMINENT XPC_GLOBAL_OBJECT(_xpc_error_termination_imminent)
#define XPC_TYPE_ARRAY (&_xpc_type_array)
#define XPC_TYPE_BOOL (&_xpc_type_bool)
#define XPC_TYPE_DICTIONARY (&_xpc_type_dictionary)
#define XPC_TYPE_ERROR (&_xpc_type_error)
#define XPC_TYPE_STRING (&_xpc_type_string)

#endif // PLATFORM(MAC) || USE(APPLE_INTERNAL_SDK)

#if USE(APPLE_INTERNAL_SDK)
#include <xpc/private.h>
#else
enum {
    DISPATCH_MACH_SEND_POSSIBLE = 0x8,
};
#endif

WTF_EXTERN_C_BEGIN

extern const struct _xpc_dictionary_s _xpc_error_connection_invalid;
extern const struct _xpc_dictionary_s _xpc_error_termination_imminent;

extern const struct _xpc_type_s _xpc_type_array;
extern const struct _xpc_type_s _xpc_type_bool;
extern const struct _xpc_type_s _xpc_type_dictionary;
extern const struct _xpc_type_s _xpc_type_error;
extern const struct _xpc_type_s _xpc_type_string;

xpc_object_t xpc_array_create(const xpc_object_t*, size_t count);
#if COMPILER_SUPPORTS(BLOCKS)
bool xpc_array_apply(xpc_object_t, xpc_array_applier_t);
bool xpc_dictionary_apply(xpc_object_t xdict, xpc_dictionary_applier_t applier);
#endif
size_t xpc_array_get_count(xpc_object_t);
const char* xpc_array_get_string(xpc_object_t, size_t index);
void xpc_array_set_string(xpc_object_t, size_t index, const char* string);
bool xpc_bool_get_value(xpc_object_t);
void xpc_connection_cancel(xpc_connection_t);
xpc_connection_t xpc_connection_create(const char* name, dispatch_queue_t);
xpc_connection_t xpc_connection_create_mach_service(const char* name, dispatch_queue_t, uint64_t flags);
pid_t xpc_connection_get_pid(xpc_connection_t);
void xpc_connection_resume(xpc_connection_t);
void xpc_connection_send_message(xpc_connection_t, xpc_object_t);
void xpc_connection_send_message_with_reply(xpc_connection_t, xpc_object_t, dispatch_queue_t, xpc_handler_t);
void xpc_connection_set_event_handler(xpc_connection_t, xpc_handler_t);
void xpc_connection_set_target_queue(xpc_connection_t, dispatch_queue_t);
xpc_object_t xpc_dictionary_create(const char*  const* keys, const xpc_object_t*, size_t count);
xpc_object_t xpc_dictionary_create_reply(xpc_object_t);
int xpc_dictionary_dup_fd(xpc_object_t, const char* key);
xpc_connection_t xpc_dictionary_get_remote_connection(xpc_object_t);
bool xpc_dictionary_get_bool(xpc_object_t, const char* key);
const char* xpc_dictionary_get_string(xpc_object_t, const char* key);
uint64_t xpc_dictionary_get_uint64(xpc_object_t, const char* key);
xpc_object_t xpc_dictionary_get_value(xpc_object_t, const char* key);
void xpc_dictionary_set_bool(xpc_object_t, const char* key, bool value);
void xpc_dictionary_set_fd(xpc_object_t, const char* key, int fd);
void xpc_dictionary_set_string(xpc_object_t, const char* key, const char* string);
void xpc_dictionary_set_uint64(xpc_object_t, const char* key, uint64_t value);
void xpc_dictionary_set_value(xpc_object_t, const char*key, xpc_object_t value);
xpc_type_t xpc_get_type(xpc_object_t);
void xpc_main(xpc_connection_handler_t);
const char* xpc_string_get_string_ptr(xpc_object_t);
void xpc_transaction_begin(void);
void xpc_transaction_end(void);
void xpc_transaction_exit_clean(void);
void xpc_track_activity(void);

xpc_object_t xpc_connection_copy_entitlement_value(xpc_connection_t, const char* entitlement);
void xpc_connection_get_audit_token(xpc_connection_t, audit_token_t*);
void xpc_connection_kill(xpc_connection_t, int);
void xpc_connection_set_instance(xpc_connection_t, uuid_t);
mach_port_t xpc_dictionary_copy_mach_send(xpc_object_t, const char*);
void xpc_dictionary_set_mach_send(xpc_object_t, const char*, mach_port_t);

void xpc_connection_set_bootstrap(xpc_connection_t, xpc_object_t bootstrap);
xpc_object_t xpc_copy_bootstrap(void);
void xpc_connection_set_oneshot_instance(xpc_connection_t, uuid_t instance);

#if OS_OBJECT_USE_OBJC_RETAIN_RELEASE
#if !defined(xpc_retain)
#define xpc_retain(object) ({ xpc_object_t _o = (object); _xpc_object_validate(_o); [_o retain]; })
#endif
#else
xpc_object_t xpc_retain(xpc_object_t);
#endif

#if OS_OBJECT_USE_OBJC_RETAIN_RELEASE
#if !defined(xpc_retain)
#define xpc_release(object) ({ xpc_object_t _o = (object); _xpc_object_validate(_o); [_o release]; })
#endif
#else
void xpc_release(xpc_object_t);
#endif

WTF_EXTERN_C_END

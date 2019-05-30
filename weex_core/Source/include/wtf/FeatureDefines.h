/*
 * Copyright (C) 2006, 2007, 2008, 2009, 2013 Apple Inc. All rights reserved.
 * Copyright (C) 2007-2009 Torch Mobile, Inc.
 * Copyright (C) 2010, 2011 Research In Motion Limited. All rights reserved.
 * Copyright (C) 2013 Samsung Electronics. All rights reserved.
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

#ifndef WTF_FeatureDefines_h
#define WTF_FeatureDefines_h

/* Use this file to list _all_ ENABLE() macros. Define the macros to be one of the following values:
 *  - "0" disables the feature by default. The feature can still be enabled for a specific port or environment.
 *  - "1" enables the feature by default. The feature can still be disabled for a specific port or environment.
 *
 * The feature defaults in this file are only taken into account if the (port specific) build system
 * has not enabled or disabled a particular feature.
 *
 * Use this file to define ENABLE() macros only. Do not use this file to define USE() or macros !
 *
 * Only define a macro if it was not defined before - always check for !defined first.
 *
 * Keep the file sorted by the name of the defines. As an exception you can change the order
 * to allow interdependencies between the default values.
 *
 * Below are a few potential commands to take advantage of this file running from the Source/WTF directory
 *
 * Get the list of feature defines: grep -o "ENABLE_\(\w\+\)" wtf/FeatureDefines.h | sort | uniq
 * Get the list of features enabled by default for a PLATFORM(XXX): gcc -E -dM -I. -DWTF_PLATFORM_XXX "wtf/Platform.h" | grep "ENABLE_\w\+ 1" | cut -d' ' -f2 | sort
 */

/* FIXME: Move out the PLATFORM specific rules into platform specific files. */

/* --------- Apple IOS (but not MAC) port --------- */
#if PLATFORM(IOS)

#if !defined(ENABLE_ASYNC_SCROLLING)
#define ENABLE_ASYNC_SCROLLING 1
#endif

#if !defined(ENABLE_CONTENT_EXTENSIONS)
#define ENABLE_CONTENT_EXTENSIONS 1
#endif

#if !defined(ENABLE_CONTEXT_MENUS)
#define ENABLE_CONTEXT_MENUS 0
#endif

#if !defined(ENABLE_CURSOR_SUPPORT)
#define ENABLE_CURSOR_SUPPORT 0
#endif

#if !defined(ENABLE_DRAG_SUPPORT)
#define ENABLE_DRAG_SUPPORT 0
#endif

#if !defined(ENABLE_GEOLOCATION)
#define ENABLE_GEOLOCATION 1
#endif

#if !defined(ENABLE_ICONDATABASE)
#define ENABLE_ICONDATABASE 0
#endif

#if !defined(ENABLE_INSPECTOR_ALTERNATE_DISPATCHERS)
#define ENABLE_INSPECTOR_ALTERNATE_DISPATCHERS 1
#endif

#if !defined(ENABLE_LETTERPRESS)
#define ENABLE_LETTERPRESS 1
#endif

#if !defined(ENABLE_IOS_AUTOCORRECT_AND_AUTOCAPITALIZE)
#define ENABLE_IOS_AUTOCORRECT_AND_AUTOCAPITALIZE 1
#endif

#if !defined(ENABLE_IOS_GESTURE_EVENTS) && USE(APPLE_INTERNAL_SDK)
#define ENABLE_IOS_GESTURE_EVENTS 1
#endif

#if !defined(ENABLE_TEXT_AUTOSIZING)
#define ENABLE_TEXT_AUTOSIZING 1
#endif

#if !defined(ENABLE_IOS_TOUCH_EVENTS) && USE(APPLE_INTERNAL_SDK)
#define ENABLE_IOS_TOUCH_EVENTS 1
#endif

#if !defined(ENABLE_METER_ELEMENT)
#define ENABLE_METER_ELEMENT 0
#endif

#if !defined(ENABLE_NETSCAPE_PLUGIN_API)
#define ENABLE_NETSCAPE_PLUGIN_API 0
#endif

#if !defined(ENABLE_ORIENTATION_EVENTS)
#define ENABLE_ORIENTATION_EVENTS 1
#endif

#if !defined(ENABLE_POINTER_LOCK)
#define ENABLE_POINTER_LOCK 0
#endif

#if !defined(ENABLE_REMOTE_INSPECTOR)
#define ENABLE_REMOTE_INSPECTOR 1
#endif

#if !defined(ENABLE_RESPECT_EXIF_ORIENTATION)
#define ENABLE_RESPECT_EXIF_ORIENTATION 1
#endif

#if !defined(ENABLE_TEXT_CARET)
#define ENABLE_TEXT_CARET 0
#endif

#if !defined(ENABLE_TEXT_SELECTION)
#define ENABLE_TEXT_SELECTION 0
#endif

/* FIXME: Remove the USE(APPLE_INTERNAL_SDK) conjunct once we support touch events when building against
the public iOS SDK. We will also need to update the FeatureDefines.xcconfig files. */
#if !defined(ENABLE_TOUCH_EVENTS) && USE(APPLE_INTERNAL_SDK)
#define ENABLE_TOUCH_EVENTS 1
#endif

#if !defined(ENABLE_WEB_ARCHIVE)
#define ENABLE_WEB_ARCHIVE 1
#endif

#if !defined(ENABLE_VIEW_MODE_CSS_MEDIA)
#define ENABLE_VIEW_MODE_CSS_MEDIA 0
#endif

#if !defined(ENABLE_WEBASSEMBLY)
#define ENABLE_WEBASSEMBLY (defined(ENABLE_B3_JIT) && ENABLE_B3_JIT)
#endif

#if !defined(ENABLE_WEBGL)
#define ENABLE_WEBGL 1
#endif

#if !defined(ENABLE_PRIMARY_SNAPSHOTTED_PLUGIN_HEURISTIC)
#define ENABLE_PRIMARY_SNAPSHOTTED_PLUGIN_HEURISTIC 1
#endif

#if !defined(ENABLE_DOWNLOAD_ATTRIBUTE)
#define ENABLE_DOWNLOAD_ATTRIBUTE 0
#endif

#endif /* PLATFORM(IOS) */

/* --------- Apple MAC port (not IOS) --------- */
#if PLATFORM(MAC)

#if !defined(ENABLE_CONTENT_EXTENSIONS)
#define ENABLE_CONTENT_EXTENSIONS 1
#endif

#if !defined(ENABLE_DASHBOARD_SUPPORT)
#define ENABLE_DASHBOARD_SUPPORT 1
#endif

#if !defined(ENABLE_LEGACY_ENCRYPTED_MEDIA)
#define ENABLE_LEGACY_ENCRYPTED_MEDIA 1
#endif

#if !defined(ENABLE_FULLSCREEN_API)
#define ENABLE_FULLSCREEN_API 1
#endif

#if !defined(ENABLE_REMOTE_INSPECTOR)
#define ENABLE_REMOTE_INSPECTOR 1
#endif

#if !defined(ENABLE_INSPECTOR_ALTERNATE_DISPATCHERS)
#define ENABLE_INSPECTOR_ALTERNATE_DISPATCHERS 1
#endif

#if !defined(ENABLE_SMOOTH_SCROLLING)
#define ENABLE_SMOOTH_SCROLLING 1
#endif

#if !defined(ENABLE_ASYNC_SCROLLING)
#define ENABLE_ASYNC_SCROLLING 1
#endif

#if ENABLE(VIDEO)
#if !defined(ENABLE_VIDEO_TRACK)
#define ENABLE_VIDEO_TRACK 1
#endif
#endif

#if !defined(ENABLE_VIEW_MODE_CSS_MEDIA)
#define ENABLE_VIEW_MODE_CSS_MEDIA 0
#endif

#if !defined(ENABLE_WEBASSEMBLY)
#define ENABLE_WEBASSEMBLY (defined(ENABLE_B3_JIT) && ENABLE_B3_JIT)
#endif

#if !defined(ENABLE_WEB_ARCHIVE)
#define ENABLE_WEB_ARCHIVE 1
#endif

#if !defined(ENABLE_WEB_AUDIO)
#define ENABLE_WEB_AUDIO 1
#endif

#if !defined(ENABLE_CURSOR_VISIBILITY)
#define ENABLE_CURSOR_VISIBILITY 1
#endif

#if !defined(ENABLE_INPUT_TYPE_COLOR)
#define ENABLE_INPUT_TYPE_COLOR 1
#endif

#if !defined(ENABLE_INPUT_TYPE_COLOR_POPOVER)
#define ENABLE_INPUT_TYPE_COLOR_POPOVER 1
#endif

#if !defined(ENABLE_PRIMARY_SNAPSHOTTED_PLUGIN_HEURISTIC)
#define ENABLE_PRIMARY_SNAPSHOTTED_PLUGIN_HEURISTIC 1
#endif

#if !defined(ENABLE_MAC_GESTURE_EVENTS) && USE(APPLE_INTERNAL_SDK)
#define ENABLE_MAC_GESTURE_EVENTS 1
#endif

#endif /* PLATFORM(MAC) */

#if PLATFORM(COCOA)

#if !defined(ENABLE_DATA_DETECTION)
#define ENABLE_DATA_DETECTION 1
#endif

#if !defined(ENABLE_FILE_REPLACEMENT)
#define ENABLE_FILE_REPLACEMENT 1
#endif

#if !defined(ENABLE_KEYBOARD_KEY_ATTRIBUTE)
#define ENABLE_KEYBOARD_KEY_ATTRIBUTE 1
#endif

#if !defined(ENABLE_KEYBOARD_CODE_ATTRIBUTE)
#define ENABLE_KEYBOARD_CODE_ATTRIBUTE 1
#endif

#endif /* PLATFORM(COCOA) */

#if !PLATFORM(COCOA)

#if !defined(JSC_OBJC_API_ENABLED)
#define JSC_OBJC_API_ENABLED 0
#endif

#endif /* !PLATFORM(COCOA) */


/* --------- Apple Windows port --------- */
#if PLATFORM(WIN) && !PLATFORM(WIN_CAIRO)

#if !defined(ENABLE_FULLSCREEN_API)
#define ENABLE_FULLSCREEN_API 1
#endif

#if !defined(ENABLE_WEB_ARCHIVE)
#define ENABLE_WEB_ARCHIVE 1
#endif

#if !defined(ENABLE_WEBGL)
#define ENABLE_WEBGL 0
#endif

#if !defined(ENABLE_GEOLOCATION)
#define ENABLE_GEOLOCATION 1
#endif

#endif /* PLATFORM(WIN) && !PLATFORM(WIN_CAIRO) */

/* --------- Windows CAIRO port --------- */
/* PLATFORM(WIN_CAIRO) is a specialization of PLATFORM(WIN). */
/* PLATFORM(WIN) is always enabled when PLATFORM(WIN_CAIRO) is enabled. */
#if PLATFORM(WIN_CAIRO)

#if !defined(ENABLE_WEB_ARCHIVE)
#define ENABLE_WEB_ARCHIVE 1
#endif

#if !defined(ENABLE_VIEW_MODE_CSS_MEDIA)
#define ENABLE_VIEW_MODE_CSS_MEDIA 0
#endif

#if !defined(ENABLE_WEBGL)
#define ENABLE_WEBGL 1
#endif

#if !defined(ENABLE_GEOLOCATION)
#define ENABLE_GEOLOCATION 1
#endif

#endif /* PLATFORM(WIN_CAIRO) */

/* --------- Gtk port (Unix, Windows, Mac) --------- */
#if PLATFORM(GTK)
#if !defined(ENABLE_KEYBOARD_KEY_ATTRIBUTE)
#define ENABLE_KEYBOARD_KEY_ATTRIBUTE 1
#endif

#if !defined(ENABLE_KEYBOARD_CODE_ATTRIBUTE)
#define ENABLE_KEYBOARD_CODE_ATTRIBUTE 1
#endif
#endif /* PLATFORM(GTK) */

/* ENABLE macro defaults for WebCore */
/* Do not use PLATFORM() tests in this section ! */

#if !defined(ENABLE_3D_TRANSFORMS)
#define ENABLE_3D_TRANSFORMS 0
#endif

#if !defined(ENABLE_ACCELERATED_2D_CANVAS)
#define ENABLE_ACCELERATED_2D_CANVAS 0
#endif

#if !defined(ENABLE_ACCELERATED_OVERFLOW_SCROLLING)
#define ENABLE_ACCELERATED_OVERFLOW_SCROLLING 0
#endif

#if !defined(ENABLE_APNG)
#define ENABLE_APNG 1
#endif

#if !defined(ENABLE_CANVAS_PATH)
#define ENABLE_CANVAS_PATH 1
#endif

#if !defined(ENABLE_CANVAS_PROXY)
#define ENABLE_CANVAS_PROXY 0
#endif

#if !defined(ENABLE_CHANNEL_MESSAGING)
#define ENABLE_CHANNEL_MESSAGING 1
#endif

#if !defined(ENABLE_CONTENT_EXTENSIONS)
#define ENABLE_CONTENT_EXTENSIONS 0
#endif

#if !defined(ENABLE_CONTEXT_MENUS)
#define ENABLE_CONTEXT_MENUS 1
#endif

#if !defined(ENABLE_CSS3_TEXT)
#define ENABLE_CSS3_TEXT 0
#endif

#if !defined(ENABLE_CSS_BOX_DECORATION_BREAK)
#define ENABLE_CSS_BOX_DECORATION_BREAK 1
#endif

#if !defined(ENABLE_CSS_DEVICE_ADAPTATION)
#define ENABLE_CSS_DEVICE_ADAPTATION 0
#endif

#if !defined(ENABLE_CSS_COMPOSITING)
#define ENABLE_CSS_COMPOSITING 0
#endif

#if !defined(ENABLE_CSS_IMAGE_ORIENTATION)
#define ENABLE_CSS_IMAGE_ORIENTATION 0
#endif

#if !defined(ENABLE_CSS_IMAGE_RESOLUTION)
#define ENABLE_CSS_IMAGE_RESOLUTION 0
#endif

#if !defined(ENABLE_CURSOR_SUPPORT)
#define ENABLE_CURSOR_SUPPORT 1
#endif

#if !defined(ENABLE_CUSTOM_SCHEME_HANDLER)
#define ENABLE_CUSTOM_SCHEME_HANDLER 0
#endif

#if !defined(ENABLE_DASHBOARD_SUPPORT)
#define ENABLE_DASHBOARD_SUPPORT 0
#endif

#if !defined(ENABLE_DATALIST_ELEMENT)
#define ENABLE_DATALIST_ELEMENT 0
#endif

#if !defined(ENABLE_DATA_TRANSFER_ITEMS)
#define ENABLE_DATA_TRANSFER_ITEMS 0
#endif

#if !defined(ENABLE_DEVICE_ORIENTATION)
#define ENABLE_DEVICE_ORIENTATION 0
#endif

#if !defined(ENABLE_DOWNLOAD_ATTRIBUTE)
#define ENABLE_DOWNLOAD_ATTRIBUTE 1
#endif

#if !defined(ENABLE_DRAG_SUPPORT)
#define ENABLE_DRAG_SUPPORT 1
#endif

#if !defined(ENABLE_ENCRYPTED_MEDIA)
#define ENABLE_ENCRYPTED_MEDIA 0
#endif

#if !defined(ENABLE_FETCH_API)
#define ENABLE_FETCH_API 1
#endif

#if !defined(ENABLE_FILTERS_LEVEL_2)
#define ENABLE_FILTERS_LEVEL_2 0
#endif

#if !defined(ENABLE_FONT_LOAD_EVENTS)
#define ENABLE_FONT_LOAD_EVENTS 0
#endif

#if !defined(ENABLE_FTPDIR)
#define ENABLE_FTPDIR 1
#endif

#if !defined(ENABLE_FULLSCREEN_API)
#define ENABLE_FULLSCREEN_API 0
#endif

#if !defined(ENABLE_GAMEPAD)
#define ENABLE_GAMEPAD 0
#endif

#if !defined(ENABLE_GAMEPAD_DEPRECATED)
#define ENABLE_GAMEPAD_DEPRECATED 0
#endif

#if !defined(ENABLE_GEOLOCATION)
#define ENABLE_GEOLOCATION 0
#endif

#if !defined(ENABLE_ICONDATABASE)
#define ENABLE_ICONDATABASE 1
#endif

#if !defined(ENABLE_IMAGE_DECODER_DOWN_SAMPLING)
#define ENABLE_IMAGE_DECODER_DOWN_SAMPLING 0
#endif

#if !defined(ENABLE_INDEXED_DATABASE)
#define ENABLE_INDEXED_DATABASE 0
#endif

#if !defined(ENABLE_INDEXED_DATABASE_IN_WORKERS)
#define ENABLE_INDEXED_DATABASE_IN_WORKERS 0
#endif

#if !defined(ENABLE_INPUT_TYPE_COLOR)
#define ENABLE_INPUT_TYPE_COLOR 0
#endif

#if !defined(ENABLE_INPUT_TYPE_COLOR_POPOVER)
#define ENABLE_INPUT_TYPE_COLOR_POPOVER 0
#endif

#if !defined(ENABLE_INPUT_TYPE_DATE)
#define ENABLE_INPUT_TYPE_DATE 0
#endif

#if !defined(ENABLE_INPUT_TYPE_DATETIME_INCOMPLETE)
#define ENABLE_INPUT_TYPE_DATETIME_INCOMPLETE 0
#endif

#if !defined(ENABLE_INPUT_TYPE_DATETIMELOCAL)
#define ENABLE_INPUT_TYPE_DATETIMELOCAL 0
#endif

#if !defined(ENABLE_INPUT_TYPE_MONTH)
#define ENABLE_INPUT_TYPE_MONTH 0
#endif

#if !defined(ENABLE_INPUT_TYPE_TIME)
#define ENABLE_INPUT_TYPE_TIME 0
#endif

#if !defined(ENABLE_INPUT_TYPE_WEEK)
#define ENABLE_INPUT_TYPE_WEEK 0
#endif

#if ENABLE(INPUT_TYPE_DATE) || ENABLE(INPUT_TYPE_DATETIME_INCOMPLETE) || ENABLE(INPUT_TYPE_DATETIMELOCAL) || ENABLE(INPUT_TYPE_MONTH) || ENABLE(INPUT_TYPE_TIME) || ENABLE(INPUT_TYPE_WEEK)
#if !defined(ENABLE_DATE_AND_TIME_INPUT_TYPES)
#define ENABLE_DATE_AND_TIME_INPUT_TYPES 1
#endif
#endif

#if !defined(ENABLE_INSPECTOR_ALTERNATE_DISPATCHERS)
#define ENABLE_INSPECTOR_ALTERNATE_DISPATCHERS 0
#endif

#if !defined(ENABLE_INTL)
#define ENABLE_INTL 0
#endif

#if !defined(ENABLE_JAVASCRIPT_I18N_API)
#define ENABLE_JAVASCRIPT_I18N_API 0
#endif

#if !defined(ENABLE_LEGACY_CSS_VENDOR_PREFIXES)
#define ENABLE_LEGACY_CSS_VENDOR_PREFIXES 0
#endif

#if !defined(ENABLE_LEGACY_ENCRYPTED_MEDIA)
#define ENABLE_LEGACY_ENCRYPTED_MEDIA 0
#endif

#if !defined(ENABLE_LEGACY_NOTIFICATIONS)
#define ENABLE_LEGACY_NOTIFICATIONS 0
#endif

#if !defined(ENABLE_LEGACY_VENDOR_PREFIXES)
#define ENABLE_LEGACY_VENDOR_PREFIXES 0
#endif

#if !defined(ENABLE_LETTERPRESS)
#define ENABLE_LETTERPRESS 0
#endif

#if !defined(ENABLE_LINK_PREFETCH)
#define ENABLE_LINK_PREFETCH 0
#endif

#if !defined(ENABLE_MATHML)
#define ENABLE_MATHML 1
#endif

#if !defined(ENABLE_MEDIA_CAPTURE)
#define ENABLE_MEDIA_CAPTURE 0
#endif

#if !defined(ENABLE_MEDIA_CONTROLS_SCRIPT)
#define ENABLE_MEDIA_CONTROLS_SCRIPT 0
#endif

#if !defined(ENABLE_MEDIA_SOURCE)
#define ENABLE_MEDIA_SOURCE 0
#endif

#if !defined(ENABLE_MEDIA_STATISTICS)
#define ENABLE_MEDIA_STATISTICS 0
#endif

#if !defined(ENABLE_MEDIA_STREAM)
#define ENABLE_MEDIA_STREAM 0
#endif

#if !defined(ENABLE_METER_ELEMENT)
#define ENABLE_METER_ELEMENT 1
#endif

#if !defined(ENABLE_MHTML)
#define ENABLE_MHTML 0
#endif

#if !defined(ENABLE_MOUSE_CURSOR_SCALE)
#define ENABLE_MOUSE_CURSOR_SCALE 0
#endif

#if !defined(ENABLE_MOUSE_FORCE_EVENTS)
#define ENABLE_MOUSE_FORCE_EVENTS 1
#endif

#if !defined(ENABLE_NAVIGATOR_CONTENT_UTILS)
#define ENABLE_NAVIGATOR_CONTENT_UTILS 0
#endif

#if !defined(ENABLE_NAVIGATOR_HWCONCURRENCY)
#define ENABLE_NAVIGATOR_HWCONCURRENCY 1
#endif

#if !defined(ENABLE_NETSCAPE_PLUGIN_API)
#define ENABLE_NETSCAPE_PLUGIN_API 1
#endif

#if !defined(ENABLE_NETSCAPE_PLUGIN_METADATA_CACHE)
#define ENABLE_NETSCAPE_PLUGIN_METADATA_CACHE 0
#endif

#if !defined(ENABLE_NOTIFICATIONS)
#define ENABLE_NOTIFICATIONS 0
#endif

#if !defined(ENABLE_OPENTYPE_VERTICAL)
#define ENABLE_OPENTYPE_VERTICAL 0
#endif

#if !defined(ENABLE_ORIENTATION_EVENTS)
#define ENABLE_ORIENTATION_EVENTS 0
#endif

#if OS(WINDOWS)
#if !defined(ENABLE_PAN_SCROLLING)
#define ENABLE_PAN_SCROLLING 1
#endif
#endif

#if !defined(ENABLE_POINTER_LOCK)
#define ENABLE_POINTER_LOCK 1
#endif

#if !defined(ENABLE_PROXIMITY_EVENTS)
#define ENABLE_PROXIMITY_EVENTS 0
#endif

#if !defined(ENABLE_QUOTA)
#define ENABLE_QUOTA 0
#endif

#if !defined(ENABLE_REMOTE_INSPECTOR)
#define ENABLE_REMOTE_INSPECTOR 0
#endif

#if !defined(ENABLE_REQUEST_AUTOCOMPLETE)
#define ENABLE_REQUEST_AUTOCOMPLETE 0
#endif

#if !defined(ENABLE_RUBBER_BANDING)
#define ENABLE_RUBBER_BANDING 0
#endif

#if !defined(ENABLE_SMOOTH_SCROLLING)
#define ENABLE_SMOOTH_SCROLLING 0
#endif

#if !defined(ENABLE_SPEECH_SYNTHESIS)
#define ENABLE_SPEECH_SYNTHESIS 0
#endif

#if !defined(ENABLE_SPELLCHECK)
#define ENABLE_SPELLCHECK 0
#endif

#if !defined(ENABLE_READABLE_STREAM_API)
#if PLATFORM(WIN)
#define ENABLE_READABLE_STREAM_API 0
#else
#define ENABLE_READABLE_STREAM_API 1
#endif
#if !defined(ENABLE_READABLE_BYTE_STREAM_API)
#if PLATFORM(WIN)
#define ENABLE_READABLE_BYTE_STREAM_API 0
#else
#define ENABLE_READABLE_BYTE_STREAM_API 1
#endif
#endif
#endif

#if !defined(ENABLE_WRITABLE_STREAM_API)
#if PLATFORM(WIN)
#define ENABLE_WRITABLE_STREAM_API 0
#else
#define ENABLE_WRITABLE_STREAM_API 1
#endif
#endif

#if !defined(ENABLE_SVG_FONTS)
#define ENABLE_SVG_FONTS 1
#endif

#if !defined(ENABLE_TEXT_CARET)
#define ENABLE_TEXT_CARET 1
#endif

#if !defined(ENABLE_TEXT_SELECTION)
#define ENABLE_TEXT_SELECTION 1
#endif

#if !defined(ENABLE_ASYNC_SCROLLING)
#define ENABLE_ASYNC_SCROLLING 0
#endif

#if !defined(ENABLE_TOUCH_EVENTS)
#define ENABLE_TOUCH_EVENTS 0
#endif

#if !defined(ENABLE_TOUCH_ICON_LOADING)
#define ENABLE_TOUCH_ICON_LOADING 0
#endif

#if !defined(ENABLE_VIBRATION)
#define ENABLE_VIBRATION 0
#endif

#if !defined(ENABLE_VIDEO)
#define ENABLE_VIDEO 0
#endif

#if !defined(ENABLE_VIDEO_TRACK)
#define ENABLE_VIDEO_TRACK 0
#endif

#if !defined(ENABLE_DATACUE_VALUE)
#define ENABLE_DATACUE_VALUE 0
#endif

#if !defined(ENABLE_VIEW_MODE_CSS_MEDIA)
#define ENABLE_VIEW_MODE_CSS_MEDIA 1
#endif

#if !defined(ENABLE_WEBGL)
#define ENABLE_WEBGL 0
#endif

#if !defined(ENABLE_GRAPHICS_CONTEXT_3D)
#define ENABLE_GRAPHICS_CONTEXT_3D ENABLE_WEBGL
#endif

#if !defined(ENABLE_WEB_ANIMATIONS)
#define ENABLE_WEB_ANIMATIONS 1
#endif

#if !defined(ENABLE_WEB_ARCHIVE)
#define ENABLE_WEB_ARCHIVE 0
#endif

#if !defined(ENABLE_WEB_AUDIO)
#define ENABLE_WEB_AUDIO 0
#endif

#if !defined(ENABLE_WEB_REPLAY)
#define ENABLE_WEB_REPLAY 0
#endif

#if !defined(ENABLE_WEB_SOCKETS)
#define ENABLE_WEB_SOCKETS 1
#endif

#if !defined(ENABLE_WEB_TIMING)
#define ENABLE_WEB_TIMING 0
#endif

#if !defined(ENABLE_WILL_REVEAL_EDGE_EVENTS)
#define ENABLE_WILL_REVEAL_EDGE_EVENTS 1
#endif

#if !defined(ENABLE_XSLT)
#define ENABLE_XSLT 1
#endif

#if !defined(ENABLE_KEYBOARD_KEY_ATTRIBUTE)
#define ENABLE_KEYBOARD_KEY_ATTRIBUTE 0
#endif

#if !defined(ENABLE_KEYBOARD_CODE_ATTRIBUTE)
#define ENABLE_KEYBOARD_CODE_ATTRIBUTE 0
#endif

#if !defined(ENABLE_DATA_INTERACTION)
#define ENABLE_DATA_INTERACTION 0
#endif

/* Asserts, invariants for macro definitions */

#if ENABLE(VIDEO_TRACK) && !ENABLE(VIDEO)
#error "ENABLE(VIDEO_TRACK) requires ENABLE(VIDEO)"
#endif

#if ENABLE(MEDIA_CONTROLS_SCRIPT) && !ENABLE(VIDEO)
#error "ENABLE(MEDIA_CONTROLS_SCRIPT) requires ENABLE(VIDEO)"
#endif

#if ENABLE(INSPECTOR_ALTERNATE_DISPATCHERS) && !ENABLE(REMOTE_INSPECTOR)
#error "ENABLE(INSPECTOR_ALTERNATE_DISPATCHERS) requires ENABLE(REMOTE_INSPECTOR)"
#endif

#if ENABLE(IOS_TOUCH_EVENTS) && !ENABLE(TOUCH_EVENTS)
#error "ENABLE(IOS_TOUCH_EVENTS) requires ENABLE(TOUCH_EVENTS)"
#endif

#if ENABLE(WEBGL) && !ENABLE(GRAPHICS_CONTEXT_3D)
#error "ENABLE(WEBGL) requires ENABLE(GRAPHICS_CONTEXT_3D)"
#endif

#endif /* WTF_FeatureDefines_h */

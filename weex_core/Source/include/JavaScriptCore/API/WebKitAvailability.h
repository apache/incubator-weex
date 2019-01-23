/**
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 */
#ifndef __WebKitAvailability__
#define __WebKitAvailability__

#if defined(__APPLE__)

#include <AvailabilityMacros.h>
#include <CoreFoundation/CoreFoundation.h>

#if !TARGET_OS_IPHONE && __MAC_OS_X_VERSION_MIN_REQUIRED < 101100
/* To support availability macros that mention newer OS X versions when building on older OS X versions,
   we provide our own definitions of the underlying macros that the availability macros expand to. We're
   free to expand the macros as no-ops since frameworks built on older OS X versions only ship bundled with
   an application rather than as part of the system.
*/

#ifndef __NSi_10_10 // Building from trunk rather than SDK.
#define __NSi_10_10 introduced=10.0 // Use 10.0 to indicate that everything is available.
#endif

#ifndef __NSi_10_11 // Building from trunk rather than SDK.
#define __NSi_10_11 introduced=10.0 // Use 10.0 to indicate that everything is available.
#endif

#ifndef __NSi_10_12 // Building from trunk rather than SDK.
#define __NSi_10_12 introduced=10.0 // Use 10.0 to indicate that everything is available.
#endif

#ifndef __AVAILABILITY_INTERNAL__MAC_10_9
#define __AVAILABILITY_INTERNAL__MAC_10_9
#endif

#ifndef __AVAILABILITY_INTERNAL__MAC_10_10
#define __AVAILABILITY_INTERNAL__MAC_10_10
#endif

#ifndef AVAILABLE_MAC_OS_X_VERSION_10_9_AND_LATER
#define AVAILABLE_MAC_OS_X_VERSION_10_9_AND_LATER
#endif

#ifndef AVAILABLE_MAC_OS_X_VERSION_10_10_AND_LATER
#define AVAILABLE_MAC_OS_X_VERSION_10_10_AND_LATER
#endif

#endif /* __MAC_OS_X_VERSION_MIN_REQUIRED <= 101100 */

#if defined(BUILDING_GTK__)
#undef CF_AVAILABLE
#define CF_AVAILABLE(_mac, _ios)
#undef CF_ENUM_AVAILABLE
#define CF_ENUM_AVAILABLE(_mac, _ios)
#endif

#else
#define CF_AVAILABLE(_mac, _ios)
#define CF_ENUM_AVAILABLE(_mac, _ios)
#endif

#endif /* __WebKitAvailability__ */

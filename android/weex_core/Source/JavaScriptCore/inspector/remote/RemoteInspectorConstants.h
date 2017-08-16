/*
 * Copyright (C) 2011, 2016 Apple Inc. All Rights Reserved.
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

// WIRConstants are "Web Inspector Relay" constants shared between
// the WebInspector framework on the OS X side, webinspectord, and
// iOS WebKit on the device side.

#define WIRSimulatorTCPPortNumber               27753
#define WIRXPCMachPortName                      "com.apple.webinspector"
#define WIRXPCDebuggerServiceName               "com.apple.webinspector.debugger"
#define WIRServiceAvailableNotification         "com.apple.webinspectord.available"
#define WIRServiceAvailabilityCheckNotification "com.apple.webinspectord.availability_check"
#define WIRServiceEnabledNotification           "com.apple.webinspectord.enabled"
#define WIRServiceDisabledNotification          "com.apple.webinspectord.disabled"
#define WIRAutomaticInspectionEnabledState      "com.apple.webinspectord.automatic_inspection_enabled"


#define WIRApplicationIdentifierKey             @"WIRApplicationIdentifierKey"
#define WIRApplicationBundleIdentifierKey       @"WIRApplicationBundleIdentifierKey"
#define WIRApplicationNameKey                   @"WIRApplicationNameKey"
#define WIRIsApplicationProxyKey                @"WIRIsApplicationProxyKey"
#define WIRIsApplicationActiveKey               @"WIRIsApplicationActiveKey"
#define WIRHostApplicationIdentifierKey         @"WIRHostApplicationIdentifierKey"
#define WIRHostApplicationNameKey               @"WIRHostApplicationNameKey"
#define WIRConnectionIdentifierKey              @"WIRConnectionIdentifierKey"
// COMPATABILITY(iOS 9): The key string is intentionally mismatched to support old relays.
#define WIRTargetIdentifierKey                  @"WIRPageIdentifierKey"
#define WIRHasLocalDebuggerKey                  @"WIRHasLocalDebuggerKey"
#define WIRTitleKey                             @"WIRTitleKey"
#define WIRURLKey                               @"WIRURLKey"
#define WIRUserInfoKey                          @"WIRUserInfoKey"
#define WIRApplicationDictionaryKey             @"WIRApplicationDictionaryKey"
#define WIRMessageDataKey                       @"WIRMessageDataKey"
#define WIRApplicationGetListingMessage         @"WIRApplicationGetListingMessage"
#define WIRIndicateMessage                      @"WIRIndicateMessage"
#define WIRIndicateEnabledKey                   @"WIRIndicateEnabledKey"
#define WIRSenderKey                            @"WIRSenderKey"
#define WIRSocketDataKey                        @"WIRSocketDataKey"
#define WIRSocketDataMessage                    @"WIRSocketDataMessage"
#define WIRSocketSetupMessage                   @"WIRSocketSetupMessage"
#define WIRWebPageCloseMessage                  @"WIRWebPageCloseMessage"
#define WIRRawDataMessage                       @"WIRRawDataMessage"
#define WIRRawDataKey                           @"WIRRawDataKey"
#define WIRListingMessage                       @"WIRListingMessage"
#define WIRListingKey                           @"WIRListingKey"
#define WIRRemoteAutomationEnabledKey           @"WIRRemoteAutomationEnabledKey"
#define WIRDestinationKey                       @"WIRDestinationKey"
#define WIRConnectionDiedMessage                @"WIRConnectionDiedMessage"
#define WIRTypeKey                              @"WIRTypeKey"
#define WIRTypeJavaScript                       @"WIRTypeJavaScript"
#define WIRTypeWeb                              @"WIRTypeWeb"
#define WIRTypeAutomation                       @"WIRTypeAutomation"
#define WIRAutomaticallyPause                   @"WIRAutomaticallyPause"

#define WIRAutomaticInspectionEnabledKey           @"WIRAutomaticInspectionEnabledKey"
#define WIRAutomaticInspectionSessionIdentifierKey @"WIRAutomaticInspectionSessionIdentifierKey"
#define WIRAutomaticInspectionConfigurationMessage @"WIRAutomaticInspectionConfigurationMessage"
#define WIRAutomaticInspectionRejectMessage        @"WIRAutomaticInspectionRejectMessage"
#define WIRAutomaticInspectionCandidateMessage     @"WIRAutomaticInspectionCandidateMessage"

#define WIRAutomationTargetIsPairedKey             @"WIRAutomationTargetIsPairedKey"
#define WIRSessionIdentifierKey                    @"WIRSessionIdentifierKey"
#define WIRAutomationSessionRequestMessage         @"WIRAutomationSessionRequestMessage"

// These definitions are shared with a Simulator webinspectord and
// OS X process communicating with it.

#define WIRSimulatorBuildKey                    @"WIRSimulatorBuildKey"
#define WIRSimulatorProductVersionKey           @"WIRSimulatorProductVersionKey"
#define WIRSimulatorNameKey                     @"WIRSimulatorNameKey"

// These definitions are shared between webinspectord and WebKit.

#define WIRPermissionDenied                     @"WIRPermissionDenied"
#define WIRProxyApplicationParentPIDKey         @"WIRProxyApplicationParentPID"
#define WIRProxyApplicationParentAuditDataKey   @"WIRProxyApplicationParentAuditData"
#define WIRProxyApplicationSetupMessage         @"WIRProxyApplicationSetupMessage"
#define WIRProxyApplicationSetupResponseMessage @"WIRProxyApplicationSetupResponseMessage"

/*
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

#import "TestSupportUtils.h"


bool WXIsDoubleApproximate(double x, double y) {
    return fabs(x - y) < 0.001;
}

bool WXTransform3DApproximateToTransform(CATransform3D a,CATransform3D b)
{
    return
    WXIsDoubleApproximate(a.m11, b.m11) &&
    WXIsDoubleApproximate(a.m12, b.m12) &&
    WXIsDoubleApproximate(a.m13, b.m13) &&
    WXIsDoubleApproximate(a.m14, b.m14) &&
    WXIsDoubleApproximate(a.m21, b.m21) &&
    WXIsDoubleApproximate(a.m22, b.m22) &&
    WXIsDoubleApproximate(a.m23, b.m23) &&
    WXIsDoubleApproximate(a.m24, b.m24) &&
    WXIsDoubleApproximate(a.m31, b.m31) &&
    WXIsDoubleApproximate(a.m32, b.m32) &&
    WXIsDoubleApproximate(a.m33, b.m33) &&
    WXIsDoubleApproximate(a.m34, b.m34) &&
    WXIsDoubleApproximate(a.m41, b.m41) &&
    WXIsDoubleApproximate(a.m42, b.m42) &&
    WXIsDoubleApproximate(a.m43, b.m43) &&
    WXIsDoubleApproximate(a.m44, b.m44);
}

bool WXRectApproximateToRect(CGRect a,CGRect b)
{
    return
    WXIsDoubleApproximate(a.origin.x, b.origin.x) &&
    WXIsDoubleApproximate(a.origin.y, b.origin.y) &&
    WXIsDoubleApproximate(a.size.width, b.size.width) &&
    WXIsDoubleApproximate(a.size.height, b.size.height);
}


@implementation TestSupportUtils

+(void)waitSecs:(NSTimeInterval)secs{
    NSDate *timeoutDate = [NSDate dateWithTimeIntervalSinceNow:secs];
    while ( [timeoutDate timeIntervalSinceNow] > 0) {
        CFRunLoopRunInMode(kCFRunLoopDefaultMode, 0.01, YES);
    }
}

@end

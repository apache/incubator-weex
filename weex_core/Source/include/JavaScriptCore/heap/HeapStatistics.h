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
#pragma once

#include "JSExportMacros.h"
#include <wtf/Vector.h>

namespace JSC {

class Heap;

class HeapStatistics {
public:
    NO_RETURN static void exitWithFailure();
    JS_EXPORT_PRIVATE static void reportSuccess();

    static void initialize();
    static void recordGCPauseTime(double start, double end);

    static void dumpObjectStatistics(Heap*);

private:
    static void logStatistics();
    static Vector<double>* s_pauseTimeStarts;
    static Vector<double>* s_pauseTimeEnds;
    static double s_startTime;
    static double s_endTime;
};

} // namespace JSC

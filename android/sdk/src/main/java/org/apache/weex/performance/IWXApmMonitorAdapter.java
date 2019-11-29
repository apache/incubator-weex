/**
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 */
package org.apache.weex.performance;

public interface IWXApmMonitorAdapter {

    /**
     * start record
     *
     * @param instanceId instanceId
     */
    void onStart(String instanceId);

    /**
     * end record
     */
    void onEnd();

    /**
     * record event
     */
    void onEvent(String name, Object value);

    /**
     * record stage
     */
    void onStage(String name, long timestamp);

    /**
     * record property
     */
    void addProperty(String key, Object value);

    /**
     * record statistic
     */
    void addStats(String key, double value);

    /**
     * record subProcedure stage
     */

    void onSubProcedureStage(String procedureName, String stageName);

    /**
     * record SubProcedure event
     */

    void onSubProcedureEvent(String procedureName, String eventName);

    /**
     * record subProcedure stats
     */
    void setSubProcedureStats(String procedureName, String name, double value);

    /**
     * record subProcedure properties
     */
    void setSubProcedureProperties(String procedureName, String name, Object value);

    void onAppear();

    void onDisappear();

    String parseReportUrl(String originUrl);
}

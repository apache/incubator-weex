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
package org.apache.weex.utils.tools;

public class LogSwitch {

    private int low_level = 4;
    private int medium_level = 2;
    private int high_level = 1;
    private int log_switch = 0;
    private boolean showLowLevelLog = false;
    private boolean showMediumLevelLog = false;
    private boolean showHighLevelLog = true;


    public void setLog_switch() {
        // Todo
        // read prop

        if (showLowLevelLog)
            log_switch |= low_level;

        if (showMediumLevelLog) {
            log_switch |= medium_level;
        }

        if (showHighLevelLog) {
            log_switch |= high_level;
        }
    }


    public int getLog_switch() {
        return log_switch;
    }


}

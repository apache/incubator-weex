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

import com.alibaba.fastjson.annotation.JSONField;

public class Info{

	public Info() {
		taskInfo = new TaskInfo();
	}

	@JSONField(name="instanceId")
	public String instanceId;

	@JSONField(name="taskName")
	public String taskName;

	@JSONField(name="taskInfo")
	public TaskInfo taskInfo;

	@JSONField(name="platform")
	public String platform;

	@JSONField(name="taskId")
	public int taskId;

	@Override
 	public String toString(){
		return 
			"Info : {" +
			"instanceId = '" + instanceId + '\'' + 
			",taskName = '" + taskName + '\'' + 
			",taskInfo = '" + taskInfo + '\'' + 
			",platform = '" + platform + '\'' + 
			",taskId = '" + taskId + '\'' + 
			"}";
		}
}
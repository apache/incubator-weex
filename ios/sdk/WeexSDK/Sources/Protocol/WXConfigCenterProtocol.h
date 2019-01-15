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

@protocol WXConfigCenterProtocol <NSObject>

/**
 get config from config center handler
 @param key the key for config
 @param defaultValue default value for key if the key does not exist.
 @param isDefault whether the value is default value
 @return the value for config key
 */
- (id)configForKey:(NSString*)key defaultValue:(id)defaultValue isDefault:(BOOL*)isDefault;

@optional
/**
 get group config from config center handler
 @param group the groupName for config
*/

- (id)configForGroup:(NSString*)group;

@end

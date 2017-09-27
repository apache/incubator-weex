#!/usr/bin/env bash
# Licensed to the Apache Software Foundation (ASF) under one
# or more contributor license agreements.  See the NOTICE file
# distributed with this work for additional information
# regarding copyright ownership.  The ASF licenses this file
# to you under the Apache License, Version 2.0 (the
# "License"); you may not use this file except in compliance
# with the License.  You may obtain a copy of the License at
#
#   http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing,
# software distributed under the License is distributed on an
# "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
# KIND, either express or implied.  See the License for the
# specific language governing permissions and limitations
# under the License.

# Validate commit log
commit_regex='[+*-] \[(android|ios|jsfm|html5|component|doc|build|website|example|test|all)\] .{1,50}|\[WEEX-[0-9]*\] .{1,50}'

if ! grep -iqE "$commit_regex" "$1"; then
    echo "ERROR: Incorrect commit log format, please refer to:"
    echo "https://github.com/apache/incubator-weex/blob/master/CONTRIBUTING.md#commit-log"
    exit 1
fi
# FIXME no effect after editor (like vim) exits
# ISSUE merge or conflict

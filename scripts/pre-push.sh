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

# Validate branch name
branch_regex="^(\d+\.\d+\.\d+)|(master|dev|android|ios|jsfm|html5|component|doc|website|example|test|all)(-(bugfix|feature|hotfix)-.+)?$"
branch_name=$(git rev-parse --abbrev-ref HEAD)

if ! (echo $branch_name | grep -iqE "$branch_regex"); then
    echo "ERROR: current branch is \"$branch_name\", name format is not correct!
     
See https://github.com/alibaba/weex/blob/dev/CONTRIBUTING.md#branch-name\n"
    exit 1
fi
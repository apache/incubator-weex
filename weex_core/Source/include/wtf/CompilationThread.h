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
#ifndef CompilationThread_h
#define CompilationThread_h

namespace WTF {

WTF_EXPORT_PRIVATE bool exchangeIsCompilationThread(bool newValue);

class CompilationScope {
public:
    CompilationScope()
        : m_oldValue(exchangeIsCompilationThread(true))
    {
    }
    
    ~CompilationScope()
    {
        exchangeIsCompilationThread(m_oldValue);
    }
    
    void leaveEarly()
    {
        exchangeIsCompilationThread(m_oldValue);
    }
private:
    bool m_oldValue;
};

} // namespace WTF

using WTF::CompilationScope;
using WTF::exchangeIsCompilationThread;

#endif // CompilationThread_h


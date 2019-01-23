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

#ifndef CORE_DATA_RENDER_BINARY_FILE_H_
#define CORE_DATA_RENDER_BINARY_FILE_H_

#include <fstream>
#include <memory>

namespace weex {
namespace core {
namespace data_render {
    enum Section {
        NULL_SECTION,
        STRING_SECTION,
        TABLE_SECTION,
        FUNCTION_SECTION,
        START_SECTION,
        GLOBAL_SECTION,
        GLOBAL_VARIABLE_SECTION,
        STYLE_SECTION,
        ARRAY_SECTION,
        REF_SECTION,
        CLASS_SECTION
    };

    class BinaryFile {
    public:
        static BinaryFile* instance();
        void set_input(const char* input);
        void set_length(unsigned long length);
        void setWriteFileUrl(const std::string& url);
        void write(const char *stream, unsigned count);
        void read(char *stream, unsigned count);
        void writeFinish();
        void readFinish();
        bool eof() {return length_ == position_;}
    private:
        BinaryFile();

        static BinaryFile* g_instance_;
        std::unique_ptr<std::ofstream> fout_;
        const char* input_;
        unsigned long position_;
        unsigned long length_;
        bool little_endian_;
    };
}  // namespace data_render
}  // namespace core
}  // namespace weex
#endif //CORE_DATA_RENDER_BINARY_FILE_H_

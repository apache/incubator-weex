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

#ifndef CORE_DATA_RENDER_EXEC_STATE_BINARY_H
#define CORE_DATA_RENDER_EXEC_STATE_BINARY_H

#include <fstream>
#include <memory>
#include "core/data_render/exec_state_section.h"

namespace weex {
namespace core {
namespace data_render {
    
#define SECTION_VALUE_MASK                   0x03
#define SECTION_EXT_MASK                     0x80
#define SECTION_TAG_MASK                     0x10
#define SECTION_LEN_MASK                     0x0c
#define SECTION_LEN_OFF                         0x2
#define SECTION_TEST_FLAG(flag, mask)        (flag & mask)
#define SECTION_GET_LENGTH_LEN(flag, len)    len = ((flags & SECTION_LEN_MASK) >> SECTION_LEN_OFF); if (len == 3) len = 4;

class ExecState;
class fStream;

class ExecStateEncoder {
    friend class Section;
public:
    ExecStateEncoder(ExecState *exec_state, FILE *fout) : exec_state_(exec_state), fout_(fout) {}
    ~ExecStateEncoder() {}
    bool encoding(std::string &err);
    ExecState *exec_state() { return exec_state_; }
private:
    FILE *fout_{nullptr};
    ExecState *exec_state_{nullptr};
};
    
class fStream {
    friend class Section;
public:
    fStream(uint8_t *buffer, size_t size) : buffer_(buffer), size_(size), seek_(0) { }
    using seek_type = int64_t;
    using size_type = size_t;
    int64_t Seek(int64_t pos, int type);
    uint32_t Read(void *buffer, uint32_t size, uint32_t cnt);
    uint32_t ReadTarget(uint16_t *target, uint8_t *buffer, uint32_t *size);
    inline int64_t Tell() { return seek_; }
private:
    uint32_t Read(void *buffer, uint32_t size);
    seek_type seek_;
    size_type size_;
    uint8_t *buffer_;
};
    
class ExecStateDecoder {
    friend class Section;
public:
    ExecStateDecoder(ExecState *exec_state, uint8_t *buffer, size_t size) : stream_(new fStream(buffer, size)), exec_state_(exec_state) {  }
    ~ExecStateDecoder() { if (stream_) delete stream_; }
    ExecState *exec_state() { return exec_state_; }
    bool decoding(std::string &err);
private:
    fStream *stream_;
    ExecState *exec_state_{nullptr};
};

bool WXExecEncoder(std::string &src, std::string &path, std::string &error);
bool WXExecDecoder(ExecState *exec_state, uint8_t *buffer, size_t size, std::string &error);

}  // namespace data_render
}  // namespace core
}  // namespace weex

#endif // CORE_DATA_RENDER_EXEC_STATE_BINARY_H

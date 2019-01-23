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

#include "core/data_render/binary_file.h"
#include "core/data_render/common_error.h"

namespace weex {
namespace core {
namespace data_render {
    BinaryFile* BinaryFile::g_instance_ = nullptr;

    BinaryFile::BinaryFile():fout_(nullptr), position_(0), length_(0) {
        int32_t i=1;
        char *b=(char *)&i;
        if (*b == 1) {
            little_endian_ = true;
        } else {
            little_endian_ = false;
        }
    }

    BinaryFile* BinaryFile::instance() {
        if (!g_instance_) {
            g_instance_ = new BinaryFile();
        }
        return g_instance_;
    }

    void BinaryFile::set_input(const char* input) {
        input_ = input;
    }

    void BinaryFile::set_length(unsigned long length) {
        length_ = length;
    }

    void BinaryFile::setWriteFileUrl(const std::string& url) {
        fout_.reset(new std::ofstream(url, std::ios::binary));
    }

    void BinaryFile::write(const char *stream, unsigned count) {
        fout_->write(stream, count);
    }

    void BinaryFile::read(char *stream, unsigned count) {
        if (position_ + count > length_) {
            throw OpcodeDecodeError("Read data is error");
        }
        if (!little_endian_ && count > 1) {
            for (int i=count-1; i>=0; i--) {
                stream[i] = input_[position_++];
            }
        } else {
            for (int i=0; i<count; i++) {
                stream[i] = input_[position_++];
            }
        }
    }

    void BinaryFile::writeFinish() {
        fout_->close();
    }

    void BinaryFile::readFinish() {
        position_ = 0;
        length_ = 0;
        input_ = nullptr;
    }
}  // namespace data_render
}  // namespace core
}  // namespace weex

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
#ifndef DATA_RENDER_SCANNER_
#define DATA_RENDER_SCANNER_

#include <string>
#include <iosfwd>
#include <vector>

namespace weex {
namespace core {
namespace data_render {

class CharacterStream {
 public:
  virtual ~CharacterStream() = default;

  virtual void putback(char ch) = 0;

  virtual int read(std::string& str, int num) = 0;

  virtual char readchar() = 0;
};

// simple scanner class
template <class StreamType, class BufferType>
class BufferedCharacterStream : public CharacterStream {
 public:
  BufferedCharacterStream(StreamType& is) : is_(is), buffer_(0) {}
  ~BufferedCharacterStream() = default;

  // read a character from stream
  int read(std::string& str, int num) override {
    str.resize(num, '\0');

    if (is_.eof())
      return -1;
    // FIXME: make it working
    // auto size = is_.readsome(str.data(), num);
    return -1;
  }

  void putback(char ch) override {
    buffer_.push_back(ch);
  }

  char readchar() override {
    if (!buffer_.empty()) {
      char ch = buffer_.back();
      buffer_.pop_back();
      return ch;
    }
    if (is_.fail() || is_.eof()) {
      return EOF;
    }
    return is_.get();
  }

 private:
  StreamType& is_;
  BufferType buffer_;
};

using Scanner = BufferedCharacterStream<std::istream, std::vector<char>>;
using StandardCharacterStream = Scanner;

}
}
}

#endif
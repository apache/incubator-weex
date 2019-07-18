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
/**
# Copyright 2018 Taobao (China) Inc.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#   http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
**/

#ifndef FLUTTER_UNICORN_RUNTIME_RUNTIME_VALUES_H_
#define FLUTTER_UNICORN_RUNTIME_RUNTIME_VALUES_H_

#include <memory>
#include <string>
#include <type_traits>
#include <unordered_map>
#include <utility>
#include <vector>
#include <cstdlib>

#include "js_runtime/runtime/base.h"


namespace unicorn {

class EngineContext;
class RuntimeObject;
class RuntimeValues;

typedef std::unique_ptr<RuntimeValues> ScopeValues;

class RuntimeClass {
 public:
  explicit RuntimeClass(JSRunTimeClass clasz) : clasz_(clasz) {}
  ~RuntimeClass() = default;
    JSRunTimeClass GetJSClass() const { return clasz_; }

 private:
    JSRunTimeClass clasz_;
};

class BaseObject {
 public:
  virtual ~BaseObject() = default;
  virtual void* GetDataPtr() = 0;
  virtual RuntimeClass* GetRuntimeClass() = 0;
};

class Object : public BaseObject {
 public:
  static std::unique_ptr<BaseObject> MakeObject(void* ptr,
                                     RuntimeClass* clasz) {
    return std::unique_ptr<BaseObject>(static_cast<BaseObject*>(
                                                   new Object(ptr, clasz)));
  }

  Object(void* ptr, RuntimeClass* clasz)
        : ptr_(ptr),
          clasz_(clasz) {
  }

  // move constructor
  Object(Object&& other) : ptr_(other.ptr_) {
    other.Clear();
  }

  Object& operator=(Object&& other) {
    this->ptr_ = other.GetDataPtr();
    other.Clear();
    return *this;
  }

  ~Object() { ptr_ = nullptr; }

  void* GetDataPtr() override { return ptr_; }
  RuntimeClass* GetRuntimeClass() override { return clasz_.get(); }
  void Clear() { ptr_ = nullptr; }

 private:
  void* ptr_;
  std::unique_ptr<RuntimeClass> clasz_;
};

class Function {
 public:
  virtual ~Function();
  virtual bool IsFromNative() = 0;
  virtual std::unique_ptr<RuntimeValues> Call(JSRunTimeContext cur_context,size_t argc,
                                    std::vector<ScopeValues>& argv) const = 0;

  RuntimeObject* GetObject() const { return this_object_; }
  void SetObject(RuntimeObject* thiz);

 private:
  RuntimeObject* this_object_{nullptr};
};

class Map {
 public:
  static std::unique_ptr<Map> CreateFromNative(EngineContext* context,
                                               ScopeValues thiz);

  virtual ~Map();

  size_t Size() { return properties_.size(); }

  void Clear() noexcept {
    properties_.clear();
  }

  void Insert(const std::string& name, RuntimeValues* value) {
    properties_[name] = value;
  }

  void Remove(const std::string& name) {
    properties_.erase(name);
  }

  RuntimeValues* operator[] (std::string name) {
    return properties_[name];
  }

//  const RuntimeValues* get(const std::string & name) const{
//    return properties_[name];
//  }

  const std::unordered_map<std::string, RuntimeValues*>& GetMap() const {
    return properties_;
  }

 private:
  std::unordered_map<std::string, RuntimeValues*> properties_;
};

class Array {
 public:

 static std::unique_ptr<Array> CreateFromNative(EngineContext* context,
                                                   ScopeValues thiz);
  Array() = default;
  virtual ~Array();

  bool Empty() { return values_.empty(); }
  size_t Size() const { return values_.size(); }
  RuntimeValues* Front() { return values_.front(); }
  RuntimeValues* Back() { return values_.back(); }
  void PushBack(RuntimeValues* value) { values_.push_back(value); }
  void PopBack() { values_.pop_back(); }

  RuntimeValues* atIndex(size_t index) const { return values_[index];}

  RuntimeValues* operator[](size_t index) { return values_[index]; }
  const std::vector<RuntimeValues*>& GetArray() const { return values_; }

 private:
  std::vector<RuntimeValues*> values_;
};

class RuntimeValues {
 public:
  enum class Type {
    UNDEFINED = 0,
    NULLVALUE,
    BOOLEAN,
    INTEGER,
    DOUBLE,
    STRING,
    FUNCTION,
    OBJECT,
    MAP,
    ARRAY,
    JSONObject
  };

  RuntimeValues(RuntimeValues&& other);
  RuntimeValues& operator=(RuntimeValues&& that);

  void InternalMoveConstructFrom(RuntimeValues&& that);

  RuntimeValues() : type_(Type::UNDEFINED) {}

  explicit RuntimeValues(std::nullptr_t) : type_(Type::NULLVALUE) {
  }
  explicit RuntimeValues(bool in_bool);
  explicit RuntimeValues(int in_int);
  explicit RuntimeValues(double in_double);
  explicit RuntimeValues(const char* in_string, size_t length);
  explicit RuntimeValues(const std::string& in_string);
  explicit RuntimeValues(std::unique_ptr<BaseObject> object);
  explicit RuntimeValues(std::unique_ptr<Map> map);
  explicit RuntimeValues(std::unique_ptr<Function> func);
  explicit RuntimeValues(std::unique_ptr<Array> array);
  //json or other storage with string
  explicit RuntimeValues(const std::string &in_string,Type type);

  RuntimeValues& operator=(const RuntimeValues& that) = delete;
  RuntimeValues(const RuntimeValues& that) = delete;

  ~RuntimeValues();

  static ScopeValues MakeUndefined() {
    return ScopeValues(new RuntimeValues());
  }

  static ScopeValues MakeNull() {
    return ScopeValues(new RuntimeValues(nullptr));
  }

  static ScopeValues MakeBool(bool in_bool) {
    return ScopeValues(new RuntimeValues(in_bool));
  }

  static ScopeValues MakeInt(int in_int) {
    return ScopeValues(new RuntimeValues(in_int));
  }

    static ScopeValues MakeDouble(double in_double) {
      return ScopeValues(new RuntimeValues(in_double));
    }

//  static ScopeValues MakeString(const char* in_str) {
//    std::string tmp(in_str);
//    return ScopeValues(new RuntimeValues(tmp));
//  }

  static ScopeValues MakeString(const std::string& in_string) {
    return ScopeValues(new RuntimeValues(in_string));
  }

  static ScopeValues MakeCommonObject(void* ptr, RuntimeClass* clasz) {
    auto object = Object::MakeObject(ptr, clasz);
    return ScopeValues(new RuntimeValues(std::move(object)));
  }

  static ScopeValues MakeCommonObject(std::unique_ptr<BaseObject> object) {
    return ScopeValues(new RuntimeValues(std::move(object)));
  }

  static ScopeValues MakeMap(std::unique_ptr<Map> map) {
    return ScopeValues(new RuntimeValues(std::move(map)));
  }

  static ScopeValues MakeFunction(std::unique_ptr<Function> func) {
    return ScopeValues(new RuntimeValues(std::move(func)));
  }

  static ScopeValues MakeArray(std::unique_ptr<Array> array) {
    return ScopeValues(new RuntimeValues(std::move(array)));
  }

  static ScopeValues MakeObjectFromJsonStr(const std::string &utf_8_json_str) {
    return ScopeValues(new RuntimeValues(utf_8_json_str,Type::JSONObject));
  }


  void SetValue(std::unique_ptr<char[]>&& value);

  void SetType(Type type);
  Type GetType() const { return type_; }

  size_t GetLength();
  bool IsUndefined() const { return type_ == Type::UNDEFINED; }
  bool IsBool() const { return type_ == Type::BOOLEAN; }
  bool IsInt() const { return type_ == Type::INTEGER; }
  bool IsDouble() const { return type_ == Type::DOUBLE; }
  bool IsNumber() const { return IsInt() || IsDouble(); }
  bool IsString() const { return type_ == Type::STRING; }
  bool IsFunction() const { return type_ == Type::FUNCTION; }
  bool IsObject() const { return type_ == Type::OBJECT; }
  bool IsMap() const { return type_ == Type::MAP; }
  bool IsArray() const { return type_ == Type::ARRAY; }
  bool IsNull() const { return type_ == Type::NULLVALUE; }
  bool IsJsonObject() const { return type_ == Type::JSONObject; }

  // get from union
  bool GetAsBoolean(bool* out_val) const;
  bool GetAsInteger(int* out_val) const;
  bool GetAsDouble(double* out_val) const;
  bool GetAsString(std::string* out_val) const;
  bool GetAsUtf8JsonStr(std::string &json_val) const;
  const Map* GetAsMap() const { return map_.get(); }
  const Array* GetAsArray() const { return array_.get(); }
  Function* GetAsFunction() const { return function_.get(); }

  BaseObject* GetAsObject() const;
  BaseObject* PassObject();

  operator int() const {
    if (IsNumber()) {
      int r = 0;
      GetAsInteger(&r);
      return r;
    }
    abort();
    return 0;
  }

  operator double() const {
    if (IsNumber()) {
      double r = 0.0;
      GetAsDouble(&r);
      return r;
    }
    abort();
    return 0.0;
  }

  operator bool() const {
    if (IsBool()) {
      bool r = false;
      GetAsBoolean(&r);
      return r;
    }
    abort();
    return false;
  }

  operator std::string() const {
    if (IsString()) {
      std::string str;
      GetAsString(&str);
      return str;
    }
    abort();
    return "";
  }

  Type type_;

  struct Data {
    bool bool_value_;
    int int_value_;
    double double_value_;
    std::string string_value_;
  };

 private:
  std::unique_ptr<BaseObject> common_object_;
  std::unique_ptr<Map> map_;
  std::unique_ptr<Function> function_;
  std::unique_ptr<Array> array_;
  Data data_;
};

template<typename T>
static auto ToRuntimeValues(void* object, std::string name) {
  using Type = typename std::remove_reference<T>::type;

  auto holder = Object::MakeObject(object, nullptr);
  auto value = RuntimeValues::MakeCommonObject(std::move(holder));
  if (std::is_integral<Type>::value) {
    value->SetType(RuntimeValues::Type::INTEGER);
  }
  if (std::is_floating_point<Type>::value) {
    value->SetType(RuntimeValues::Type::DOUBLE);
  }
  if (std::is_same<std::string, Type>::value) {
    value->SetType(RuntimeValues::Type::STRING);
  }

  return value;
}

}  // namespace unicorn
#endif  // FLUTTER_UNICORN_RUNTIME_RUNTIME_VALUES_H_

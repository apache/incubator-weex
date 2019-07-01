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

#include "js_runtime/runtime/runtime_values.h"
#include <utility>

#include "js_runtime/runtime/runtime_object.h"
#include "js_runtime/utils/log_utils.h"

namespace unicorn {

    Map::~Map() {
        for (auto &iter : properties_) {
            delete iter.second;
        }
    }

    Array::~Array() {
        for (size_t i = 0; i < Size(); i++) {
            delete values_[i];
        }
    }

    Function::~Function() {
        if (this_object_)
            delete this_object_;
    }

    void Function::SetObject(RuntimeObject *thiz) {
        this_object_ = thiz;
    }

    RuntimeValues::RuntimeValues(RuntimeValues &&origin) {
        InternalMoveConstructFrom(std::move(origin));
    }

    void RuntimeValues::InternalMoveConstructFrom(RuntimeValues &&that) {
        type_ = that.type_;

        switch (type_) {
            case Type::UNDEFINED:
                return;
            case Type::NULLVALUE:
                return;
            case Type::BOOLEAN:
                data_.bool_value_ = that.data_.bool_value_;
                return;
            case Type::INTEGER:
                data_.int_value_ = that.data_.int_value_;
                return;
            case Type::DOUBLE:
                data_.double_value_ = that.data_.double_value_;
                return;
            case Type::STRING:
                new(&data_.string_value_) std::string(
                        std::move(that.data_.string_value_));
                break;
            case Type::OBJECT:
                common_object_ = std::move(that.common_object_);
                break;
            case Type::FUNCTION:
                function_ = std::move(that.function_);
                break;
            case Type::MAP:
                map_ = std::move(that.map_);
                break;
            case Type::ARRAY:
                array_ = std::move(that.array_);
            case Type::JSONObject:
                data_.string_value_=that.data_.string_value_;
                break;
        }
    }

    RuntimeValues::RuntimeValues(bool in_bool) : type_(Type::BOOLEAN) {
        data_.bool_value_ = in_bool;
    }

    RuntimeValues::RuntimeValues(int in_int) : type_(Type::INTEGER) {
        data_.int_value_ = in_int;
    }

    RuntimeValues::RuntimeValues(double in_double) : type_(Type::DOUBLE) {
        data_.double_value_ = in_double;
    }

    RuntimeValues::RuntimeValues(const char *in_string, size_t length)
            : type_(Type::STRING) {
        data_.string_value_ = std::string(in_string, length);
    }

    RuntimeValues::RuntimeValues(const std::string &in_string)
            : type_(Type::STRING) {
        data_.string_value_ = in_string;
       // new(&data_.string_value_) std::string(in_string);
    }

    RuntimeValues::RuntimeValues(const std::string &in_string,Type type){
        data_.string_value_ = in_string;
        type_ = type;
        // new(&data_.string_value_) std::string(in_string);
    }

    RuntimeValues::RuntimeValues(std::unique_ptr<BaseObject> object)
            : type_(Type::OBJECT),
              common_object_(std::move(object)) {
    }

    RuntimeValues::RuntimeValues(std::unique_ptr<Map> map)
            : type_(Type::MAP) {
        map_ = std::move(map);
    }

    RuntimeValues::RuntimeValues(std::unique_ptr<Function> func)
            : type_(Type::FUNCTION) {
        function_ = std::move(func);
    }

    RuntimeValues::RuntimeValues(std::unique_ptr<Array> array)
            : type_(Type::ARRAY) {
        array_ = std::move(array);
    }

    RuntimeValues &RuntimeValues::operator=(RuntimeValues &&that) {
        if (this == &that)
            return *this;

        InternalMoveConstructFrom(std::move(that));

        return *this;
    }

    RuntimeValues::~RuntimeValues() {
    }

    void RuntimeValues::SetValue(std::unique_ptr<char[]> &&value) {}

    void RuntimeValues::SetType(Type type) {
        if (common_object_) {
            switch (type) {
                case Type::UNDEFINED:
                    break;
                case Type::NULLVALUE:
                    break;
                case Type::BOOLEAN: {
                    Object *origin_b = static_cast<Object *>(common_object_.release());
                    data_.bool_value_ = *(static_cast<bool *>(origin_b->GetDataPtr()));
                }
                    break;
                case Type::INTEGER: {
                    Object *origin_i = static_cast<Object *>(common_object_.release());
                    data_.int_value_ = *(static_cast<int *>(origin_i->GetDataPtr()));
                }
                    break;
                case Type::DOUBLE: {
                    Object *origin_d = static_cast<Object *>(common_object_.release());
                    data_.double_value_ = *(static_cast<double *>(
                            origin_d->GetDataPtr()));
                }
                    break;
                case Type::STRING: {
                    Object *origin_s = static_cast<Object *>(common_object_.release());
                    data_.string_value_ = *(static_cast<std::string *>(
                            origin_s->GetDataPtr()));
                }
                    break;
                case Type::OBJECT:
                    break;
                case Type::FUNCTION:
                    break;
                case Type::MAP:
                    break;
                case Type::ARRAY:
                    break;
                default:
                    break;
            }
        }
        type_ = type;
    }

    bool RuntimeValues::GetAsBoolean(bool *out_value) const {
        if (IsBool()) {
            *out_value = data_.bool_value_;
            return true;
        }

        return false;
    }

    bool RuntimeValues::GetAsInteger(int *out_value) const {
        if (IsDouble()) {
            *out_value = static_cast<int>(data_.double_value_);
            return true;
        }
        if (IsInt()) {
            *out_value = data_.int_value_;
            return true;
        }

        return false;
    }

    bool RuntimeValues::GetAsDouble(double *out_value) const {
        if (IsDouble()) {
            *out_value = data_.double_value_;
            return true;
        }
        if (IsInt()) {
            *out_value = static_cast<double>(data_.int_value_);
            return true;
        }

        return false;
    }

    bool RuntimeValues::GetAsUtf8JsonStr(std::string &out_value) const {
        if (IsJsonObject()) {
            out_value.assign(data_.string_value_);
            return true;
        }
        return false;
    }


    bool RuntimeValues::GetAsString(std::string *out_value) const {
        if (IsString()) {
            out_value->assign(data_.string_value_);
            return true;
        }

        return false;
    }

    BaseObject *RuntimeValues::GetAsObject() const {
        if (IsObject()) {
            return common_object_.get();
        }

        return nullptr;
    }


    BaseObject *RuntimeValues::PassObject() {
        if (IsObject()) {
            return common_object_.release();
        }

        return nullptr;
    }

}  // namespace unicorn

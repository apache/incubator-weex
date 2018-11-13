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

#include "core/data_render/exec_state.h"
#include "core/data_render/code_generator.h"
#include "core/data_render/string_table.h"
#include "core/data_render/vm.h"
#include "core/data_render/parser.h"
#include "core/data_render/rax_parser_builder.h"
#include "core/data_render/rax_parser.h"
#include "core/data_render/rax_parser_context.h"
#include "core/data_render/common_error.h"
#include "core/data_render/binary_file.h"
#include "core/data_render/class_array.h"

#if DEBUG
#include "core/data_render/monitor/vm_monitor.h"
#endif

namespace weex {
namespace core {
namespace data_render {
    
int ValueRef::gs_ref_id = 0;

void ExecStack::reset() {
    size_t size = (VM_EXEC_STACK_SIZE - (top_ - base()) - 1) * sizeof(Value);
    //LOGD("reset:%i=>\n", (int)(top_ - base()));
    memset(top_, 0, size);
}

ExecState::ExecState(VM *vm)
    : vm_(vm),
      frames_(),
      refs_(),
      global_(new Variables),
      stack_(new ExecStack),
      func_state_(nullptr),
      string_table_(new StringTable),
      render_context_(new VNodeRenderContext),
      class_factory_(new ClassFactory()),
      global_variables_() {}

void ExecState::Compile(std::string& err) {

#if DEBUG
  TimeCost tc("Compile");
#endif
  ValueRef::gs_ref_id = 0;
  err.clear();
  CodeGenerator generator(this);
  if (!context()->raw_json().is_null()) {
      ParseResult result = Parser::Parse(context()->raw_json(),err);
      generator.Visit(result.expr().get(), nullptr);
  }
  else if (context()->raw_source().length() > 0) {
      weex::core::data_render::RAXParserBuilder builder(context()->raw_source());
      weex::core::data_render::RAXParser *parser = builder.parser();
      Handle<Expression> expr = nullptr;
      try {
          expr = ParseProgram(parser);
      }
      catch (std::exception &e) {
          auto error = static_cast<Error *>(&e);
          if (error) {
              err = error->what();
              std::cerr << error->what() << std::endl;
          }
          return;
      }
      if (expr->IsChunkStatement()) {
          try {
              generator.Visit(expr->AsChunkStatement().get(), nullptr);
          }
          catch (std::exception &e) {
              auto error = static_cast<Error *>(&e);
              if (error) {
                  err = error->what();
                  std::cerr << error->what() << std::endl;
              }
              return;
          }
      }
  }
}
    
void ExecState::Execute(std::string& err) {
#if DEBUG
  TimeCost tc("Execute");
#endif
  err.clear();
  Value chunk;
  chunk.type = Value::Type::FUNC;
  chunk.f = func_state_.get();
  // reset stack top pointer when main call
  *stack_->top() = stack_->base();
  *stack_->base() = chunk;
  try {
      CallFunction(stack_->base(), 0, nullptr);
  } catch (std::exception &e) {
      auto error = static_cast<Error *>(&e);
      if (error) {
          err = error->what();
          std::cerr << error->what() << std::endl;
      }
      return;
  }
}

void ExecState::startEncode() {
    BinaryFile* file = BinaryFile::instance();

    int magic_number = 0x6d736100;
    unsigned version = 1;
    file->write((char*)&magic_number, sizeof(int32_t));
    file->write((char*)&version, sizeof(u_int32_t));

    encodeStringSection();
    encodeTableSection();
    encodeFunctionSection();
    encodeStartSection();
    encodeGlobalSection();
    encodeGlobalVariableSection();
    encodeStyleSection();
    encodeArraySection();
    encodeRefSection();
    encodeClassSection();
}

void ExecState::encodeStringSection() {
    unsigned id = Section::STRING_SECTION; //string_section

    const std::vector<std::pair<std::string, std::unique_ptr<String>>>& store = string_table_->store();
    unsigned size = static_cast<unsigned>(store.size());
    BinaryFile* file = BinaryFile::instance();

    file->write((char*)&id, sizeof(u_int32_t));
    file->write((char*)&size, sizeof(u_int32_t));
    for (auto &mapString : store) {
        unsigned length = static_cast<unsigned int>(mapString.first.length()) + 1;
        file->write(mapString.first.c_str(), static_cast<unsigned int>(sizeof(char) * length));
    }
}

void ExecState::encodeArraySection() {
    unsigned id = Section::ARRAY_SECTION;
    std::vector<Array*> arrays = class_factory_->arrays();
    unsigned size = static_cast<unsigned>(arrays.size());
    BinaryFile* file = BinaryFile::instance();

    file->write((char*)&id, sizeof(u_int32_t));
    file->write((char*)&size, sizeof(u_int32_t));
    for (auto array : arrays) {
        unsigned itemSize = static_cast<unsigned>(array->items.size());
        file->write((char*)&itemSize, sizeof(u_int32_t));
        for (auto &value : array->items) {
            encodeValue(value);
        }
    }
}

void ExecState::encodeTableSection() {
    unsigned id = Section::TABLE_SECTION;
    std::vector<Table *> tables = class_factory_->tables();

    unsigned size = static_cast<unsigned>(tables.size());
    BinaryFile* file = BinaryFile::instance();

    file->write((char*)&id, sizeof(u_int32_t));
    file->write((char*)&size, sizeof(u_int32_t));

    for (auto table : tables) {
        unsigned mapSize = static_cast<unsigned>(table->map.size());
        file->write((char*)&mapSize, sizeof(u_int32_t));
        for (auto &map : table->map) {
            std::string name = map.first;
            Value value = map.second;
            String str(name);
            unsigned length = static_cast<unsigned>(str.length()) + 1;
            file->write(str.c_str(), static_cast<unsigned>(sizeof(char) * length));
            encodeValue(value);
        }
    }
}

void ExecState::encodeFunctionSection() {
    BinaryFile* file = BinaryFile::instance();
    unsigned id = Section::FUNCTION_SECTION;

    FuncState* func_state_base = func_state_.get();
    std::vector<FuncState*> children = func_state_base->getAllChildren();
    std::vector<FuncState*> func_states;
    func_states.push_back(func_state_base);
    for (auto &func : children) {
        func_states.push_back(func);
    }
    unsigned size = static_cast<unsigned>(func_states.size());
    file->write((char*)&id, sizeof(u_int32_t));
    file->write((char*)&size, sizeof(u_int32_t));

    for (auto &func_state : func_states) {
        auto it = std::find(func_states.begin(), func_states.end(), func_state->super_func());
        int super_index = -1;
        if (it != func_states.end()) {
            super_index = static_cast<int>(it - func_states.begin());
        }
        file->write((char*)&super_index, sizeof(int32_t));

        bool is_class_func = func_state->is_class_func();
        file->write((char*)&is_class_func, sizeof(int8_t));

        int argc = func_state->argc();
        file->write((char*)&argc, sizeof(int32_t));

        unsigned opcodeSize = static_cast<unsigned>(func_state->instructions().size());
        file->write((char*)&opcodeSize, sizeof(u_int32_t));
        for (int i=0; i<opcodeSize; i++) {
            unsigned opcode = static_cast<unsigned>(func_state->instructions()[i]);
            file->write((char*)&opcode, sizeof(u_int32_t));
        }

        unsigned constantSize = static_cast<unsigned>(func_state->GetConstantSize());
        file->write((char*)&constantSize, sizeof(u_int32_t));
        for (int i=0; i<constantSize; i++) {
            encodeValue(*(func_state->GetConstant(i)));
        }
    }
}

void ExecState::encodeStartSection() {
    BinaryFile* file = BinaryFile::instance();
    unsigned id = Section::START_SECTION;
    unsigned index = 0;
    file->write((char*)&id, sizeof(u_int32_t));
    file->write((char*)&index, sizeof(u_int32_t));
}

void ExecState::encodeGlobalSection() {
    BinaryFile* file = BinaryFile::instance();
    unsigned id = Section::GLOBAL_SECTION;
    unsigned size = static_cast<unsigned>(global_->size()) - global_->register_size();

    file->write((char*)&id, sizeof(u_int32_t));
    file->write((char*)&size, sizeof(u_int32_t));
    int init_data_index = global_->IndexOf("_init_data");
    int weex_data_index = global_->IndexOf("__weex_data__");
    file->write((char*)&init_data_index, sizeof(int32_t));
    file->write((char*)&weex_data_index, sizeof(int32_t));

    for (int i=global_->register_size(); i<global_->size(); i++) {
        Value* value = global_->Find(i);
        encodeValue(*value);
    }
}

void ExecState::encodeGlobalVariableSection() {
    BinaryFile* file = BinaryFile::instance();
    unsigned id = Section::GLOBAL_VARIABLE_SECTION;
    unsigned size = static_cast<unsigned>(global_variables_.size());

    file->write((char*)&id, sizeof(u_int32_t));
    file->write((char*)&size, sizeof(u_int32_t));
    for (auto variable : global_variables_) {
        unsigned length = static_cast<unsigned>(variable.first.length()) + 1;
        file->write(variable.first.c_str(), static_cast<unsigned int>(sizeof(char) * length));
        file->write((char*)&variable.second, sizeof(int64_t));
    }
}

void ExecState::encodeStyleSection() {
    unsigned id = Section::STYLE_SECTION;
    std::map<std::string, json11::Json>& styles = render_context_->style_json();

    unsigned size = static_cast<unsigned>(styles.size());
    BinaryFile* file = BinaryFile::instance();

    file->write((char*)&id, sizeof(u_int32_t));
    file->write((char*)&size, sizeof(u_int32_t));

    for (auto style : styles) {
        Value value(string_table()->StringFromUTF8(style.first));
        encodeValue(value);
        const json11::Json::object& items = style.second.object_items();
        unsigned itemsSize = static_cast<unsigned>(items.size());
        file->write((char*)&itemsSize, sizeof(u_int32_t));
        for (auto it = items.begin(); it != items.end(); it++) {
            unsigned length = static_cast<unsigned>(it->first.length()) + 1;
            file->write(it->first.c_str(), static_cast<unsigned int>(sizeof(char) * length));

            unsigned strLen = static_cast<unsigned int>(it->second.string_value().length()) + 1;
            file->write(it->second.string_value().c_str(), static_cast<unsigned int>(sizeof(char) * strLen));
        }
    }
}

void ExecState::encodeRefSection() {
    unsigned id = Section::REF_SECTION;
    unsigned size = static_cast<unsigned>(refs_.size());
    BinaryFile* file = BinaryFile::instance();

    file->write((char*)&id, sizeof(u_int32_t));
    file->write((char*)&size, sizeof(u_int32_t));
    for (auto ref : refs_) {
        Value value;
        value.type = Value::Type::FUNC;
        value.f = ref->func_state_;
        encodeValue(value);
        long register_id = ref->register_id();
        file->write((char*)&register_id, sizeof(int64_t));
        encodeValue(ref->value_);
    }
}

void ExecState::encodeClassSection() {
    unsigned id = Section::CLASS_SECTION;
    std::vector<ClassDescriptor *> descs = class_factory_->descs();

    unsigned size = static_cast<unsigned>(descs.size());
    BinaryFile* file = BinaryFile::instance();

    file->write((char*)&id, sizeof(u_int32_t));
    file->write((char*)&size, sizeof(u_int32_t));

    int count = 0;
    for (auto desc : descs) {
        count++;
        if (count <= 3) {
            continue;
        }

        int superIndex = class_factory_->findDesc(desc->p_super_);
        file->write((char*)&superIndex, sizeof(int32_t));
        unsigned static_func_size = static_cast<unsigned>(desc->static_funcs_->size());
        file->write((char*)&static_func_size, sizeof(u_int32_t));
        for (int i=0; i<static_func_size; i++) {
            Value* value = desc->static_funcs_->Find(i);
            encodeValue(*value);

            unsigned length = 0;
            for (auto &item : desc->static_funcs_->map()) {
                if (item.second == i) {
                    length = static_cast<unsigned>(item.first.length()) + 1;
                    file->write((char*)&length, sizeof(u_int32_t));
                    file->write(item.first.c_str(), static_cast<unsigned int>(sizeof(char) * length));
                    break;
                }
            }
            if (length == 0) {
                file->write((char*)&length, sizeof(u_int32_t));
            }
        }

        unsigned class_func_size = static_cast<unsigned>(desc->funcs_->size());
        file->write((char*)&class_func_size, sizeof(u_int32_t));
        for (int i=0; i<class_func_size; i++) {
            Value* value = desc->funcs_->Find(i);
            encodeValue(*value);

            unsigned length = 0;
            for (auto &item : desc->funcs_->map()) {
                if (item.second == i) {
                    length = static_cast<unsigned>(item.first.length()) + 1;
                    file->write((char*)&length, sizeof(u_int32_t));
                    file->write(item.first.c_str(), static_cast<unsigned int>(sizeof(char) * length));
                    break;
                }
            }
            if (length == 0) {
                file->write((char*)&length, sizeof(u_int32_t));
            }
        }
    }
}

void ExecState::encodeValue(const Value &value) {
    BinaryFile* file = BinaryFile::instance();
    file->write((char*)&value.type, sizeof(u_int32_t));

    if (value.type == Value::Type::TABLE) {
        std::vector<Table *> tables = class_factory_->tables();
        int payload = 0;
        for (auto table : tables) {
            if (value.gc == reinterpret_cast<GCObject *>(table)) {
                break;
            }
            payload++;
        }
        file->write((char*)&payload, sizeof(int32_t));
    }

    if (value.type == Value::Type::STRING) {
        //int payload = static_cast<int>(strings.size());
        int payload = 0;
        for (auto &store : string_table_->store() ) {
            if (value.str == store.second.get()) {
                // printf("---- %s  \n", str.get()->c_str());
                break;
            }
            payload++;
        }
        file->write((char*)&payload, sizeof(int32_t));
    }

    if (value.type == Value::Type::INT) {
        file->write((char*)&value.i, sizeof(int64_t));
    }

    if (value.type == Value::Type::NUMBER) {
        file->write((char*)&value.n, sizeof(double));
    }

    if (value.type == Value::Type::BOOL) {
        file->write((char*)&value.b, sizeof(char));
    }

    if (value.type == Value::Type::FUNC) {
        FuncState* func_state_base = func_state_.get();
        const std::vector<FuncState*> &children = func_state_base->getAllChildren();
        std::vector<FuncState*> func_states;
        func_states.push_back(func_state_base);
        for (auto &func : children) {
            func_states.push_back(func);
        }

        int payload = 0;
        for (auto func_state : func_states) {
            if (value.f == func_state) {
                break;
            }
            payload++;
        }
        file->write((char*)&payload, sizeof(int32_t));
    }

    if (value.type == Value::Type::ARRAY) {
        std::vector<Array*> arrays = class_factory_->arrays();
        int payload = 0;
        for (auto array : arrays ) {
            if (value.gc == reinterpret_cast<GCObject *>(array)) {
                break;
            }
            payload++;
        }
        file->write((char*)&payload, sizeof(int32_t));
    }

    if (value.type == Value::Type::CLASS_DESC) {
        int payload = 0;
        std::vector<ClassDescriptor *> descs = class_factory_->descs();
        for (auto desc : descs) {
            if (value.gc == reinterpret_cast<GCObject *>(desc)) {
                break;
            }
            payload++;
        }
        file->write((char*)&payload, sizeof(int32_t));
    }
}

void ExecState::endEncode() {
    BinaryFile* file = BinaryFile::instance();
    file->writeFinish();
}

bool ExecState::startDecode() {
    ValueRef::gs_ref_id = 0;

    BinaryFile* file = BinaryFile::instance();
    int magic_number;
    file->read((char*)&magic_number, sizeof(int32_t));
    if (magic_number != 0x6d736100) {
        throw OpcodeDecodeError("The format of file is not wasm");
    }
    unsigned version;
    file->read((char*)&version, sizeof(u_int32_t));
    if (version > 1) {
        throw OpcodeDecodeError("The version of file can not support");
    }

    while (!file->eof()) {
        Section section_id = NULL_SECTION;
        file->read((char*)&section_id, sizeof(u_int32_t));
        switch (section_id) {
            case STRING_SECTION:
                decodeStringSection();
                break;
            case TABLE_SECTION:
                decodeTableSection();
                break;
            case FUNCTION_SECTION:
                decodeFunctionSection();
                break;
            case START_SECTION:
                decodeStartSection();
                break;
            case GLOBAL_SECTION:
                decodeGlobalSection();
                break;
            case GLOBAL_VARIABLE_SECTION:
                decodeGlobalVariableSection();
                break;
            case STYLE_SECTION:
                decodeStyleSection();
                break;
            case ARRAY_SECTION:
                decodeArraySection();
                break;
            case REF_SECTION:
                decodeRefSection();
                break;
            case CLASS_SECTION:
                decodeClassSection();
                break;
            default:
                break;
        }
    }
    return true;
}

void ExecState::decodeStringSection()
{
    BinaryFile* file = BinaryFile::instance();

    unsigned count = 0;
    file->read((char*)&count, sizeof(u_int32_t));
    if (count == 0) {
        return;
    }

    for (int i=0; i<count; i++) {
        std::string str;
        char c;
        while (true) {
            file->read(&c, sizeof(char));
            if (c == 0) {
                break;
            }
            str += c;
        }
        string_table_->StringFromUTF8(str);
    }
}

void ExecState::decodeTableSection() {
    BinaryFile* file = BinaryFile::instance();

    unsigned count = 0;
    file->read((char*)&count, sizeof(u_int32_t));
    if (count == 0) {
        return;
    }

    for (int i=0; i<count; i++) {
        Value tableValue = class_factory_->CreateTable();
        Table* table = reinterpret_cast<Table *>(tableValue.gc);
        unsigned mapSize=0;
        file->read((char*)&mapSize, sizeof(u_int32_t));
        for (int j=0; j<mapSize; j++) {
            std::string str;
            char c;
            while (true) {
                file->read(&c, sizeof(char));
                if (c == 0) {
                    break;
                }
                str += c;
            }
            Value value;
            decodeValue(value);
            table->map.insert(std::make_pair(str, value));
        }
    }
}

void ExecState::decodeFunctionSection() {
    BinaryFile* file = BinaryFile::instance();

    unsigned count = 0;
    file->read((char*)&count, sizeof(u_int32_t));
    if (count == 0) {
        return;
    }

    std::vector<FuncState*> func_states;
    for (int i=0; i<count; i++) {
        FuncState* func_state = new FuncState;
        int super_index = -1;
        file->read((char*)&super_index, sizeof(int32_t));
        func_state->set_super_index(super_index);

        bool is_class_func = false;
        file->read((char*)&is_class_func, sizeof(int8_t));
        func_state->set_is_class_func(is_class_func);

        int argc = 0;
        file->read((char*)&argc, sizeof(int32_t));
        func_state->argc() = argc;

        unsigned op_code_count = 0;
        file->read((char*)&op_code_count, sizeof(u_int32_t));
        for (int j=0; j<op_code_count; j++) {
            unsigned opcode;
            file->read((char*)&opcode, sizeof(u_int32_t));
            func_state->AddInstruction(opcode);
        }
        unsigned local_count = 0;
        file->read((char*)&local_count, sizeof(u_int32_t));
        for (int j=0; j<local_count; j++) {
            Value value;
            decodeValue(value);
            func_state->AddConstant(value);
        }
        func_states.push_back(func_state);
    }

    func_state_.reset(func_states[0]);
    for (auto func : func_states) {
        int index = func->super_index();
        if (index > -1) {
            func_states[index]->AddChild(func);
        }
    }
}

void ExecState::decodeStartSection() {
    BinaryFile* file = BinaryFile::instance();
    unsigned start_index;
    file->read((char*)&start_index, sizeof(u_int32_t));
}

void ExecState::decodeGlobalSection() {
    BinaryFile* file = BinaryFile::instance();

    unsigned count = 0;
    file->read((char*)&count, sizeof(u_int32_t));

    int init_data_index = -1;
    int weex_data_index = -1;
    file->read((char*)&init_data_index, sizeof(int32_t));
    file->read((char*)&weex_data_index, sizeof(int32_t));

    unsigned register_size = global_->register_size();
    for (int i=0; i<count; i++) {
        Value value;
        decodeValue(value);
        if (value.type != Value::Type::CFUNC) {
            if (register_size + i == init_data_index) {
                global_->Add("_init_data", value);
            } else if (register_size + i == weex_data_index) {
                global_->Add("__weex_data__", value);
            } else  {
                global_->Add(value);
            }
        }
    }
}

void ExecState::decodeGlobalVariableSection() {
    BinaryFile* file = BinaryFile::instance();

    unsigned count = 0;
    file->read((char*)&count, sizeof(u_int32_t));
    if (count == 0) {
        return;
    }

    for (int i=0; i<count; i++) {
        std::string key;
        char c;
        while (true) {
            file->read(&c, sizeof(char));
            if (c == 0) {
                break;
            }
            key += c;
        }

        long value;
        file->read((char*)&value, sizeof(int64_t));
        global_variables_.insert(std::pair<std::string, long>(key, value));
    }
}

void ExecState::decodeStyleSection() {
    BinaryFile* file = BinaryFile::instance();

    unsigned count = 0;
    file->read((char*)&count, sizeof(u_int32_t));
    if (count == 0) {
        return;
    }

    for (int i=0; i<count; i++) {
        Value value;
        decodeValue(value);

        unsigned itemsSize = 0;
        file->read((char*)&itemsSize, sizeof(u_int32_t));
        //json11::Json::object items;
        std::unordered_map<std::string, json11::Json> items;
        for (int j=0; j<itemsSize; j++) {
            std::string key;
            char c;
            while (true) {
                file->read(&c, sizeof(char));
                if (c == 0) {
                    break;
                }
                key += c;
            }

            std::string value;
            char v;
            while (true) {
                file->read(&v, sizeof(char));
                if (v == 0) {
                    break;
                }
                value += v;
            }
            json11::Json json(value);
            items.insert(std::make_pair(std::move(key), std::move(json)));
        }
        json11::Json json(std::move(items));
        styles_.insert(std::make_pair(value.index, std::move(json)));
    }
}

void ExecState::decodeArraySection() {
    BinaryFile* file = BinaryFile::instance();
    unsigned count = 0;
    file->read((char*)&count, sizeof(u_int32_t));
    for (int i=0; i<count; i++) {
        Value value = class_factory_->CreateArray();
        Array* array = reinterpret_cast<Array *>(value.gc);
        unsigned itemSize = 0;
        file->read((char*)&itemSize, sizeof(u_int32_t));
        for (int j=0; j<itemSize; j++) {
            Value itemValue;
            decodeValue(itemValue);
            array->items.push_back(itemValue);
        }
    }
}

void ExecState::decodeRefSection() {
    BinaryFile* file = BinaryFile::instance();
    unsigned count = 0;
    file->read((char*)&count, sizeof(u_int32_t));
    for (int i=0; i<count; i++) {
        Value value;
        decodeValue(value);
        long register_id;
        file->read((char*)&register_id, sizeof(int64_t));
        ValueRef *ref = new ValueRef(nullptr, register_id);
        ref->func_index_ = value.index;
        decodeValue(ref->value_);
        refs_.push_back(ref);
    }
}

void ExecState::decodeClassSection() {
    BinaryFile* file = BinaryFile::instance();
    unsigned count = 0;
    file->read((char*)&count, sizeof(u_int32_t));

    size_t size = class_factory_->descs().size();
    for (int i=0; i<count - size; i++) {
        Value value = class_factory_->CreateClassDescriptor(nullptr);
        ClassDescriptor* desc = reinterpret_cast<ClassDescriptor *>(value.gc);
        int super_index = -1;
        file->read((char*)&super_index, sizeof(int32_t));
        desc->super_index_ = super_index;

        unsigned static_func_size = 0;
        file->read((char*)&static_func_size, sizeof(u_int32_t));
        for (int j=0; j<static_func_size; j++) {
            Value static_value;
            decodeValue(static_value);
            unsigned length = 0;
            file->read((char*)&length, sizeof(u_int32_t));
            if (length > 0) {
                std::unique_ptr<char[]> str(new char[length]);
                file->read(str.get(), sizeof(char)*length);
                desc->static_funcs_->Add(str.get(), static_value);
            } else {
                desc->static_funcs_->Add(static_value);
            }
        }

        unsigned class_func_size = 0;
        file->read((char*)&class_func_size, sizeof(u_int32_t));
        for (int j=0; j<class_func_size; j++) {
            Value class_value;
            decodeValue(class_value);

            unsigned length = 0;
            file->read((char*)&length, sizeof(u_int32_t));
            if (length > 0) {
                std::unique_ptr<char[]> str(new char[length]);
                file->read(str.get(), sizeof(char)*length);
                desc->funcs_->Add(str.get(), class_value);
            } else {
                desc->funcs_->Add(class_value);
            }
        }
    }
}

void ExecState::decodeValue(Value &value) {
    BinaryFile* file = BinaryFile::instance();
    unsigned valueType;
    file->read((char*)&valueType, sizeof(u_int32_t));
    value.type = Value::Type(valueType);

    switch (valueType) {
        case Value::Type::TABLE:
        case Value::Type::STRING:
        case Value::Type::FUNC:
        case Value::Type::ARRAY:
        case Value::Type::CLASS_DESC: {
            value.gc = nullptr;
            file->read((char*)&(value.index), sizeof(int32_t));
        }
            break;
        case Value::Type::INT: {
            file->read((char*)&(value.i), sizeof(int64_t));
        }
            break;
        case Value::Type::NUMBER: {
            file->read((char*)&(value.n), sizeof(double));
        }
            break;
        case Value::Type::BOOL: {
            file->read((char*)&(value.b), sizeof(char));
        }
            break;
        default:
            break;
    }
}

void ExecState::endDecode() {
    BinaryFile* file = BinaryFile::instance();
    file->readFinish();

    std::vector<Table *> tables = class_factory_->tables();
    for (auto table : tables ) {
        for (auto &map : table->map) {
            Value& value = map.second;
            serializeValue(value);
        }
    }

    std::vector<Value>& constants = func_state_->constants();
    for (auto &value : constants) {
        serializeValue(value);
    }
    std::vector<FuncState*> children = func_state_->getAllChildren();
    for (auto func_state : children) {
        std::vector<Value>& child_constants = func_state->constants();
        for (auto &value : child_constants) {
            serializeValue(value);
        }
    }

    for (int i=0; i<global_->size(); i++) {
        Value* value = global_->Find(i);
        serializeValue(*value);
    }

    std::map<std::string, json11::Json>& styles = render_context_->style_json();
    for (auto &style : styles_) {
        Value value;
        value.index = style.first;
        value.type = Value::Type::STRING;
        serializeValue(value);
        styles.insert(std::make_pair(value.str->c_str(), style.second));
    }

    std::vector<Array *> arrays = class_factory_->arrays();
    for (auto array : arrays) {
        for (auto &value : array->items) {
            serializeValue(value);
        }
    }

    for (auto ref : refs_) {
        if (ref->func_index_ == 0) {
            ref->func_state_ = func_state_.get();
        } else {
            ref->func_state_ = func_state_->getAllChildren()[ref->func_index_-1];
        }
        serializeValue(ref->value_);
    }

    std::vector<ClassDescriptor *> descs = class_factory_->descs();
    int count = 0;
    for (auto desc : descs) {
        count++;
        if (count <= 4) {
            continue;
        }
        if (desc->super_index_ != -1) {
            desc->p_super_ = descs[desc->super_index_];
        }
        for (int i=0; i<desc->static_funcs_->size(); i++) {
            Value* value = desc->static_funcs_->Find(i);
            serializeValue(*value);
        }
        for (int i=0; i<desc->funcs_->size(); i++) {
            Value* value = desc->funcs_->Find(i);
            serializeValue(*value);
        }
    }
}

void ExecState::serializeValue(Value &value) {
    if (value.index == -1) {
        return;
    }

    if (value.type == Value::Type::TABLE) {
        std::vector<Table *> tables = class_factory_->tables();
        value.gc = reinterpret_cast<GCObject *>(tables[value.index]);
    }

    if (value.type == Value::Type::STRING) {
        const std::vector<std::pair<std::string, std::unique_ptr<String>>>& store = string_table_->store();
        if (value.index >= store.size()) {
            value.str = store[0].second.get();
        } else {
            value.str = store[value.index].second.get();
        }
    }

    if (value.type == Value::Type::FUNC) {
        if (value.index == 0) {
            value.f = func_state_.get();
        } else {
            value.f = func_state_->getAllChildren()[value.index-1];
        }
    }

    if (value.type == Value::Type::ARRAY) {
        std::vector<Array *> arrays = class_factory_->arrays();
        value.gc = reinterpret_cast<GCObject *>(arrays[value.index]);
    }

    if (value.type == Value::Type::CLASS_DESC) {
        std::vector<ClassDescriptor *> descs = class_factory_->descs();
        value.gc = reinterpret_cast<GCObject *>(descs[value.index]);
    }
}

const Value ExecState::Call(const std::string& func_name, const std::vector<Value>& args) {
    Value ret;
    do {
        int index = global()->IndexOf(func_name);
        long caller = -1;
        if (index >= 0) {
            **stack_->top() = *(global()->Find(index));
        }
        else {
            auto iter = global_variables_.find(func_name);
            if (iter == global_variables_.end()) {
                break;
            }
            caller = iter->second;
            if (caller < 0) {
                break;
            }
            **stack_->top() = *(stack_->base() + caller);
        }
        for (int i = 0; i < args.size(); ++i) {
            *(*stack_->top() + i + 1) = args[i];
        }
        CallFunction(*stack_->top(), args.size(), &ret);

    } while (0);
    
    return ret;
}
    
const Value ExecState::Call(FuncState *func_state, const std::vector<Value>& args) {
    Value ret;
    do {
        Value func(func_state);
        **stack_->top() = func;
        int index = 0;
        for (int i = 0; i < args.size(); i++) {
            *(*stack_->top() + i + 1 + index) = args[i];
        }
        CallFunction(*stack_->top(), args.size(), &ret);
        
    } while (0);
    
    return ret;
}
    
const Value ExecState::Call(Value *func, const std::vector<Value>& args) {
    Value ret;
    do {
        // 首先检查函数是否属于堆栈函数
        long reg = func - stack_->base();
        if (reg >= VM_EXEC_STACK_SIZE) {
            throw VMExecError("call function out of stack");
            break;
        }
        for (int i = 0; i < args.size(); i++) {
            *(func + i + 1) = args[i];
        }
        CallFunction(func, args.size(), &ret);
        
    } while (0);
    
    return ret;
}
    
void ExecState::resetArguments(Value *func, size_t argc) {
    do {
        auto iter = global_variables_.find(JS_GLOBAL_ARGUMENTS);
        if (iter == global_variables_.end()) {
            break;
        }
        long caller = iter->second;
        if (caller < 0) {
            break;
        }
        Value *arguments = stack_->base() + caller;
        if (!IsArray(arguments)) {
            break;
        }
        ClearArray(ValueTo<Array>(arguments));
        for (int i = 0; i < argc; i++) {
            PushArray(ValueTo<Array>(arguments), *(func + i + 1));
        }
        
    } while (0);
}
    
void ExecState::Register(const std::string& name, CFunction func) {
    Value funcVal;
    funcVal.type = Value::Type::CFUNC;
    funcVal.cf = reinterpret_cast<void *>(func);
    Register(name, funcVal);
}
    
void ExecState::Register(const std::string& name, Value value) {
    global()->Add(name, value);
    global()->incrementRegisterSize();
}

void ExecState::CallFunction(Value *func, size_t argc, Value *ret) {
    if (func->type == Value::Type::CFUNC) {
        *stack_->top() = func + argc;
        Frame frame;
        frame.reg = func;
        frames_.push_back(frame);
        auto result = reinterpret_cast<CFunction>(func->cf)(this);
        if (ret != nullptr && !IsNil(&result)) {
            *ret = result;
        }
        stack_->reset();
        frames_.pop_back();
    }
    else {
        if (argc < func->f->argc()) {
            size_t size = (func->f->argc() - argc) * sizeof(Value);
            memset(func + argc + 1, 0, size);
        }
        *stack_->top() = func + argc;
        Frame frame;
        frame.func = func;
        frame.reg = func;
        frame.pc = &(*func->f->instructions().begin());
        frame.end = &(*func->f->instructions().end());
        frames_.push_back(frame);
        resetArguments(func, argc);
        vm_->RunFrame(this, frame, ret);
        stack_->reset();
        frames_.pop_back();
    }
}
    
ValueRef* ExecState::AddRef(FuncState *func_state, long register_id) {
    ValueRef *ref = nullptr;
    for (int i = 0; i < refs_.size(); i++) {
        ValueRef *ref_cur = refs_[i];
        if (ref_cur->func_state_ == func_state && ref_cur->register_id_ == register_id) {
            ref = ref_cur;
            break;
        }
    }
    if (!ref) {
        ref = new ValueRef(func_state, register_id);
        refs_.push_back(ref);
    }
    return ref;
}
    
ValueRef *ExecState::FindRef(int index) {
    ValueRef *ref = nullptr;
    for (int i = 0; i < refs_.size(); i++) {
        ValueRef *ref_cur = refs_[i];
        if (ref_cur->ref_id_ == index) {
            ref = ref_cur;
            break;
        }
    }
    return ref;
}

size_t ExecState::GetArgumentCount() {
  return *stack_->top() - frames_.back().reg;
}

Value* ExecState::GetArgument(int index) {
  return frames_.back().reg + index + 1;
}
    
}  // namespace data_render
}  // namespace core
}  // namespace weex

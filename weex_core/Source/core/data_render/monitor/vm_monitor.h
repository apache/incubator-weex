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
//
// Created by chad on 2018/8/2.
//

#ifndef PROJECT_VM_MONITOR_H
#define PROJECT_VM_MONITOR_H

#include <string>
#include <chrono>
#include <unordered_map>
#include <vector>
#include "base/LogDefines.h"

using namespace std::chrono;

typedef steady_clock::time_point tp;

static std::string OP_NAME[] = {"OP_MOVE",
                                "OP_LOADK",
                                "OP_LOADNULL",

                                "OP_GETGLOBAL",
                                "OP_GETFUNC",

                                "OP_GETCLASS",
                                "OP_GETCLASSVAR",
                                "OP_SETCLASSVAR",
                                "OP_GETSUPER",

                                "OP_SETVALUE",
                                "OP_GETVALUE",
                                "OP_SETTABLE",

                                "OP_GETTABLE",
                                "OP_SETARRAY",
                                "OP_GETARRAY",
                                "OP_ADD",

                                "OP_SUB",
                                "OP_MUL",
                                "OP_MOD",
                                "OP_POW",
                                "OP_DIV",

                                "OP_IDIV",
                                "OP_BAND",
                                "OP_BOR",
                                "OP_BXOR",
                                "OP_SHL",

                                "OP_SHR",
                                "OP_UNM",
                                "OP_BNOT",

                                "OP_JMP",
                                "OP_GOTO",
                                "OP_EQ",
                                "OP_LT",
                                "OP_CALL",
                                "OP_RETURN0",
                                "OP_RETURN1",
                                "OP_INVALID",
                                "OP_PRE_INCR",
                                "OP_PRE_DECR"
};

inline tp now() {
  return steady_clock::now();
}

inline int64_t getTimeMs(tp s, tp e) {
  return duration_cast<nanoseconds>(e - s).count();
}

typedef struct OpCost {
  std::string op;
  int op_index = -1;
  tp s;
  tp e;
  int64_t total = 0;
  float total_ms = 0;
  int64_t max = 0;
  float max_ms = 0;
  int64_t min = 0;
  int count = 0;

  inline int64_t getCost() {
    if (cost < 0) {
      cost = getTimeMs(s, e);
    }
    return cost;
  }

  inline float getAvg() {
    if (avg < 0) {
      avg = total / (float) count;
    }
    return avg;
  }

 private:
  int64_t cost = -1;
  float avg = -1;
}OpCost;

typedef struct TimeCost {
  tp fc_s;
  tp fc_e;

  int64_t total;
  float total_ms;

  std::vector<OpCost> op_cost;
  std::unordered_map<std::string, OpCost> ret;

  std::string tag = "Execute";

  TimeCost(const char* tag) : tag(tag) {
    fc_s = now();
  }

  TimeCost() {
    fc_s = now();
  }

  ~TimeCost() {
    fc_e = now();
    process();
    print();
  }

  inline void op_start(std::string op) {
    last.op = op;
    last.s = now();
  }

  inline void op_start(int index) {
    last.op_index = index;
    last.s = now();
  }

  inline void op_end() {
    last.e = now();
    op_cost.push_back(last);
  }

 private:

  inline void process() {

    total = getTimeMs(fc_s, fc_e);
    total_ms = total / (float) 1000000;

    for (OpCost oc : op_cost) {

      oc.count = 1;
      oc.total = oc.getCost();
      oc.total_ms = oc.total / (float) 1000000;
      oc.max = oc.getCost();
      oc.min = oc.getCost();
      oc.max_ms = oc.max / (float) 1000000;

      if (oc.op_index >= 0) {
        oc.op = OP_NAME[oc.op_index];
      }

      auto it = ret.find(oc.op);
      if (it != ret.end()) {

        it->second.count++;
        it->second.total += oc.total;
        it->second.total_ms = it->second.total / (float) 1000000;

        if (it->second.max < oc.max) {
          it->second.max = oc.max;
          it->second.max_ms = oc.max / (float) 1000000;
        }
        if (it->second.min > oc.min) {
          it->second.min = oc.min;
        }
      } else {
        ret.insert(std::make_pair(oc.op, oc));
      }
    }
  }

  inline void print() {

    LOGE("\n%s Execute cost %lld ns(%0.2f ms)\n", tag.c_str(), total, total_ms);

    if (op_cost.size() == 0) {
      return;
    }

    LOGE("There are %lu OP_CODEs. Each OP_CODE Info:\n", op_cost.size());

    int i = 0;
    for (auto& it : ret) {
      LOGE(
          "0x%02d:OP[%2d:%-12s] total:%lldns(%.2fms), count:%d, max:%lldns(%.2fms), min:%lldns, avg:%.2fns \n",
          ++i, it.second.op_index, it.second.op.c_str(),
          it.second.total, it.second.total_ms, it.second.count, it.second.max,
          it.second.max_ms, it.second.min, it.second.getAvg());
    }
  }

 private:
  OpCost last;
}TimeCost;

#endif //PROJECT_VM_MONITOR_H

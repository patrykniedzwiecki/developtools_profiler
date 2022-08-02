/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef RPC_WASM_FUNC_H
#define RPC_WASM_FUNC_H

#include <cstdio>
#include <emscripten.h>
#include "rpc_server.h"

namespace SysTuning {
namespace TraceStreamer {
extern "C" {
int TraceStreamerParseData(const uint8_t* data, int dataLen);
int TraceStreamerParseDataEx(int dataLen);
int TraceStreamerParseDataOver();
int TraceStreamerSqlOperate(const uint8_t* sql, int sqlLen);
int TraceStreamerSqlOperateEx(int sqlLen);
int TraceStreamerReset();
int TraceStreamerSqlQuery(const uint8_t* sql, int sqlLen, uint8_t* out, int outLen);

int TraceStreamerSqlQueryEx(int sqlLen);
int TraceStreamerCancel();
} // extern "C"
} // namespace TraceStreamer
} // namespace SysTuning

#endif // RPC_WASM_FUNC_H

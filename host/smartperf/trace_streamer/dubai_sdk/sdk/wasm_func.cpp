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

#include "wasm_func.h"
#include <deque>
#include "meta.h"
#include "sdk_data_parser.h"
#include "sdk_plugin_data_parser.h"
#include "table_base.h"
#include "trace_stdtype.h"
#include "ts_sdk_api.h"

namespace SysTuning {
namespace TraceStreamer {
RpcServer g_wasmTraceStreamer;

extern "C" {
using QueryResultCallbackFunction = void (*)(const char* data, uint32_t len, int finish, int isConfig);
using TraceRangeCallbackFunction = void (*)(const char* data, uint32_t len);
QueryResultCallbackFunction g_reply;
TraceRangeCallbackFunction g_traceRange;
uint8_t* g_reqBuf;
uint32_t g_reqBufferSize;
uint8_t* g_traceRangeBuf;
uint32_t g_traceRangeSize;

void QueryResultCallback(const std::string& jsonResult, int finish, int isConfig)
{
    g_reply(jsonResult.data(), jsonResult.size(), finish, isConfig);
}
void TraceRangeCallback(const std::string& jsonResult)
{
    g_traceRange(jsonResult.data(), jsonResult.size());
}
EMSCRIPTEN_KEEPALIVE uint8_t* Init(QueryResultCallbackFunction queryResultCallbackFunction, uint32_t reqBufferSize)
{
    SetRpcServer(&g_wasmTraceStreamer);
    sdk_plugin_init_table_name();
    g_wasmTraceStreamer.ts_->sdkDataParser_->CreateTableByJson();
    g_reply = queryResultCallbackFunction;
    g_reqBuf = new uint8_t[reqBufferSize];
    g_reqBufferSize = reqBufferSize;
    return g_reqBuf;
}

// Get PluginName
EMSCRIPTEN_KEEPALIVE int TraceStreamer_In_PluginName(const uint8_t* pluginName, int len)
{
    g_wasmTraceStreamer.ts_->sdkDataParser_->GetPluginName(pluginName, len);
    return 0;
}

EMSCRIPTEN_KEEPALIVE uint8_t* InitTraceRange(TraceRangeCallbackFunction traceRangeCallbackFunction,
                                             uint32_t reqBufferSize)
{
    g_traceRange = traceRangeCallbackFunction;
    g_traceRangeBuf = new uint8_t[reqBufferSize];
    g_traceRangeSize = reqBufferSize;
    return g_traceRangeBuf;
}

// The whole file is parsed, and the third party is notified by JS
EMSCRIPTEN_KEEPALIVE int TraceStreamer_In_ParseDataOver()
{
    MetaData* metaData = g_wasmTraceStreamer.ts_->GetMetaData();
    metaData->InitMetaData();
    metaData->SetParserToolVersion(SDK_VERSION);
    metaData->SetParserToolPublishDateTime(SDK_PUBLISHVERSION);
    g_wasmTraceStreamer.ts_->sdkDataParser_->ParseDataOver(&TraceRangeCallback);
    return 0;
}

// Get Json configuration interface
EMSCRIPTEN_KEEPALIVE int TraceStreamer_In_JsonConfig()
{
    g_wasmTraceStreamer.ts_->sdkDataParser_->GetJsonConfig(&QueryResultCallback);
    return 0;
}

EMSCRIPTEN_KEEPALIVE int TraceStreamerSqlOperate(const uint8_t* sql, int sqlLen)
{
    if (g_wasmTraceStreamer.SqlOperate(sql, sqlLen, nullptr)) {
        return 0;
    }
    return -1;
}

// JS calls third-party parsing interface
EMSCRIPTEN_KEEPALIVE int ParserData(int len, int componentId)
{
    TS_LOGI("wasm ParserData, len = %u", len);
    g_wasmTraceStreamer.ts_->sdkDataParser_->ParserData(g_reqBuf, len, componentId);
    return 0;
}

// return the length of result, -1 while failed
EMSCRIPTEN_KEEPALIVE int TraceStreamerSqlQuery(const uint8_t* sql, int sqlLen, uint8_t* out, int outLen)
{
    return g_wasmTraceStreamer.WasmSqlQuery(sql, sqlLen, out, outLen);
}
// return the length of result, -1 while failed
EMSCRIPTEN_KEEPALIVE int TraceStreamerSqlQueryEx(int sqlLen)
{
    return g_wasmTraceStreamer.WasmSqlQueryWithCallback(g_reqBuf, sqlLen, &QueryResultCallback);
}

} // extern "C"
} // namespace TraceStreamer
} // namespace SysTuning
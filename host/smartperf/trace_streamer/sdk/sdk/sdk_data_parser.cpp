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

#include "sdk_data_parser.h"
#include <cstdint>
#include <cstring>
#include <functional>
#include <sstream>
#include "clock_filter.h"
#include "gpu_counter_object_table.h"
#include "gpu_counter_table.h"
#include "json.hpp"
#include "log.h"
#include "meta.h"
#include "meta_table.h"
#include "sdk_plugin_data_parser.h"
#include "slice_object_table.h"
#include "slice_table.h"
#include "ts_common.h"
#include "ts_sdk_api.h"


namespace SysTuning {
namespace TraceStreamer {
SDKDataParser::SDKDataParser(TraceDataCache* dataCache, const TraceStreamerFilters* ctx)
    : HtracePluginTimeParser(dataCache, ctx)
{
}

int SDKDataParser::GetPluginName(const uint8_t* data, int len)
{
    std::string pluginName(reinterpret_cast<const char*>(data), len);
    pluginName.replace(pluginName.find("-"), 1, "_");
    counterTableName_ = pluginName + "_" + "counter_table";
    counterObjectTableName_ = pluginName + "_" + "counterobj_table";
    sliceTableName_ = pluginName + "_" + "slice_table";
    sliceObjectName_ = pluginName + "_" + "sliceobj_table";
    return 0;
}
int SDKDataParser::ParseDataOver(TraceRangeCallbackFunction traceRangeCallbackFunction)
{
    traceDataCache_->MixTraceTime(GetPluginStartTime(), GetPluginEndTime());
    std::string traceRangeStr =
        std::to_string(traceDataCache_->traceStartTime_) + ";" + std::to_string(traceDataCache_->traceEndTime_) + ";";
    traceRangeCallbackFunction(traceRangeStr);
    return 0;
}

int SDKDataParser::GetJsonConfig(QueryResultCallbackFunction queryResultCallbackFunction)
{
    queryResultCallbackFunction(jsonConfig_, 1, 1);
    return 0;
}

int SDKDataParser::ParserData(const uint8_t* data, int len, int componentId)
{
    if (componentId == DATA_TYPE_CLOCK) {
        ParserClock(data, len);
        return 0;
    }
    sdk_plugin_data_parser(data, len);
    return 0;
}

int SDKDataParser::ParserClock(const uint8_t* data, int len)
{
    return streamFilters_->clockFilter_->InitSnapShotTimeRange(data, len);
}

int SDKDataParser::SetTableName(const char* counterTableName,
                                              const char* counterObjectTableName,
                                              const char* sliceTableName,
                                              const char* sliceObjectName)
{
    if (!g_isUseExternalModify) {
        counterTableName_ = counterTableName;
        counterObjectTableName_ = counterObjectTableName;
        sliceTableName_ = sliceTableName;
        sliceObjectName_ = sliceObjectName;
    }
    UpdateJson();
    return 0;
}

int SDKDataParser::UpdateJson()
{
    using json = nlohmann::json;
    std::stringstream stream;
    json jMessage = json::parse(jsonConfig_);
    if (jMessage.is_discarded()) {
        return -1;
    }

    jMessage["tableConfig"]["showType"][0]["tableName"] = counterTableName_;
    jMessage["tableConfig"]["showType"][0]["inner"]["tableName"] = counterObjectTableName_;
    jMessage["tableConfig"]["showType"][1]["tableName"] = sliceTableName_;
    jMessage["tableConfig"]["showType"][1]["inner"]["tableName"] = sliceObjectName_;

    jsonConfig_ = jMessage.dump();
    return 0;
}

// 创建对应的表
int SDKDataParser::CreateTableByJson()
{
#ifdef USE_VTABLE
    TableBase::TableDeclare<MetaTable>(*(traceDataCache_->db_), traceDataCache_, "meta");
#else
    TableBase::TableDeclare<MetaTable>(*(traceDataCache_->db_), traceDataCache_, "_meta");
#endif
    // 创建对应的表
    CreateCounterObjectTable(counterObjectTableName_);
    CreateCounterTable(counterTableName_);
    CreateSliceObjectTable(sliceObjectName_);
    CreateSliceTable(sliceTableName_);
    return 0;
}

// 根据Json配置创建couter object表
int SDKDataParser::CreateCounterObjectTable(std::string tableName)
{
#ifdef USE_VTABLE
    TableBase::TableDeclare<GpuCounterObjectTable>(*(traceDataCache_->db_), traceDataCache_, tableName);
#else
    TableBase::TableDeclare<GpuCounterObjectTable>(*(traceDataCache_->db_), traceDataCache_, "_" + tableName);
#endif
    return 0;
}

// 根据Json配置创建couter表
int SDKDataParser::CreateCounterTable(std::string tableName)
{
#ifdef USE_VTABLE
    TableBase::TableDeclare<GpuCounterTable>(*(traceDataCache_->db_), traceDataCache_, tableName);
#else
    TableBase::TableDeclare<GpuCounterTable>(*(traceDataCache_->db_), traceDataCache_, "_" + tableName);
#endif
    return 0;
}

// 根据Json配置创建slice object表
int SDKDataParser::CreateSliceObjectTable(std::string tableName)
{
#ifdef USE_VTABLE
    TableBase::TableDeclare<SliceObjectTable>(*(traceDataCache_->db_), traceDataCache_, tableName);
#else
    TableBase::TableDeclare<SliceObjectTable>(*(traceDataCache_->db_), traceDataCache_, "_" + tableName);
#endif
    return 0;
}

// 根据Json配置创建slice表
int SDKDataParser::CreateSliceTable(std::string tableName)
{
#ifdef USE_VTABLE
    TableBase::TableDeclare<SliceTable>(*(traceDataCache_->db_), traceDataCache_, tableName);
#else
    TableBase::TableDeclare<SliceTable>(*(traceDataCache_->db_), traceDataCache_, "_" + tableName);
#endif
    return 0;
}

// Counter业务
int SDKDataParser::AppendCounterObject(int counterId, const char* columnName)
{
    traceDataCache_->GetGpuCounterObjectData()->AppendNewData(counterId, columnName);
    return 0;
}

int SDKDataParser::AppendCounter(int counterId, uint64_t ts, int value)
{
    auto newTs = streamFilters_->clockFilter_->ToPrimaryTraceTime(TS_CLOCK_REALTIME, ts);
    UpdatePluginTimeRange(TS_CLOCK_BOOTTIME, ts, newTs);
    traceDataCache_->GetGpuCounterData()->AppendNewData(newTs, counterId, value);
    return 0;
}

// Slice业务
int SDKDataParser::AppendSliceObject(int sliceId, const char* columnName)
{
    traceDataCache_->GetSliceObjectData()->AppendNewData(sliceId, columnName);
    return 0;
}

int SDKDataParser::AppendSlice(int sliceId, uint64_t ts, uint64_t endTs, int value)
{
    auto newTs = streamFilters_->clockFilter_->ToPrimaryTraceTime(TS_CLOCK_REALTIME, ts);
    auto newEndTs = streamFilters_->clockFilter_->ToPrimaryTraceTime(TS_CLOCK_REALTIME, endTs);
    UpdatePluginTimeRange(TS_CLOCK_BOOTTIME, ts, newTs);
    UpdatePluginTimeRange(TS_CLOCK_BOOTTIME, endTs, newEndTs);
    traceDataCache_->GetSliceTableData()->AppendNewData(sliceId, newTs, newEndTs, value);
    return 0;
}
} // namespace TraceStreamer
} // namespace SysTuning

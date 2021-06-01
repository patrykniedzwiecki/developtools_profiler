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

#ifndef TRACE_DATA_CACHE_WRITER_H
#define TRACE_DATA_CACHE_WRITER_H

#include "trace_data_cache_reader.h"

namespace SysTuning {
namespace TraceStreamer {
using namespace TraceStdtype;
class TraceDataCacheWriter : public TraceDataCacheReader {
public:
    TraceDataCacheWriter() = default;
    TraceDataCacheWriter(const TraceDataCacheWriter&) = delete;
    TraceDataCacheWriter& operator=(const TraceDataCacheWriter&) = delete;
    ~TraceDataCacheWriter() override;
public:
    InternalPid GetProcessInternalPid(uint32_t pid);
    Process* GetProcessData(InternalPid internalPid);
    InternalTid GetInternalThread(uint32_t tid);
    Thread* GetThreadData(InternalTid internalTid);
    void UpdateBoundTime(uint64_t timestamp);
    Slices* GetSlicesData();
    InternalSlices* GetInternalSlicesData();
    Filter* GetFilterData();
    Raw* GetRawData();
    Counter* GetCounterData();
    ThreadState* GetThreadStateData();
    SchedSlice* GetSchedSliceData();
    CpuCounter* GetCpuCountersData();
    ThreadCounterFilter* GetThreadCounterFilterData();
    ThreadCounterFilter* GetThreadFilterData();
    Instants* GetInstantsData();
    ProcessCounterFilter* GetProcessFilterData();
    ProcessCounterFilter* GetProcessCounterFilterData();
};
} // namespace TraceStreamer
} // namespace trace_data_cache
#endif

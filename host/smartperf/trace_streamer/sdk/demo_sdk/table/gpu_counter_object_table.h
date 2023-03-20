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

#ifndef GPU_COUNTER_OBJECT_TABLE_H
#define GPU_COUNTER_OBJECT_TABLE_H

#include "table_base.h"
#include "trace_data_cache.h"

namespace SysTuning {
namespace TraceStreamer {
class GpuCounterObjectTable : public TableBase {
public:
    explicit GpuCounterObjectTable(const TraceDataCache* storage);
    ~GpuCounterObjectTable() override;
    std::unique_ptr<TableBase::Cursor> CreateCursor() override;

private:
    void EstimateFilterCost(FilterConstraints& fc, EstimatedIndexInfo& ei) override {};
    // bool CanFilterId(const char op, size_t& rowCount);
    // void FilterByConstraint(FilterConstraints& fc, double& filterCost, size_t rowCount);
    class Cursor : public TableBase::Cursor {
    public:
        explicit Cursor(const TraceDataCache* dataCache, TableBase* table);
        ~Cursor() override;
        int Filter(const FilterConstraints& fc, sqlite3_value** argv) override
        {
            UNUSED(fc);
            UNUSED(argv);
            return 0;
        };
        // void FilterSorted(int col, unsigned char op, sqlite3_value* argv){};
        int Column(int column) const override;

    private:
        const GpuCounterObject& gpuCounterObjectDataObj_;
    };
};
} // namespace TraceStreamer
} // namespace SysTuning

#endif // GPU_COUNTER_OBJECT_TABLE_H
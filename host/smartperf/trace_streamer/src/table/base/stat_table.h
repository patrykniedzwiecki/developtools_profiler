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

#ifndef STAT_TABLE_H
#define STAT_TABLE_H

#include "table_base.h"
#include "trace_data_cache.h"

namespace SysTuning {
namespace TraceStreamer {
class StatTable : public TableBase {
public:
    explicit StatTable(const TraceDataCache* dataCache);
    ~StatTable() override;
    std::unique_ptr<TableBase::Cursor> CreateCursor() override;

private:
    void EstimateFilterCost(FilterConstraints& fc, EstimatedIndexInfo& ei) override {}

    class Cursor : public TableBase::Cursor {
    public:
        explicit Cursor(const TraceDataCache* dataCache, TableBase* table);
        ~Cursor() override;
        int32_t Filter(const FilterConstraints& fc, sqlite3_value** argv) override
        {
            UNUSED(fc);
            UNUSED(argv);
            return 0;
        }

        int32_t Column(int32_t column) const override;
    };
};
} // namespace TraceStreamer
} // namespace SysTuning
#endif // STAT_TABLE_H

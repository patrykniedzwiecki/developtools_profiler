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

#include "slice_table.h"
#include <cmath>

namespace SysTuning {
namespace TraceStreamer {
namespace {
enum Index { TS = 0, ENDTS = 1, VALUE = 2, SLICE_ID = 3 };
}
SliceTable::SliceTable(const TraceDataCache* dataCache) : TableBase(dataCache)
{
    tableColumn_.push_back(TableBase::ColumnInfo("start_ts", "INTEGER"));
    tableColumn_.push_back(TableBase::ColumnInfo("end_ts", "INTEGER"));
    tableColumn_.push_back(TableBase::ColumnInfo("value", "INTEGER"));
    tableColumn_.push_back(TableBase::ColumnInfo("slice_id", "INTEGER"));
    tablePriKey_.push_back("start_ts");
    tablePriKey_.push_back("slice_id");
}

SliceTable::~SliceTable() {}

// void SliceTable::EstimateFilterCost(FilterConstraints& fc, EstimatedIndexInfo& ei)
// {
//     constexpr double filterBaseCost = 1000.0; // set-up and tear-down
//     constexpr double indexCost = 2.0;
//     ei.estimatedCost = filterBaseCost;

//     auto rowCount = dataCache_->GetConstSliceData().Size();
//     if (rowCount == 0 || rowCount == 1) {
//         ei.estimatedRows = rowCount;
//         ei.estimatedCost += indexCost * rowCount;
//         return;
//     }

//     double filterCost = 0.0;
//     auto constraints = fc.GetConstraints();
//     if (constraints.empty()) { // scan all rows
//         filterCost = rowCount;
//     } else {
//         FilterByConstraint(fc, filterCost, rowCount);
//     }
//     ei.estimatedCost += filterCost;
//     ei.estimatedRows = rowCount;
//     ei.estimatedCost += rowCount * indexCost;

//     ei.isOrdered = true;
//     auto orderbys = fc.GetOrderBys();
//     for (auto i = 0; i < orderbys.size(); i++) {
//         switch (orderbys[i].iColumn) {
//             case SLICE_ID:
//                 break;
//             default: // other columns can be sorted by SQLite
//                 ei.isOrdered = false;
//                 break;
//         }
//     }
// }

// void SliceTable::FilterByConstraint(FilterConstraints& fc, double& filterCost, size_t rowCount)
// {
//     auto fcConstraints = fc.GetConstraints();
//     for (int i = 0; i < static_cast<int>(fcConstraints.size()); i++) {
//         if (rowCount <= 1) {
//             // only one row or nothing, needn't filter by constraint
//             filterCost += rowCount;
//             break;
//         }
//         const auto& c = fcConstraints[i];
//         switch (c.col) {
//             case SLICE_ID: {
//                 auto oldRowCount = rowCount;
//                 if (CanFilterSorted(c.op, rowCount)) {
//                     fc.UpdateConstraint(i, true);
//                     filterCost += log2(oldRowCount); // binary search
//                 } else {
//                     filterCost += oldRowCount;
//                 }
//                 break;
//             }
//             default:                    // other column
//                 filterCost += rowCount; // scan all rows
//                 break;
//         }
//     }
// }

// bool SliceTable::CanFilterSorted(const char op, size_t& rowCount)
// {
//     switch (op) {
//         case SQLITE_INDEX_CONSTRAINT_EQ:
//             rowCount = rowCount / log2(rowCount);
//             break;
//         case SQLITE_INDEX_CONSTRAINT_GT:
//         case SQLITE_INDEX_CONSTRAINT_GE:
//         case SQLITE_INDEX_CONSTRAINT_LE:
//         case SQLITE_INDEX_CONSTRAINT_LT:
//             rowCount = (rowCount >> 1);
//             break;
//         default:
//             return false;
//     }
//     return true;
// }

std::unique_ptr<TableBase::Cursor> SliceTable::CreateCursor()
{
    return std::make_unique<Cursor>(dataCache_, this);
}

SliceTable::Cursor::Cursor(const TraceDataCache* dataCache, TableBase* table)
    : TableBase::Cursor(dataCache, table, static_cast<uint32_t>(dataCache->GetConstSliceData().Size())),
      sliceDataObj_(dataCache->GetConstSliceData())
{
}

SliceTable::Cursor::~Cursor() {}

// int SliceTable::Cursor::Filter(const FilterConstraints& fc, sqlite3_value** argv)
// {
//     // reset indexMap_
//     indexMap_ = std::make_unique<IndexMap>(0, rowCount_);

//     if (rowCount_ <= 0) {
//         return SQLITE_OK;
//     }

//     auto& cs = fc.GetConstraints();
//     for (size_t i = 0; i < cs.size(); i++) {
//         const auto& c = cs[i];
//         switch (c.col) {
//             case SLICE_ID:
//                 FilterSorted(c.col, c.op, argv[i]);
//                 break;
//             default:
//                 break;
//         }
//     }

//     auto orderbys = fc.GetOrderBys();
//     for (auto i = orderbys.size(); i > 0;) {
//         i--;
//         switch (orderbys[i].iColumn) {
//             case SLICE_ID:
//                 indexMap_->SortBy(orderbys[i].desc);
//                 break;
//             default:
//                 break;
//         }
//     }

//     return SQLITE_OK;
// }

int SliceTable::Cursor::Column(int column) const
{
    switch (column) {
        case TS: {
            sqlite3_result_int64(context_, static_cast<int64_t>(sliceDataObj_.TimeStamp()[CurrentRow()]));
            break;
        }
        case ENDTS: {
            sqlite3_result_int64(context_, static_cast<int64_t>(sliceDataObj_.EndTs()[CurrentRow()]));
            break;
        }
        case VALUE: {
            sqlite3_result_int64(context_, static_cast<int64_t>(sliceDataObj_.Value()[CurrentRow()]));
            break;
        }
        case SLICE_ID: {
            sqlite3_result_int64(context_, static_cast<int64_t>(sliceDataObj_.SliceId()[CurrentRow()]));
            break;
        }
        default:
            TS_LOGF("Unregistered column : %d", column);
            break;
    }
    return SQLITE_OK;
}
} // namespace TraceStreamer
} // namespace SysTuning
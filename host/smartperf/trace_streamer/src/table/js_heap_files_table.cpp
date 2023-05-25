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

#include "js_heap_files_table.h"

namespace SysTuning {
namespace TraceStreamer {
namespace {
enum Index { ID = 0, FILE_NAME, START_TIME, END_TIME };
}
JsHeapFilesTable::JsHeapFilesTable(const TraceDataCache* dataCache) : TableBase(dataCache)
{
    tableColumn_.push_back(TableBase::ColumnInfo("id", "INTEGER"));
    tableColumn_.push_back(TableBase::ColumnInfo("file_name", "TEXT"));
    tableColumn_.push_back(TableBase::ColumnInfo("start_time", "INTEGER"));
    tableColumn_.push_back(TableBase::ColumnInfo("end_time", "INTEGER"));
    tablePriKey_.push_back("id");
}

JsHeapFilesTable::~JsHeapFilesTable() {}

std::unique_ptr<TableBase::Cursor> JsHeapFilesTable::CreateCursor()
{
    return std::make_unique<Cursor>(dataCache_, this);
}

JsHeapFilesTable::Cursor::Cursor(const TraceDataCache* dataCache, TableBase* table)
    : TableBase::Cursor(dataCache, table, static_cast<uint32_t>(dataCache->GetConstJsHeapFilesData().Size())),
      jsHeapFiles_(dataCache->GetConstJsHeapFilesData())
{
}

JsHeapFilesTable::Cursor::~Cursor() {}

int JsHeapFilesTable::Cursor::Column(int col) const
{
    switch (col) {
        case ID:
            sqlite3_result_int64(context_, static_cast<int64_t>(jsHeapFiles_.IDs()[CurrentRow()]));
            break;
        case FILE_NAME:
            sqlite3_result_text(context_, jsHeapFiles_.FilePaths()[CurrentRow()].c_str(), STR_DEFAULT_LEN, nullptr);
            break;
        case START_TIME:
            sqlite3_result_int64(context_, static_cast<int64_t>(jsHeapFiles_.IDs()[CurrentRow()]));
            break;
        case END_TIME:
            sqlite3_result_int64(context_, static_cast<int64_t>(jsHeapFiles_.IDs()[CurrentRow()]));
            break;
        default:
            TS_LOGF("Unregistered column : %d", col);
            break;
    }
    return SQLITE_OK;
}
} // namespace TraceStreamer
} // namespace SysTuning
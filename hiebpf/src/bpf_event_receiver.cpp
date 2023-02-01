/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
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

#include "bpf_event_receiver.h"

#include <chrono>

using namespace std::chrono_literals;
void BPFEventReceiver::DoWork()
{
    for (__u32 cnt = MAX_BUSY_LOOPS; cnt != 0; --cnt) {
        if (buf_->GetDataSize()) {
            break;
        }
    }
    if (buf_->GetDataSize() == 0) {
        std::unique_lock lk {mtx_};
        cond_.wait(lk, [this]()->bool{return this->stop_ || this->buf_->GetDataSize();});
        if (stop_) {
            return;
        }
    }

    while (buf_->GetDataSize() != 0) {
        __u32 tracer {0};
        if (buf_->Peek(&tracer) == 0) {
            switch (tracer) {
                case FSTRACE: {
                    ReceiveFSTraceEvent();
                    continue;
                }
                case PFTRACE: {
                    ReceivePFTraceEvent();
                    continue;
                }
                case BIOTRACE: {
                    ReceiveBIOTraceEvent();
                    continue;
                }
                case STRTRACE: {
                    ReceiveSTRTraceEvent();
                    continue;
                }
                case DLOPEN_TRACE: {
                    ReceiveDlopenTraceEvent();
                    continue;
                }
                default: {
                    DiscardEvent();
                    continue;
                }
            }
        } else {
            HHLOGE(true, "failed to peek the tracer type");
            return;
        }
    }
}

void BPFEventReceiver::ReceiveFSTraceEvent()
{
    struct fstrace_cmplt_event_t cmplt_event {};
    size_t dataSize = buf_->Get((char*)&cmplt_event, sizeof(cmplt_event));
    if (dataSize != sizeof(cmplt_event)) {
        HHLOGE(true,"imcomplete fstrace event data receiveed");
        return;
    }
    WriteEventMaps(cmplt_event.tgid);
    auto file = file_.lock();
    if (file == nullptr) {
        HHLOGE(true, "failed to receive fstrace event: hiebpf data file closed");
        return;
    }
    __u32 tlvItemSize = GetFSTraceTLVItemSize(cmplt_event);
    void *dest = file->Reserve(tlvItemSize);
    if (dest == nullptr) {
        HHLOGE(true, "failed to reserve space for fstrace event tlv item");
        return;
    }
    if (EncodeFSTraceEvent(&cmplt_event, dest, tlvItemSize) != 0) {
        HHLOGE(true, "failed to encode fstrace event");
        file->Discard(dest);
        return;
    }
    file->Submit(dest);
}

void BPFEventReceiver::ReceivePFTraceEvent()
{
    struct pftrace_cmplt_event_t cmplt_event {};
    buf_->Get((char*)&cmplt_event, sizeof(cmplt_event));
    WriteEventMaps(cmplt_event.tgid);
    auto file = file_.lock();
    if (file == nullptr) {
        HHLOGE(true, "failed to receive pftrace event: hiebpf data file closed");
        return;
    }
    __u32 tlvItemSize = GetPFTraceTLVItemSize(cmplt_event);
    void *dest = file->Reserve(tlvItemSize);
    if (dest == nullptr) {
        HHLOGE(true, "failed to reserve space for pftrace event tlv item");
        return;
    }
    if (EncodePFTraceEvent(&cmplt_event, dest, tlvItemSize) != 0) {
        HHLOGE(true, "failed to encode pftrace event");
        file->Discard(dest);
        return;
    }
    file->Submit(dest);
}

void BPFEventReceiver::ReceiveBIOTraceEvent()
{
    struct biotrace_cmplt_event_t cmplt_event {};
    buf_->Get((char*)&cmplt_event, sizeof(cmplt_event));
    WriteEventMaps(cmplt_event.start_event.pid);
    auto file = file_.lock();
    if (file == nullptr) {
        HHLOGE(true, "failed to receive biotrace event: hiebpf data file closed");
        return;
    }
    __u32 tlvItemSize = GetBIOTraceTLVItemSize(cmplt_event);
    void *dest = file->Reserve(tlvItemSize);
    if (dest == nullptr) {
        HHLOGE(true, "failed to reserve space for biotrace event tlv item");
        return;
    }
    if (EncodeBIOTraceEvent(&cmplt_event, dest, tlvItemSize) != 0) {
        HHLOGE(true, "failed to encode biotrace event");
        file->Discard(dest);
        return;
    }
    file->Submit(dest);
}

void BPFEventReceiver::ReceiveSTRTraceEvent()
{
    struct strtrace_cmplt_event_t cmplt_event {};
    buf_->Get((char*)&cmplt_event, sizeof(cmplt_event));
    if (cmplt_event.len == 0) {
        return;
    }
    auto file = file_.lock();
    if (file == nullptr) {
        HHLOGE(true, "failed to receive strtrace event: hiebpf data file closed");
        return;
    }
    __u32 tlvItemSize = GetSTRTraceTLVItemSize(cmplt_event);
    void *dest = file->Reserve(tlvItemSize);
    if (dest == nullptr) {
        HHLOGE(true, "failed to reserve space for strtrace event tlv item");
        return;
    }
    if (EncodeSTRTraceEvent(&cmplt_event, dest, tlvItemSize) != 0) {
        HHLOGE(true, "failed to encode strtrace event");
        file->Discard(dest);
        return;
    }
    file->Submit(dest);
}

void BPFEventReceiver::ReceiveDlopenTraceEvent()
{
    struct dlopen_trace_start_event_t cmplt_event {};
    buf_->Get((char*)&cmplt_event, sizeof(cmplt_event));
    mapsInfo_.RemovePid(cmplt_event.tgid);
}

void BPFEventReceiver::WriteEventMaps(uint32_t pid)
{
    if (mapsInfo_.IsCached(pid)) {
        return;
    }
    using OHOS::Developtools::Hiebpf::MapsInfo;
    std::vector<MapsInfo::MapsItem> mapsItems;
    mapsInfo_.GetMaps(pid, mapsItems);
    auto file = file_.lock();
    for (auto &item : mapsItems) {
        size_t size = sizeof(FixedMapTLVItem) + item.fileName_.size() + 1;
        void *dest = file->Reserve(size);
        if (dest == nullptr) {
            HHLOGE(true, "failed to reserve space for strtrace event tlv item");
            continue;
        }
        FixedMapTLVItem *mapItem = (FixedMapTLVItem *)dest;
        mapItem->type = MAPSTRACE;
        mapItem->len = size - sizeof(uint32_t) * 2;
        mapItem->start = item.start_;
        mapItem->end = item.end_;
        mapItem->offset = item.offset_;
        mapItem->pid = item.pid_;
        mapItem->fileNameLen = item.fileName_.size() + 1;
        char* tmp = (char*)dest;
        char* fileName = tmp + sizeof(FixedMapTLVItem);
        (void)memcpy_s(fileName, mapItem->fileNameLen, item.fileName_.c_str(), mapItem->fileNameLen);
        file->Submit(dest);
        WriteSymbolInfo(item.fileName_);
    }
    mapsInfo_.CachePid(pid);
}

void BPFEventReceiver::WriteSymbolInfo(const std::string &fileName)
{
    if (elfSymbolInfo_.IsCached(fileName)) {
        return;
    }
    using OHOS::Developtools::Hiebpf::ElfSymbolInfo;
    ElfSymbolInfo::ElfSymbolTable symbolInfo;
    if (elfSymbolInfo_.GetSymbolTable(fileName, symbolInfo)) {
        auto file = file_.lock();
        size_t size = sizeof(FixedSymbolTLVItem) + symbolInfo.strTable_.size() + symbolInfo.symTable_.size()
            + symbolInfo.fileName_.size() + 1;
        void *dest = file->Reserve(size);
        if (dest == nullptr) {
            HHLOGE(true, "file reserve failed");
            return;
        }
        FixedSymbolTLVItem *sym = (FixedSymbolTLVItem *)dest;
        sym->type = SYMBOLTRACE;
        sym->len = size - sizeof(uint32_t) * 2;
        sym->textVaddr = symbolInfo.textVaddr_;
        sym->textOffset = symbolInfo.textOffset_;
        sym->strTabLen = symbolInfo.strTable_.size();
        sym->symTabLen = symbolInfo.symTable_.size();
        sym->fileNameLen = symbolInfo.fileName_.size() + 1;
        sym->symEntLen = symbolInfo.symEntSize_;
        char* tmp = (char*)dest;
        char* strTab = tmp + sizeof(FixedSymbolTLVItem);
        size -= sizeof(FixedSymbolTLVItem);
        if (memcpy_s(strTab, size, symbolInfo.strTable_.data(), sym->strTabLen) != EOK) {
            HHLOGE(true, "memcpy_s failed");
            return;
        }
        size -= sym->strTabLen;
        char* symTab = tmp + sizeof(FixedSymbolTLVItem) + symbolInfo.strTable_.size();
        if (memcpy_s(symTab, size, symbolInfo.symTable_.data(), sym->symTabLen) != EOK) {
            HHLOGE(true, "memcpy_s failed");
            return;
        }
        size -= sym->symTabLen;
        char* fileName = tmp + sizeof(FixedSymbolTLVItem) + symbolInfo.strTable_.size() + symbolInfo.symTable_.size();
        if (memcpy_s(fileName, size, symbolInfo.fileName_.c_str(), sym->fileNameLen) != EOK) {
            HHLOGE(true, "memcpy_s failed");
            return;
        }
        file->Submit(dest);
    }
    elfSymbolInfo_.CacheFileName(fileName);
}

void BPFEventReceiver::ReverseStr(char* left, char* right)
{
    while (left < right) {
        char tmp = 0;
        tmp = *left;
        *left = *right;
        *right = tmp;
        left++;
        right--;
    }
}

void BPFEventReceiver::DiscardEvent()
{
    struct strtrace_cmplt_event_t cmplt_event {};
    buf_->Get((char*)&cmplt_event, sizeof(cmplt_event));
    HHLOGE(true, "unkown tracer type = %u, event data will be discarded", cmplt_event.tracer);
}

int BPFEventReceiver::EncodeFSTraceEvent(
    const struct fstrace_cmplt_event_t *cmplt_event,
    void* tlvItem,
    const size_t itemLen)
{
    struct FixedFSTraceTLVItem *item = (struct FixedFSTraceTLVItem *) tlvItem;
    item->tracer_ = FSTRACE;
    item->itemLen_ = itemLen - sizeof(uint32_t) * 2;
    item->pid_ = cmplt_event->tgid;
    item->tid_ = cmplt_event->pid;
    if (strncpy_s(item->tracerName_, MAX_TRACER_NAME_LEN,
                  gTracerTable[FSTRACE].c_str(), gTracerTable[FSTRACE].size()) != EOK) {
        HHLOGE(true, "failed to copy fstrace tracer name");
        return -1;
    }
    item->stime_ = cmplt_event->start_event.stime;
    item->ctime_ = cmplt_event->ctime;
    item->retval_ = cmplt_event->retval;
    item->nips_ = static_cast<uint16_t>(cmplt_event->nips);
    item->type_ = static_cast<uint16_t>(cmplt_event->start_event.type);
    if (strncpy_s(item->typeName_, MAX_TYPE_NAME_LEN,
                  gFSTraceTypeTable[item->type_].c_str(),
                  gFSTraceTypeTable[item->type_].size() != EOK)) {
        HHLOGE(true, "failed to copy fstrace type name");
        return -1;
    }
    if (ConvertFSTraceArgsToArray(item->args_, &cmplt_event->start_event) != 0) {
        HHLOGE(true, "failed to convert fstrace event args");
        return -1;
    }
    (void)memcpy_s(item->comm_, MAX_COMM_LEN, cmplt_event->comm, MAX_COMM_LEN);
    if (cmplt_event->nips and cmplt_event->ustack_id >= 0) {
        char* tmp = (char*) tlvItem;
        __u64 *ips = (__u64 *) (tmp + sizeof(struct FixedFSTraceTLVItem));
        if (ReadCallChain(cmplt_event->nips, cmplt_event->ustack_id, ips) != 0) {
            return -1;
        }
    }
    return 0;
}

int BPFEventReceiver::EncodePFTraceEvent(
    const struct pftrace_cmplt_event_t *cmplt_event,
    void* tlvItem,
    const size_t itemLen)
{
    struct FixedPFTraceTLVItem *item = (struct FixedPFTraceTLVItem *) tlvItem;
    item->tracer_ = PFTRACE;
    item->itemLen_ = itemLen - sizeof(uint32_t) * 2;
    item->pid_ = cmplt_event->tgid;
    item->tid_ = cmplt_event->pid;
    if (strncpy_s(item->tracerName_, MAX_TRACER_NAME_LEN,
                  gTracerTable[PFTRACE].c_str(), gTracerTable[PFTRACE].size() != EOK)) {
        HHLOGE(true, "failed to copy pftrace tracer name");
        return -1;
    }
    item->stime_ = cmplt_event->start_event.stime;
    item->ctime_ = cmplt_event->ctime;
    item->addr_ = cmplt_event->start_event.addr;
    item->size_ = cmplt_event->size;
    item->nips_ = static_cast<uint16_t>(cmplt_event->nips);
    item->type_ = static_cast<uint16_t>(cmplt_event->start_event.type);
    if (strncpy_s(item->typeName_,
                  MAX_TYPE_NAME_LEN,
                  gPFTraceTypeTable[item->type_].c_str(),
                  gPFTraceTypeTable[item->type_].size()) != EOK) {
        HHLOGE(true, "failed to copy pftrace type name");
        return -1;
    }
    (void)memcpy_s(item->comm_, MAX_COMM_LEN, cmplt_event->comm, MAX_COMM_LEN);
    if (cmplt_event->nips and cmplt_event->ustack_id >= 0) {
        char* tmp = (char*) tlvItem;
        __u64 *ips = (__u64 *) (tmp + sizeof(struct FixedPFTraceTLVItem));
        if (ReadCallChain(cmplt_event->nips, cmplt_event->ustack_id, ips) != 0) {
            return -1;
        }
    }
    return 0;
}

int BPFEventReceiver::EncodeBIOTraceEvent(
    const struct biotrace_cmplt_event_t *cmplt_event,
    void* tlvItem,
    const size_t itemLen)
{
    struct FixedBIOTraceTLVItem *item = (struct FixedBIOTraceTLVItem *) tlvItem;
    item->tracer_ = BIOTRACE;
    item->itemLen_ = itemLen - sizeof(uint32_t) * 2;
    item->pid_ = cmplt_event->start_event.tgid;
    item->tid_ = cmplt_event->start_event.pid;
    (void)memcpy_s(item->comm_, MAX_COMM_LEN, cmplt_event->start_event.comm, MAX_COMM_LEN);
    item->stime_ = cmplt_event->start_event.stime;
    item->ctime_ = cmplt_event->ctime;
    item->prio_ = cmplt_event->prio;
    item->size_ = cmplt_event->start_event.size;
    item->blkcnt_ = cmplt_event->blkcnt;
    item->nips_ = cmplt_event->nips;
    item->type_ = cmplt_event->start_event.type;
    if (strncpy_s(item->typeName_,
                  MAX_TYPE_NAME_LEN,
                  gBIOTraceTypeTable[item->type_].c_str(),
                  gBIOTraceTypeTable[item->type_].size()) != EOK) {
        HHLOGE(true, "failed to copy BIOstrace type name");
        return -1;
    }
    if (cmplt_event->nips and cmplt_event->start_event.ustack_id >= 0) {
        char* tmp = (char*) tlvItem;
        __u64 *ips = (__u64 *) (tmp + sizeof(struct FixedBIOTraceTLVItem));
        if (ReadCallChain(cmplt_event->nips, cmplt_event->start_event.ustack_id, ips) != 0) {
            return -1;
        }
    }
    return 0;
}

int BPFEventReceiver::EncodeSTRTraceEvent(
    const struct strtrace_cmplt_event_t *cmplt_event,
    void* tlvItem,
    const size_t itemLen)
{
    struct FixedSTRTraceTLVItem *item = (struct FixedSTRTraceTLVItem *) tlvItem;
    item->tracer_ = STRTRACE;
    item->itemLen_ = itemLen - sizeof(uint32_t) * 2;
    item->pid_ = cmplt_event->tgid;
    item->tid_ = cmplt_event->pid;
    item->stime_ = cmplt_event->start_event.stime;
    item->srcTracer_ = cmplt_event->start_event.stracer;
    item->srcType_ = cmplt_event->start_event.type;
    item->strLen_ = cmplt_event->len;
    if (item->strLen_ and item->strLen_ <= MAX_FILENAME_LEN) {
        char *filename = (char*) tlvItem;
        filename += sizeof(struct FixedSTRTraceTLVItem);
        if (strncpy_s(filename, MAX_FILENAME_LEN, cmplt_event->filename, item->strLen_) != EOK) {
            HHLOGE(true, "failed to copy cmplt_event file name");
            return -1;
        }
        if (item->srcTracer_ == BIOTRACE || (item->srcTracer_ == FSTRACE && item->srcType_ == SYS_CLOSE)) {
            ReverseStr(filename, filename + item->strLen_ - 2);
            char* start = filename;
            while (*start != '\0') {
                char* end = start;
                while (*end != '/' && *end != '\0') {
                    end++;
                }
                ReverseStr(start, end - 1);
                if (*end != '\0') {
                    start = end + 1;
                }
                else {
                    start = end;
                }
            }
        }
    }
    return 0;
}

void BPFEventReceiver::GetUStackMapFd()
{
    const __u32 ustack_map_index {FSTRACE_STACK_TRACE_INDEX};
    __u32 ustack_map_id;
    int err = bpf_map_lookup_elem(
        bpf_map__fd(skel_->maps.ustack_maps_array),
        &ustack_map_index,
        &ustack_map_id);
    if (err) {
        HHLOGE(true, "lookup ustack strace map id error: %s", strerror(-err));
        return;
    }
    ustackMapFd_ = bpf_map_get_fd_by_id(ustack_map_id);
}

int BPFEventReceiver::ReadCallChain(
    const __u32 nips,
    const int32_t ustack_id,
    __u64 *ips)
{
    if (!hasUStackMapFd_) {
        GetUStackMapFd();
        hasUStackMapFd_ = true;
    }
    if (memset_s(ips, nips * sizeof(__u64), 0, nips * sizeof(__u64)) != EOK) {
        HHLOGE(true, "memset_s failed");
        return -1;
    }
    int err = bpf_map_lookup_elem(ustackMapFd_, &ustack_id, ips);
    if (err) {
        HHLOGE(true, "lookup user callchain ips error: %s", strerror(-err));
        return -1;
    }
    return 0;
}
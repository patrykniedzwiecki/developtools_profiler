/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef HOOK_MANAGER_H
#define HOOK_MANAGER_H

#include <map>
#include <memory>
#include <string>
#include <vector>

#include "manager_interface.h"
#include "hook_service.h"
#include "epoll_event_poller.h"
#include "share_memory_allocator.h"
#include "event_notifier.h"
#include "native_hook_config.pb.h"
#include "virtual_runtime.h"

class ProfilerPluginConfig;
class PluginResult;
class CommandPoller;

class HookManager : public ManagerInterface {
public:
    HookManager();
    bool RegisterAgentPlugin(const std::string& pluginPath);
    bool UnregisterAgentPlugin(const std::string& pluginPath);

    bool LoadPlugin(const std::string& pluginPath) override;
    bool UnloadPlugin(const std::string& pluginPath) override;
    bool UnloadPlugin(const uint32_t pluginId) override;

    // CommandPoller will call the following four interfaces after receiving the command
    bool CreatePluginSession(const std::vector<ProfilerPluginConfig>& config) override;
    bool DestroyPluginSession(const std::vector<uint32_t>& pluginIds) override;
    bool StartPluginSession(const std::vector<uint32_t>& pluginIds,
                            const std::vector<ProfilerPluginConfig>& config) override;
    bool StopPluginSession(const std::vector<uint32_t>& pluginIds) override;

    bool CreateWriter(std::string pluginName, uint32_t bufferSize, int smbFd, int eventFd) override;
    bool ResetWriter(uint32_t pluginId) override;
    void SetCommandPoller(const std::shared_ptr<CommandPoller>& p) override;

private:
    void writeFrames(int type, const struct timespec& ts, void* addr, uint32_t size,
        const std::vector<OHOS::Developtools::NativeDaemon::CallFrame>& callsFrames);
    void ReadShareMemory();
    std::shared_ptr<HookService> hookService_;
    std::shared_ptr<CommandPoller> commandPoller_;
    int agentIndex_ = -1;
    std::string agentPluginName_;
    std::unique_ptr<EpollEventPoller> eventPoller_;
    std::shared_ptr<ShareMemoryBlock> shareMemoryBlock_;
    std::shared_ptr<EventNotifier> eventNotifier_;
    NativeHookConfig hookConfig_;
    std::string smbName_;
    std::unique_ptr<FILE, decltype(&fclose)> fpHookData_;
    std::shared_ptr<OHOS::Developtools::NativeDaemon::VirtualRuntime> runtime_instance;
};

#endif // AGENT_MANAGER_H
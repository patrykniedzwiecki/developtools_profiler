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
#include "hiperf_module.h"

#include <array>
#include <poll.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <vector>

#include "hiperf_plugin_config.pb.h"
#include "logging.h"
#include "securec.h"

namespace {
constexpr uint32_t MAX_BUFFER_SIZE = 4 * 1024 * 1024;
constexpr uint32_t READ_BUFFER_SIZE = 4096;
const std::string SU_ROOT = "su root";
const std::string HIPERF_CMD = " hiperf";
const std::string HIPERF_RECORD_CMD = " record";
const std::string HIPERF_RECORD_PREPARE = " --control prepare";
const std::string HIPERF_RECORD_START = " --control start";
const std::string HIPERF_RECORD_STOP = " --control stop";
const std::string HIPERF_RECORD_OK = "sampling success";

std::mutex g_taskMutex;
bool g_isRoot = false;
std::string g_logLevel = "";

bool ParseConfigToCmd(const HiperfPluginConfig& config, std::vector<std::string>& cmds)
{
    g_isRoot = config.is_root();
    auto logLevel = config.log_level();
    if (logLevel == HiperfPluginConfig_LogLevel_MUCH) {
        g_logLevel = " --hilog --much";
    } else if (logLevel == HiperfPluginConfig_LogLevel_VERBOSE) {
        g_logLevel = " --hilog --verbose";
    } else if (logLevel == HiperfPluginConfig_LogLevel_DEBUG) {
        g_logLevel = " --hilog --debug";
    }

    // command of prepare
    std::string traceCmd;
    auto &prepareCmd = cmds.emplace_back();
    prepareCmd = g_isRoot ? SU_ROOT : "";
    prepareCmd += HIPERF_CMD + g_logLevel + HIPERF_RECORD_CMD + HIPERF_RECORD_PREPARE;
    if (!config.outfile_name().empty()) {
        prepareCmd += " -o " + config.outfile_name();
    }
    if (!config.record_args().empty()) {
        prepareCmd += " " + config.record_args();
    }

    // command of start
    auto &startCmd = cmds.emplace_back();
    startCmd = g_isRoot ? SU_ROOT : "";
    startCmd += HIPERF_CMD + g_logLevel + HIPERF_RECORD_CMD + HIPERF_RECORD_START;
    return true;
}

bool RunCommand(const std::string& cmd)
{
    HILOG_DEBUG(LOG_CORE, "run command: %s", cmd.c_str());
    std::unique_ptr<FILE, decltype(&pclose)> pipe(popen(cmd.c_str(), "r"), pclose);
    CHECK_TRUE(pipe, false, "HiperfPlugin::RunCommand: create popen FAILED!");

    std::array<char, READ_BUFFER_SIZE> buffer;
    std::string result;
    while (fgets(buffer.data(), buffer.size(), pipe.get()) != nullptr) {
        result += buffer.data();
    }
    HILOG_DEBUG(LOG_CORE, "run command result: %s", result.c_str());
    bool res = result.find(HIPERF_RECORD_OK) != std::string::npos;
    CHECK_TRUE(res, false, "HiperfPlugin::RunCommand: execute command FAILED!");
    return true;
}
} // namespace

int HiperfPluginSessionStart(const uint8_t* configData, const uint32_t configSize)
{
    std::lock_guard<std::mutex> guard(g_taskMutex);
    HiperfPluginConfig config;
    bool res = config.ParseFromArray(configData, configSize);
    CHECK_TRUE(res, -1, "HiperfPluginSessionStart, parse config from array FAILED! configSize: %u", configSize);

    std::vector<std::string> cmds;
    res = ParseConfigToCmd(config, cmds);
    CHECK_TRUE(res, -1, "HiperfPluginSessionStart, parse config FAILED!");

    for (const auto &cmd : cmds) {
        res = RunCommand(cmd);
        CHECK_TRUE(res, -1, "HiperfPluginSessionStart, RunCommand(%s) FAILED!", cmd.c_str());
    }

    return 0;
}

int HiperfPluginSessionStop(void)
{
    std::lock_guard<std::mutex> guard(g_taskMutex);
    std::string cmd;
    if (g_isRoot) {
        cmd = SU_ROOT;
    }
    cmd += HIPERF_CMD + g_logLevel + HIPERF_RECORD_CMD;
    cmd += HIPERF_RECORD_STOP;
    RunCommand(cmd);

    return 0;
}

int HiperfRegisterWriterStruct(const WriterStruct* writer)
{
    HILOG_INFO(LOG_CORE, "%s:writer %p", __func__, writer);
    return 0;
}

static PluginModuleCallbacks g_callbacks = {
    HiperfPluginSessionStart,
    nullptr, // onPluginReportResult
    HiperfPluginSessionStop,
    HiperfRegisterWriterStruct,
};

PluginModuleStruct g_pluginModule = {&g_callbacks, "hiperf-plugin", MAX_BUFFER_SIZE};
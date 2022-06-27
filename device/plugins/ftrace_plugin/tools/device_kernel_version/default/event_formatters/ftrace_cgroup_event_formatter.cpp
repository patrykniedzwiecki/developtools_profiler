/* THIS FILE IS GENERATE BY ftrace_cpp_generator.py, PLEASE DON'T EDIT IT!
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
#include <cinttypes>

#include "event_formatter.h"
#include "logging.h"
#include "trace_events.h"

FTRACE_NS_BEGIN
namespace {
const int BUFFER_SIZE = 512;

REGISTER_FTRACE_EVENT_FORMATTER(
    cgroup_attach_task,
    [](const ForStandard::FtraceEvent& event) -> bool { return event.has_cgroup_attach_task_format(); },
    [](const ForStandard::FtraceEvent& event) -> std::string {
        auto msg = event.cgroup_attach_task_format();
        char buffer[BUFFER_SIZE];
        int len = snprintf(
            buffer, sizeof(buffer), "cgroup_attach_task: dst_root=%d dst_id=%d dst_level=%d dst_path=%s pid=%d comm=%s",
            msg.dst_root(), msg.dst_id(), msg.dst_level(), msg.dst_path().c_str(), msg.pid(), msg.comm().c_str());
        if (len >= BUFFER_SIZE - 1) {
            HILOG_WARN(LOG_CORE, "maybe, the contents of print event msg had be cut off in outfile");
        }
        return std::string(buffer);
    });

REGISTER_FTRACE_EVENT_FORMATTER(
    cgroup_destroy_root,
    [](const ForStandard::FtraceEvent& event) -> bool { return event.has_cgroup_destroy_root_format(); },
    [](const ForStandard::FtraceEvent& event) -> std::string {
        auto msg = event.cgroup_destroy_root_format();
        char buffer[BUFFER_SIZE];
        int len = snprintf(buffer, sizeof(buffer), "cgroup_destroy_root: root=%d ss_mask=%#x name=%s", msg.root(),
                           msg.ss_mask(), msg.name().c_str());
        if (len >= BUFFER_SIZE - 1) {
            HILOG_WARN(LOG_CORE, "maybe, the contents of print event msg had be cut off in outfile");
        }
        return std::string(buffer);
    });

REGISTER_FTRACE_EVENT_FORMATTER(
    cgroup_mkdir,
    [](const ForStandard::FtraceEvent& event) -> bool { return event.has_cgroup_mkdir_format(); },
    [](const ForStandard::FtraceEvent& event) -> std::string {
        auto msg = event.cgroup_mkdir_format();
        char buffer[BUFFER_SIZE];
        int len = snprintf(buffer, sizeof(buffer), "cgroup_mkdir: root=%d id=%d level=%d path=%s", msg.root(), msg.id(),
                           msg.level(), msg.path().c_str());
        if (len >= BUFFER_SIZE - 1) {
            HILOG_WARN(LOG_CORE, "maybe, the contents of print event msg had be cut off in outfile");
        }
        return std::string(buffer);
    });

REGISTER_FTRACE_EVENT_FORMATTER(
    cgroup_release,
    [](const ForStandard::FtraceEvent& event) -> bool { return event.has_cgroup_release_format(); },
    [](const ForStandard::FtraceEvent& event) -> std::string {
        auto msg = event.cgroup_release_format();
        char buffer[BUFFER_SIZE];
        int len = snprintf(buffer, sizeof(buffer), "cgroup_release: root=%d id=%d level=%d path=%s", msg.root(),
                           msg.id(), msg.level(), msg.path().c_str());
        if (len >= BUFFER_SIZE - 1) {
            HILOG_WARN(LOG_CORE, "maybe, the contents of print event msg had be cut off in outfile");
        }
        return std::string(buffer);
    });

REGISTER_FTRACE_EVENT_FORMATTER(
    cgroup_remount,
    [](const ForStandard::FtraceEvent& event) -> bool { return event.has_cgroup_remount_format(); },
    [](const ForStandard::FtraceEvent& event) -> std::string {
        auto msg = event.cgroup_remount_format();
        char buffer[BUFFER_SIZE];
        int len = snprintf(buffer, sizeof(buffer), "cgroup_remount: root=%d ss_mask=%#x name=%s", msg.root(),
                           msg.ss_mask(), msg.name().c_str());
        if (len >= BUFFER_SIZE - 1) {
            HILOG_WARN(LOG_CORE, "maybe, the contents of print event msg had be cut off in outfile");
        }
        return std::string(buffer);
    });

REGISTER_FTRACE_EVENT_FORMATTER(
    cgroup_rename,
    [](const ForStandard::FtraceEvent& event) -> bool { return event.has_cgroup_rename_format(); },
    [](const ForStandard::FtraceEvent& event) -> std::string {
        auto msg = event.cgroup_rename_format();
        char buffer[BUFFER_SIZE];
        int len = snprintf(buffer, sizeof(buffer), "cgroup_rename: root=%d id=%d level=%d path=%s", msg.root(),
                           msg.id(), msg.level(), msg.path().c_str());
        if (len >= BUFFER_SIZE - 1) {
            HILOG_WARN(LOG_CORE, "maybe, the contents of print event msg had be cut off in outfile");
        }
        return std::string(buffer);
    });

REGISTER_FTRACE_EVENT_FORMATTER(
    cgroup_rmdir,
    [](const ForStandard::FtraceEvent& event) -> bool { return event.has_cgroup_rmdir_format(); },
    [](const ForStandard::FtraceEvent& event) -> std::string {
        auto msg = event.cgroup_rmdir_format();
        char buffer[BUFFER_SIZE];
        int len = snprintf(buffer, sizeof(buffer), "cgroup_rmdir: root=%d id=%d level=%d path=%s", msg.root(), msg.id(),
                           msg.level(), msg.path().c_str());
        if (len >= BUFFER_SIZE - 1) {
            HILOG_WARN(LOG_CORE, "maybe, the contents of print event msg had be cut off in outfile");
        }
        return std::string(buffer);
    });

REGISTER_FTRACE_EVENT_FORMATTER(
    cgroup_setup_root,
    [](const ForStandard::FtraceEvent& event) -> bool { return event.has_cgroup_setup_root_format(); },
    [](const ForStandard::FtraceEvent& event) -> std::string {
        auto msg = event.cgroup_setup_root_format();
        char buffer[BUFFER_SIZE];
        int len = snprintf(buffer, sizeof(buffer), "cgroup_setup_root: root=%d ss_mask=%#x name=%s", msg.root(),
                           msg.ss_mask(), msg.name().c_str());
        if (len >= BUFFER_SIZE - 1) {
            HILOG_WARN(LOG_CORE, "maybe, the contents of print event msg had be cut off in outfile");
        }
        return std::string(buffer);
    });

REGISTER_FTRACE_EVENT_FORMATTER(
    cgroup_transfer_tasks,
    [](const ForStandard::FtraceEvent& event) -> bool { return event.has_cgroup_transfer_tasks_format(); },
    [](const ForStandard::FtraceEvent& event) -> std::string {
        auto msg = event.cgroup_transfer_tasks_format();
        char buffer[BUFFER_SIZE];
        int len = snprintf(buffer, sizeof(buffer),
                           "cgroup_transfer_tasks: dst_root=%d dst_id=%d dst_level=%d dst_path=%s pid=%d comm=%s",
                           msg.dst_root(), msg.dst_id(), msg.dst_level(), msg.dst_path().c_str(), msg.pid(),
                           msg.comm().c_str());
        if (len >= BUFFER_SIZE - 1) {
            HILOG_WARN(LOG_CORE, "maybe, the contents of print event msg had be cut off in outfile");
        }
        return std::string(buffer);
    });
} // namespace
FTRACE_NS_END
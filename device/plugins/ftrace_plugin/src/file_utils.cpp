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
 *
 * Description: FileUtils implements
 */
#include "file_utils.h"

#include <cerrno>
#include <cstring>
#include <dirent.h>
#include <fcntl.h>
#include <unistd.h>
#include "logging.h"

namespace {
constexpr size_t DEFAULT_READ_SIZE = 4096;
}

std::string FileUtils::ReadFile(int fd)
{
    std::string content;
    size_t count = 0;
    while (true) {
        if (content.size() - count < DEFAULT_READ_SIZE) {
            content.resize(content.size() + DEFAULT_READ_SIZE);
        }
        ssize_t nBytes = TEMP_FAILURE_RETRY(read(fd, &content[count], content.size() - count));
        if (nBytes <= 0) {
            break;
        }
        count += nBytes;
    }
    content.resize(count);
    return content;
}

std::string FileUtils::ReadFile(const std::string& path)
{
    int fd = open(path.c_str(), O_RDONLY);
    if (fd < 0) {
        HILOG_WARN(LOG_CORE, "open file %s FAILED: %s!", path.c_str(), strerror(errno));
        return "";
    }

    std::string content = ReadFile(fd);
    CHECK_TRUE(close(fd) != -1, content, "close %s failed, %s", path.c_str(), strerror(errno));
    return content;
}

int FileUtils::WriteFile(const std::string& path, const std::string& content)
{
    return WriteFile(path, content, O_WRONLY);
}

int FileUtils::WriteFile(const std::string& path, const std::string& content, int flags)
{
    return WriteFile(path, content, flags, 0);
}

int FileUtils::WriteFile(const std::string& path, const std::string& content, int flags, int mode)
{
    int fd = open(path.c_str(), flags, mode);
    CHECK_TRUE(fd >= 0, -1, "open %s failed, %s", path.c_str(), strerror(errno));

    int retval = write(fd, content.data(), content.size());
    CHECK_TRUE(close(fd) != -1, -1, "close %s failed, %s", path.c_str(), strerror(errno));
    HILOG_DEBUG(LOG_CORE, "write(%s) with '%s' done!", path.c_str(), content.c_str());
    return retval;
}

std::vector<std::string> FileUtils::ListDir(const std::string& dirPath)
{
    std::vector<std::string> result;
    DIR* dir = opendir(dirPath.c_str());
    if (dir == nullptr) {
        return result;
    }

    struct dirent* ent = nullptr;
    while ((ent = readdir(dir)) != nullptr) {
        std::string name = ent->d_name;
        if (name == "." || name == "..") {
            continue;
        }
        result.push_back(name);
    }
    closedir(dir);
    return result;
}
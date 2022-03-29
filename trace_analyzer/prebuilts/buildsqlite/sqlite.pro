# Copyright (C) 2021 Huawei Device Co., Ltd.
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
QT -= gui core
TEMPLATE = lib
#TEMPLATE = app
CONFIG += c++14 lib
#CONFIG += c++14
TARGET = sqlite

DEFINES += HAVE_PTHREAD
LIBDIR = $$PWD/../../third_party/sqlite
ROOTSRCDIR = $$PWD/../../src/
include($$PWD/../../src/multi_platform/global.pri)

LIBS += -L$$DESTDIR/ -lstdc++

#INCLUDEPATH += $$PWD/$${PROTOBUDIR}/src
INCLUDEPATH += $$PWD/../../third_party/sqlite/include

message("includepath is:"$$INCLUDEPATH)
SOURCES += $${LIBDIR}/src/sqlite3.c

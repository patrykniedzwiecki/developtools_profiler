#!/usr/bin/env python3
# -*- coding: utf-8 -*-
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
import os
import subprocess
import sys
import argparse
import logging
sys.path.append(os.getcwd())
sys.path.append("../../")
from ftrace_format_parser import FtraceEventCodeGenerator
from ftrace_format_parser import ProtoType

AUTO_GENERATED_GNI = 'autogenerated.gni'

THIS_FILE = os.path.basename(__file__)
logging.basicConfig(format='%(asctime)s %(levelname)s %(message)s',
    level=logging.INFO)
logger = logging.getLogger(THIS_FILE)

CPP_COPYRIGHT_HEADER = '''\
/* THIS FILE IS GENERATE BY {}, PLEASE DON'T EDIT IT!
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
'''.format(THIS_FILE)

GN_COPYRIGHT_HEADER = '''\
# THIS FILE IS GENERATE BY {}, PLEASE DON'T EDIT IT!
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
'''.format(THIS_FILE)

PARSE_FUNCTION_ARGS = '({}, {}, {}, {})'.format('FtraceEvent& ftraceEvent',
    'uint8_t data[]', 'size_t size', 'const EventFormat& format')
PARSE_REGISTER_MACRO = 'REGISTER_FTRACE_EVENT_PARSE_FUNCTION'

CHECK_FUNCTION_ARGS = '(const ForStandard::FtraceEvent& event) -> bool'
FORMAT_FUNCTION_ARGS = '(const ForStandard::FtraceEvent& event) -> std::string'
FORMAT_REGISTER_MACRO = 'REGISTER_FTRACE_EVENT_FORMATTER'


def to_camel_case(name):
    return ''.join([p.capitalize() for p in name.split('_')])


def fix_field_name(name):
    replace_map = {
        'errno': 'error_code',
        'sa_handler': 'sig_handler',
        'sa_flags': 'sig_flags',
        'new': 'seq_new'
    }
    if name in replace_map:
        name = replace_map[name]
    return str.lower(name)


def ensure_dir_exists(file_path):
    file_dir = os.path.dirname(file_path)
    if not os.path.exists(file_dir):
        os.mkdir(file_dir)

class Common:
    this_file = os.path.basename(__file__)
    logging.basicConfig(
        format="%(asctime)s %(levelname)s %(message)s", level=logging.INFO
    )
    logger = logging.getLogger(this_file)
    cpp_copyright_header = CPP_COPYRIGHT_HEADER
    parse_register_macro = PARSE_REGISTER_MACRO
    auto_generated_gni = AUTO_GENERATED_GNI
    parse_function_args = PARSE_FUNCTION_ARGS
    gn_copyright_header = GN_COPYRIGHT_HEADER

class EventParserCodeGenerator(FtraceEventCodeGenerator):
    def __init__(self, events_dir, allow_list):
        super().__init__(events_dir, allow_list)

    def parser_file_path(self, category):
        file_name = 'ftrace_{}_event_parser.cpp'.format(category)
        return os.path.join(self.output_dir, file_name)

    def generate_code(self):
        generated_cpp_sources = []

        for event in self.target_event_formats:
            type_name = '{}/{}'.format(event.category, event.name)
            logger.info('ftrace_events: "{}"'.format(type_name))

        # generate sub event parser code
        for category in self.grouped_event_formats:
            parser_src_file = self.parser_file_path(category)
            generated_cpp_sources.append(parser_src_file)

            logger.info('Generate {} ...'.format(parser_src_file))
            ensure_dir_exists(parser_src_file)

            with open(parser_src_file, 'w', encoding='utf-8') as f:
                f.write(CPP_COPYRIGHT_HEADER)
                f.write('#include "sub_event_parser.h"\n')
                f.write('\n')
                f.write("FTRACE_NS_BEGIN\n")
                f.write("namespace {\n")
                self.generate_parse_functions(category, f)
                f.write("} // namespace\n")
                f.write("FTRACE_NS_END\n")
                f.write('\n')

        # generate .gni
        generated_cpp_gni = os.path.join(self.output_dir, AUTO_GENERATED_GNI)
        logger.info('Generate {} ...'.format(generated_cpp_gni))
        with open(generated_cpp_gni, 'w', encoding='utf-8') as f:
            f.write(GN_COPYRIGHT_HEADER)
            f.write('\n')
            f.write('auto_generated_cpp_sources = [\n')
            for path in generated_cpp_sources:
                src = '{}'.format(os.path.basename(path))
                f.write('  "{}",\n'.format(src))
            f.write(']\n')

    def generate_parse_functions(self, category, f):
        count = 0
        for event in self.grouped_event_formats[category]:
            count += 1
            if count > 1:
                f.write('\n')
            f.write('{}({},\n'.format(PARSE_REGISTER_MACRO, event.name))
            f.write('[] {} {{\n'.format(PARSE_FUNCTION_ARGS))
            f.write('    int i = 0;\n')
            f.write('    auto msg = ftraceEvent.mutable_{}_format();\n'.format(
                str.lower(event.name)))
            for i in range(len(event.remain_fields)):
                self.generate_parse_field_lines(event, f, i)
            f.write("});\n")

    @staticmethod
    def generate_parse_field_lines(event, f, i):
        field_info = event.remain_fields[i]
        field_name = fix_field_name(field_info.name)
        type_info = field_info.to_proto_type()
        parse_func = None
        if type_info.tid == ProtoType.STRING:
            parse_func = 'ParseStrField'
        elif type_info.tid == ProtoType.ARRAY:
            parse_func = 'ParseVectorIntField'
        elif type_info.tid == ProtoType.INTEGER:
            assert type_info.size in [4, 8]
            c_type = None
            if type_info.size == 4:
                c_type = 'int32_t' if type_info.signed else 'uint32_t'
            elif type_info.size == 8:
                c_type = 'int64_t' if type_info.signed else 'uint64_t'
            parse_func = 'ParseIntField<{}>'.format(c_type)
        else:
            logger.warning('WARNING: unkown proto type:{} {}'.format(
                event.name, field_name))
        assert parse_func
        if (type_info.tid == ProtoType.ARRAY):
            f.write('    std::vector<uint64_t> retvalVec = FtraceFieldParser::ParseVectorIntField<uint64_t>(format.fields, i++, data, size);')
            f.write('    for (size_t i = 0; i < retvalVec.size(); i++) {')
            f.write('    msg->add_{}(retvalVec[i]);'.format(field_name))
            f.write('    }')
        else:
            f.write('    msg->set_{}(FtraceFieldParser::'.format(field_name))
            f.write('{}(format.fields, i++, data, size));\n'.format(parse_func))


class EventFormatterCodeGenerator(FtraceEventCodeGenerator):
    def __init__(self, events_dir, allow_list):
        super().__init__(events_dir, allow_list)

    def formatter_file_path(self, category):
        file_name = 'ftrace_{}_event_formatter.cpp'.format(category)
        return os.path.join(self.output_dir, file_name)

    def generate_code(self):
        generated_cpp_sources = []

        # generate sub event parser code
        for category in self.grouped_event_formats:
            formatter_src_file = self.formatter_file_path(category)
            generated_cpp_sources.append(formatter_src_file)

            logger.info('Generate {} ...'.format(formatter_src_file))
            ensure_dir_exists(formatter_src_file)

            with open(formatter_src_file, 'w', encoding='utf-8') as f:
                f.write(CPP_COPYRIGHT_HEADER)
                f.write('#include <cinttypes>\n')
                f.write('\n')
                f.write('#include "event_formatter.h"\n')
                if (category == "binder") :
                    f.write('#include "binder.h"\n')
                f.write('#include "logging.h"\n')
                if (category == "f2fs") :
                    f.write('#include "f2fs.h"\n')
                elif (category == "power") :
                    f.write('#include "pm_qos.h"\n')
                elif (category == "writeback") :
                    f.write('#include "pq.h"\n')
                elif (category == "compaction")  | (category == "kmem") | (category == "vmscan") :
                    f.write('#include "type.h"\n')
                elif (category == "timer") :
                    f.write('#include "hrtimer.h"\n')
                f.write('#include "trace_events.h"\n')
                f.write('\n')
                f.write("FTRACE_NS_BEGIN\n")
                f.write("namespace {\n")
                f.write('const int BUFFER_SIZE = 512;\n')
                f.write('\n')
                self.generate_format_functions(category, f)
                f.write("} // namespace\n")
                f.write("FTRACE_NS_END\n")
                f.write('\n')

        # generate .gni
        generated_cpp_gni = os.path.join(self.output_dir, AUTO_GENERATED_GNI)
        logger.info('Generate {} ...'.format(generated_cpp_gni))
        with open(generated_cpp_gni, 'w', encoding='utf-8') as f:
            f.write(GN_COPYRIGHT_HEADER)
            f.write('\n')
            f.write('auto_generated_cpp_sources = [\n')
            for path in generated_cpp_sources:
                src = '{}'.format(os.path.basename(path))
                f.write('  "{}",\n'.format(src))
            f.write(']\n')

    def generate_format_functions(self, category, f):
        count = 0
        for event in self.grouped_event_formats[category]:
            count += 1
            if count > 1:
                f.write('\n')
            f.write('{}({},\n'.format(FORMAT_REGISTER_MACRO, str.lower(event.name)))
            f.write('[] {} {{\n'.format(CHECK_FUNCTION_ARGS, ))
            f.write('    return event.has_{}_format();'.format(
                str.lower(event.name)))
            f.write('},')  # end of check function
            f.write('[] {} {{\n'.format(FORMAT_FUNCTION_ARGS))
            f.write('    auto msg = event.{}_format();\n'.format(
                str.lower(event.name)))
            f.write("    char buffer[BUFFER_SIZE];\n")
            event.print_fmt = "\"{}: {}".format(event.name, event.print_fmt[2:])
            event.print_fmt = str.replace(event.print_fmt, "%zu", "%\" PRIu64 \"")
            event.print_fmt = str.replace(event.print_fmt, "%zd", "%\" PRIu64 \"")
            event.print_fmt = str.replace(event.print_fmt, "%llu", "%\" PRIu64 \"")
            event.print_fmt = str.replace(event.print_fmt, "%lld", "%\" PRIu64 \"")
            event.print_fmt = str.replace(event.print_fmt, "%lx", "%\" PRIx64 \"")
            event.print_fmt = str.replace(event.print_fmt, "%llx", "%\" PRIx64 \"")
            event.print_fmt = str.replace(event.print_fmt, "(unsigned long long)", "")

            if (category == "kmem") | (category == "filemap") :
                event.print_fmt = str.replace(event.print_fmt, "page=%p", "page=%s")
                event.print_fmt = str.replace(event.print_fmt, "REC->pfn != -1UL ? (((struct page *)vmemmap_base) + (REC->pfn)) : ((void *)0)", "\"0000000000000000\"")
                event.print_fmt = str.replace(event.print_fmt, "(((struct page *)vmemmap_base) + (REC->pfn))", "\"0000000000000000\"")
                event.print_fmt = str.replace(event.print_fmt, " (void *)", " ")
                event.print_fmt = str.replace(event.print_fmt, " name=%s\"", "\"")
                event.print_fmt = str.replace(event.print_fmt, "REC->ptr, __get_str(name)", "REC->ptr")
                event.print_fmt = str.replace(event.print_fmt, "%lu", "%\" PRIu64 \"")
            elif (category == "filelock") :
                event.print_fmt = str.replace(event.print_fmt, "=%lu", "=%\" PRIu64 \"")
                event.print_fmt = str.replace(event.print_fmt, "REC->fl_blocker", "msg.fl_next()")
                event.print_fmt = str.replace(event.print_fmt, "REC->fl_owner", "msg.fl_owner()")
                event.print_fmt = str.replace(event.print_fmt, "REC->fl_flags", "msg.fl_flags()")
                event.print_fmt = str.replace(event.print_fmt, "REC->fl_type", "msg.fl_type()")
                event.print_fmt = str.replace(event.print_fmt, "REC->fl_break_time", "msg.fl_break_time()")
                event.print_fmt = str.replace(event.print_fmt, "REC->fl_downgrade_time", "msg.fl_downgrade_time()")
                event.print_fmt = str.replace(event.print_fmt, "REC->rcount", "msg.dcount()")
            elif (category == "ext4") :
                event.print_fmt = str.replace(event.print_fmt, "%lu", "%\" PRIu64 \"")
                event.print_fmt = str.replace(event.print_fmt, ", (unsigned long)", ",")
            elif (category == "vmscan") :
                event.print_fmt = str.replace(event.print_fmt, "%ld", "%\" PRIu64 \"")
                event.print_fmt = str.replace(event.print_fmt, "%lu", "%\" PRIu64 \"")
            elif (category == "writeback") :
                event.print_fmt = str.replace(event.print_fmt, ", (unsigned long)", ", ")
                event.print_fmt = str.replace(event.print_fmt, "cgroup_ino=%lu", "cgroup_ino=%\" PRId32 \"")
                event.print_fmt = str.replace(event.print_fmt, "%lu", "%\" PRIu64 \"")
                event.print_fmt = str.replace(event.print_fmt, "%ld", "%\" PRIu64 \"")
            elif (category == "sunrpc") :
                event.print_fmt = str.replace(event.print_fmt, "=%lu", "=%\" PRIu64 \"")
            elif (category == "sched") :
                event.print_fmt = str.replace(event.print_fmt, "%Lu", "%\" PRIu64 \"")
            elif (category == "ftrace") :
                event.print_fmt = str.replace(event.print_fmt, "(unsigned long)", "")

            if (event.name == "binder_command") :
                event.print_fmt = "\"binder_command: cmd=0x%x\", msg.cmd()"
            elif (event.name == "binder_return") :
                event.print_fmt = "\"binder_return: cmd=0x%x\", msg.cmd()"
            elif (event.name == "task_newtask") | (event.name == "task_rename") :
                event.print_fmt = str.replace(event.print_fmt, "oom_score_adj=%hd", "oom_score_adj=%d")
            elif (event.name == "branch_format"):
                event.print_fmt = "\"branch: %u:%s:%s (%u)%s\", msg.line(), msg.func().c_str(), msg.file().c_str(), msg.correct(), msg.constant() ? \" CONSTANT\" : \"\""
            elif (event.name == "mm_lru_activate") | (event.name == "mm_lru_insertion") :
                event.print_fmt = str.replace(event.print_fmt, "pfn=%lu", "pfn=%\" PRIu64 \"")
            elif (event.name == "sched_stick_numa"):
                event.print_fmt = "\"sched_stick_numa: src_pid=%d src_tgid=%d src_ngid=%d src_cpu=%d src_nid=%d dst_pid=%d dst_tgid=%d dst_ngid=%d dst_cpu=%d dst_nid=%d\", msg.pid(), msg.tgid(), msg.ngid(), msg.src_cpu(), msg.src_nid(), msg.pid(), msg.tgid(), msg.ngid(), msg.dst_cpu(), msg.dst_nid()"
            elif (event.name == "workqueue_execute_end"):
                event.print_fmt = "\"workqueue_execute_end: work struct %p\", msg.work()"
            elif (event.name == "workqueue_queue_work"):
                event.print_fmt = str.replace(event.print_fmt, "REC->workqueue", "msg.workqueue()")
            elif (event.name == "i2c_reply") | (event.name == "i2c_write") :
                event.print_fmt = str.replace(event.print_fmt, "l=%u [%*phD]", "l=%\" PRId32 \" [%\" PRId32 \"]")
                event.print_fmt = str.replace(event.print_fmt, "REC->len, REC->len", "REC->len")
            elif (event.name == "mm_compaction_isolate_freepages") | (event.name == "mm_compaction_isolate_migratepages") \
                    | (event.name == "mm_compaction_begin") | (event.name == "mm_compaction_end") \
                    | (event.name == "mm_compaction_migratepages") :
                event.print_fmt = str.replace(event.print_fmt, "%lu", "=0x%\" PRIu64 \"")
            elif (event.name == "mm_compaction_defer_compaction") | (event.name == "mm_compaction_defer_reset") \
                    | (event.name == "mm_compaction_deferred") | (event.name == "mm_compaction_end") \
                    | (event.name == "mm_compaction_migratepages") :
                event.print_fmt = str.replace(event.print_fmt, "REC->order_failed", "msg.order_failed()")
            elif (event.name == "signal_deliver") | (event.name == "signal_generate") :
                event.print_fmt = str.replace(event.print_fmt, "REC->errno", "msg.error_code()")
                event.print_fmt = str.replace(event.print_fmt, "REC->sa_handler", "msg.sig_handler()")
                event.print_fmt = str.replace(event.print_fmt, "REC->sa_flags", "msg.sig_flags()")
            elif (event.name == "sys_enter") | (event.name == "sys_exit"):
                event.print_fmt = str.replace(event.print_fmt, "%ld", "%\" PRIu64 \"")
            elif (event.name == "oom_score_adj_update"):
                event.print_fmt = str.replace(event.print_fmt, "oom_score_adj=%hd", "oom_score_adj=%\" PRId32 \"")
            elif (event.name == "ext4_da_write_pages"):
                event.print_fmt = str.replace(event.print_fmt, "first_page %\" PRIu64 \" nr_to_write %ld", "first_page %\" PRIu64 \" nr_to_write %\" PRIu64 \"")
            elif (event.name == "ext4_discard_preallocations"):
                event.print_fmt = str.replace(event.print_fmt, "ino %\" PRIu64 \" len: %u needed %u", "ino %\" PRIu64 \"")
                event.print_fmt = str.replace(event.print_fmt, " REC->ino, REC->len, REC->needed", " REC->ino")
            elif (event.name == "ext4_ext_remove_space_done") | (event.name == "ext4_ext_rm_leaf") | (event.name == "ext4_remove_blocks") :
                event.print_fmt = str.replace(event.print_fmt, " [pclu %\" PRIu64 \" lblk %u state %d]", "%\" PRIu64 \"")
                event.print_fmt = str.replace(event.print_fmt, "(long long) REC->pc_pclu, (unsigned int) REC->pc_lblk, (int) REC->pc_state", "msg.partial()")
            elif (event.name == "ext4_free_blocks"):
                event.print_fmt = str.replace(event.print_fmt, "count %\" PRIu64 \"", "count %\" PRIu64 \"")
            elif (event.name == "ext4_journal_start"):
                event.print_fmt = str.replace(event.print_fmt, ", revoke_creds %d,", ",")
                event.print_fmt = str.replace(event.print_fmt, ", REC->revoke_creds,", ",")
            elif (event.name == "ext4_read_block_bitmap_load"):
                event.print_fmt = str.replace(event.print_fmt, "%u prefetch %d", "%u")
                event.print_fmt = str.replace(event.print_fmt, "REC->group, REC->prefetch", "msg.group()")
            elif (event.name == "ext4_writepages"):
                event.print_fmt = str.replace(event.print_fmt, "nr_to_write %ld", "nr_to_write %\" PRIu64 \"")
                event.print_fmt = str.replace(event.print_fmt, "pages_skipped %ld", "pages_skipped %\" PRIu64 \"")
            elif (event.name == "ext4_writepages_result"):
                event.print_fmt = str.replace(event.print_fmt, "pages_skipped %ld", "pages_skipped %\" PRIu64 \"")
            elif (event.name == "ext4_load_inode"):
                event.print_fmt = str.replace(event.print_fmt, "ino %ld", "ino %\" PRIu64 \"")
            elif (event.name == "itimer_state") :
                event.print_fmt = str.replace(event.print_fmt, "it_value=%ld.%06ld it_interval=%ld.%06ld", "it_value=%\" PRIu64 \"it_interval=%\" PRIu64 \"")
                event.print_fmt = str.replace(event.print_fmt, "REC->value_nsec / 1000L, REC->interval_sec, REC->interval_nsec / 1000L", "msg.interval_sec()")
            elif (event.name == "timer_expire_entry") :
                event.print_fmt = str.replace(event.print_fmt, "now=%lu baseclk=%lu", "now=%\" PRIu64 \"")
                event.print_fmt = str.replace(event.print_fmt, "REC->now, REC->baseclk", "msg.now()")
            elif (event.name == "timer_start") :
                event.print_fmt = str.replace(event.print_fmt, "expires=%lu [timeout=%ld]", "expires=%\" PRIu64 \" [timeout=%\" PRIu64 \"]")
                event.print_fmt = str.replace(event.print_fmt, ", (long)", ",")
            elif (event.name == "mm_shrink_slab_end") | (event.name == "mm_shrink_slab_start") :
                event.print_fmt = str.replace(event.print_fmt, "REC->shrink", "msg.shrink()")
            elif (event.name == "mm_vmscan_lru_shrink_inactive") :
                event.print_fmt = str.replace(event.print_fmt, "nr_activate_anon=%d nr_activate_file=%d", "nr_activate=%\" PRIu64 \"")
                event.print_fmt = str.replace(event.print_fmt, "REC->nr_activate0, REC->nr_activate1", "msg.nr_activate()")
            elif (event.name == "mm_vmscan_writepage") :
                event.print_fmt = str.replace(event.print_fmt, "page=%p", "page=%s")
                event.print_fmt = "{} \"0000000000000000\"{}".format(event.print_fmt[:56], event.print_fmt[3728:])
            elif (event.name == "mm_vmscan_lru_isolate") :
                event.print_fmt = str.replace(event.print_fmt, "REC->highest_zoneidx", "msg.classzone_idx()")
            elif (event.name == "balance_dirty_pages") | (event.name == "bdi_dirty_ratelimit") :
                event.print_fmt = str.replace(event.print_fmt, "REC->bdi_dirty", "msg.bdi_dirty()")
                event.print_fmt = str.replace(event.print_fmt, "REC->bdi_setpoint", "msg.bdi_setpoint()")
                event.print_fmt = str.replace(event.print_fmt, "REC->dirty_ratelimit", "msg.dirty_ratelimit()")
                event.print_fmt = str.replace(event.print_fmt, "REC->dirtied_pause", "msg.dirtied_pause()")
            elif (event.name == "sched_migrate_task") :
                event.print_fmt = str.replace(event.print_fmt, "dest_cpu=%d running=%d", "dest_cpu=%d")
                event.print_fmt = str.replace(event.print_fmt, "REC->dest_cpu, REC->running", "msg.dest_cpu()")
            elif (event.name == "ext4_find_delalloc_range") :
                event.print_fmt = str.replace(event.print_fmt, "REC->found_blk", "msg.found_blk()")
            elif (event.name == "mm_filemap_add_to_page_cache") | (event.name == "mm_filemap_delete_from_page_cache"):
                event.print_fmt = "\"{}: dev %\" PRIu64 \":%\" PRIu64 \" ino %p page=%s pfn=%\" PRIu64 \" ofs=%\" PRIu64 \"\", (((msg.s_dev()) >> 20)), (((msg.s_dev()) & ((1U << 20) - 1))), msg.i_ino(), \"0000000000000000\", msg.pfn(), msg.index() << 12".format(event.name)
            elif (event.name == "ipi_raise") :
                event.print_fmt = str.replace(event.print_fmt, "target_mask=%s", "target_mask=%\" PRIu64 \"")
                event.print_fmt = str.replace(event.print_fmt, "__get_bitmask(target_cpus)", "msg.target_cpus()")
            elif (event.name == "mm_page_alloc") :
                event.print_fmt = "{} \"0000000000000000\"{}".format(event.print_fmt[:78], event.print_fmt[3783:])
            elif (event.name == "mm_page_alloc_extfrag") :
                event.print_fmt = "{} \"0000000000000000\"{}".format(event.print_fmt[:181], event.print_fmt[3853:])
            elif (event.name == "mm_page_alloc_zone_locked") :
                event.print_fmt = "{} \"0000000000000000\"{}".format(event.print_fmt[:181], event.print_fmt[3853:])
            elif (event.name == "mm_page_free") :
                event.print_fmt = "{} \"0000000000000000\"{}".format(event.print_fmt[:49], event.print_fmt[3721:])
            elif (event.name == "mm_page_free_batched") :
                event.print_fmt = "{} \"0000000000000000\"{}".format(event.print_fmt[:56], event.print_fmt[3728:])
            elif (event.name == "mm_page_pcpu_drain") :
                event.print_fmt = "{} \"0000000000000000\"{}".format(event.print_fmt[:70], event.print_fmt[3742:])
            elif (event.name == "xprt_transmit"):
                event.print_fmt = "\"xprt_transmit: xid=0x%08x status=%d\", msg.xid(), msg.status()"
            elif (event.name == "rss_stat") :
                event.print_fmt = str.replace(event.print_fmt, "size=%ldB", "size=%\" PRIu64 \"B")
            elif (event.name == "file_check_and_advance_wb_err") :
                event.print_fmt = str.replace(event.print_fmt, "REC->new", "msg.seq_new()")
            elif (event.name == "f2fs_sync_file_enter") :
                event.print_fmt = str.replace(event.print_fmt, "i_mode = 0x%hx", "i_mode = 0x%x")
            elif (event.name == "wakeup_source_activate") | (event.name == "wakeup_source_deactivate") :
                event.print_fmt = str.replace(event.print_fmt, "(unsigned long)", "")
            elif (event.name == "print") :
                f.write("    std::string fieldBuf = msg.buf();\n")
                f.write("    if (fieldBuf.size() > 1 && fieldBuf[fieldBuf.size() - 1] == '\\n') {\n")
                f.write("        fieldBuf = fieldBuf.substr(0, fieldBuf.size() - 1);\n")
                f.write("    }\n")
                event.print_fmt = "\"print: %s\", fieldBuf.c_str()"

            event.print_fmt = str.replace(event.print_fmt, "%pS", "%p")

            if (event.print_fmt.find("%ps") == -1) :
                self.handle_field_name_functions(event, f, False)
            else :
                if ((event.print_fmt.find("%ps") != -1) & (event.print_fmt.find("REC->function") != -1)) :
                    if (event.name != "workqueue_execute_end") :
                        self.handle_format_ps_functions(event, f, "function")
                elif ((event.print_fmt.find("%ps") != -1) & (event.print_fmt.find("REC->ip") != -1)) :
                    self.handle_format_ps_functions(event, f, "ip", True)
                elif ((event.print_fmt.find("%ps") != -1) & (event.print_fmt.find("REC->action") != -1)) :
                    self.handle_format_ps_functions(event, f, "action")
                elif ((category == "cpuhp") & (event.print_fmt.find("%ps") != -1) & (event.print_fmt.find("REC->fun") != -1)) :
                    self.handle_format_ps_functions(event, f, "fun")
                elif ((event.name == "funcgraph_entry") | (event.name == "funcgraph_exit")) :
                    self.handle_format_ps_functions(event, f, "func", True)
                elif (event.name == "kernel_stack") | (event.name == "user_stack") :
                    f.write("    int len = 0;\n")
                    f.write("    std::string kernelSymbolsStr = \"\";\n")
                    f.write("    auto kernelSymbols = EventFormatter::GetInstance().kernelSymbols_;\n")
                    f.write("    if (kernelSymbols.count(msg.caller()[0]) > 0) {\n")
                    f.write("        kernelSymbolsStr = kernelSymbols[msg.caller()[0]];\n")
                    f.write("    }\n")
                    f.write("    if (kernelSymbolsStr != \"\") {\n        ")
                    print_fmt = event.print_fmt
                    event.print_fmt = str.replace(event.print_fmt, "%ps", "%s")
                    offset = 0
                    if (event.name == "kernel_stack") :
                        offset = 2
                    event.print_fmt = "{}kernelSymbols[msg.caller()[{}]].c_str(), kernelSymbols[msg.caller()[{}]].c_str(), kernelSymbols[msg.caller()[{}]].c_str()," \
                        " kernelSymbols[msg.caller()[{}]].c_str(), kernelSymbols[msg.caller()[{}]].c_str(), kernelSymbols[msg.caller()[{}]].c_str()," \
                        " kernelSymbols[msg.caller()[{}]].c_str(), kernelSymbols[msg.caller()[{}]].c_str()".format(event.print_fmt[:94+offset], event.print_fmt[114+offset],
                        event.print_fmt[138+offset], event.print_fmt[162+offset], event.print_fmt[186+offset], event.print_fmt[210+offset], event.print_fmt[234+offset],
                        event.print_fmt[258+offset], event.print_fmt[282+offset])
                    f.write("    len = snprintf_s(buffer, BUFFER_SIZE, BUFFER_SIZE - 1, {});\n".format(event.print_fmt))
                    f.write("    } else {\n")
                    event.print_fmt = print_fmt
                    event.print_fmt = str.replace(event.print_fmt, "%ps", "%p")
                    self.handle_field_name_functions(event, f)
                    f.write("    }\n")

            f.write('    if (len >= BUFFER_SIZE - 1) {\n')
            f.write('      HILOG_WARN(LOG_CORE, "maybe, the contents of print event({}) msg had be cut off in outfile");\n'.format(event.name))
            f.write('    }\n')
            f.write("    return std::string(buffer);\n")
            f.write("});\n")  # end of format function

    def handle_field_name_functions(self, event, f, is_define_len=True):
        for field_info in event.remain_fields:
            field_name = fix_field_name(field_info.name)
            event.print_fmt = str.replace(event.print_fmt, "__get_str({})".format(field_name),
                                                "msg.{}().c_str()".format(field_name))
            event.print_fmt = str.replace(event.print_fmt, "__get_dynamic_array({})".format(field_name),
                                                "msg.{}()".format(field_name))
            if field_info.field.startswith('char {}['.format(field_name)) \
                | field_info.field.startswith('const char {}['.format(field_name)) \
                | field_info.field.startswith('char *') | field_info.field.startswith('const char *'):
                event.print_fmt = str.replace(event.print_fmt, "REC->{}".format(field_name), "msg.{}().c_str()".format(field_name))
            else:
                event.print_fmt = str.replace(event.print_fmt, "REC->{}".format(field_name), "msg.{}()".format(field_name))
        if (event.name == "sched_switch"):
            f.write("    int len = 0;\n")
            f.write("    if (msg.prev_state() > 0x0400) {\n")
            f.write('        len = snprintf_s(buffer, BUFFER_SIZE, BUFFER_SIZE - 1, "sched_switch: prev_comm=%s prev_pid=%d prev_prio=%d prev_state=? ==> next_comm=%s next_pid=%d next_prio=%d", msg.prev_comm().c_str(), msg.prev_pid(), msg.prev_prio(), msg.next_comm().c_str(), msg.next_pid(), msg.next_prio());\n')
            f.write("    } else {\n")
            f.write('        len = snprintf_s(buffer, BUFFER_SIZE, BUFFER_SIZE - 1, "sched_switch: prev_comm=%s prev_pid=%d prev_prio=%d prev_state=%s%s ==> next_comm=%s next_pid=%d next_prio=%d", msg.prev_comm().c_str(), msg.prev_pid(), msg.prev_prio(), (msg.prev_state() & ((((0x0000 | 0x0001 | 0x0002 | 0x0004 | 0x0008 | 0x0010 | 0x0020 | 0x0040) + 1) << 1) - 1)) ? __print_flags(msg.prev_state() & ((((0x0000 | 0x0001 | 0x0002 | 0x0004 | 0x0008 | 0x0010 | 0x0020 | 0x0040) + 1) << 1) - 1), "|", { 0x0001, "S" }, { 0x0002, "D" }, { 0x0004, "T" }, { 0x0008, "t" }, { 0x0010, "X" }, { 0x0020, "Z" }, { 0x0040, "I" }, { 0x0080, "K" }, { 0x0100, "W" }, { 0x0200, "P" }, { 0x0400, "N" }) : "R", msg.prev_state() & (((0x0000 | 0x0001 | 0x0002 | 0x0004 | 0x0008 | 0x0010 | 0x0020 | 0x0040) + 1) << 1) ? "+" : "", msg.next_comm().c_str(), msg.next_pid(), msg.next_prio());\n')
            f.write("    }\n")
        elif (is_define_len) :
            f.write("    len = snprintf_s(buffer, BUFFER_SIZE, BUFFER_SIZE - 1, {});\n".format(event.print_fmt))
        else :
            f.write("    int len = snprintf_s(buffer, BUFFER_SIZE, BUFFER_SIZE - 1, {});\n".format(event.print_fmt))

    def handle_format_ps_functions(self, event, f, filed_name, is_void=False):
        f.write("    int len = 0;\n")
        f.write("    std::string functionStr = \"\";\n")
        f.write("    auto kernelSymbols = EventFormatter::GetInstance().kernelSymbols_;\n")
        f.write("    if (kernelSymbols.count(msg.{}()) > 0) ".format(filed_name))
        f.write("{\n")
        f.write("        functionStr = kernelSymbols[msg.{}()];\n".format(filed_name))
        f.write("    }\n")
        f.write("    if (functionStr != \"\") {\n        ")
        print_fmt = event.print_fmt
        event.print_fmt = str.replace(event.print_fmt, "%ps", "%s")
        if (is_void) :
            event.print_fmt = str.replace(event.print_fmt, "(void *)REC->{}".format(filed_name), "functionStr.c_str()")
        else :
            event.print_fmt = str.replace(event.print_fmt, "REC->{}".format(filed_name), "functionStr.c_str()")
        self.handle_field_name_functions(event, f)
        f.write("    } else {\n")
        event.print_fmt = print_fmt
        event.print_fmt = str.replace(event.print_fmt, "%ps", "%p")
        self.handle_field_name_functions(event, f)
        f.write("    }\n")

def main():
    parser = argparse.ArgumentParser(
        description='FTrace C++ code generator.')
    parser.add_argument('-a', dest='allow_list', required=True, type=str,
                        help='event allow list file path')
    parser.add_argument('-e', dest='events_dir', required=True, type=str,
                        help='event formats directory')
    parser.add_argument('-p', dest='parser_out', required=False, type=str,
                        help='parser code output directory')
    parser.add_argument('-f', dest='formatter_out', required=False, type=str,
                        help='formaater code output directory')

    args = parser.parse_args(sys.argv[1:])
    allow_list = args.allow_list
    events_dir = args.events_dir
    parser_out = args.parser_out
    formatter_out = args.formatter_out

    # check arguments
    if not os.path.isfile(allow_list):
        parser.print_usage()
        exit(1)
    if not os.path.isdir(events_dir):
        parser.print_usage()
        exit(2)

    if parser_out:
        if not os.path.isdir(parser_out):
            parser.print_usage()
            exit(3)
        parser_gen = EventParserCodeGenerator(events_dir, allow_list)
        parser_gen.generate(os.path.join(parser_out))
        sh_path = "../../../format-code.sh "
        if (os.getcwd().find("device_kernel_version") != -1):
            sh_path = "../../{}".format(sh_path)
        subprocess.run("{}{}".format(sh_path, parser_out), shell=True)
        subprocess.run("chmod 775 {}*.cpp".format(parser_out), shell=True)

    if formatter_out:
        if not os.path.isdir(formatter_out):
            parser.print_usage()
            exit(4)
        fmtter_gen = EventFormatterCodeGenerator(events_dir, allow_list)
        fmtter_gen.generate(formatter_out)
        sh_path = "../../../format-code.sh "
        if (os.getcwd().find("device_kernel_version") != -1):
            sh_path = "../../{}".format(sh_path)
        subprocess.run("{}{}".format(sh_path, formatter_out), shell=True)
        subprocess.run("chmod 775 {}*.cpp".format(,formatter_out), shell=True)


if __name__ == '__main__':
    main()

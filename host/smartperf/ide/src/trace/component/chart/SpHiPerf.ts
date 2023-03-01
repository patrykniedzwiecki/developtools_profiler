/*
 * Copyright (C) 2022 Huawei Device Co., Ltd.
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

import {SpSystemTrace} from "../SpSystemTrace.js";
import {TraceRow} from "../trace/base/TraceRow.js";
import {procedurePool} from "../../database/Procedure.js";
import {
    queryHiPerfCpuData,
    queryHiPerfCpuMergeData,
    queryHiPerfCpuMergeData2,
    queryHiPerfEventData,
    queryHiPerfEventList,
    queryHiPerfEventListData,
    queryHiPerfProcessData,
    queryHiPerfThreadData,
    queryPerfCmdline,
    queryPerfThread
} from "../../database/SqlLite.js";
import {Utils} from "../trace/base/Utils.js";
import {PerfThread} from "../../bean/PerfProfile.js";
import {HiperfCpuRender, HiPerfCpuStruct} from "../../database/ui-worker/ProcedureWorkerHiPerfCPU.js";
import {HiperfThreadRender, HiPerfThreadStruct} from "../../database/ui-worker/ProcedureWorkerHiPerfThread.js";
import {HiperfProcessRender, HiPerfProcessStruct} from "../../database/ui-worker/ProcedureWorkerHiPerfProcess.js";
import {info} from "../../../log/Log.js";
import {HiperfEventRender, HiPerfEventStruct} from "../../database/ui-worker/ProcedureWorkerHiPerfEvent.js";
import {perfDataQuery} from "./PerfDataQuery.js";
import {renders} from "../../database/ui-worker/ProcedureWorker.js";
import {CpuRender, EmptyRender} from "../../database/ui-worker/ProcedureWorkerCPU.js";
import {HiperfReportRender, HiPerfReportStruct} from "../../database/ui-worker/ProcedureWorkerHiPerfReport.js";
import {BaseStruct} from "../../database/ui-worker/ProcedureWorkerCommon.js";
import {ProcessRender} from "../../database/ui-worker/ProcedureWorkerProcess.js";

export interface ResultData {
    existA: boolean | null | undefined,
    existF: boolean | null | undefined,
    fValue: number
}

export class SpHiPerf {
    static selectCpuStruct: HiPerfCpuStruct | undefined;
    static selectProcessStruct: HiPerfProcessStruct | undefined;
    static selectThreadStruct: HiPerfThreadStruct | undefined;
    static stringResult: ResultData | undefined;

    private cpuData: Array<any> | undefined
    public maxCpuId: number = 0
    private rowFolder!: TraceRow<any>;
    private perfThreads: Array<PerfThread> | undefined;
    private trace: SpSystemTrace;
    private group: any;
    private eventTypeList: Array<{ id: number, report_value: string }> = [];

    constructor(trace: SpSystemTrace) {
        this.trace = trace;
    }

    async init() {
        await this.initCmdLine()
        this.perfThreads = await queryPerfThread();
        this.eventTypeList = await queryHiPerfEventList();
        info("PerfThread Data size is: ", this.perfThreads!.length)
        this.group = Utils.groupBy(this.perfThreads || [], "pid");
        this.cpuData = await queryHiPerfCpuMergeData2();
        this.maxCpuId = this.cpuData.length > 0 ? this.cpuData[0].cpu_id : -Infinity;
        if (this.cpuData.length > 0) {
            await this.initFolder();
            await this.initCpuMerge();
            await this.initCpu();
            // await this.initReport();
            await this.initProcess();
        }
        info("HiPerf Data initialized")
    }

    getStringResult(s: string = "") {
        let list = s.split(" ").filter((e) => e);
        let sA = list.findIndex((item) => item == "-a");
        let sF = list.findIndex((item) => item == "-f");
        SpHiPerf.stringResult = {
            existA: sA !== -1,
            existF: sF !== -1,
            fValue: Number((1000 / (sF !== -1 ? parseInt(list[sF + 1]) : 1000)).toFixed(1)),
        }
    }

    async initCmdLine() {
        let perfCmdLines = await queryPerfCmdline();
        if (perfCmdLines.length > 0) {
            this.getStringResult(perfCmdLines[0].report_value)
        } else {
            SpHiPerf.stringResult = {
                existA: true,
                existF: false,
                fValue: 1,
            }
        }
    }

    async initFolder() {
        let row = TraceRow.skeleton();
        row.setAttribute('disabled-check', '')
        row.rowId = `HiPerf`;
        row.index = 0;
        row.rowType = TraceRow.ROW_TYPE_HIPERF
        row.rowParentId = '';
        row.folder = true;
        row.style.height = '40px'
        if (SpHiPerf.stringResult?.existA === true) {
            row.name = `HiPerf (All)`;
        } else {
            let names = Reflect.ownKeys(this.group).map((pid: any) => {
                let array = this.group[pid] as Array<PerfThread>;
                let process = array.filter(th => th.pid === th.tid)[0];
                return process.processName;
            }).join(',');
            row.name = `HiPerf (${names})`;
        }
        row.supplier = () => new Promise<Array<any>>((resolve) => resolve([]));
        row.onThreadHandler = (useCache) => {
            row.canvasSave(this.trace.canvasPanelCtx!);
            if (row.expansion) {
                this.trace.canvasPanelCtx?.clearRect(0, 0, row.frame.width, row.frame.height);
            } else {
                (renders["empty"] as EmptyRender).renderMainThread(
                    {
                        context: this.trace.canvasPanelCtx,
                        useCache: useCache,
                        type: ``,
                    },
                    row,
                );
            }
            row.canvasRestore(this.trace.canvasPanelCtx!);
        }
        this.rowFolder = row;
        this.trace.rowsEL?.appendChild(row)
    }

    async initCpuMerge() {
        let row = TraceRow.skeleton<HiPerfCpuStruct>()
        row.rowId = `HiPerf-cpu-merge`;
        row.index = 0;
        row.rowType = TraceRow.ROW_TYPE_HIPERF_CPU
        row.rowParentId = 'HiPerf';
        row.rowHidden = !this.rowFolder.expansion
        row.folder = false;
        row.name = `HiPerf`;
        row.style.height = '40px'
        row.setAttribute('children', '')
        row.favoriteChangeHandler = this.trace.favoriteChangeHandler;
        row.selectChangeHandler = this.trace.selectChangeHandler;
        row.supplier = () => queryHiPerfCpuMergeData();
        row.focusHandler = () => this.hoverTip(row, HiPerfCpuStruct.hoverStruct);
        row.onThreadHandler = (useCache) => {
            let context = row.collect ? this.trace.canvasFavoritePanelCtx! : this.trace.canvasPanelCtx!;
            row.canvasSave(context);
            (renders["HiPerf-Cpu"] as HiperfCpuRender).renderMainThread(
                {
                    context: context,
                    useCache: useCache,
                    scale: TraceRow.range?.scale || 50,
                    type: `HiPerf-Cpu-Merge`,
                    maxCpu: (this.maxCpuId + 1),
                    intervalPerf: SpHiPerf.stringResult?.fValue || 1,
                    range: TraceRow.range,
                },
                row
            );
            row.canvasRestore(context);
        }
        this.trace.rowsEL?.appendChild(row)
    }

    async initCpu() {
        for (let i = 0; i <= this.maxCpuId; i++) {
            let row = TraceRow.skeleton<HiPerfCpuStruct>()
            row.rowId = `HiPerf-cpu-${i}`;
            row.index = i;
            row.rowType = TraceRow.ROW_TYPE_HIPERF_CPU
            row.rowParentId = 'HiPerf';
            row.rowHidden = !this.rowFolder.expansion
            row.folder = false;
            row.name = `Cpu ${i}`;
            row.setAttribute('children', '')
            row.favoriteChangeHandler = this.trace.favoriteChangeHandler;
            row.selectChangeHandler = this.trace.selectChangeHandler;
            row.style.height = '40px'
            row.supplier = () => queryHiPerfCpuData(i);
            row.focusHandler = () => this.hoverTip(row, HiPerfCpuStruct.hoverStruct);
            row.onThreadHandler = (useCache) => {
                let context = row.collect ? this.trace.canvasFavoritePanelCtx! : this.trace.canvasPanelCtx!;
                row.canvasSave(context);
                (renders["HiPerf-Cpu"] as HiperfCpuRender).renderMainThread(
                    {
                        context: context,
                        useCache: useCache,
                        scale: TraceRow.range?.scale || 50,
                        type: `HiPerf-Cpu-${i}`,
                        maxCpu: (this.maxCpuId + 1),
                        intervalPerf: SpHiPerf.stringResult?.fValue || 1,
                        range: TraceRow.range,
                    },
                    row
                );
                row.canvasRestore(context);
            }
            this.trace.rowsEL?.appendChild(row)
        }
    }

    async initReport() {
        this.eventTypeList.forEach((it, index) => {
            let fold = TraceRow.skeleton<HiPerfReportStruct>()
            fold.rowId = `Perf-Report-${it.id}-${it.report_value}`;
            fold.index = index;
            fold.rowType = TraceRow.ROW_TYPE_HIPERF_REPORT
            fold.rowParentId = 'HiPerf';
            fold.rowHidden = !this.rowFolder.expansion
            fold.folder = true;
            fold.name = `Event :${it.report_value}`;
            fold.folderPaddingLeft = 30;
            fold.favoriteChangeHandler = this.trace.favoriteChangeHandler;
            fold.selectChangeHandler = this.trace.selectChangeHandler;
            fold.supplier = () => queryHiPerfEventListData(it.id);
            fold.onThreadHandler = (useCache) => {
                let context = fold.collect ? this.trace.canvasFavoritePanelCtx! : this.trace.canvasPanelCtx!;
                fold.canvasSave(context);
                (renders["HiPerf-Report-Fold"] as HiperfReportRender).renderMainThread(
                    {
                        context: context,
                        useCache: useCache,
                        scale: TraceRow.range?.scale || 50,
                        type: `HiPerf-Report-Fold-${it.report_value}-${it.id}`,
                        maxCpu: (this.maxCpuId + 1),
                        intervalPerf: SpHiPerf.stringResult?.fValue || 1,
                        range: TraceRow.range,
                    },
                    fold
                );
                fold.canvasRestore(context);
            }
            this.trace.rowsEL?.appendChild(fold)
            for (let i = 0; i <= this.maxCpuId; i++) {
                let row = TraceRow.skeleton<HiPerfEventStruct>();
                row.rowId = `HiPerf-Report-Event-${it.report_value}-${i}`;
                row.index = i;
                row.rowType = TraceRow.ROW_TYPE_HIPERF_EVENT
                row.rowParentId = fold.rowId;
                row.rowHidden = !fold.expansion
                row.folder = false;
                row.name = `Cpu ${i}`;
                row.style.height = '40px'
                row.setAttribute('children', '')
                row.favoriteChangeHandler = this.trace.favoriteChangeHandler;
                row.selectChangeHandler = this.trace.selectChangeHandler;
                row.supplier = () => queryHiPerfEventData(it.id, row.index);
                row.focusHandler = () => this.hoverTip(row, HiPerfEventStruct.hoverStruct)
                row.onThreadHandler = (useCache) => {
                    let context = fold.collect ? this.trace.canvasFavoritePanelCtx! : this.trace.canvasPanelCtx!;
                    fold.canvasSave(context);
                    (renders["HiPerf-Report-Event"] as HiperfEventRender).renderMainThread(
                        {
                            context: context,
                            useCache: useCache,
                            scale: TraceRow.range?.scale || 50,
                            type: `HiPerf-Report-Event-${it.report_value}-${i}`,
                            maxCpu: (this.maxCpuId + 1),
                            intervalPerf: SpHiPerf.stringResult?.fValue || 1,
                            range: TraceRow.range,
                        },
                        row
                    );
                    fold.canvasRestore(context);
                }
                this.trace.rowsEL?.appendChild(row)
            }
        })
    }

    async initProcess() {
        Reflect.ownKeys(this.group).forEach((key, index) => {
            let array = this.group[key] as Array<PerfThread>;
            let process = array.filter(th => th.pid === th.tid)[0];
            let row = TraceRow.skeleton<HiPerfProcessStruct>();
            row.rowId = `${process.pid}-Perf-Process`;
            row.index = index;
            row.rowType = TraceRow.ROW_TYPE_HIPERF_PROCESS
            row.rowParentId = 'HiPerf';
            row.rowHidden = !this.rowFolder.expansion
            row.folder = true;
            row.name = `${process.processName || 'Process'} [${process.pid}]`;
            row.folderPaddingLeft = 30;
            row.style.height = '40px'
            row.favoriteChangeHandler = this.trace.favoriteChangeHandler;
            row.selectChangeHandler = this.trace.selectChangeHandler;
            row.supplier = () => queryHiPerfProcessData(process.pid);
            row.focusHandler = () => this.hoverTip(row, HiPerfProcessStruct.hoverStruct);
            row.onThreadHandler = (useCache) => {
                let context = this.trace.canvasPanelCtx!;
                row.canvasSave(context);
                if (row.expansion) {
                    this.trace.canvasPanelCtx?.clearRect(0, 0, row.frame.width, row.frame.height);
                } else {
                    (renders["HiPerf-Process"] as HiperfProcessRender).renderMainThread(
                        {
                            context: context,
                            useCache: useCache,
                            scale: TraceRow.range?.scale || 50,
                            type: `HiPerf-Process-${row.index}`,
                            intervalPerf: SpHiPerf.stringResult?.fValue || 1,
                            range: TraceRow.range,
                        },
                        row
                    );
                }
                row.canvasRestore(context);
            }
            this.trace.rowsEL?.appendChild(row)
            array.forEach((thObj, thIdx) => {
                let thread = TraceRow.skeleton<HiPerfThreadStruct>()
                thread.rowId = `${thObj.tid}-Perf-Thread`;
                thread.index = thIdx;
                thread.rowType = TraceRow.ROW_TYPE_HIPERF_THREAD
                thread.rowParentId = row.rowId;
                thread.rowHidden = !row.expansion
                thread.folder = false;
                thread.name = `${thObj.threadName || 'Thread'} [${thObj.tid}]`;
                thread.setAttribute('children', '')
                thread.folderPaddingLeft = 30;
                thread.style.height = '40px'
                thread.favoriteChangeHandler = this.trace.favoriteChangeHandler;
                thread.selectChangeHandler = this.trace.selectChangeHandler;
                thread.supplier = () => queryHiPerfThreadData(thObj.tid);
                thread.focusHandler = () => this.hoverTip(thread, HiPerfThreadStruct.hoverStruct);
                thread.onThreadHandler = (useCache) => {
                    let context = thread.collect ? this.trace.canvasFavoritePanelCtx! : this.trace.canvasPanelCtx!;
                    thread.canvasSave(context);
                    (renders["HiPerf-Thread"] as HiperfThreadRender).renderMainThread(
                        {
                            context: context,
                            useCache: useCache,
                            scale: TraceRow.range?.scale || 50,
                            type: `HiPerf-Thread-${row.index}-${thread.index}`,
                            intervalPerf: SpHiPerf.stringResult?.fValue || 1,
                            range: TraceRow.range,
                        },
                        thread
                    );
                    thread.canvasRestore(context);
                }
                this.trace.rowsEL?.appendChild(thread)
            });
        })
    }

    hoverTip(row: TraceRow<any>, struct: HiPerfThreadStruct | HiPerfProcessStruct | HiPerfEventStruct | HiPerfReportStruct | HiPerfCpuStruct | undefined) {
        let tip = "";
        if (struct) {
            let num = 0;
            if (struct instanceof HiPerfEventStruct) {
                num = Math.trunc((struct.sum || 0) / (struct.max || 0) * 100);
            } else {
                num = Math.trunc((struct.height || 0) / 40 * 100);
            }
            if (num > 0) {
                tip = `<span>${num * (this.maxCpuId + 1)}% (10.00ms)</span>`
            } else {
                let perfCall = perfDataQuery.callChainMap.get(struct.callchain_id || 0);
                tip = `<span>${perfCall ? perfCall.name : ''} (${perfCall ? perfCall.depth : '0'} other frames)</span>`
            }
        }
        this.trace?.displayTip(row, struct, tip)
    }
}

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
    queryHiPerfProcessData,
    queryHiPerfThreadData,
    queryPerfCmdline,
    queryPerfThread
} from "../../database/SqlLite.js";
import {Utils} from "../trace/base/Utils.js";
import {PerfThread} from "../../bean/PerfProfile.js";
import {HiPerfCpuStruct} from "../../database/ProcedureWorkerHiPerfCPU.js";
import {HiPerfThreadStruct} from "../../database/ProcedureWorkerHiPerfThread.js";
import {HiPerfProcessStruct} from "../../database/ProcedureWorkerHiPerfProcess.js";
import {PerfDataQuery} from "./PerfDataQuery.js";

export interface ResultData {
    existA: boolean | null | undefined,
    existF: boolean | null | undefined,
    fValue: string | null | undefined,
}

export class SpHiPerf {
    static hoverCpuStruct: HiPerfCpuStruct | undefined;
    static selectCpuStruct: HiPerfCpuStruct | undefined;
    static hoverProcessStruct: HiPerfProcessStruct | undefined;
    static selectProcessStruct: HiPerfProcessStruct | undefined;
    static hoverThreadStruct: HiPerfThreadStruct | undefined;
    static selectThreadStruct: HiPerfThreadStruct | undefined;
    static stringResult: ResultData | undefined;

    private cpuData: Array<any> | undefined
    public maxCpuId: number = 0
    private rowFolder!: TraceRow<any>;
    private perfThreads: Array<PerfThread> | undefined;
    private trace: SpSystemTrace;
    private group: any;

    constructor(trace: SpSystemTrace) {
        this.trace = trace;
    }

    async init() {
        await this.initCmdLine()
        this.perfThreads = await queryPerfThread();
        this.group = Utils.groupBy(this.perfThreads || [], "pid");
        this.cpuData = await queryHiPerfCpuMergeData();
        this.maxCpuId = this.cpuData.length > 0 ? this.cpuData.reduce((max, v) => max.cpu_id >= v.cpu_id ? max : v).cpu_id : -Infinity;
        if (this.cpuData.length > 0) {
            await this.initFolder();
            await this.initCpuMerge();
            await this.initCpu();
            await this.initProcess();
        }
    }

    getStringResult(s:string = ""){
        let list = s.split(" ").filter((e) => e);
        let sA = list.findIndex((item) => item == "-a");
        let sF = list.findIndex((item) => item == "-f");
        SpHiPerf.stringResult = {
            existA: sA!==-1,
            existF: sF!==-1,
            fValue: sF!==-1?list[sF + 1]:"1000",
        }
    }

    async initCmdLine(){
        let perfCmdLines = await queryPerfCmdline();
        if(perfCmdLines.length > 0){
            this.getStringResult(perfCmdLines[0].report_value)
        }else {
            SpHiPerf.stringResult = {
                existA: true,
                existF: false,
                fValue: "1000",
            }
        }
    }

    async initFolder() {
        let row = new TraceRow({
            canvasNumber: 1,
            alpha: false,
            contextId: '2d',
            isOffScreen: SpSystemTrace.isCanvasOffScreen
        });
        row.setAttribute('disabled-check', '')
        row.rowId = `HiPerf`;
        row.index = 0;
        row.rowType = TraceRow.ROW_TYPE_HIPERF
        row.rowParentId = '';
        row.folder = true;
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
            procedurePool.submitWithName(`process${row.index}`, `HiPerf-Group`, {
                list: row.must ? row.dataList : undefined,
                offscreen: row.must ? row.offscreen[0] : undefined,
                xs: TraceRow.range?.xs,
                dpr: row.dpr,
                isHover: row.isHover,
                flagMoveInfo: this.trace.hoverFlag,
                flagSelectedInfo: this.trace.selectFlag,
                hoverX: row.hoverX,
                hoverY: row.hoverY,
                canvasWidth: row.canvasWidth,
                canvasHeight: row.canvasHeight,
                isRangeSelect: row.rangeSelect,
                rangeSelectObject: TraceRow.rangeSelectObject,
                useCache: useCache,
                lineColor: row.getLineColor(),
                startNS: TraceRow.range?.startNS || 0,
                endNS: TraceRow.range?.endNS || 0,
                totalNS: TraceRow.range?.totalNS || 0,
                slicesTime: TraceRow.range?.slicesTime,
                scale: TraceRow.range?.scale || 50,
                frame: row.frame
            }, row.must && row.args.isOffScreen ? row.offscreen[0] : undefined, (res: any) => {
                row.must = false;
            })
        }
        this.rowFolder = row;
        this.trace.rowsEL?.appendChild(row)
    }

    async initCpuMerge() {
        let row = new TraceRow({
            canvasNumber: 1,
            alpha: false,
            contextId: '2d',
            isOffScreen: SpSystemTrace.isCanvasOffScreen
        });
        row.rowId = `HiPerf-cpu-merge`;
        row.index = 0;
        row.rowType = TraceRow.ROW_TYPE_HIPERF_CPU
        row.rowParentId = 'HiPerf';
        row.rowHidden = !this.rowFolder.expansion
        row.folder = false;
        row.name = `HiPerf`;
        row.setAttribute('children', '')
        row.favoriteChangeHandler = this.trace.favoriteChangeHandler;
        row.selectChangeHandler = this.trace.selectChangeHandler;
        let that = this;
        row.supplier = () => queryHiPerfCpuMergeData();
        row.onThreadHandler = (useCache) => {
            procedurePool.submitWithName(`freq${row.index}`, `HiPerf-Cpu-Merge`, {
                list: row.must ? row.dataList : undefined,
                offscreen: row.must ? row.offscreen[0] : undefined,
                xs: TraceRow.range?.xs,
                dpr: row.dpr,
                isHover: row.isHover,
                flagMoveInfo: this.trace.hoverFlag,
                flagSelectedInfo: this.trace.selectFlag,
                hoverX: row.hoverX,
                hoverY: row.hoverY,
                canvasWidth: row.canvasWidth,
                canvasHeight: row.canvasHeight,
                hoverStruct: SpHiPerf.hoverCpuStruct,
                selectStruct: SpHiPerf.selectCpuStruct,
                isRangeSelect: row.rangeSelect,
                rangeSelectObject: TraceRow.rangeSelectObject,
                useCache: useCache,
                lineColor: row.getLineColor(),
                startNS: TraceRow.range?.startNS || 0,
                endNS: TraceRow.range?.endNS || 0,
                totalNS: TraceRow.range?.totalNS || 0,
                slicesTime: TraceRow.range?.slicesTime,
                scale: TraceRow.range?.scale || 50,
                frame: row.frame,
                maxCpu: (this.maxCpuId + 1)
            }, row.must && row.args.isOffScreen ? row.offscreen[0] : undefined, (res: any, hover: any) => {
                row.must = false;
                if (row.isHover) {
                    SpHiPerf.hoverCpuStruct = hover;
                    // this.trace.visibleRows.filter(it => it.rowType === TraceRow.ROW_TYPE_HIPERF_CPU && it.name !== row.name).forEach(it => it.draw(true));
                }
            })
        }
        this.trace.rowsEL?.appendChild(row)
    }

    async initCpu() {
        for (let i = 0; i <= this.maxCpuId; i++) {
            let row = new TraceRow({
                canvasNumber: 1,
                alpha: false,
                contextId: '2d',
                isOffScreen: SpSystemTrace.isCanvasOffScreen
            });
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
            let that = this;
            row.supplier = () => queryHiPerfCpuData(i);
            row.onThreadHandler = (useCache) => {
                procedurePool.submitWithName(`cpu${row.index}`, `HiPerf-Cpu-${i}`, {
                    list: row.must ? row.dataList : undefined,
                    offscreen: row.must ? row.offscreen[0] : undefined,
                    xs: TraceRow.range?.xs,
                    dpr: row.dpr,
                    isHover: row.isHover,
                    flagMoveInfo: this.trace.hoverFlag,
                    flagSelectedInfo: this.trace.selectFlag,
                    hoverX: row.hoverX,
                    hoverY: row.hoverY,
                    canvasWidth: row.canvasWidth,
                    canvasHeight: row.canvasHeight,
                    hoverStruct: SpHiPerf.hoverCpuStruct,
                    selectStruct: SpHiPerf.selectCpuStruct,
                    isRangeSelect: row.rangeSelect,
                    rangeSelectObject: TraceRow.rangeSelectObject,
                    useCache: useCache,
                    lineColor: row.getLineColor(),
                    startNS: TraceRow.range?.startNS || 0,
                    endNS: TraceRow.range?.endNS || 0,
                    totalNS: TraceRow.range?.totalNS || 0,
                    slicesTime: TraceRow.range?.slicesTime,
                    scale: TraceRow.range?.scale || 50,
                    frame: row.frame,
                    maxCpu: undefined
                }, row.must && row.args.isOffScreen ? row.offscreen[0] : undefined, (res: any, hover: any) => {
                    row.must = false;
                    if (row.isHover) {
                        SpHiPerf.hoverCpuStruct = hover;
                        // this.trace.visibleRows.filter(it => it.rowType === TraceRow.ROW_TYPE_HIPERF_CPU && it.name !== row.name).forEach(it => it.draw(true));
                    }
                })
            }
            this.trace.rowsEL?.appendChild(row)
        }
    }

    async initProcess() {
        Reflect.ownKeys(this.group).forEach((key, index) => {
            let array = this.group[key] as Array<PerfThread>;
            let process = array.filter(th => th.pid === th.tid)[0];
            let row = new TraceRow({
                canvasNumber: 1,
                alpha: false,
                contextId: '2d',
                isOffScreen: SpSystemTrace.isCanvasOffScreen
            });
            row.rowId = `${process.pid}-Perf-Process`;
            row.index = index;
            row.rowType = TraceRow.ROW_TYPE_HIPERF_PROCESS
            row.rowParentId = 'HiPerf';
            row.rowHidden = !this.rowFolder.expansion
            row.folder = true;
            row.name = `${process.processName} [${process.pid}]`;
            row.folderPaddingLeft = 30;
            row.favoriteChangeHandler = this.trace.favoriteChangeHandler;
            row.selectChangeHandler = this.trace.selectChangeHandler;
            let that = this;
            row.supplier = () => queryHiPerfProcessData(process.pid);
            row.onThreadHandler = (useCache) => {
                procedurePool.submitWithName(`process${(row.index) % procedurePool.processLen.length}`, `HiPerf-Process-${row.index}`, {
                    list: row.must ? row.dataList : undefined,
                    offscreen: row.must ? row.offscreen[0] : undefined,
                    xs: TraceRow.range?.xs,
                    dpr: row.dpr,
                    isHover: row.isHover,
                    flagMoveInfo: this.trace.hoverFlag,
                    flagSelectedInfo: this.trace.selectFlag,
                    hoverX: row.hoverX,
                    hoverY: row.hoverY,
                    canvasWidth: row.canvasWidth,
                    canvasHeight: row.canvasHeight,
                    hoverStruct: SpHiPerf.hoverProcessStruct,
                    selectStruct: SpHiPerf.selectProcessStruct,
                    isRangeSelect: row.rangeSelect,
                    rangeSelectObject: TraceRow.rangeSelectObject,
                    useCache: useCache,
                    lineColor: row.getLineColor(),
                    startNS: TraceRow.range?.startNS || 0,
                    endNS: TraceRow.range?.endNS || 0,
                    totalNS: TraceRow.range?.totalNS || 0,
                    slicesTime: TraceRow.range?.slicesTime,
                    scale: TraceRow.range?.scale || 50,
                    frame: row.frame
                }, row.must && row.args.isOffScreen ? row.offscreen[0] : undefined, (res: any, hover: any) => {
                    row.must = false;
                    if (row.isHover) {
                        SpHiPerf.hoverProcessStruct = hover;
                        // this.trace.visibleRows.filter(it => it.rowType === TraceRow.ROW_TYPE_HIPERF_PROCESS && it.name !== row.name).forEach(it => it.draw(true));
                    }
                })
            }
            this.trace.rowsEL?.appendChild(row)

            array.forEach((thObj, thIdx) => {
                let thread = new TraceRow({
                    canvasNumber: 1,
                    alpha: false,
                    contextId: '2d',
                    isOffScreen: SpSystemTrace.isCanvasOffScreen
                });
                thread.rowId = `${thObj.tid}-Perf-Thread`;
                thread.index = thIdx;
                thread.rowType = TraceRow.ROW_TYPE_HIPERF_THREAD
                thread.rowParentId = row.rowId;
                thread.rowHidden = !row.expansion
                thread.folder = false;
                thread.name = `${thObj.threadName} [${thObj.tid}]`;
                thread.setAttribute('children', '')
                thread.folderPaddingLeft = 30;
                thread.favoriteChangeHandler = this.trace.favoriteChangeHandler;
                thread.selectChangeHandler = this.trace.selectChangeHandler;
                let that = this;
                thread.supplier = () => queryHiPerfThreadData(thObj.tid);
                thread.onThreadHandler = (useCache) => {
                    procedurePool.submitWithName(`process${(thread.index) % procedurePool.processLen.length}`, `HiPerf-Thread-${row.index}-${thread.index}`, {
                        list: thread.must ? thread.dataList : undefined,
                        offscreen: thread.must ? thread.offscreen[0] : undefined,
                        xs: TraceRow.range?.xs,
                        dpr: thread.dpr,
                        isHover: thread.isHover,
                        flagMoveInfo: this.trace.hoverFlag,
                        flagSelectedInfo: this.trace.selectFlag,
                        hoverX: thread.hoverX,
                        hoverY: thread.hoverY,
                        canvasWidth: thread.canvasWidth,
                        canvasHeight: thread.canvasHeight,
                        hoverStruct: SpHiPerf.hoverThreadStruct,
                        selectStruct: SpHiPerf.selectThreadStruct,
                        isRangeSelect: thread.rangeSelect,
                        rangeSelectObject: TraceRow.rangeSelectObject,
                        useCache: useCache,
                        lineColor: thread.getLineColor(),
                        startNS: TraceRow.range?.startNS || 0,
                        endNS: TraceRow.range?.endNS || 0,
                        totalNS: TraceRow.range?.totalNS || 0,
                        slicesTime: TraceRow.range?.slicesTime,
                        scale: TraceRow.range?.scale || 50,
                        frame: thread.frame
                    }, thread.must && thread.args.isOffScreen ? thread.offscreen[0] : undefined, (res: any, hover: any) => {
                        thread.must = false;
                        if (thread.isHover) {
                            SpHiPerf.hoverThreadStruct = hover;
                        }
                    })
                }
                this.trace.rowsEL?.appendChild(thread)
            });
        })
    }
}
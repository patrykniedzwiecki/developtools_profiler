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

import { ColorUtils } from '../../component/trace/base/ColorUtils.js';
import {
    BaseStruct,
    dataFilterHandler,
    drawFlagLine,
    drawLines,
    drawLoading,
    drawSelection,
    drawWakeUp,
    isFrameContainPoint,
    ns2x,
    Render,
    RequestMessage,
} from './ProcedureWorkerCommon.js';
import { TraceRow } from '../../component/trace/base/TraceRow.js';
import { DiskAbilityMonitorStruct } from './ProcedureWorkerDiskIoAbility.js';

export class CpuAbilityRender extends Render {
    renderMainThread(
        req: {
            context: CanvasRenderingContext2D;
            useCache: boolean;
            type: string;
            maxCpuUtilization: number;
            maxCpuUtilizationName: string;
        },
        row: TraceRow<CpuAbilityMonitorStruct>
    ) {
        let list = row.dataList;
        let filter = row.dataListCache;
        dataFilterHandler(list, filter, {
            startKey: 'startNS',
            durKey: 'dur',
            startNS: TraceRow.range?.startNS ?? 0,
            endNS: TraceRow.range?.endNS ?? 0,
            totalNS: TraceRow.range?.totalNS ?? 0,
            frame: row.frame,
            paddingTop: 5,
            useCache: req.useCache || !(TraceRow.range?.refresh ?? false),
        });
        req.context.beginPath();
        let find = false;
        for (let re of filter) {
            CpuAbilityMonitorStruct.draw(
                req.context,
                re,
                req.maxCpuUtilization,
                row.isHover
            );
            if (
                row.isHover &&
                re.frame &&
                isFrameContainPoint(re.frame, row.hoverX, row.hoverY)
            ) {
                CpuAbilityMonitorStruct.hoverCpuAbilityStruct = re;
                find = true;
            }
        }
        if (!find && row.isHover)
            CpuAbilityMonitorStruct.hoverCpuAbilityStruct = undefined;
        req.context.closePath();
        let textMetrics = req.context.measureText(req.maxCpuUtilizationName);
        req.context.globalAlpha = 0.8;
        req.context.fillStyle = '#f0f0f0';
        req.context.fillRect(0, 5, textMetrics.width + 8, 18);
        req.context.globalAlpha = 1;
        req.context.fillStyle = '#333';
        req.context.textBaseline = 'middle';
        req.context.fillText(req.maxCpuUtilizationName, 4, 5 + 9);
    }

    render(req: RequestMessage, list: Array<any>, filter: Array<any>) {}
}

export function cpuAbility(
    list: Array<any>,
    res: Array<any>,
    startNS: number,
    endNS: number,
    totalNS: number,
    frame: any,
    use: boolean
) {
    if (use && res.length > 0) {
        for (let index = 0; index < res.length; index++) {
            let item = res[index];
            if (
                (item.startNS || 0) + (item.dur || 0) > (startNS || 0) &&
                (item.startNS || 0) < (endNS || 0)
            ) {
                CpuAbilityMonitorStruct.setCpuAbilityFrame(
                    res[index],
                    5,
                    startNS || 0,
                    endNS || 0,
                    totalNS || 0,
                    frame
                );
            } else {
                res[index].frame = null;
            }
        }
        return;
    }
    res.length = 0;
    if (list) {
        for (let index = 0; index < list.length; index++) {
            let item = list[index];
            if (index === list.length - 1) {
                item.dur = (endNS || 0) - (item.startNS || 0);
            } else {
                item.dur = (list[index + 1].startNS || 0) - (item.startNS || 0);
            }
            if (
                (item.startNS || 0) + (item.dur || 0) > (startNS || 0) &&
                (item.startNS || 0) < (endNS || 0)
            ) {
                CpuAbilityMonitorStruct.setCpuAbilityFrame(
                    list[index],
                    5,
                    startNS || 0,
                    endNS || 0,
                    totalNS || 0,
                    frame
                );
                if (
                    index > 0 &&
                    (list[index - 1].frame?.x || 0) ==
                        (list[index].frame?.x || 0) &&
                    (list[index - 1].frame?.width || 0) ==
                        (list[index].frame?.width || 0)
                ) {
                } else {
                    res.push(item);
                }
            }
        }
    }
}

export class CpuAbilityMonitorStruct extends BaseStruct {
    static maxCpuUtilization: number = 0;
    static maxCpuUtilizationName: string = '0 %';
    static hoverCpuAbilityStruct: CpuAbilityMonitorStruct | undefined;
    static selectCpuAbilityStruct: CpuAbilityMonitorStruct | undefined;

    type: number | undefined;
    value: number | undefined;
    startNS: number | undefined;
    dur: number | undefined; //自补充，数据库没有返回

    static draw(
        context2D: CanvasRenderingContext2D,
        data: CpuAbilityMonitorStruct,
        maxCpuUtilization: number,
        isHover: boolean
    ) {
        if (data.frame) {
            let width = data.frame.width || 0;
            let index = 2;
            context2D.fillStyle = ColorUtils.colorForTid(index);
            context2D.strokeStyle = ColorUtils.colorForTid(index);
            if (
                data.startNS ===
                    CpuAbilityMonitorStruct.hoverCpuAbilityStruct?.startNS &&
                isHover
            ) {
                context2D.lineWidth = 1;
                context2D.globalAlpha = 0.6;
                let drawHeight: number = Math.floor(
                    ((data.value || 0) * (data.frame.height || 0) * 1.0) /
                        maxCpuUtilization
                );
                context2D.fillRect(
                    data.frame.x,
                    data.frame.y + data.frame.height - drawHeight + 4,
                    width,
                    drawHeight
                );
                context2D.beginPath();
                context2D.arc(
                    data.frame.x,
                    data.frame.y + data.frame.height - drawHeight + 4,
                    3,
                    0,
                    2 * Math.PI,
                    true
                );
                context2D.fill();
                context2D.globalAlpha = 1.0;
                context2D.stroke();
                context2D.beginPath();
                context2D.moveTo(
                    data.frame.x + 3,
                    data.frame.y + data.frame.height - drawHeight + 4
                );
                context2D.lineWidth = 3;
                context2D.lineTo(
                    data.frame.x + width,
                    data.frame.y + data.frame.height - drawHeight + 4
                );
                context2D.stroke();
            } else {
                context2D.globalAlpha = 0.6;
                context2D.lineWidth = 1;
                let drawHeight: number = Math.floor(
                    ((data.value || 0) * (data.frame.height || 0)) /
                        maxCpuUtilization
                );
                context2D.fillRect(
                    data.frame.x,
                    data.frame.y + data.frame.height - drawHeight + 4,
                    width,
                    drawHeight
                );
            }
        }
        context2D.globalAlpha = 1.0;
        context2D.lineWidth = 1;
    }

    static setCpuAbilityFrame(
        node: any,
        padding: number,
        startNS: number,
        endNS: number,
        totalNS: number,
        frame: any
    ) {
        let startPointX: number, endPointX: number;

        if ((node.startNS || 0) < startNS) {
            startPointX = 0;
        } else {
            startPointX = ns2x(
                node.startNS || 0,
                startNS,
                endNS,
                totalNS,
                frame
            );
        }
        if ((node.startNS || 0) + (node.dur || 0) > endNS) {
            endPointX = frame.width;
        } else {
            endPointX = ns2x(
                (node.startNS || 0) + (node.dur || 0),
                startNS,
                endNS,
                totalNS,
                frame
            );
        }
        let frameWidth: number =
            endPointX - startPointX <= 1 ? 1 : endPointX - startPointX;
        if (!node.frame) {
            node.frame = {};
        }
        node.frame.x = Math.floor(startPointX);
        node.frame.y = frame.y + padding;
        node.frame.width = Math.ceil(frameWidth);
        node.frame.height = Math.floor(frame.height - padding * 2);
    }
}

export class CpuAbility {
    context: any;
    params: any;
}

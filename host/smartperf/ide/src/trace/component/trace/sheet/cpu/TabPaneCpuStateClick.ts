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

import { BaseElement, element } from '../../../../../base-ui/BaseElement.js';
import { LitTable } from '../../../../../base-ui/table/lit-table.js';
import { Utils } from '../../base/Utils.js';

@element('tabpane-cpu-state-click')
export class TabPaneCpuStateClick extends BaseElement {
    private tbl: LitTable | null | undefined;

    set data(val: any) {
        if (val) {
            this.tbl!.dataSource = [
                {
                    startNS: Utils.getTimeString(val.startTs),
                    absoluteTime:
                        (val.startTs + (window as any).recordStartNS) /
                        1000000000,
                    dur: Utils.getProbablyTime(val.dur),
                    state: val.value,
                    cpu: `Cpu ${val.cpu}`,
                },
            ];
        }
    }

    initElements(): void {
        this.tbl = this.shadowRoot?.querySelector<LitTable>('#tb-freq');
    }

    connectedCallback() {
        super.connectedCallback();
        new ResizeObserver((entries) => {
            if (this.parentElement?.clientHeight != 0) {
                // @ts-ignore
                this.tbl?.shadowRoot.querySelector('.table').style.height = this.parentElement.clientHeight - 45 + 'px';
                this.tbl?.reMeauseHeight();
            }
        }).observe(this.parentElement!);
    }

    initHtml(): string {
        return `
        <style>
        :host{
            display: flex;
            flex-direction: column;
            padding: 10px 10px;
        }
        </style>
        <lit-table id="tb-freq" style="height: auto">
            <lit-table-column width="1fr" title="StartTime(Relative)" data-index="startNS" key="startNS" align="flex-start">
            </lit-table-column>
            <lit-table-column width="1fr" title="StartTime(Absolute)" data-index="absoluteTime" key="absoluteTime" align="flex-start">
            </lit-table-column>
            <lit-table-column width="1fr" title="Duration" data-index="dur" key="dur" align="flex-start" >
            </lit-table-column>
            <lit-table-column width="1fr" title="Cpu" data-index="cpu" key="cpu" align="flex-start" >
            </lit-table-column>
            <lit-table-column width="1fr" title="State" data-index="state" key="state" align="flex-start" >
            </lit-table-column>
        </lit-table>
        `;
    }
}

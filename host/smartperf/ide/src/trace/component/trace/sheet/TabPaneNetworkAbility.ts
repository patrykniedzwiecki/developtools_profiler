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

import {BaseElement, element} from "../../../../base-ui/BaseElement.js";
import {LitTable} from "../../../../base-ui/table/lit-table.js";
import {SelectionParam} from "../../../bean/BoxSelection.js";
import {getTabNetworkAbilityData} from "../../../database/SqlLite.js";
import {SystemNetworkSummary} from "../../../bean/AbilityMonitor.js";
import {Utils} from "../base/Utils.js";
import "../../../component/SpFilter.js";
import {ColorUtils} from "../base/ColorUtils.js";

@element('tabpane-network-ability')
export class TabPaneNetworkAbility extends BaseElement {
    private tbl: LitTable | null | undefined;
    private source: Array<SystemNetworkSummary> = [];
    private float: HTMLDivElement | null | undefined;
    private queryResult: Array<SystemNetworkSummary> = []
    private search: HTMLInputElement | undefined | null

    set data(val: SelectionParam | any) {
        // @ts-ignore
        this.tbl?.shadowRoot.querySelector(".table").style.height = (this.parentElement.clientHeight - 45) + "px"
        this.queryDataByDB(val)
    }

    initElements(): void {
        this.tbl = this.shadowRoot?.querySelector<LitTable>('#tb-network-ability');
        new ResizeObserver((entries) => {
            if (this.parentElement?.clientHeight != 0) {
                // @ts-ignore
                this.tbl?.shadowRoot.querySelector(".table").style.height = (this.parentElement.clientHeight - 45) + "px"
                this.tbl?.reMeauseHeight()
            }
        }).observe(this.parentElement!)
        this.tbl!.addEventListener('column-click', (evt) => {
            // @ts-ignore
            this.sortByColumn(evt.detail)
        });
    }

    filterData() {
        if (this.queryResult.length > 0) {
            let filter = this.queryResult.filter((item) => {
                let array = this.toNetWorkAbilityArray(item)
                let isInclude = array.filter(value => value.indexOf(this.search!.value) > -1);
                return isInclude.length > 0
            });
            if (filter.length > 0) {
                this.source = filter;
                this.tbl!.recycleDataSource = this.source;
            } else {
                this.source = []
                this.tbl!.recycleDataSource = [];
            }
        }
    }

    toNetWorkAbilityArray(systemNetworkSummary: SystemNetworkSummary): any[] {
        let array: Array<string> = []
        array.push(systemNetworkSummary.startTimeStr)
        array.push(systemNetworkSummary.durationStr)
        array.push(systemNetworkSummary.dataReceivedStr)
        array.push(systemNetworkSummary.dataReceivedSecStr)
        array.push(systemNetworkSummary.dataSendSecStr)
        array.push(systemNetworkSummary.dataSendStr)
        array.push(systemNetworkSummary.packetsIn.toString())
        array.push(systemNetworkSummary.packetsOut.toString())
        array.push(systemNetworkSummary.packetsOutSec.toString())
        return array
    }

    queryDataByDB(val: SelectionParam | any) {
        getTabNetworkAbilityData(val.leftNs, val.rightNs).then(item => {
            if (item.length != null && item.length > 0) {
                for (const systemNetworkSummary of item) {
                    if (systemNetworkSummary.startTime == 0) {
                        systemNetworkSummary.startTimeStr = '0:000.000.000';
                    } else {
                        systemNetworkSummary.startTimeStr = Utils.getTimeStampHMS(systemNetworkSummary.startTime);
                    }
                    systemNetworkSummary.durationStr = Utils.getDurString(systemNetworkSummary.duration);
                    systemNetworkSummary.dataReceivedStr = Utils.getBinaryByteWithUnit(systemNetworkSummary.dataReceived);
                    systemNetworkSummary.dataReceivedSecStr = Utils.getBinaryByteWithUnit(systemNetworkSummary.dataReceivedSec);
                    systemNetworkSummary.dataSendStr = Utils.getBinaryByteWithUnit(systemNetworkSummary.dataSend);
                    systemNetworkSummary.dataSendSecStr = Utils.getBinaryByteWithUnit(systemNetworkSummary.dataSendSec);
                    systemNetworkSummary.packetsInStr = ColorUtils.formatNumberComma(systemNetworkSummary.packetsIn);
                    systemNetworkSummary.packetsInSecStr = systemNetworkSummary.packetsInSec.toFixed(2);
                    systemNetworkSummary.packetsOutStr = ColorUtils.formatNumberComma(systemNetworkSummary.packetsOut);
                    systemNetworkSummary.packetsOutSecStr = systemNetworkSummary.packetsOutSec.toFixed(2);
                }
                this.source = item
                this.queryResult = item;
                this.tbl!.recycleDataSource = this.source;
            } else {
                this.source = []
                this.queryResult = []
                this.tbl!.recycleDataSource = [];
            }
        })
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
<lit-table id="tb-network-ability" style="height: auto">
    <lit-table-column order width="1fr" title="StartTime" data-index="startTimeStr" key="startTimeStr" align="flex-start"></lit-table-column>
    <lit-table-column order width="1fr" title="Duration" data-index="durationStr" key="durationStr" align="flex-start" ></lit-table-column>
    <lit-table-column order width="1fr" title="Data Received" data-index="dataReceivedStr" key="dataReceivedStr" align="flex-start" ></lit-table-column>
    <lit-table-column order width="1fr" title="Data Received/sec" data-index="dataReceivedSecStr" key="dataReceivedSecStr" align="flex-start" ></lit-table-column>
    <lit-table-column order width="1fr" title="Data Send" data-index="dataSendStr" key="dataSendStr" align="flex-start" ></lit-table-column>
    <lit-table-column order width="1fr" title="Data Send/sec" data-index="dataSendSecStr" key="dataSendSecStr" align="flex-start" ></lit-table-column>
    <lit-table-column order width="1fr" title="Packets In" data-index="packetsInStr" key="packetsInStr" align="flex-start" ></lit-table-column>
    <lit-table-column order width="1fr" title="Packets In/sec" data-index="packetsInSecStr" key="packetsInSecStr" align="flex-start" ></lit-table-column>
    <lit-table-column order width="1fr" title="Packets Out" data-index="packetsOutStr" key="packetsOutStr" align="flex-start" ></lit-table-column>
    <lit-table-column order width="1fr" title="Packets Out/sec" data-index="packetsOutSecStr" key="packetsOutSecStr" align="flex-start" ></lit-table-column>
</lit-table>
        `;
    }

    sortByColumn(detail: any) {
        // @ts-ignore
        function compare(property, sort, type) {
            return function (a: SystemNetworkSummary, b: SystemNetworkSummary) {
                if (type === 'number') {
                    // @ts-ignore
                    return sort === 2 ? parseFloat(b[property]) - parseFloat(a[property]) : parseFloat(a[property]) - parseFloat(b[property]);
                } else {
                    // @ts-ignore
                    if (b[property] > a[property]) {
                        return sort === 2 ? 1 : -1;
                    } else { // @ts-ignore
                        if (b[property] == a[property]) {
                            return 0;
                        } else {
                            return sort === 2 ? -1 : 1;
                        }
                    }
                }
            }
        }

        if (detail.key === 'startTime') {
            this.source.sort(compare(detail.key, detail.sort, 'string'))
        } else {
            this.source.sort(compare(detail.key, detail.sort, 'number'))
        }
        this.tbl!.dataSource = this.source;
    }
}
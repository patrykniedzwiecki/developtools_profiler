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
import { SelectionData, SelectionParam } from '../../../../bean/BoxSelection.js';
import { getTabCpuByProcess } from '../../../../database/SqlLite.js';
import { log } from '../../../../../log/Log.js';
import { Utils } from '../../base/Utils.js';

@element('tabpane-cpu-process')
export class TabPaneCpuByProcess extends BaseElement {
  private tbl: LitTable | null | undefined;
  private range: HTMLLabelElement | null | undefined;
  private source: Array<SelectionData> = [];

  set data(val: SelectionParam | any) {
    this.range!.textContent =
      'Selected range: ' + parseFloat(((val.rightNs - val.leftNs) / 1000000.0).toFixed(5)) + ' ms';
    // @ts-ignore
    this.tbl!.shadowRoot!.querySelector('.table')?.style?.height = this.parentElement!.clientHeight - 45 + 'px';
    this.tbl!.recycleDataSource = [];
    getTabCpuByProcess(val.cpus, val.leftNs, val.rightNs).then((result) => {
      if (result != null && result.length > 0) {
        log('getTabCpuByProcess size :' + result.length);
        let sumWall = 0.0;
        let sumOcc = 0;
        for (let e of result) {
          let process = Utils.PROCESS_MAP.get(e.pid);
          e.process = process == null || process.length == 0 ? '[NULL]' : process;
          sumWall += e.wallDuration;
          sumOcc += e.occurrences;
          e.wallDuration = parseFloat((e.wallDuration / 1000000.0).toFixed(5));
          e.avgDuration = parseFloat((parseFloat(e.avgDuration) / 1000000.0).toFixed(5)).toString();
        }
        let count = new SelectionData();
        count.process = ' ';
        count.wallDuration = parseFloat((sumWall / 1000000.0).toFixed(5));
        count.occurrences = sumOcc;
        result.splice(0, 0, count);
        this.source = result;
        this.tbl!.recycleDataSource = result;
      } else {
        this.source = [];
        this.tbl!.recycleDataSource = this.source;
      }
    });
  }

  initElements(): void {
    this.tbl = this.shadowRoot?.querySelector<LitTable>('#tb-cpu-process');
    this.range = this.shadowRoot?.querySelector('#time-range');
    this.tbl!.addEventListener('column-click', (evt) => {
      // @ts-ignore
      this.sortByColumn(evt.detail);
    });
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
        <label id="time-range" style="width: 100%;height: 20px;text-align: end;font-size: 10pt;margin-bottom: 5px">Selected range:0.0 ms</label>
        <lit-table id="tb-cpu-process" style="height: auto">
            <lit-table-column order width="30%" title="Process" data-index="process" key="process" align="flex-start" order>
            </lit-table-column>
            <lit-table-column order width="1fr" title="PID" data-index="pid" key="pid" align="flex-start" order>
            </lit-table-column>
            <lit-table-column order width="1fr" title="Wall duration(ms)" data-index="wallDuration" key="wallDuration" align="flex-start" order>
            </lit-table-column>
            <lit-table-column order width="1fr" title="Avg Wall duration(ms)" data-index="avgDuration" key="avgDuration" align="flex-start" order>
            </lit-table-column>
            <lit-table-column order width="1fr" title="Occurrences" data-index="occurrences" key="occurrences" align="flex-start" order>
            </lit-table-column>
        </lit-table>
        `;
  }

  sortByColumn(detail: any) {
    // @ts-ignore
    function compare(property, sort, type) {
      return function (a: SelectionData, b: SelectionData) {
        if (a.process == ' ' || b.process == ' ') {
          return 0;
        }
        if (type === 'number') {
          // @ts-ignore
          return sort === 2 ? parseFloat(b[property]) - parseFloat(a[property]) : parseFloat(a[property]) - parseFloat(b[property]);
        } else {
          // @ts-ignore
          if (b[property] > a[property]) {
            return sort === 2 ? 1 : -1;
          } else {
            // @ts-ignore
            if (b[property] == a[property]) {
              return 0;
            } else {
              return sort === 2 ? -1 : 1;
            }
          }
        }
      };
    }

    if (
      detail.key === 'pid' ||
      detail.key === 'wallDuration' ||
      detail.key === 'avgDuration' ||
      detail.key === 'occurrences'
    ) {
      this.source.sort(compare(detail.key, detail.sort, 'number'));
    } else {
      this.source.sort(compare(detail.key, detail.sort, 'string'));
    }
    this.tbl!.recycleDataSource = this.source;
  }
}

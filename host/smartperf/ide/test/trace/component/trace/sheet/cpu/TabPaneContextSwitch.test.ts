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

// @ts-ignore
import { TabPaneContextSwitch } from '../../../../../../dist/trace/component/trace/sheet/cpu/TabPaneContextSwitch.js';
// @ts-ignore
import { LitTable } from '../../../../../../dist/base-ui/table/lit-table.js';
// @ts-ignore
import { SpSystemTrace } from '../../../../../../dist/trace/component/SpSystemTrace.js';

const sqlit = require('../../../../../../dist/trace/database/SqlLite.js');
jest.mock('../../../../../../dist/trace/database/SqlLite.js');

window.ResizeObserver =
    window.ResizeObserver ||
    jest.fn().mockImplementation(() => ({
        disconnect: jest.fn(),
        observe: jest.fn(),
        unobserve: jest.fn(),
    }));
describe('TabPaneContextSwitch Test', () => {
    document.body.innerHTML = `<div class="ddd"><lit-table id="tb-states"></lit-table><div>`;
    let tab = document.querySelector('.ddd') as HTMLDivElement;
    let tabPaneContextSwitch = new TabPaneContextSwitch();
    tabPaneContextSwitch.tbl = jest.fn(() => tab);
    tabPaneContextSwitch.tbl.treeElement = jest.fn(() => tab);
    tabPaneContextSwitch.tbl.tableElement = jest.fn(() => tab);
    SpSystemTrace.SPT_DATA = [
        {
            process: '',
            processId: 0,
            thread: '',
            threadId: 0,
            state: '',
            dur: 0,
            start_ts: 0,
            end_ts: 0,
            cpu: 0,
            priority: '-',
            note: '-',
        },
        {
            process: '',
            processId: 1,
            thread: '',
            threadId: 1,
            state: '',
            dur: 0,
            start_ts: 0,
            end_ts: 0,
            cpu: 0,
            priority: '-',
            note: '-',
        },
        {
            process: '',
            processId: 2,
            thread: '',
            threadId: 2,
            state: '',
            dur: 0,
            start_ts: 0,
            end_ts: 0,
            cpu: 0,
            priority: '-',
            note: '-',
        },
    ];

    let dataArray = {
        id: '',
        pid: '',
        title: '',
        children: [],
        process: '',
        processId: 0,
        thread: '',
        threadId: 0,
        state: '',
        wallDuration: 0,
        avgDuration: '',
        count: 0,
        minDuration: 0,
        maxDuration: 0,
        stdDuration: '',
    };

    it('TabPaneContextSwitchTest01', function () {
        expect(tabPaneContextSwitch).not.toBeUndefined();
    });

    it('TabPanePTSTest02', function () {
        expect(tabPaneContextSwitch.getDataBySPT(0, 0, [])).toBeUndefined();
    });

    it('TabPaneContextSwitchTest03', function () {
        let mockgetProcessThreadDataByRange =
            sqlit.getStatesProcessThreadDataByRange;
        mockgetProcessThreadDataByRange.mockResolvedValue([
            {
                process: 'process',
                processId: 1,
                thread: 'thread',
                state: 'state',
                threadId: 1,
                dur: 1000,
                end_ts: 2000,
                start_ts: 2000,
                cpu: 1111,
            },
        ]);
        expect((tabPaneContextSwitch.data = dataArray)).toBeTruthy();
    });

    it('TabPaneContextSwitchTest04', function () {
        expect(tabPaneContextSwitch.initHtml()).toMatchInlineSnapshot(`
"
        <style>
        :host{
            display: flex;
            flex-direction: column;
            padding: 10px 10px;
        }
        </style>
        <label id=\\"time-range\\" style=\\"width: 100%;height: 20px;text-align: end;font-size: 10pt;margin-bottom: 5px\\">Selected range:0.0 ms</label>
        <lit-table id=\\"tb-cs\\" style=\\"height: auto\\" tree>
            <lit-table-column width=\\"500px\\" title=\\"Process/Thread/Event\\" data-index=\\"title\\" key=\\"title\\" align=\\"flex-start\\">
            </lit-table-column>
            <lit-table-column width=\\"1fr\\" title=\\"Count\\" data-index=\\"count\\" key=\\"count\\" align=\\"flex-start\\" >
            </lit-table-column>
        </lit-table>
        "
`);
    });

    it('TabPaneContextSwitchTest04', function () {
        document.body.innerHTML = `<tabpane-context-switch id="Switch"></tabpane-context-switch>`;
        let tabPaneContextSwitch = document.querySelector(
            '#Switch'
        ) as TabPaneContextSwitch;
        expect(tabPaneContextSwitch.data).toBe(tabPaneContextSwitch.dataArray);
    });

    it('TabPaneContextSwitchTest05', function () {
        document.body.innerHTML = `<tabpane-context-switch id="Switch"></tabpane-context-switch>`;
        let tabPaneContextSwitch = document.querySelector(
            '#Switch'
        ) as TabPaneContextSwitch;
        tabPaneContextSwitch.loadDataInCache = false;
        expect(tabPaneContextSwitch.data).toBe(tabPaneContextSwitch.dataArray);
    });
});

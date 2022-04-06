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


import {RangeSelect} from "../../../../../dist/trace/component/trace/base/RangeSelect.js";

describe("RangeSelect Test", () => {
    beforeAll(() => {
    })

    it('Utils Test01', () => {
        let rangeSelect = new RangeSelect();
        expect(rangeSelect).not.toBeUndefined();
    });

    it('Utils Test02', () => {
        let rangeSelect = new RangeSelect();
        rangeSelect.rowsEL= document.createElement('div')
        let mouseEvent = new MouseEvent("mousedown",{
            button: 0,
            buttons: 0,
            clientX: 2,
            clientY: 100,
            screenX: 255,
            screenY: 325
        });
        expect(rangeSelect.isInRowsEl(mouseEvent)).toBeFalsy();
    });

    it('Utils Test03', () => {
        let rangeSelect = new RangeSelect();
        let rowsEL= document.createElement('div')
        expect(rangeSelect.printFrame(rowsEL)).toBeUndefined()
    });

    it('Utils Test04', () => {
        let rangeSelect = new RangeSelect();
        let rowsEL= document.createElement('div')
        let mouseEvent = new MouseEvent("mousedown",{
            button: 0,
            buttons: 0,
            clientX: 2,
            clientY: 100,
            screenX: 255,
            screenY: 325
        });
        expect(rangeSelect.printEventFrame(mouseEvent)).toBeUndefined()
    });

    it('Utils Test05', () => {
        let rangeSelect = new RangeSelect();
        rangeSelect.isInRowsEl = jest.fn(()=>true)
        rangeSelect.rowsEL= {
            offsetTop: 100,
            offsetHeight:200,
            offsetLeft:0,
            offsetWidth:100
        }
        let mouseEvent = new MouseEvent("mousedown",{
            // @ts-ignore
            offsetY:1,
            offsetX:1,
            button: 0,
            buttons: 0,
            clientX: 2,
            clientY: 100,
            screenX: 255,
            screenY: 325
        });
        expect(rangeSelect.mouseDown(mouseEvent)).toBeUndefined()
    });

    it('Utils Test06', () => {
        let rangeSelect = new RangeSelect();
        rangeSelect.isInRowsEl = jest.fn(()=>true)
        rangeSelect.rowsEL= {
            offsetTop: 100,
            offsetHeight:200,
            offsetLeft:0,
            offsetWidth:100
        }
        let mouseEvent = new MouseEvent("mousedown",{
            // @ts-ignore
            offsetY:1,
            offsetX:1,
            button: 0,
            buttons: 0,
            clientX: 2,
            clientY: 100,
            screenX: 255,
            screenY: 325
        });
        expect(rangeSelect.mouseDown(mouseEvent)).toBeUndefined()
    });


    it('Utils Test07', () => {
        let rangeSelect = new RangeSelect();
        rangeSelect.isInRowsEl = jest.fn(()=>true)
        rangeSelect.isDrag = jest.fn(()=>true)

        rangeSelect.rowsEL= {
            offsetTop: 100,
            offsetHeight:200,
            offsetLeft:0,
            offsetWidth:100
        }
        let mouseEvent = new MouseEvent("mousedown",{
            // @ts-ignore
            offsetY:1,
            offsetX:1,
            button: 0,
            buttons: 0,
            clientX: 2,
            clientY: 100,
            screenX: 255,
            screenY: 325
        });
        expect(rangeSelect.mouseUp(mouseEvent)).toBeUndefined();
    });

    it('Utils Test08', () => {
        let rangeSelect = new RangeSelect();
        rangeSelect.isInRowsEl = jest.fn(()=>true)
        rangeSelect.isDrag = jest.fn(()=>true)
        rangeSelect.isMouseDown = true
       let rowsEL= [{
            frame: {
                x:1,
                width:10,
                y:2,
                height:10
            },
            offsetTop: 100,
            offsetHeight:200,
            offsetLeft:0,
            offsetWidth:100,
       }]
        rangeSelect.rowsEL = rowsEL;
        let mouseEvent = new MouseEvent("mousedown",{
            // @ts-ignore
            offsetY:1,
            offsetX:1,
            button: 0,
            buttons: 0,
            clientX: 2,
            clientY: 100,
            screenX: 255,
            screenY: 325
        });
        expect(rangeSelect.mouseMove(rowsEL,mouseEvent)).toBeUndefined();
    });
})
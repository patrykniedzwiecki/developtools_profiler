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

import {BaseElement, element} from "../BaseElement.js";
import {SpCheckDesBox} from "../../trace/component/setting/SpCheckDesBox.js";

@element('lit-check-text')
export class LitCheckBoxWithText extends BaseElement {
    private _checkBox: SpCheckDesBox | undefined;
    private _lowerLimit: HTMLInputElement | undefined;
    private _upLimit: HTMLInputElement | undefined;

    static get observedAttributes() {
        return ['text', 'lowerlimit', 'uplimit', 'checked']
    }

    get text(): string {
        return this.getAttribute("text") || ""
    }

    set text(text: string) {
        this.setAttribute("text", text)
    }

    get lowerlimit(): string {
        return this.getAttribute("lowerlimit") || "0"
    }

    set lowerlimit(lower: string) {
        this.setAttribute("lowerlimit", lower)
    }

    get uplimit(): string {
        return this.getAttribute("uplimit") || "∞"
    }

    set uplimit(uplimit: string) {
        this.setAttribute("uplimit", uplimit)
    }

    get checked() {
        return this.getAttribute("checked") != null;
    }

    set checked(checked: boolean) {
        if (checked) {
            this.setAttribute('checked', '');
        } else {
            this.removeAttribute('checked');
        }
    }

    initElements(): void {
        this._checkBox = this.shadowRoot?.getElementById('checkbox') as SpCheckDesBox;
        this._lowerLimit = this.shadowRoot?.getElementById('textLowerLimit') as HTMLInputElement;
        this._upLimit = this.shadowRoot?.getElementById('_upLimit') as HTMLInputElement;
    }

    initHtml(): string {
        return `
<style>
:host{
 display: grid;
 grid-template-columns: 1fr min-content min-content;
 grid-column-gap: 10px;
 text-align: center;
 height: 16px;
}
.input-style {
  width: 48px;
  height: 16px;
  border: 1px solid #B3B3B3;
  text-align: center;
}

</style>
<check-des-box id ='checkbox' value="${this.text}"></check-des-box>
<input class="input-style" id="textLowerLimit" value="${this.lowerlimit}"/><input class="input-style" id="_upLimit" value="${this.uplimit}"/>
        `;
    }

    attributeChangedCallback(name: string, oldValue: string, newValue: string) {
        if (name == 'checked') {
            if (newValue !== null) {
                this._checkBox!.checked = true;
            } else {
                this._checkBox!.checked = false;
            }
        }
        if (name == 'text') {
            this._checkBox?.setAttribute('value', newValue);
        }
        if (name == 'lowerlimit') {
            this._lowerLimit!.textContent = newValue
        }
        if (name == 'uplimit') {
            this._upLimit!.textContent = newValue
        }
    }
}

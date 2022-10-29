// Copyright 2021 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

/** @fileoverview Definitions for chrome.settingsPrivate API */
// TODO(crbug.com/1203307): Auto-generate this file.

import {ChromeEvent} from './chrome_event.js';

declare global {
  export namespace chrome {
    export namespace settingsPrivate {
      export enum PrefType {
        BOOLEAN = 'BOOLEAN',
        NUMBER = 'NUMBER',
        STRING = 'STRING',
        URL = 'URL',
        LIST = 'LIST',
        DICTIONARY = 'DICTIONARY',
      }

      export enum ControlledBy {
        DEVICE_POLICY = 'DEVICE_POLICY',
        USER_POLICY = 'USER_POLICY',
        OWNER = 'OWNER',
        PRIMARY_USER = 'PRIMARY_USER',
        EXTENSION = 'EXTENSION',
        PARENT = 'PARENT',
        CHILD_RESTRICTION = 'CHILD_RESTRICTION',
      }

      export enum Enforcement {
        ENFORCED = 'ENFORCED',
        RECOMMENDED = 'RECOMMENDED',
        PARENT_SUPERVISED = 'PARENT_SUPERVISED',
      }

      // TODO(crbug/1373934) Update existing usages of PrefObject to be typed,
      // removing the need to use any here.
      export interface PrefObject<T = any> {
        key: string;
        type:
            // clang-format off
            T extends boolean ? PrefType.BOOLEAN :
            T extends number ? PrefType.NUMBER :
            T extends string ? PrefType.STRING | PrefType.URL :
            T extends unknown[] ? PrefType.LIST :
            T extends Record<string|number, unknown> ? PrefType.DICTIONARY :
            never;
        // clang-format on
        value: T;
        controlledBy?: ControlledBy;
        controlledByName?: string;
        enforcement?: Enforcement;
        recommendedValue?: T;
        userSelectableValues?: T[];
        userControlDisabled?: boolean;
        extensionId?: string;
        extensionCanBeDisabled?: boolean;
      }

      type PrefsCallback = (prefs: PrefObject[]) => void;

      export function getAllPrefs(callback: PrefsCallback): void;
      export function getPref(
          name: string, callback: (pref: PrefObject) => void): void;
      export function setPref(
          name: string, value: any, pageId?: string,
          callback?: (success: boolean) => void): void;

      export function getDefaultZoom(callback: (zoom: number) => void): void;
      export function setDefaultZoom(zoom: number): void;

      export const onPrefsChanged: ChromeEvent<PrefsCallback>;
    }
  }
}

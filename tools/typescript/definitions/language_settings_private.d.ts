// Copyright 2021 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

/** @fileoverview Definitions for chrome.languageSettingsPrivate API */
// TODO(crbug.com/1203307): Auto-generate this file.

import {ChromeEvent} from './chrome_event.js';

declare global {
  export namespace chrome {
    export namespace languageSettingsPrivate {
      export enum MoveType {
        TOP = 'TOP',
        UP = 'UP',
        DOWN = 'DOWN',
        UNKNOWN = 'UNKNOWN',
      }

      export interface Language {
        code: string;
        displayName: string;
        nativeDisplayName: string;
        supportsUI?: boolean;
        supportsSpellcheck?: boolean;
        supportsTranslate?: boolean;
        isProhibitedLanguage?: boolean;
      }

      export interface SpellcheckDictionaryStatus {
        languageCode: string;
        isReady: boolean;
        isDownloading?: boolean;
        downloadFailed?: boolean;
      }

      export interface InputMethod {
        id: string;
        displayName: string;
        languageCodes: string[];
        tags: string[];
        enabled?: boolean;
        hasOptionsPage?: boolean;
        isProhibitedByPolicy?: boolean;
      }

      export interface InputMethodLists {
        componentExtensionImes: InputMethod[];
        thirdPartyExtensionImes: InputMethod[];
      }

      type StringArrayCallback = (strings: string[]) => void;

      export function getLanguageList(
          callback: (languages: Language[]) => void): void;
      export function enableLanguage(languageCode: string): void;
      export function disableLanguage(languageCode: string): void;
      export function setEnableTranslationForLanguage(
          languageCode: string, enable: boolean): void;
      export function moveLanguage(languageCode: string, moveType: MoveType):
          void;
      export function getAlwaysTranslateLanguages(
          callback: StringArrayCallback): void;
      export function setLanguageAlwaysTranslateState(
          languageCode: string, alwaysTranslate: boolean): void;
      export function getNeverTranslateLanguages(callback: StringArrayCallback):
          void;
      export function getSpellcheckDictionaryStatuses(
          callback: (statuses: SpellcheckDictionaryStatus[]) => void): void;
      export function getSpellcheckWords(callback: StringArrayCallback): void;
      export function addSpellcheckWord(word: string): void;
      export function removeSpellcheckWord(word: string): void;
      export function getTranslateTargetLanguage(
          callback: (languageCode: string) => void): void;
      export function setTranslateTargetLanguage(languageCode: string): void;
      export function getInputMethodLists(
          callback: (lists: InputMethodLists) => void): void;
      export function addInputMethod(inputMethodId: string): void;
      export function removeInputMethod(inputMethodId: string): void;
      export function retryDownloadDictionary(languageCode: string): void;

      export const onSpellcheckDictionariesChanged:
          ChromeEvent<(statuses: SpellcheckDictionaryStatus[]) => void>;
      export const onCustomDictionaryChanged:
          ChromeEvent<(added: string[], removed: string[]) => void>;
      export const onInputMethodAdded: ChromeEvent<(id: string) => void>;
      export const onInputMethodRemoved: ChromeEvent<(id: string) => void>;
    }
  }
}

// Copyright 2021 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

/** @fileoverview Definitions for chrome.bookmarkManagerPrivate API. */
// TODO(crbug.com/1203307): Auto-generate this file.

import {ChromeEvent} from './chrome_event.js';

declare global {
  export namespace chrome {
    export namespace bookmarkManagerPrivate {
      export interface BookmarkNodeDataElement {
        id?: string;
        parentId?: string;
        title: string;
        url?: string;
        children: BookmarkNodeDataElement[];
      }

      export interface BookmarkNodeData {
        sameProfile: boolean;
        elements: BookmarkNodeDataElement[];
      }

      export function copy(idList: string[], callback: () => void): void;
      export function cut(idList: string[], callback?: () => void): void;
      export function paste(
          parentId: string, selectedIdList?: string[],
          callback?: () => void): void;
      export function canPaste(
          parentId: string, callback: (p1: boolean) => void): void;
      export function sortChildren(parentId: string): void;
      export function startDrag(
          idList: string[], dragNodeIndex: number, isFromTouch: boolean,
          x: number, y: number): void;
      export function drop(
          parentId: string, index?: number, callback?: () => void): void;
      export function getSubtree(
          id: string, foldersOnly: boolean,
          callback: (p1: chrome.bookmarks.BookmarkTreeNode[]) => void): void;
      export function removeTrees(idList: string[], callback?: () => void):
          void;
      export function undo(): void;
      export function redo(): void;
      export function openInNewTab(id: string, active: boolean): void;
      export function openInNewWindow(idList: string[], incognito: boolean):
          void;

      export interface DragData {
        elements: chrome.bookmarks.BookmarkTreeNode[]|null;
        sameProfile: boolean;
      }

      export const onDragEnter: ChromeEvent<(p1: DragData) => void>;
      export const onDragLeave: ChromeEvent<() => void>;
      export const onDrop: ChromeEvent<() => void>;
    }
  }
}

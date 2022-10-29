" Copyright 2014 The Chromium Authors
" Use of this source code is governed by a BSD-style license that can be
" found in the LICENSE file.

" Binds cmd-shift-i (on Mac) or ctrl-i (elsewhere) to invoke clang-format.py.
" It will format the current selection (and if there's no selection, the
" current line.)

let s:script = expand('<sfile>:p:h') .
  \'/../../buildtools/clang_format/script/clang-format.py'
let s:shortcut = has('mac') ? "<D-I>" : "<C-I>"
let s:pyf = has("python3") ? ":py3f" : ":pyf"

execute "map" s:shortcut s:pyf s:script . "<CR>"
execute "imap" s:shortcut "<ESC>" s:pyf s:script . "<CR>i"

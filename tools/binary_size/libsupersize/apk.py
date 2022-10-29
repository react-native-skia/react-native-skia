# Copyright 2022 The Chromium Authors
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
"""Functions for creating APK symbols."""

import logging
import os
import posixpath
import re
import zipfile

import archive_util
import file_format
import models
import zip_util


class _ResourcePathDeobfuscator:
  def __init__(self, pathmap_path):
    self._pathmap = self._LoadResourcesPathmap(pathmap_path)

  def _LoadResourcesPathmap(self, pathmap_path):
    """Loads the pathmap of obfuscated resource paths.

    Returns: A dict mapping from obfuscated paths to original paths or an
             empty dict if passed a None |pathmap_path|.
    """
    if pathmap_path is None:
      return {}

    pathmap = {}
    with open(pathmap_path, 'r') as f:
      for line in f:
        line = line.strip()
        if line.startswith('--') or line == '':
          continue
        original, renamed = line.split(' -> ')
        pathmap[renamed] = original
    return pathmap

  def MaybeRemapPath(self, path):
    long_path = self._pathmap.get(path)
    if long_path:
      return long_path
    # if processing a .minimal.apks, we are actually just processing the base
    # split.
    long_path = self._pathmap.get('base/{}'.format(path))
    if long_path:
      # The first 5 chars are 'base/', which we don't need because we are
      # looking directly inside the base apk.
      return long_path[5:]
    return path


class _ResourceSourceMapper:
  def __init__(self, size_info_prefix, path_defaults):
    self._path_defaults = path_defaults or {}
    self._res_info = self._LoadResInfo(size_info_prefix)
    self._pattern_dollar_underscore = re.compile(r'\$+(.*?)(?:__\d)+')
    self._pattern_version_suffix = re.compile(r'-v\d+/')

  @staticmethod
  def _ParseResInfoFile(res_info_path):
    with open(res_info_path, 'r') as info_file:
      return dict(l.rstrip().split('\t') for l in info_file)

  def _LoadResInfo(self, size_info_prefix):
    apk_res_info_path = size_info_prefix + '.res.info'
    res_info_without_root = self._ParseResInfoFile(apk_res_info_path)
    # We package resources in the res/ folder only in the apk.
    res_info = {
        os.path.join('res', dest): source
        for dest, source in res_info_without_root.items()
    }
    res_info.update(self._path_defaults)
    return res_info

  def FindSourceForPath(self, path):
    # Sometimes android adds $ in front and __# before extension.
    path = self._pattern_dollar_underscore.sub(r'\1', path)
    ret = self._res_info.get(path)
    if ret:
      return ret
    # Android build tools may append extra -v flags for the root dir.
    path = self._pattern_version_suffix.sub('/', path)
    ret = self._res_info.get(path)
    if ret:
      return ret
    return ''


def CreateApkOtherSymbols(apk_spec):
  """Creates symbols for resources / assets within the apk.

  Returns:
    A tuple of (section_ranges, raw_symbols, apk_metadata).
  """
  logging.info('Creating symbols for other APK entries')
  res_source_mapper = _ResourceSourceMapper(apk_spec.size_info_prefix,
                                            apk_spec.path_defaults)
  resource_deobfuscator = _ResourcePathDeobfuscator(
      apk_spec.resources_pathmap_path)
  raw_symbols = []
  zip_info_total = 0
  zipalign_total = 0
  with zipfile.ZipFile(apk_spec.apk_path) as z:
    signing_block_size = zip_util.MeasureApkSignatureBlock(z)
    for zip_info in z.infolist():
      zip_info_total += zip_info.compress_size
      # Account for zipalign overhead that exists in local file header.
      zipalign_total += zip_util.ReadZipInfoExtraFieldLength(z, zip_info)
      # Account for zipalign overhead that exists in central directory header.
      # Happens when python aligns entries in apkbuilder.py, but does not
      # exist when using Android's zipalign. E.g. for bundle .apks files.
      zipalign_total += len(zip_info.extra)
      # Skip files that we explicitly analyze: .so, .dex, and .pak.
      if zip_info.filename in apk_spec.ignore_apk_paths:
        continue

      resource_filename = resource_deobfuscator.MaybeRemapPath(
          zip_info.filename)
      source_path = res_source_mapper.FindSourceForPath(resource_filename)
      if not source_path:
        source_path = posixpath.join(models.APK_PREFIX_PATH, resource_filename)
      raw_symbols.append(
          models.Symbol(
              models.SECTION_OTHER,
              zip_info.compress_size,
              source_path=source_path,
              full_name=resource_filename))  # Full name must disambiguate

  # Store zipalign overhead and signing block size as metadata rather than an
  # "Overhead:" symbol because they fluctuate in size, and would be a source of
  # noise in symbol diffs if included as symbols (http://crbug.com/1130754).
  # Might be even better if we had an option in Tiger Viewer to ignore certain
  # symbols, but taking this as a short-cut for now.
  apk_metadata = {
      models.METADATA_ZIPALIGN_OVERHEAD: zipalign_total,
      models.METADATA_SIGNING_BLOCK_SIZE: signing_block_size,
  }

  # Overhead includes:
  #  * Size of all local zip headers (minus zipalign padding).
  #  * Size of central directory & end of central directory.
  overhead_size = (os.path.getsize(apk_spec.apk_path) - zip_info_total -
                   zipalign_total - signing_block_size)
  assert overhead_size >= 0, 'Apk overhead must be non-negative'
  zip_overhead_symbol = models.Symbol(models.SECTION_OTHER,
                                      overhead_size,
                                      full_name='Overhead: APK file')
  raw_symbols.append(zip_overhead_symbol)

  section_ranges = {}
  archive_util.ExtendSectionRange(section_ranges, models.SECTION_OTHER,
                                  sum(s.size for s in raw_symbols))
  file_format.SortSymbols(raw_symbols)
  return section_ranges, raw_symbols, apk_metadata

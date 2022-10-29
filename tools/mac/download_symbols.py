#!/usr/bin/env python3
# Copyright 2021 The Chromium Authors
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
"""Downloads symbols for official, Google Chrome builds.

Usage:
    ./download_symbols.py -v 91.0.4449.6 -a x86_64 -o /dest/path

This can also be used as a Python module.
"""

import argparse
import csv
import os.path
import platform
import shutil
import subprocess
import sys
import urllib.request as request

_OMAHAPROXY_HISTORY = 'https://omahaproxy.appspot.com/history?os=mac&format=json'

_DSYM_URL_TEMPLATE = 'https://dl.google.com/chrome/mac/{channel}/dsym/googlechrome-{version}-{arch}-dsym.tar.bz2'


def download_chrome_symbols(version, channel, arch, dest_dir):
    """Downloads and extracts the official Google Chrome dSYM files to a
    subdirectory of `dest_dir`.

    Args:
        version: The version to download symbols for.
        channel: The release channel to download symbols for. If None, attempts
                 to guess the channel.
        arch: The CPU architecture to download symbols for.
        dest_dir: The location to download symbols to. The dSYMs will be
                  extracted to a subdirectory of this directory.

    Returns:
        The path to the directory containing the dSYMs, which will be a
        subdirectory of `dest_dir`.
    """
    if channel is None:
        channel = _identify_channel(version, arch)
        if channel:
            print('Using release channel {} for {}'.format(channel, version),
                  file=sys.stderr)
        else:
            print('Could not identify channel for Chrome version {}'.format(
                version),
                  file=sys.stderr)
            return None

    # The symbol storage uses "arm64" rather than "aarch64".
    if arch == 'aarch64':
        arch = 'arm64'

    extracted_dir = _download_and_extract(version, channel, arch, dest_dir)
    if not extracted_dir:
        print('Could not find dSYMs for Chrome {} {}'.format(version, arch),
              file=sys.stderr)
    return extracted_dir


def get_symbol_directory(version, channel, arch, dest_dir):
    """Returns the parent directory for dSYMs given the specified parameters."""
    _, dest = _get_url_and_dest(version, channel, arch, dest_dir)
    return dest


def _identify_channel(version, arch):
    """Attempts to guess the release channel given a Chrome version and CPU
    architecture."""
    # First try querying OmahaProxy for the release.
    with request.urlopen(_OMAHAPROXY_HISTORY) as release_history:
        history = csv.DictReader(
            release_history.read().decode('utf8').split('\n'))
        for row in history:
            if row['version'] == version:
                return row['channel']

    # Fall back to sending HEAD HTTP requests to each of the possible symbol
    # locations.
    print(
        'Unable to identify release channel for {}, now brute-force searching'.
        format(version),
        file=sys.stderr)
    for channel in ('stable', 'beta', 'dev', 'canary'):
        url, _ = _get_url_and_dest(version, channel, arch, '')
        req = request.Request(url, method='HEAD')
        try:
            resp = request.urlopen(req)
            if resp.code == 200:
                return channel
        except:
            continue

    return None


def _get_url_and_dest(version, channel, arch, dest_dir):
    """Returns a the symbol archive URL and local destination directory given
    the format parameters."""
    args = {'channel': channel, 'arch': arch, 'version': version}
    url = _DSYM_URL_TEMPLATE.format(**args)
    dest_dir = os.path.join(dest_dir,
                            'googlechrome-{version}-{arch}-dsym'.format(**args))
    return url, dest_dir


def _download_and_extract(version, channel, arch, dest_dir):
    """Performs the download and extraction of the symbol files. Returns the
    path to the extracted symbol files on success, None on error.
    """
    url, dest_dir = _get_url_and_dest(version, channel, arch, dest_dir)
    remove_on_failure = False
    if not os.path.isdir(dest_dir):
        os.mkdir(dest_dir)
        remove_on_failure = True

    try:
        with request.urlopen(url) as symbol_request:
            print('Downloading and extracting symbols to {}'.format(dest_dir),
                  file=sys.stderr)
            print('This will take a minute...', file=sys.stderr)
            if _extract_symbols_to(symbol_request, dest_dir):
                return dest_dir
    except:
        pass

    if remove_on_failure:
        shutil.rmtree(dest_dir)
    return None


def _extract_symbols_to(symbol_request, dest_dir):
    """Performs a streaming extract of the symbol files.

    Args:
        symbol_request: The HTTPResponse object for the symbol URL.
        dest_dir: The destination directory into which the files will be
                  extracted.

    Returns: True on successful download and extraction, False on error.
    """
    proc = subprocess.Popen(['tar', 'xjf', '-'],
                            cwd=dest_dir,
                            stdin=subprocess.PIPE,
                            stdout=sys.stderr,
                            stderr=sys.stderr)
    while True:
        data = symbol_request.read(4096)
        if not data:
            proc.stdin.close()
            break
        proc.stdin.write(data)
    proc.wait()

    return proc.returncode == 0


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('--version',
                        '-v',
                        required=True,
                        help='Version to download.')
    parser.add_argument(
        '--channel',
        '-c',
        help='Chrome release channel for the version. The channel will be ' \
             'guessed if not specified.'
    )
    parser.add_argument(
        '--arch',
        '-a',
        help='CPU architecture to download, defaults to that of the current OS.'
    )
    parser.add_argument('--out',
                        '-o',
                        required=True,
                        help='Directory to download the symbols to.')
    args = parser.parse_args()

    arch = args.arch
    if not arch:
        arch = platform.machine()

    if not os.path.isdir(args.out):
        print('--out destination is not a directory.', file=sys.stderr)
        return False

    return download_chrome_symbols(args.version, args.channel, arch, args.out)


if __name__ == '__main__':
    sys.exit(0 if main() else 1)

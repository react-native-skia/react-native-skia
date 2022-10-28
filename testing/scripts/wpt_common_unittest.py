#!/usr/bin/env vpython
# Copyright (c) 2020 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

"""Unit tests for common functionality of wpt testing scripts."""

import base64
import json
import os
import re
import unittest

from wpt_common import (
    BaseWptScriptAdapter, EXTERNAL_WPT_TESTS_DIR, WEB_TESTS_DIR,
    LAYOUT_TEST_RESULTS_SUBDIR
)

from blinkpy.common.host_mock import MockHost
from blinkpy.web_tests.port.factory_mock import MockPortFactory
from blinkpy.w3c.wpt_manifest import BASE_MANIFEST_NAME

# The path where the output of a wpt run was written. This is the file that
# gets processed by BaseWptScriptAdapter.
OUTPUT_JSON_FILENAME = "out.json"


class MockResultSink(object):

    def __init__(self):
        self.sink_requests = []
        self.host = MockHost()

    def report_individual_test_result(self, test_name, result,
                                      artifacts_sub_dir,
                                      expectation, test_path):
        del artifacts_sub_dir
        assert not expectation, 'expectation parameter should always be None'
        self.sink_requests.append(
            {'test': test_name,
             'test_path': test_path,
             'result': {'actual': result.actual,
                        'expected': result.expected,
                        'unexpected': result.unexpected,
                        'artifacts': result.artifacts}})

class BaseWptScriptAdapterTest(unittest.TestCase):
    def setUp(self):
        self.host = MockHost()
        self.host.port_factory = MockPortFactory(self.host)
        self.port = self.host.port_factory.get()

        # Create a testing manifest containing any test files that we
        # might interact with.
        self.host.filesystem.write_text_file(
            self.port.web_tests_dir() + '/external/' + BASE_MANIFEST_NAME,
            json.dumps({
                'items': {
                    'reftest': {
                        'reftest.html': [
                            'c3f2fb6f436da59d43aeda0a7e8a018084557033',
                            [None, [['reftest-ref.html', '==']], {}],
                        ]
                    },
                    'testharness': {
                        'test.html': [
                            'd933fd981d4a33ba82fb2b000234859bdda1494e',
                            [None, {}]
                        ],
                        'crash.html': [
                            'd933fd981d4a33ba82fb2b000234859bdda1494e',
                            [None, {}]
                        ],
                        'variant.html': [
                            'b8db5972284d1ac6bbda0da81621d9bca5d04ee7',
                            ['variant.html?foo=bar/abc', {}],
                            ['variant.html?foo=baz', {}],
                        ],
                        'dir': {
                            'multiglob.https.any.js': [
                                'd6498c3e388e0c637830fa080cca78b0ab0e5305',
                                ['dir/multiglob.https.any.window.html', {}],
                                ['dir/multiglob.https.any.worker.html', {}],
                            ],
                        },
                    },
                },
            }))
        self.host.filesystem.write_text_file(
            os.path.join(WEB_TESTS_DIR, "fast", "harness", "results.html"),
            "results-viewer-body")
        self.wpt_adapter = BaseWptScriptAdapter(self.host)
        self.wpt_adapter.wpt_output = OUTPUT_JSON_FILENAME
        self.wpt_adapter.sink = MockResultSink()

    def _create_json_output(self, json_dict):
        """Writing some json output for processing."""
        self.host.filesystem.write_text_file(OUTPUT_JSON_FILENAME,
                                             json.dumps(json_dict))

    def _load_json_output(self, filename=OUTPUT_JSON_FILENAME):
        """Loads the json output after post-processing."""
        return json.loads(self.host.filesystem.read_text_file(filename))

    def test_result_sink_for_test_expected_result(self):
        json_dict = {
            'tests': {
                'fail': {
                    'test.html?variant1': {
                        'expected': 'PASS FAIL',
                        'actual': 'FAIL',
                        'artifacts': {
                            'wpt_actual_status': ['OK'],
                            'wpt_actual_metadata': ['test.html actual text'],
                        },
                    },
                },
            },
            'path_delimiter': os.path.sep,
        }
        test_abs_path = os.path.join(WEB_TESTS_DIR,
                                     'external', 'wpt', 'fail', 'test.html')
        self._create_json_output(json_dict)
        self.wpt_adapter.do_post_test_run_tasks()

        baseline_artifacts = {'wpt_actual_status': ['OK'],
                              'actual_text': [
                                  (os.path.join('layout-test-results',
                                                'external', 'wpt', 'fail',
                                                'test_variant1-actual.txt'))]}
        self.assertEquals(self.wpt_adapter.sink.sink_requests,
                          [{'test': os.path.join('external','wpt', 'fail',
                                                 'test.html?variant1'),
                            'test_path': test_abs_path,
                            'result': {'actual': 'FAIL',
                                       'expected': set(['PASS', 'FAIL']),
                                       'unexpected': False,
                                       'artifacts': baseline_artifacts}}])

    def test_result_sink_for_test_variant(self):
        json_dict = {
            'tests': {
                'fail': {
                    'test.html?variant1': {
                        'expected': 'PASS',
                        'actual': 'FAIL',
                        'artifacts': {
                            'wpt_actual_status': ['OK'],
                            'wpt_actual_metadata': ['test.html actual text'],
                        },
                    },
                },
            },
            'path_delimiter': os.path.sep,
        }
        test_abs_path = os.path.join(WEB_TESTS_DIR,
                                     'external', 'wpt', 'fail', 'test.html')
        self._create_json_output(json_dict)
        self.wpt_adapter.do_post_test_run_tasks()
        baseline_artifacts = {'wpt_actual_status': ['OK'],
                              'actual_text': [
                                  (os.path.join('layout-test-results',
                                                'external', 'wpt', 'fail',
                                                'test_variant1-actual.txt'))]}
        self.assertEquals(self.wpt_adapter.sink.sink_requests,
                          [{'test': os.path.join('external', 'wpt', 'fail',
                                                 'test.html?variant1'),
                            'test_path': test_abs_path,
                            'result': {'actual': 'FAIL',
                                       'expected': set(['PASS']),
                                       'unexpected': True,
                                       'artifacts': baseline_artifacts}}])

    def test_result_sink_artifacts(self):
        json_dict = {
            'tests': {
                'fail': {
                    'test.html': {
                        'expected': 'PASS',
                        'actual': 'FAIL',
                        'artifacts': {
                            'wpt_actual_status': ['OK'],
                            'wpt_actual_metadata': ['test.html actual text'],
                        },
                    },
                },
            },
            'path_delimiter': os.path.sep,
        }
        self._create_json_output(json_dict)
        self.wpt_adapter.do_post_test_run_tasks()
        test_abs_path = os.path.join(WEB_TESTS_DIR,
                                     'external', 'wpt', 'fail', 'test.html')
        baseline_artifacts = {'wpt_actual_status': ['OK'],
                              'actual_text': [
                                  os.path.join('layout-test-results',
                                               'external', 'wpt', 'fail',
                                               'test-actual.txt')]}
        self.assertEquals(self.wpt_adapter.sink.sink_requests,
                          [{'test': os.path.join('external', 'wpt', 'fail',
                                                 'test.html'),
                            'test_path': test_abs_path,
                            'result': {'actual': 'FAIL',
                                       'expected': set(['PASS']),
                                       'unexpected': True,
                                       'artifacts': baseline_artifacts}}])

    def test_write_jsons(self):
        # Ensure that various JSONs are written to the correct location.

        json_dict = {
            'tests': {
                'pass': {
                    'test.html': {
                        'expected': 'PASS',
                        'actual': 'PASS',
                        'artifacts': {
                            'wpt_actual_status': ['OK'],
                        },
                    },
                },
                'unexpected_pass.html': {
                    'expected': 'FAIL',
                    'actual': 'PASS',
                    'artifacts': {
                        'wpt_actual_status': ['OK'],
                    },
                    'is_unexpected': True,
                },
                'fail.html': {
                    'expected': 'PASS',
                    'actual': 'FAIL',
                    'artifacts': {
                        'wpt_actual_status': ['ERROR'],
                    },
                    'is_unexpected': True,
                    'is_regression': True,
                },
            },
            'path_delimiter': os.path.sep,
        }
        self._create_json_output(json_dict)
        self.wpt_adapter.do_post_test_run_tasks()
        # The correctness of the output JSON is verified by other tests.
        written_files = self.wpt_adapter.fs.written_files
        self.assertEqual(written_files[OUTPUT_JSON_FILENAME],
                         written_files[os.path.join(
                             LAYOUT_TEST_RESULTS_SUBDIR, 'full_results.json')])
        # Verify JSONP
        full_results_jsonp = written_files[os.path.join(
            LAYOUT_TEST_RESULTS_SUBDIR, 'full_results_jsonp.js')]
        match = re.match(r'ADD_FULL_RESULTS\((.*)\);$', full_results_jsonp)
        self.assertIsNotNone(match)
        self.assertEqual(match.group(1),
            written_files[OUTPUT_JSON_FILENAME].decode(encoding='utf-8'))
        failing_results_jsonp = written_files[os.path.join(
            LAYOUT_TEST_RESULTS_SUBDIR, 'failing_results.json')]
        match = re.match(r'ADD_RESULTS\((.*)\);$', failing_results_jsonp)
        self.assertIsNotNone(match)
        failing_results = json.loads(match.group(1))
        # Verify filtering of failing_results.json
        self.assertIn('fail.html', failing_results['tests']['external']['wpt'])
        # We shouldn't have unexpected passes or empty dirs after filtering.
        self.assertNotIn('unexpected_pass.html',
                         failing_results['tests']['external']['wpt'])
        self.assertNotIn('pass', failing_results['tests']['external']['wpt'])

    def test_write_text_outputs(self):
        # Ensure that text outputs are written to the correct location.

        # We only generate an actual.txt if our actual wasn't PASS, so in this
        # case we shouldn't write anything.
        json_dict = {
            'tests': {
                'test.html': {
                    'expected': 'PASS',
                    'actual': 'PASS',
                    'artifacts': {
                        'wpt_actual_status': ['OK'],
                        'wpt_actual_metadata': ['test.html actual text'],
                    },
                },
            },
            'path_delimiter': os.path.sep,
        }
        self._create_json_output(json_dict)
        self.wpt_adapter.do_post_test_run_tasks()
        written_files = self.wpt_adapter.fs.written_files
        artifact_subdir = os.path.join(LAYOUT_TEST_RESULTS_SUBDIR,
                                       "external", "wpt")
        actual_path = os.path.join(artifact_subdir,
                                   "test-actual.txt")
        diff_path = os.path.join(artifact_subdir, "test-diff.txt")
        pretty_diff_path = os.path.join(artifact_subdir,
                                        "test-pretty-diff.html")
        self.assertNotIn(actual_path, written_files)
        self.assertNotIn(diff_path, written_files)
        self.assertNotIn(pretty_diff_path, written_files)

        # Now we change the outcome to be a FAIL, which should generate an
        # actual.txt
        json_dict['tests']['test.html']['actual'] = 'FAIL'
        self._create_json_output(json_dict)
        self.wpt_adapter.do_post_test_run_tasks()
        written_files = self.wpt_adapter.fs.written_files
        actual_path = os.path.join(artifact_subdir, "test-actual.txt")
        self.assertEqual("test.html actual text",
            written_files[actual_path].decode(encoding='utf-8'))
        # Ensure the artifact in the json was replaced with the location of
        # the newly-created file.
        updated_json = self._load_json_output()
        self.assertFalse(
            "wpt_actual_metadata" in updated_json["tests"]["external"]["wpt"]
                ["test.html"]["artifacts"])
        self.assertEqual(
            [actual_path],
            updated_json["tests"]["external"]["wpt"]["test.html"]["artifacts"]
                ["actual_text"])

        self.assertIn(actual_path, written_files)
        self.assertNotIn(diff_path, written_files)
        self.assertNotIn(pretty_diff_path, written_files)

    def test_write_log_artifact(self):
        # Ensure that crash log artifacts are written to the correct location.
        json_dict = {
            'tests': {
                'test.html': {
                    'expected': 'PASS',
                    'actual': 'FAIL',
                    'artifacts': {
                        'wpt_actual_status': ['ERROR'],
                        'wpt_log': ['test.html exceptions'],
                    },
                },
            },
            'path_delimiter': os.path.sep,
        }
        self._create_json_output(json_dict)
        self.wpt_adapter.do_post_test_run_tasks()
        written_files = self.wpt_adapter.fs.written_files
        artifact_subdir = os.path.join(LAYOUT_TEST_RESULTS_SUBDIR,
                                       "external", "wpt")
        stderr_path = os.path.join(artifact_subdir,
                                   "test-stderr.txt")
        self.assertEqual("test.html exceptions",
            written_files[stderr_path].decode(encoding='utf-8'))
        # Ensure the artifact in the json was replaced with the location of
        # the newly-created file.
        updated_json = self._load_json_output()
        self.assertFalse(
            "wpt_log" in updated_json["tests"]["external"]["wpt"]["test.html"]
                ["artifacts"])
        self.assertEqual(
            [stderr_path], updated_json["tests"]["external"]["wpt"]["test.html"]
                ["artifacts"]["stderr"])
        self.assertTrue(
            updated_json["tests"]["external"]["wpt"]["test.html"]["has_stderr"])

    def test_write_crashlog_artifact(self):
        # Ensure that crash log artifacts are written to the correct location.
        json_dict = {
            'tests': {
                'test.html': {
                    'expected': 'PASS',
                    'actual': 'CRASH',
                    'artifacts': {
                        'wpt_actual_status': ['CRASH'],
                        'wpt_crash_log': ['test.html crashed!'],
                    },
                },
            },
            'path_delimiter': os.path.sep,
        }
        self._create_json_output(json_dict)
        self.wpt_adapter.do_post_test_run_tasks()
        written_files = self.wpt_adapter.fs.written_files
        artifact_subdir = os.path.join(LAYOUT_TEST_RESULTS_SUBDIR,
                                       "external", "wpt")
        crash_log_path = os.path.join(artifact_subdir,
                                      "test-crash-log.txt")
        self.assertEqual("test.html crashed!",
            written_files[crash_log_path].decode(encoding='utf-8'))
        # Ensure the artifact in the json was replaced with the location of
        # the newly-created file.
        updated_json = self._load_json_output()
        self.assertFalse(
            "wpt_crash_log" in updated_json["tests"]["external"]["wpt"]
                ["test.html"]["artifacts"])
        self.assertEqual(
            [crash_log_path], updated_json["tests"]["external"]["wpt"]
                ["test.html"]["artifacts"]["crash_log"])

    def test_write_screenshot_artifacts(self):
        # Ensure that screenshots are written to the correct filenames and
        # their bytes are base64 decoded. The image diff should also be created.
        json_dict = {
            'tests': {
                'reftest.html': {
                    'expected': 'PASS',
                    'actual': 'PASS',
                    'artifacts': {
                        'wpt_actual_status': ['PASS'],
                        'screenshots': [
                            'reftest.html:abcd',
                            'reftest-ref.html:bcde',
                        ],
                    },
                },
            },
            'path_delimiter': os.path.sep,
        }
        self._create_json_output(json_dict)
        self.wpt_adapter.do_post_test_run_tasks()
        written_files = self.wpt_adapter.fs.written_files
        artifact_subdir = os.path.join(LAYOUT_TEST_RESULTS_SUBDIR,
                                       "external", "wpt")
        actual_image_path = os.path.join(artifact_subdir,
                                         "reftest-actual.png")
        self.assertEqual(base64.b64decode('abcd'),
                         written_files[actual_image_path])
        expected_image_path = os.path.join(artifact_subdir,
                                           "reftest-expected.png")
        self.assertEqual(base64.b64decode('bcde'),
                         written_files[expected_image_path])
        diff_image_path = os.path.join(artifact_subdir,
                                       "reftest-diff.png")
        # Make sure the diff image was written but don't check the contents,
        # assume that diff_image does the right thing.
        self.assertIsNotNone(written_files[diff_image_path])
        # Ensure the artifacts in the json were replaced with the location of
        # the newly-created files.
        updated_json = self._load_json_output()
        self.assertFalse(
            "screenshots" in updated_json["tests"]["external"]["wpt"]
                ["reftest.html"]["artifacts"])
        self.assertEqual(
            [actual_image_path],
            updated_json["tests"]["external"]["wpt"]["reftest.html"]
                ["artifacts"]["actual_image"])
        self.assertEqual(
            [expected_image_path],
            updated_json["tests"]["external"]["wpt"]["reftest.html"]
                ["artifacts"]["expected_image"])
        self.assertEqual(
            [diff_image_path],
            updated_json["tests"]["external"]["wpt"]["reftest.html"]
                ["artifacts"]["image_diff"])

    def test_copy_expected_output(self):
        # Check that an -expected.txt file is created from a checked-in metadata
        # ini file if it exists for a test
        json_dict = {
            'tests': {
                'test.html': {
                    'expected': 'PASS',
                    'actual': 'FAIL',
                    'artifacts': {
                        'wpt_actual_status': ['OK'],
                        'wpt_actual_metadata': ['test.html actual text'],
                    },
                },
            },
            'path_delimiter': os.path.sep,
        }
        self._create_json_output(json_dict)
        # Also create a checked-in metadata file for this test
        self.host.filesystem.write_text_file(
            os.path.join(EXTERNAL_WPT_TESTS_DIR, "test.html.ini"),
            "test.html checked-in metadata")
        self.wpt_adapter.do_post_test_run_tasks()
        written_files = self.wpt_adapter.fs.written_files
        artifact_subdir = os.path.join(LAYOUT_TEST_RESULTS_SUBDIR,
                                       "external", "wpt")
        actual_path = os.path.join(artifact_subdir,
                                   "test-actual.txt")
        self.assertEqual("test.html actual text",
            written_files[actual_path].decode(encoding='utf-8'))
        # The checked-in metadata file gets renamed from .ini to -expected.txt
        expected_path = os.path.join(artifact_subdir,
                                     "test-expected.txt")
        self.assertEqual("test.html checked-in metadata",
                         written_files[expected_path].decode(encoding='utf-8'))
        # Ensure the artifacts in the json were replaced with the locations of
        # the newly-created files.
        updated_json = self._load_json_output()
        self.assertFalse(
            "wpt_actual_metadata" in updated_json["tests"]["external"]["wpt"]
                ["test.html"]["artifacts"])
        self.assertEqual(
            [actual_path],
            updated_json["tests"]["external"]["wpt"]["test.html"]["artifacts"]
                ["actual_text"])
        self.assertEqual(
            [expected_path],
            updated_json["tests"]["external"]["wpt"]["test.html"]["artifacts"]
                ["expected_text"])

        # Ensure that a diff was also generated. There should be both additions
        # and deletions for this test since we have expected output. We don't
        # validate the entire diff files to avoid checking line numbers/markup.
        diff_path = os.path.join(artifact_subdir, "test-diff.txt")
        self.assertIn("-test.html checked-in metadata",
                      written_files[diff_path].decode(encoding='utf-8'))
        self.assertIn("+test.html actual text",
            written_files[diff_path].decode(encoding='utf-8'))
        self.assertEqual(
            [diff_path],
            updated_json["tests"]["external"]["wpt"]["test.html"]["artifacts"]
                ["text_diff"])
        pretty_diff_path = os.path.join(artifact_subdir,
                                        "test-pretty-diff.html")
        self.assertIn("test.html checked-in metadata",
                      written_files[pretty_diff_path].decode(encoding='utf-8'))
        self.assertIn("test.html actual text",
            written_files[pretty_diff_path].decode(encoding='utf-8'))
        self.assertEqual(
            [pretty_diff_path],
            updated_json["tests"]["external"]["wpt"]["test.html"]["artifacts"]
                ["pretty_text_diff"])

    def test_expected_output_for_variant(self):
        # Check that an -expected.txt file is created from a checked-in metadata
        # ini file for a variant test. Variants are a little different because
        # we have to use the manifest to map a test name to the test file, and
        # determine the associated metadata from the test file.
        # Check that an -expected.txt file is created from a checked-in metadata
        # ini file if it exists for a test
        json_dict = {
            'tests': {
                'variant.html?foo=bar/abc': {
                    'expected': 'PASS',
                    'actual': 'FAIL',
                    'artifacts': {
                        'wpt_actual_status': ['OK'],
                        'wpt_actual_metadata': ['variant bar/abc actual text'],
                    },
                },
            },
            'path_delimiter': os.path.sep,
        }
        self._create_json_output(json_dict)
        # Also create a checked-in metadata file for this test. This filename
        # matches the test *file* name, not the test name (which includes the
        # variant).
        self.host.filesystem.write_text_file(
            os.path.join(EXTERNAL_WPT_TESTS_DIR, "variant.html.ini"),
            "variant.html checked-in metadata")
        self.wpt_adapter.do_post_test_run_tasks()
        written_files = self.wpt_adapter.fs.written_files
        artifact_subdir = os.path.join(LAYOUT_TEST_RESULTS_SUBDIR,
                                       "external", "wpt")
        actual_path = os.path.join(artifact_subdir,
                                   "variant_foo=bar_abc-actual.txt")
        self.assertEqual("variant bar/abc actual text",
                         written_files[actual_path].decode(encoding='utf-8'))
        # The checked-in metadata file gets renamed from .ini to -expected.txt
        expected_path = os.path.join(artifact_subdir,
                                     "variant_foo=bar_abc-expected.txt")
        self.assertEqual("variant.html checked-in metadata",
                         written_files[expected_path].decode(encoding='utf-8'))
        # Ensure the artifacts in the json were replaced with the locations of
        # the newly-created files.
        updated_json = self._load_json_output()
        self.assertFalse(
            "wpt_actual_metadata" in updated_json["tests"]["external"]["wpt"]
                ["variant.html?foo=bar/abc"]["artifacts"])
        self.assertEqual(
            [actual_path],
            updated_json["tests"]["external"]["wpt"]["variant.html?foo=bar/abc"]
                ["artifacts"]["actual_text"])
        self.assertEqual(
            [expected_path],
            updated_json["tests"]["external"]["wpt"]["variant.html?foo=bar/abc"]
                ["artifacts"]["expected_text"])

    def test_expected_output_for_multiglob(self):
        # Check that an -expected.txt file is created from a checked-in metadata
        # ini file for a multiglobal test. Multi-globals are a little different
        # because we have to use the manifest to map a test name to the test
        # file, and determine the associated metadata from the test file.
        #
        # Also note that the "dir" is both a directory and a part of the test
        # name, so the path delimiter remains a / (ie: dir/multiglob) even on
        # Windows.
        json_dict = {
            'tests': {
                'dir/multiglob.https.any.worker.html': {
                    'expected': 'PASS',
                    'actual': 'FAIL',
                    'artifacts': {
                        'wpt_actual_status': ['OK'],
                        'wpt_actual_metadata':
                        ['dir/multiglob worker actual text'],
                    },
                },
            },
            'path_delimiter': os.path.sep,
        }
        self._create_json_output(json_dict)
        # Also create a checked-in metadata file for this test. This filename
        # matches the test *file* name, not the test name (which includes test
        # scope).
        self.host.filesystem.write_text_file(
            os.path.join(EXTERNAL_WPT_TESTS_DIR,
                         "dir/multiglob.https.any.js.ini"),
            "dir/multiglob checked-in metadata")
        self.wpt_adapter.do_post_test_run_tasks()
        written_files = self.wpt_adapter.fs.written_files
        artifact_subdir = os.path.join(LAYOUT_TEST_RESULTS_SUBDIR,
                                       "external", "wpt")
        actual_path = os.path.join(artifact_subdir,
                                   "dir/multiglob.https.any.worker-actual.txt")
        self.assertEqual("dir/multiglob worker actual text",
                         written_files[actual_path].decode(encoding='utf-8'))
        # The checked-in metadata file gets renamed from .ini to -expected.txt
        expected_path = os.path.join(
            artifact_subdir,
            "dir/multiglob.https.any.worker-expected.txt")
        self.assertEqual("dir/multiglob checked-in metadata",
                         written_files[expected_path].decode(encoding='utf-8'))
        # Ensure the artifacts in the json were replaced with the locations of
        # the newly-created files.
        updated_json = self._load_json_output()
        self.assertFalse(
            "wpt_actual_metadata" in updated_json["tests"]["external"]["wpt"]
                ["dir/multiglob.https.any.worker.html"]["artifacts"])
        self.assertEqual(
            [actual_path],
            updated_json["tests"]["external"]["wpt"]
                ["dir/multiglob.https.any.worker.html"]["artifacts"]
                ["actual_text"])
        self.assertEqual(
            [expected_path],
            updated_json["tests"]["external"]["wpt"]
                ["dir/multiglob.https.any.worker.html"]["artifacts"]
                ["expected_text"])

if __name__ == '__main__':
    unittest.main()

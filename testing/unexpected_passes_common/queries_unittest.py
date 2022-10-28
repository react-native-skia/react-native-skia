#!/usr/bin/env vpython3
# Copyright 2020 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

from __future__ import print_function

import copy
import json
import subprocess
import sys
import unittest

if sys.version_info[0] == 2:
  import mock
else:
  import unittest.mock as mock

from unexpected_passes_common import builders
from unexpected_passes_common import data_types
from unexpected_passes_common import multiprocessing_utils
from unexpected_passes_common import queries
from unexpected_passes_common import unittest_utils

queries.QUERY_DELAY = 0


class HelperMethodUnittest(unittest.TestCase):
  def testStripPrefixFromBuildIdValidId(self):
    self.assertEqual(queries._StripPrefixFromBuildId('build-1'), '1')

  def testStripPrefixFromBuildIdInvalidId(self):
    with self.assertRaises(AssertionError):
      queries._StripPrefixFromBuildId('build1')
    with self.assertRaises(AssertionError):
      queries._StripPrefixFromBuildId('build-1-2')

  def testConvertActualResultToExpectationFileFormatAbort(self):
    self.assertEqual(
        queries._ConvertActualResultToExpectationFileFormat('ABORT'), 'Timeout')


class QueryGeneratorUnittest(unittest.TestCase):
  def testSplitQueryGeneratorInitialSplit(self):
    """Tests that initial query splitting works as expected."""
    test_filter = queries.SplitQueryGenerator('ci', ['1', '2', '3'], 2)
    self.assertEqual(test_filter._test_id_lists, [['1', '2'], ['3']])
    self.assertEqual(len(test_filter.GetClauses()), 2)
    test_filter = queries.SplitQueryGenerator('ci', ['1', '2', '3'], 3)
    self.assertEqual(test_filter._test_id_lists, [['1', '2', '3']])
    self.assertEqual(len(test_filter.GetClauses()), 1)

  def testSplitQueryGeneratorSplitQuery(self):
    """Tests that SplitQueryGenerator's query splitting works."""
    test_filter = queries.SplitQueryGenerator('ci', ['1', '2'], 10)
    self.assertEqual(len(test_filter.GetClauses()), 1)
    test_filter.SplitQuery()
    self.assertEqual(len(test_filter.GetClauses()), 2)

  def testSplitQueryGeneratorSplitQueryCannotSplitFurther(self):
    """Tests that SplitQueryGenerator's failure mode."""
    test_filter = queries.SplitQueryGenerator('ci', ['1'], 1)
    with self.assertRaises(queries.QuerySplitError):
      test_filter.SplitQuery()


class QueryBuilderUnittest(unittest.TestCase):
  def setUp(self):
    self._patcher = mock.patch.object(subprocess, 'Popen')
    self._popen_mock = self._patcher.start()
    self.addCleanup(self._patcher.stop)

    builders.ClearInstance()
    unittest_utils.RegisterGenericBuildersImplementation()
    self._querier = unittest_utils.CreateGenericQuerier()

    self._relevant_file_patcher = mock.patch.object(
        self._querier,
        '_GetRelevantExpectationFilesForQueryResult',
        return_value=None)
    self._relevant_file_mock = self._relevant_file_patcher.start()
    self.addCleanup(self._relevant_file_patcher.stop)

  def testQueryFailureRaised(self):
    """Tests that a query failure is properly surfaced."""
    self._popen_mock.return_value = unittest_utils.FakeProcess(returncode=1)
    with self.assertRaises(RuntimeError):
      self._querier.QueryBuilder('builder', 'ci')

  def testInvalidNumSamples(self):
    """Tests that the number of samples is validated."""
    with self.assertRaises(AssertionError):
      unittest_utils.CreateGenericQuerier(num_samples=-1)

  def testNoResults(self):
    """Tests functionality if the query returns no results."""
    self._popen_mock.return_value = unittest_utils.FakeProcess(stdout='[]')
    results, expectation_files = self._querier.QueryBuilder('builder', 'ci')
    self.assertEqual(results, [])
    self.assertIsNone(expectation_files, None)

  def testValidResults(self):
    """Tests functionality when valid results are returned."""
    self._relevant_file_mock.return_value = ['foo_expectations']
    query_results = [
        {
            'id':
            'build-1234',
            'test_id': ('ninja://chrome/test:telemetry_gpu_integration_test/'
                        'gpu_tests.pixel_integration_test.'
                        'PixelIntegrationTest.test_name'),
            'status':
            'FAIL',
            'typ_expectations': [
                'RetryOnFailure',
            ],
            'typ_tags': [
                'win',
                'intel',
            ],
            'step_name':
            'step_name',
        },
    ]
    self._popen_mock.return_value = unittest_utils.FakeProcess(
        stdout=json.dumps(query_results))
    results, expectation_files = self._querier.QueryBuilder('builder', 'ci')
    self.assertEqual(len(results), 1)
    self.assertEqual(
        results[0],
        data_types.Result('test_name', ['win', 'intel'], 'Failure', 'step_name',
                          '1234'))
    self.assertEqual(expectation_files, ['foo_expectations'])

  def testFilterInsertion(self):
    """Tests that test filters are properly inserted into the query."""
    with mock.patch.object(
        self._querier,
        '_GetQueryGeneratorForBuilder',
        return_value=unittest_utils.SimpleFixedQueryGenerator(
            'ci', 'a real filter')), mock.patch.object(
                self._querier,
                '_RunBigQueryCommandsForJsonOutput') as query_mock:
      self._querier.QueryBuilder('builder', 'ci')
      query_mock.assert_called_once()
      query = query_mock.call_args[0][0][0]
      self.assertIn('a real filter', query)

  def testEarlyReturnOnNoFilter(self):
    """Tests that the absence of a test filter results in an early return."""
    with mock.patch.object(
        self._querier, '_GetQueryGeneratorForBuilder',
        return_value=None), mock.patch.object(
            self._querier, '_RunBigQueryCommandsForJsonOutput') as query_mock:
      results, expectation_files = self._querier.QueryBuilder('builder', 'ci')
      query_mock.assert_not_called()
      self.assertEqual(results, [])
      self.assertEqual(expectation_files, None)

  def testRetryOnMemoryLimit(self):
    """Tests that queries are split and retried if the memory limit is hit."""

    def SideEffect(*_, **__):
      SideEffect.call_count += 1
      if SideEffect.call_count == 1:
        raise queries.MemoryLimitError()
      return []

    SideEffect.call_count = 0

    with mock.patch.object(
        self._querier,
        '_GetQueryGeneratorForBuilder',
        return_value=unittest_utils.SimpleSplitQueryGenerator(
            'ci', ['filter_a', 'filter_b'], 10)), mock.patch.object(
                self._querier,
                '_RunBigQueryCommandsForJsonOutput') as query_mock:
      query_mock.side_effect = SideEffect
      self._querier.QueryBuilder('builder', 'ci')
      self.assertEqual(query_mock.call_count, 2)

      args, _ = unittest_utils.GetArgsForMockCall(query_mock.call_args_list, 0)
      first_query = args[0][0]
      self.assertIn('filter_a', first_query)
      self.assertIn('filter_b', first_query)

      args, _ = unittest_utils.GetArgsForMockCall(query_mock.call_args_list, 1)
      second_query_first_half = args[0][0]
      self.assertIn('filter_a', second_query_first_half)
      self.assertNotIn('filter_b', second_query_first_half)

      second_query_second_half = args[0][1]
      self.assertIn('filter_b', second_query_second_half)
      self.assertNotIn('filter_a', second_query_second_half)


class FillExpectationMapForBuildersUnittest(unittest.TestCase):
  def setUp(self):
    self._querier = unittest_utils.CreateGenericQuerier()

    self._query_patcher = mock.patch.object(self._querier, 'QueryBuilder')
    self._query_mock = self._query_patcher.start()
    self.addCleanup(self._query_patcher.stop)
    self._pool_patcher = mock.patch.object(multiprocessing_utils,
                                           'GetProcessPool')
    self._pool_mock = self._pool_patcher.start()
    self._pool_mock.return_value = unittest_utils.FakePool()
    self.addCleanup(self._pool_patcher.stop)
    self._filter_patcher = mock.patch.object(self._querier,
                                             '_FilterOutInactiveBuilders')
    self._filter_mock = self._filter_patcher.start()
    self._filter_mock.side_effect = lambda b, _: b
    self.addCleanup(self._filter_patcher.stop)

  def testValidResults(self):
    """Tests functionality when valid results are returned by the query."""

    def SideEffect(builder, *args):
      del args
      if builder == 'matched_builder':
        return ([
            data_types.Result('foo', ['win'], 'Pass', 'step_name', 'build_id')
        ], None)
      else:
        return ([data_types.Result('bar', [], 'Pass', 'step_name',
                                   'build_id')], None)

    self._query_mock.side_effect = SideEffect

    expectation = data_types.Expectation('foo', ['win'], 'RetryOnFailure')
    expectation_map = data_types.TestExpectationMap({
        'foo':
        data_types.ExpectationBuilderMap({
            expectation:
            data_types.BuilderStepMap(),
        }),
    })
    unmatched_results = self._querier._FillExpectationMapForBuilders(
        expectation_map, ['matched_builder', 'unmatched_builder'], 'ci')
    stats = data_types.BuildStats()
    stats.AddPassedBuild()
    expected_expectation_map = {
        'foo': {
            expectation: {
                'ci:matched_builder': {
                    'step_name': stats,
                },
            },
        },
    }
    self.assertEqual(expectation_map, expected_expectation_map)
    self.assertEqual(
        unmatched_results, {
            'ci:unmatched_builder': [
                data_types.Result('bar', [], 'Pass', 'step_name', 'build_id'),
            ],
        })

  def testQueryFailureIsSurfaced(self):
    """Tests that a query failure is properly surfaced despite being async."""
    self._query_mock.side_effect = IndexError('failure')
    with self.assertRaises(IndexError):
      self._querier._FillExpectationMapForBuilders(
          data_types.TestExpectationMap(), ['matched_builder'], 'ci')


class FilterOutInactiveBuildersUnittest(unittest.TestCase):
  def setUp(self):
    self._subprocess_patcher = mock.patch(
        'unexpected_passes_common.queries.subprocess.Popen')
    self._subprocess_mock = self._subprocess_patcher.start()
    self.addCleanup(self._subprocess_patcher.stop)

    self._querier = unittest_utils.CreateGenericQuerier()

  def testAllActiveBuilders(self):
    """Tests that no builders are removed if no inactive builders are found."""
    results = [{
        'builder_name': 'foo_builder',
    }, {
        'builder_name': 'bar_builder',
    }]
    fake_process = unittest_utils.FakeProcess(stdout=json.dumps(results))
    self._subprocess_mock.return_value = fake_process
    initial_builders = [
        'foo_builder',
        'bar_builder',
    ]
    expected_builders = copy.copy(initial_builders)
    filtered_builders = self._querier._FilterOutInactiveBuilders(
        initial_builders, 'ci')
    self.assertEqual(filtered_builders, expected_builders)

  def testInactiveBuilders(self):
    """Tests that inactive builders are removed."""
    results = [{
        'builder_name': 'foo_builder',
    }]
    fake_process = unittest_utils.FakeProcess(stdout=json.dumps(results))
    self._subprocess_mock.return_value = fake_process
    initial_builders = [
        'foo_builder',
        'bar_builder',
    ]
    expected_builders = ['foo_builder']
    filtered_builders = self._querier._FilterOutInactiveBuilders(
        initial_builders, 'ci')
    self.assertEqual(filtered_builders, expected_builders)

  def testByteConversion(self):
    """Tests that bytes are properly handled if returned."""
    results = [{
        'builder_name': 'foo_builder',
    }]
    fake_process = unittest_utils.FakeProcess(
        stdout=json.dumps(results).encode('utf-8'))
    self._subprocess_mock.return_value = fake_process
    initial_builders = [
        'foo_builder',
        'bar_builder',
    ]
    expected_builders = ['foo_builder']
    filtered_builders = self._querier._FilterOutInactiveBuilders(
        initial_builders, 'ci')
    self.assertEqual(filtered_builders, expected_builders)


class RunBigQueryCommandsForJsonOutputUnittest(unittest.TestCase):
  def setUp(self):
    self._popen_patcher = mock.patch.object(subprocess, 'Popen')
    self._popen_mock = self._popen_patcher.start()
    self.addCleanup(self._popen_patcher.stop)

    self._querier = unittest_utils.CreateGenericQuerier()

  def testJsonReturned(self):
    """Tests that valid JSON parsed from stdout is returned."""
    query_output = [{'foo': 'bar'}]
    self._popen_mock.return_value = unittest_utils.FakeProcess(
        stdout=json.dumps(query_output))
    result = self._querier._RunBigQueryCommandsForJsonOutput('', {})
    self.assertEqual(result, query_output)
    self._popen_mock.assert_called_once()

  def testJsonReturnedSplitQuery(self):
    """Tests that valid JSON is returned when a split query is used."""

    def SideEffect(*_, **__):
      SideEffect.call_count += 1
      if SideEffect.call_count == 1:
        return unittest_utils.FakeProcess(stdout=json.dumps([{'foo': 'bar'}]))
      else:
        return unittest_utils.FakeProcess(stdout=json.dumps([{'bar': 'baz'}]))

    SideEffect.call_count = 0

    self._popen_mock.side_effect = SideEffect
    result = self._querier._RunBigQueryCommandsForJsonOutput(['1', '2'], {})

    self.assertEqual(len(result), 2)
    self.assertIn({'foo': 'bar'}, result)
    self.assertIn({'bar': 'baz'}, result)

  def testExceptionRaisedOnFailure(self):
    """Tests that an exception is raised if the query fails."""
    self._popen_mock.return_value = unittest_utils.FakeProcess(returncode=1)
    with self.assertRaises(RuntimeError):
      self._querier._RunBigQueryCommandsForJsonOutput('', {})

  def testRateLimitRetrySuccess(self):
    """Tests that rate limit errors result in retries."""

    def SideEffect(*_, **__):
      SideEffect.call_count += 1
      if SideEffect.call_count == 1:
        return unittest_utils.FakeProcess(
            returncode=1, stdout='Exceeded rate limits for foo.')
      return unittest_utils.FakeProcess(stdout='[]')

    SideEffect.call_count = 0

    self._popen_mock.side_effect = SideEffect
    self._querier._RunBigQueryCommandsForJsonOutput('', {})
    self.assertEqual(self._popen_mock.call_count, 2)

  def testRateLimitRetryFailure(self):
    """Tests that rate limit errors stop retrying after enough iterations."""
    self._popen_mock.return_value = unittest_utils.FakeProcess(
        returncode=1, stdout='Exceeded rate limits for foo.')
    with self.assertRaises(RuntimeError):
      self._querier._RunBigQueryCommandsForJsonOutput('', {})
    self.assertEqual(self._popen_mock.call_count, queries.MAX_QUERY_TRIES)


class GenerateBigQueryCommandUnittest(unittest.TestCase):
  def testNoParametersSpecified(self):
    """Tests that no parameters are added if none are specified."""
    cmd = queries._GenerateBigQueryCommand('project', {})
    for element in cmd:
      self.assertFalse(element.startswith('--parameter'))

  def testParameterAddition(self):
    """Tests that specified parameters are added appropriately."""
    cmd = queries._GenerateBigQueryCommand('project', {
        '': {
            'string': 'string_value'
        },
        'INT64': {
            'int': 1
        }
    })
    self.assertIn('--parameter=string::string_value', cmd)
    self.assertIn('--parameter=int:INT64:1', cmd)

  def testBatchMode(self):
    """Tests that batch mode adds the necessary arg."""
    cmd = queries._GenerateBigQueryCommand('project', {}, batch=True)
    self.assertIn('--batch', cmd)


if __name__ == '__main__':
  unittest.main(verbosity=2)

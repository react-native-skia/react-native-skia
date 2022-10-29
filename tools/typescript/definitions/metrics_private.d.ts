// Copyright 2021 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

/** @fileoverview Definitions for chrome.metricsPrivate API */
// TODO(crbug.com/1203307): Auto-generate this file.

declare namespace chrome {
  export namespace metricsPrivate {

    export enum MetricTypeType {
      HISTOGRAM_LOG = 'histogram-log',
      HISTOGRAM_LINEAR = 'histogram-linear'
    }

    export interface MetricType {
      metricName: string;
      type: MetricTypeType;
      min: number;
      max: number;
      buckets: number;
    }

    export interface HistogramBucket {
      min: number;
      max: number;
      count: number;
    }

    export interface Histogram {
      sum: number;
      buckets: HistogramBucket[];
    }

    export function getHistogram(
        name: string, callback: (p1: Histogram) => void): void;
    export function getIsCrashReportingEnabled(callback: (p1: boolean) => void):
        void;
    export function getFieldTrial(name: string, callback: (p1: string) => void):
        void;
    export function getVariationParams(
        name: string, callback: (p1: Object|undefined) => void): void;
    export function recordUserAction(name: string): void;
    export function recordPercentage(metricName: string, value: number): void;
    export function recordCount(metricName: string, value: number): void;
    export function recordSmallCount(metricName: string, value: number): void;
    export function recordMediumCount(metricName: string, value: number): void;
    export function recordTime(metricName: string, value: number): void;
    export function recordMediumTime(metricName: string, value: number): void;
    export function recordLongTime(metricName: string, value: number): void;
    export function recordSparseValueWithHashMetricName(
      metricName: string, value: string): void;
    export function recordSparseValueWithPersistentHash(
      metricName: string, value: string): void;
    export function recordSparseValue(metricName: string, value: number): void;
    export function recordValue(metric: MetricType, value: number): void;
    export function recordBoolean(metricName: string, value: boolean): void;
    export function recordEnumerationValue(
        metricName: string, value: number, enumSize: number): void;
  }
}

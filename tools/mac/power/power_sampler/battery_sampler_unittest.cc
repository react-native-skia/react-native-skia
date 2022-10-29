// Copyright 2021 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "tools/mac/power/power_sampler/battery_sampler.h"

#include <memory>

#include <IOKit/IOKitLib.h>

#include "base/time/time.h"
#include "testing/gmock/include/gmock/gmock.h"
#include "testing/gtest/include/gtest/gtest.h"

namespace power_sampler {

namespace {

using testing::UnorderedElementsAre;

class TestBatterySampler : public BatterySampler {
 public:
  // Make public for testing.
  using BatterySampler::BatteryData;
  using BatterySampler::MaybeComputeAvgPowerConsumption;
  static std::unique_ptr<BatterySampler> CreateForTesting();
};

class BatterySamplerTest : public testing::Test {
 protected:
  using BatteryData = TestBatterySampler::BatteryData;

  void SetUp() override { set_battery_data(absl::nullopt); }

  static void set_battery_data(absl::optional<BatteryData> battery_data) {
    battery_data_ = battery_data;
  }

  // The gmock *ElementsAre* matchers are too exacting for the double values
  // in our samples, but this poor man's substitute will do for our needs.
  template <size_t N>
  void ExpectSampleMatchesArray(
      const Sampler::Sample& sample,
      const std::pair<std::string, double> (&datums)[N]) {
    EXPECT_EQ(N, sample.size());
    for (size_t i = 0; i < N; ++i) {
      const auto& name = datums[i].first;
      const double value = datums[i].second;

      auto it = sample.find(name);
      EXPECT_TRUE(it != sample.end()) << " for " << name;
      if (it != sample.end())
        EXPECT_DOUBLE_EQ(it->second, value) << " for " << name;
    }
  }

 private:
  friend class TestBatterySampler;

  static absl::optional<BatteryData> GetStaticBatteryData(
      io_service_t power_source) {
    return battery_data_;
  }

  static absl::optional<BatteryData> battery_data_;
};

absl::optional<BatterySamplerTest::BatteryData>
    BatterySamplerTest::battery_data_;

// static
std::unique_ptr<BatterySampler> TestBatterySampler::CreateForTesting() {
  return BatterySampler::CreateImpl(BatterySamplerTest::GetStaticBatteryData,
                                    base::mac::ScopedIOObject<io_service_t>());
}

}  // namespace

TEST_F(BatterySamplerTest, CreateFailsWhenNoData) {
  EXPECT_EQ(nullptr, TestBatterySampler::CreateForTesting());
}

TEST_F(BatterySamplerTest, CreateSucceedsWithData) {
  set_battery_data(TestBatterySampler::BatteryData{});
  EXPECT_NE(nullptr, TestBatterySampler::CreateForTesting());
}

TEST_F(BatterySamplerTest, NameAndGetDatumNameUnits) {
  set_battery_data(TestBatterySampler::BatteryData{});
  std::unique_ptr<BatterySampler> sampler(
      TestBatterySampler::CreateForTesting());
  ASSERT_NE(nullptr, sampler.get());

  EXPECT_EQ("battery", sampler->GetName());

  auto datum_name_units = sampler->GetDatumNameUnits();
  EXPECT_THAT(datum_name_units,
              UnorderedElementsAre(std::make_pair("external_connected", "bool"),
                                   std::make_pair("voltage", "V"),
                                   std::make_pair("current_capacity", "Ah"),
                                   std::make_pair("max_capacity", "Ah"),
                                   std::make_pair("avg_power", "W")));
}

TEST_F(BatterySamplerTest, MaybeComputeAvgPowerConsumption) {
  TestBatterySampler::BatteryData prev_data{
      .voltage_mv = 11100,           // 11.1V.
      .current_capacity_mah = 2001,  // 2.001 Ah remaining.
      .max_capacity_mah = 5225       // Corresponds to 58Wh/11.1V in mAh.
  };
  TestBatterySampler::BatteryData new_data = prev_data;

  base::TimeDelta delta = base::Minutes(1);
  // No power if the data is identical.
  auto power = TestBatterySampler::MaybeComputeAvgPowerConsumption(
      delta, prev_data, new_data);
  EXPECT_FALSE(power.has_value());

  // Adjust current capacity and max capacity by the same value, which means
  // net zero consumption.
  new_data.current_capacity_mah -= 51;
  new_data.max_capacity_mah -= 51;
  power = TestBatterySampler::MaybeComputeAvgPowerConsumption(delta, prev_data,
                                                              new_data);
  EXPECT_FALSE(power.has_value());

  // Consume 1mAh.
  new_data.current_capacity_mah -= 1;
  power = TestBatterySampler::MaybeComputeAvgPowerConsumption(delta, prev_data,
                                                              new_data);
  ASSERT_TRUE(power.has_value());
  double expected_power_w =
      (11.1 + 11.1) / 2.0 *      // Average voltage (V).
      (1.0 * 3600.0 / 1000.0) /  // Current consumption (As).
      60.0;                      // 1 minute (s).
  EXPECT_DOUBLE_EQ(expected_power_w, power.value());

  // Try a voltage change.
  new_data.voltage_mv = 11200;  // 11.2V.

  // And compute the consumption over two minutes.
  power = TestBatterySampler::MaybeComputeAvgPowerConsumption(
      2 * delta, prev_data, new_data);
  ASSERT_TRUE(power.has_value());
  expected_power_w = (11.1 + 11.2) / 2.0 *      // Average voltage (V).
                     (1.0 * 3600.0 / 1000.0) /  // Current consumption (As).
                     120.0;                     // 2 minutes (s).
}

TEST_F(BatterySamplerTest, ReturnsSamplesAndComputesPower) {
  TestBatterySampler::BatteryData battery_data{
      .external_connected = true,
      .voltage_mv = 11100,           // 11.1V.
      .current_capacity_mah = 2001,  // 2.001 Ah remaining.
      .max_capacity_mah = 5225       // Corresponds to 58Wh/11.1V in mAh.
  };
  set_battery_data(battery_data);
  std::unique_ptr<BatterySampler> sampler(
      TestBatterySampler::CreateForTesting());

  ASSERT_NE(nullptr, sampler.get());
  base::TimeTicks now = base::TimeTicks::Now();

  set_battery_data(battery_data);
  Sampler::Sample datums = sampler->GetSample(now);

  // There's no power estimate for the initial sample.
  ExpectSampleMatchesArray(datums, {std::make_pair("external_connected", true),
                                    std::make_pair("voltage", 11.1),
                                    std::make_pair("current_capacity", 2.001),
                                    std::make_pair("max_capacity", 5.225)});

  battery_data.current_capacity_mah -= 1;
  set_battery_data(battery_data);
  constexpr base::TimeDelta kOneMinute = base::Minutes(1);

  now += kOneMinute;
  datums = sampler->GetSample(now);
  // There's no power estimate because the consumed capacity in the previous
  // sample is identical to the initial state. Since the consumed capacity in
  // this sample is different than the initial state, it will be considered for
  // a power estimate in a future sample.
  ExpectSampleMatchesArray(datums, {std::make_pair("external_connected", true),
                                    std::make_pair("voltage", 11.1),
                                    std::make_pair("current_capacity", 2),
                                    std::make_pair("max_capacity", 5.225)});

  battery_data.voltage_mv = 11200;  // 11.2V.
  set_battery_data(battery_data);
  now += kOneMinute;
  datums = sampler->GetSample(now);
  // So long as there's no current consumption, there's no power estimate.
  ExpectSampleMatchesArray(datums, {std::make_pair("external_connected", true),
                                    std::make_pair("voltage", 11.2),
                                    std::make_pair("current_capacity", 2),
                                    std::make_pair("max_capacity", 5.225)});

  battery_data.current_capacity_mah -= 1;
  set_battery_data(battery_data);
  now += kOneMinute;
  datums = sampler->GetSample(now);

  double expected_power_w =
      (11.1 + 11.2) / 2.0 *      // Average voltage (V).
      (1.0 * 3600.0 / 1000.0) /  // Current consumption (As).
      120.0;                     // 2 minutes (s).
  // The above makes roughly 330mW.
  EXPECT_DOUBLE_EQ(expected_power_w, 0.3345);
  ExpectSampleMatchesArray(datums,
                           {std::make_pair("external_connected", true),
                            std::make_pair("voltage", 11.2),
                            std::make_pair("current_capacity", 1.999),
                            std::make_pair("max_capacity", 5.225),
                            std::make_pair("avg_power", expected_power_w)});
}

}  // namespace power_sampler

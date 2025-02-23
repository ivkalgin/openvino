// Copyright (C) 2018-2022 Intel Corporation
// SPDX-License-Identifier: Apache-2.0
//
#include "behavior/ov_executable_network/exec_graph_info.hpp"

#include "ie_plugin_config.hpp"
#include <common_test_utils/test_constants.hpp>

using namespace ov::test::behavior;
namespace {
const std::vector<ov::element::Type_t> netPrecisions = {
        ov::element::i8,
        ov::element::i16,
        ov::element::i32,
        ov::element::i64,
        ov::element::u8,
        ov::element::u16,
        ov::element::u32,
        ov::element::u64,
        ov::element::f16,
        ov::element::f32,
};
const std::vector<std::map<std::string, std::string>> configs = {
        {},
};
const std::vector<std::map<std::string, std::string>> multiConfigs = {
        {{InferenceEngine::MultiDeviceConfigParams::KEY_MULTI_DEVICE_PRIORITIES, CommonTestUtils::DEVICE_CPU}}};

const std::vector<std::map<std::string, std::string>> heteroConfigs = {
        {{"TARGET_FALLBACK", CommonTestUtils::DEVICE_CPU}}};

INSTANTIATE_TEST_SUITE_P(smoke_BehaviorTests,
                         OVExecGraphImportExportTest,
                         ::testing::Combine(
                                 ::testing::ValuesIn(netPrecisions),
                                 ::testing::Values(CommonTestUtils::DEVICE_CPU),
                                 ::testing::ValuesIn(configs)),
                         OVExecGraphImportExportTest::getTestCaseName);

INSTANTIATE_TEST_SUITE_P(smoke_Auto_BehaviorTests,
         OVExecGraphImportExportTest,
        ::testing::Combine(
                ::testing::ValuesIn(netPrecisions),
                ::testing::Values(CommonTestUtils::DEVICE_AUTO),
                ::testing::ValuesIn(multiConfigs)),
        OVExecGraphImportExportTest::getTestCaseName);

INSTANTIATE_TEST_SUITE_P(smoke_Hetero_BehaviorTests,
         OVExecGraphImportExportTest,
        ::testing::Combine(::testing::ValuesIn(netPrecisions),
                           ::testing::Values(CommonTestUtils::DEVICE_HETERO),
                           ::testing::ValuesIn(heteroConfigs)),
        OVExecGraphImportExportTest::getTestCaseName);

}  // namespace
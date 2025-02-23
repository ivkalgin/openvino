// Copyright (C) 2018-2022 Intel Corporation
// SPDX-License-Identifier: Apache-2.0
//

#pragma once

#include <chrono>
#include <future>
#include <gtest/gtest.h>
#include <tuple>
#include <vector>
#include <string>
#include <memory>
#include "functional_test_utils/ov_plugin_cache.hpp"
#include "ie_extension.h"
#include <condition_variable>
#include "openvino/core/shape.hpp"
#include "shared_test_classes/base/layer_test_utils.hpp"
#include "ngraph_functions/utils/ngraph_helpers.hpp"
#include "ngraph_functions/builders.hpp"
#include "transformations/utils/utils.hpp"
#include <string>
#include <ie_core.hpp>
#include <thread>
#include <base/behavior_test_utils.hpp>
#include "common_test_utils/common_utils.hpp"
#include "functional_test_utils/plugin_cache.hpp"
#include "functional_test_utils/blob_utils.hpp"
#include "ngraph_functions/subgraph_builders.hpp"
#include "shared_test_classes/subgraph/basic_lstm.hpp"

// TODO [mandrono]: move current test case inside CPU plug-in and return the original tests
namespace ov {
namespace test {
namespace behavior {

using OVInferRequestDynamicParams = std::tuple<
        std::shared_ptr<Model>,                                         // ov Model
        std::vector<std::pair<std::vector<size_t>, std::vector<size_t>>>,  // input/expected output shapes per inference
        std::string,                                                       // Device name
        std::map<std::string, std::string>                                 // Config
>;

class OVInferRequestDynamicTests : public testing::WithParamInterface<OVInferRequestDynamicParams>,
                                   public CommonTestUtils::TestsCommon {
public:
    static std::string getTestCaseName(testing::TestParamInfo<OVInferRequestDynamicParams> obj);

protected:
    void SetUp() override;

    void TearDown() override;

    std::shared_ptr<runtime::Core> ie = utils::PluginCache::get().core();
    std::shared_ptr<Model> function;
    std::string targetDevice;
    std::map<std::string, std::string> configuration;
    std::vector<std::pair<std::vector<size_t>, std::vector<size_t>>> inOutShapes;
};
using OVNotSupportRequestDynamicTests = OVInferRequestDynamicTests;
}  // namespace behavior
}  // namespace test
}  // namespace ov

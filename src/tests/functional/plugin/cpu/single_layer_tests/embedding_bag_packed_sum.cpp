// Copyright (C) 2018-2022 Intel Corporation
// SPDX-License-Identifier: Apache-2.0
//

#include <string>
#include <sstream>
#include <vector>

#include <openvino/core/partial_shape.hpp>
#include "ngraph_functions/builders.hpp"
#include "shared_test_classes/base/layer_test_utils.hpp"
#include "shared_test_classes/base/ov_subgraph.hpp"
#include "test_utils/cpu_test_utils.hpp"

using namespace InferenceEngine;
using namespace CPUTestUtils;
using namespace ov::test;

namespace CPULayerTestsDefinitions {

typedef std::tuple<
    InputShape, // input_shapes
    std::vector<std::vector<size_t>>, // indices
    bool                 // with_weights
    > embeddingBagPackedSumParams;

typedef std::tuple<
    embeddingBagPackedSumParams,
    ElementType, // embedding table
    ElementType, // indices
    LayerTestsUtils::TargetDevice> embeddingBagPackedSumLayerTestParamsSet;

class EmbeddingBagPackedSumLayerCPUTest :
        public testing::WithParamInterface<embeddingBagPackedSumLayerTestParamsSet>,
        virtual public SubgraphBaseTest,
        public CPUTestsBase {
public:
    static std::string getTestCaseName(const testing::TestParamInfo<embeddingBagPackedSumLayerTestParamsSet>& obj) {
        embeddingBagPackedSumParams params;
        ElementType netPrecision, indPrecision;
        std::string targetDevice;
        std::tie(params, netPrecision, indPrecision, targetDevice) = obj.param;

        InputShape inputShapes;
        std::vector<std::vector<size_t>> indices;
        bool withWeights;
        std::tie(inputShapes, indices, withWeights) = params;

        std::ostringstream result;
        result << "IS=" << inputShapes << "_";
        result << "I" << CommonTestUtils::vec2str(indices) << "_";
        result << "WW" << withWeights << "_";
        result << "netPRC=" << netPrecision << "_";
        result << "indPRC=" << indPrecision << "_";
        result << "targetDevice=" << targetDevice;
        return result.str();
    }

protected:
    void SetUp() override {
        embeddingBagPackedSumParams embParams;
        ElementType indPrecision;
        std::tie(embParams, inType, indPrecision, targetDevice) = this->GetParam();

        InputShape inputShapes;
        std::vector<std::vector<size_t>> indices;
        bool withWeights;
        std::tie(inputShapes, indices, withWeights) = embParams;

        selectedType = makeSelectedTypeStr("ref", inType);
        targetDevice = CommonTestUtils::DEVICE_CPU;

        init_input_shapes({ inputShapes });

        auto emb_table_node = std::make_shared<ngraph::opset1::Parameter>(inType, inputShapes.first);
        ngraph::ParameterVector params = {emb_table_node};

        auto embBag = std::dynamic_pointer_cast<ngraph::opset3::EmbeddingBagPackedSum>(ngraph::builder::makeEmbeddingBagPackedSum(
            inType,
            indPrecision,
            emb_table_node,
            indices,
            withWeights));
        ngraph::ResultVector results{std::make_shared<ngraph::opset1::Result>(embBag)};
        function = std::make_shared<ngraph::Function>(results, params, "embeddingBagPackedSum");
    }
};

TEST_P(EmbeddingBagPackedSumLayerCPUTest, CompareWithRefs) {
    SKIP_IF_CURRENT_TEST_IS_DISABLED()
    run();
    CheckPluginRelatedResults(executableNetwork, "embeddingBagPackedSum");
}

namespace {

const std::vector<ElementType> netPrecisions = {
        ElementType::f32,
        ElementType::i32,
        ElementType::u8
};

const std::vector<ElementType> indPrecisions = {
        ElementType::i64,
        ElementType::i32
};

const std::vector<InputShape> input_shapes = {
        // dynamic input shapes
        {
            // input model dynamic shapes
            {ov::Dimension::dynamic(), ov::Dimension::dynamic()},
            // input tensor shapes
            {{{5, 6}}, {10, 35}}
        },
        {
            // input model dynamic shapes
            {ov::Dimension::dynamic(), ov::Dimension::dynamic(), ov::Dimension::dynamic()},
            // input tensor shapes
            {{5, 4, 16}, {10, 12, 8}}
        },
        {
            // input model dynamic shapes with limits
            {{5, 10}, {6, 35}, {4, 8}},
            // input tensor shapes
            {{5, 6, 4}, {10, 35, 8}, {5, 6, 4}}
        },
        // static shapes
        {{5, 6}, {{5, 6}}},
        {{10, 35}, {{10, 35}}},
        {{5, 4, 16}, {{5, 4, 16}}},
};

const std::vector<std::vector<std::vector<size_t>>> indices =
        {{{0, 1}, {2, 2}, {3, 4}}, {{4, 4, 3}, {1, 0, 2}}, {{1, 2, 1, 2}, {1, 2, 1, 2}}};
const std::vector<bool> with_weights = {false, true};

const auto embBagPackedSumArgSet = ::testing::Combine(
        ::testing::ValuesIn(input_shapes),
        ::testing::ValuesIn(indices),
        ::testing::ValuesIn(with_weights)
);

INSTANTIATE_TEST_SUITE_P(smoke, EmbeddingBagPackedSumLayerCPUTest,
        ::testing::Combine(
                embBagPackedSumArgSet,
                ::testing::ValuesIn(netPrecisions),
                ::testing::ValuesIn(indPrecisions),
                ::testing::Values(CommonTestUtils::DEVICE_CPU)),
        EmbeddingBagPackedSumLayerCPUTest::getTestCaseName);
}  // namespace
}  // namespace CPULayerTestsDefinitions

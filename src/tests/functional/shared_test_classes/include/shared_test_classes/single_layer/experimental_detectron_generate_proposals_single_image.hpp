// Copyright (C) 2018-2022 Intel Corporation
// SPDX-License-Identifier: Apache-2.0
//

#include "ngraph_functions/utils/ngraph_helpers.hpp"
#include "common_test_utils/common_utils.hpp"
#include "shared_test_classes/base/ov_subgraph.hpp"

namespace ov {
namespace test {
namespace subgraph {

typedef std::tuple<
        std::vector<InputShape>,                // Input shapes
        float,                                  // min_size: minimum box width & height
        float,                                  // nms_threshold: specifies NMS threshold
        int64_t,                                // post_nms_count: number of top-n proposals after NMS
        int64_t,                                // pre_nms_count: number of top-n proposals after NMS
        std::pair<std::string, std::vector<ov::runtime::Tensor>>, // input tensors
        ElementType,                            // Network precision
        std::string                             // Device name>;
> ExperimentalDetectronGenerateProposalsSingleImageTestParams;

class ExperimentalDetectronGenerateProposalsSingleImageLayerTest :
        public testing::WithParamInterface<ExperimentalDetectronGenerateProposalsSingleImageTestParams>,
        virtual public SubgraphBaseTest {
protected:
    void SetUp() override;
    void generate_inputs(const std::vector<ngraph::Shape>& targetInputStaticShapes) override;

public:
    static std::string getTestCaseName(const testing::TestParamInfo<ExperimentalDetectronGenerateProposalsSingleImageTestParams>& obj);
};
} // namespace subgraph
} // namespace test
} // namespace ov

// Copyright (C) 2018-2022 Intel Corporation
// SPDX-License-Identifier: Apache-2.0
//

#include <gtest/gtest.h>

#include "openvino/op/ctc_loss.hpp"
#include "base_reference_test.hpp"

using namespace reference_tests;
using namespace ov;

namespace {

struct CTCLossParams {
    CTCLossParams(const bool collapseRepeated, const bool mergeRepeated, const bool findUnique, const Tensor& logitsTensor, const Tensor& logitsLenTensor,
                  const Tensor& labelsTensor, const Tensor& labelsLenTensor, const Tensor& blankIdxTensor, const Tensor& expectedTensor)
        : preprocessCollapseRepeated(collapseRepeated),
          ctcMergeRepeated(mergeRepeated),
          unique(findUnique),
          logits(logitsTensor),
          logitsLen(logitsLenTensor),
          labels(labelsTensor),
          labelsLen(labelsLenTensor),
          blankIdx(blankIdxTensor),
          expected(expectedTensor) {}

    bool preprocessCollapseRepeated;
    bool ctcMergeRepeated;
    bool unique;
    Tensor logits;
    Tensor logitsLen;
    Tensor labels;
    Tensor labelsLen;
    Tensor blankIdx;
    Tensor expected;
};

class ReferenceCTCLossLayerTest : public testing::TestWithParam<CTCLossParams>, public CommonReferenceTest {
public:
    void SetUp() override {
        auto params = GetParam();
        function = CreateFunction(params);
        inputData = {params.logits.data, params.logitsLen.data, params.labels.data, params.labelsLen.data, params.blankIdx.data};
        refOutData = {params.expected.data};
    }
    static std::string getTestCaseName(const testing::TestParamInfo<CTCLossParams>& obj) {
        auto param = obj.param;
        std::ostringstream result;
        result << "fl_pr=" << param.logits.type << "_";
        result << "int_pr=" << param.logitsLen.type << "_";
        result << "collapse=" << param.preprocessCollapseRepeated << "_";
        result << "merge=" << param.ctcMergeRepeated << "_";
        result << "unique=" << param.unique << "_";
        result << "logits_shape=" << param.logits.shape << "_";
        result << "logits_len_shape=" << param.logitsLen.shape << "_";
        result << "labels_shape=" << param.labels.shape << "_";
        result << "labels_len_shape=" << param.labelsLen.shape << "_";
        result << "blank_idx_shape=" << param.blankIdx.shape << "_";
        return result.str();
    }

private:
    static std::shared_ptr<Model> CreateFunction(const CTCLossParams& params) {
        const auto A = std::make_shared<op::v0::Parameter>(params.logits.type, params.logits.shape);        // logits
        const auto B = std::make_shared<op::v0::Parameter>(params.logitsLen.type, params.logitsLen.shape);  // logitsLen
        const auto C = std::make_shared<op::v0::Parameter>(params.labels.type, params.labels.shape);        // labels
        const auto D = std::make_shared<op::v0::Parameter>(params.labelsLen.type, params.labelsLen.shape);  // labelsLen
        const auto E = std::make_shared<op::v0::Parameter>(params.blankIdx.type, params.blankIdx.shape);    // blankIdx

        const auto ctcLoss = std::make_shared<op::v4::CTCLoss>(A, B, C, D, E, params.preprocessCollapseRepeated, params.ctcMergeRepeated, params.unique);
        return std::make_shared<ov::Model>(NodeVector {ctcLoss}, ParameterVector {A, B, C, D, E});
    }
};

TEST_P(ReferenceCTCLossLayerTest, CompareWithRefs) {
    Exec();
}

INSTANTIATE_TEST_SUITE_P(
    smoke_CTCLoss_With_Hardcoded_Refs, ReferenceCTCLossLayerTest,
    ::testing::Values(CTCLossParams(false, false, false,  // collapse repeated, merge repeated, unique
                                    Tensor({2, 3, 3}, element::f32, std::vector<float> {0, 1, 8, 5, 5, 2, 0, 7, 7, 10, 4, 5, 9, 0, 0, 5, 7, 0}),  // logits
                                    Tensor({2}, element::i32, std::vector<int> {3, 3}),                                                           // logitsLen
                                    Tensor({2, 3}, element::i32, std::vector<int> {0, 1, 2, 1, 1, 1}),                                            // labels
                                    Tensor({2}, element::i32, std::vector<int> {2, 1}),                                                           // labelsLen
                                    Tensor({}, element::i32, std::vector<int> {2}),                                                               // blankIdx
                                    Tensor({2}, element::f32, std::vector<float> {1.41223f, 14.1359f})),                                          // refOut
                      CTCLossParams(false, false, true,  // collapse repeated, merge repeated, unique
                                    Tensor({2, 3, 3}, element::f32, std::vector<float> {0, 1, 8, 5, 5, 2, 0, 7, 7, 10, 4, 5, 9, 0, 0, 5, 7, 0}),  // logits
                                    Tensor({2}, element::i32, std::vector<int> {3, 3}),                                                           // logitsLen
                                    Tensor({2, 3}, element::i32, std::vector<int> {0, 1, 2, 1, 1, 1}),                                            // labels
                                    Tensor({2}, element::i32, std::vector<int> {2, 1}),                                                           // labelsLen
                                    Tensor({}, element::i32, std::vector<int> {2}),                                                               // blankIdx
                                    Tensor({2}, element::f32, std::vector<float> {1.41223f, 14.1359f})),                                          // refOut
                      CTCLossParams(false, true, false,  // collapse repeated, merge repeated, unique
                                    Tensor({2, 3, 3}, element::f32, std::vector<float> {0, 1, 8, 5, 5, 2, 0, 7, 7, 10, 4, 5, 9, 0, 0, 5, 7, 0}),  // logits
                                    Tensor({2}, element::i32, std::vector<int> {3, 3}),                                                           // logitsLen
                                    Tensor({2, 3}, element::i32, std::vector<int> {0, 1, 2, 1, 1, 1}),                                            // labels
                                    Tensor({2}, element::i32, std::vector<int> {2, 1}),                                                           // labelsLen
                                    Tensor({}, element::i32, std::vector<int> {2}),                                                               // blankIdx
                                    Tensor({2}, element::f32, std::vector<float> {1.41156f, 13.2745f})),                                          // refOut
                      CTCLossParams(true, false, false,  // collapse repeated, merge repeated, unique
                                    Tensor({2, 3, 3}, element::f32, std::vector<float> {0, 1, 8, 5, 5, 2, 0, 7, 7, 10, 4, 5, 9, 0, 0, 5, 7, 0}),  // logits
                                    Tensor({2}, element::i32, std::vector<int> {3, 3}),                                                           // logitsLen
                                    Tensor({2, 3}, element::i32, std::vector<int> {0, 1, 2, 1, 1, 1}),                                            // labels
                                    Tensor({2}, element::i32, std::vector<int> {2, 1}),                                                           // labelsLen
                                    Tensor({}, element::i32, std::vector<int> {2}),                                                               // blankIdx
                                    Tensor({2}, element::f32, std::vector<float> {1.41223f, 14.1359f})),                                          // refOut
                      CTCLossParams(false, true, true,  // collapse repeated, merge repeated, unique
                                    Tensor({2, 3, 3}, element::f32, std::vector<float> {0, 1, 8, 5, 5, 2, 0, 7, 7, 10, 4, 5, 9, 0, 0, 5, 7, 0}),  // logits
                                    Tensor({2}, element::i32, std::vector<int> {3, 3}),                                                           // logitsLen
                                    Tensor({2, 3}, element::i32, std::vector<int> {0, 1, 2, 1, 1, 1}),                                            // labels
                                    Tensor({2}, element::i32, std::vector<int> {2, 1}),                                                           // labelsLen
                                    Tensor({}, element::i32, std::vector<int> {2}),                                                               // blankIdx
                                    Tensor({2}, element::f32, std::vector<float> {1.41156f, 13.2745f})),                                          // refOut
                      CTCLossParams(true, true, true,  // collapse repeated, merge repeated, unique
                                    Tensor({2, 3, 3}, element::f32, std::vector<float> {0, 1, 8, 5, 5, 2, 0, 7, 7, 10, 4, 5, 9, 0, 0, 5, 7, 0}),  // logits
                                    Tensor({2}, element::i32, std::vector<int> {3, 3}),                                                           // logitsLen
                                    Tensor({2, 3}, element::i32, std::vector<int> {0, 1, 2, 1, 1, 1}),                                            // labels
                                    Tensor({2}, element::i32, std::vector<int> {2, 1}),                                                           // labelsLen
                                    Tensor({}, element::i32, std::vector<int> {2}),                                                               // blankIdx
                                    Tensor({2}, element::f32, std::vector<float> {1.41223f, 13.2745f})),                                          // refOut
                    // floating point type - float16
                    CTCLossParams(false, false, false,  // collapse repeated, merge repeated, unique
                                    Tensor({2, 3, 3}, element::f16, std::vector<float16> {0, 1, 8, 5, 5, 2, 0, 7, 7, 10, 4, 5, 9, 0, 0, 5, 7, 0}),  // logits
                                    Tensor({2}, element::i32, std::vector<int> {3, 3}),                                                           // logitsLen
                                    Tensor({2, 3}, element::i32, std::vector<int> {0, 1, 2, 1, 1, 1}),                                            // labels
                                    Tensor({2}, element::i32, std::vector<int> {2, 1}),                                                           // labelsLen
                                    Tensor({}, element::i32, std::vector<int> {2}),                                                               // blankIdx
                                    Tensor({2}, element::f16, std::vector<float16> {1.41223f, 14.1359f})),                                          // refOut
                      CTCLossParams(false, false, true,  // collapse repeated, merge repeated, unique
                                    Tensor({2, 3, 3}, element::f16, std::vector<float16> {0, 1, 8, 5, 5, 2, 0, 7, 7, 10, 4, 5, 9, 0, 0, 5, 7, 0}),  // logits
                                    Tensor({2}, element::i32, std::vector<int> {3, 3}),                                                           // logitsLen
                                    Tensor({2, 3}, element::i32, std::vector<int> {0, 1, 2, 1, 1, 1}),                                            // labels
                                    Tensor({2}, element::i32, std::vector<int> {2, 1}),                                                           // labelsLen
                                    Tensor({}, element::i32, std::vector<int> {2}),                                                               // blankIdx
                                    Tensor({2}, element::f16, std::vector<float16> {1.41223f, 14.1359f})),                                          // refOut
                      CTCLossParams(false, true, false,  // collapse repeated, merge repeated, unique
                                    Tensor({2, 3, 3}, element::f16, std::vector<float16> {0, 1, 8, 5, 5, 2, 0, 7, 7, 10, 4, 5, 9, 0, 0, 5, 7, 0}),  // logits
                                    Tensor({2}, element::i32, std::vector<int> {3, 3}),                                                           // logitsLen
                                    Tensor({2, 3}, element::i32, std::vector<int> {0, 1, 2, 1, 1, 1}),                                            // labels
                                    Tensor({2}, element::i32, std::vector<int> {2, 1}),                                                           // labelsLen
                                    Tensor({}, element::i32, std::vector<int> {2}),                                                               // blankIdx
                                    Tensor({2}, element::f16, std::vector<float16> {1.41156f, 13.2745f})),                                          // refOut
                      CTCLossParams(true, false, false,  // collapse repeated, merge repeated, unique
                                    Tensor({2, 3, 3}, element::f16, std::vector<float16> {0, 1, 8, 5, 5, 2, 0, 7, 7, 10, 4, 5, 9, 0, 0, 5, 7, 0}),  // logits
                                    Tensor({2}, element::i32, std::vector<int> {3, 3}),                                                           // logitsLen
                                    Tensor({2, 3}, element::i32, std::vector<int> {0, 1, 2, 1, 1, 1}),                                            // labels
                                    Tensor({2}, element::i32, std::vector<int> {2, 1}),                                                           // labelsLen
                                    Tensor({}, element::i32, std::vector<int> {2}),                                                               // blankIdx
                                    Tensor({2}, element::f16, std::vector<float16> {1.41223f, 14.1359f})),                                          // refOut
                      CTCLossParams(false, true, true,  // collapse repeated, merge repeated, unique
                                    Tensor({2, 3, 3}, element::f16, std::vector<float16> {0, 1, 8, 5, 5, 2, 0, 7, 7, 10, 4, 5, 9, 0, 0, 5, 7, 0}),  // logits
                                    Tensor({2}, element::i32, std::vector<int> {3, 3}),                                                           // logitsLen
                                    Tensor({2, 3}, element::i32, std::vector<int> {0, 1, 2, 1, 1, 1}),                                            // labels
                                    Tensor({2}, element::i32, std::vector<int> {2, 1}),                                                           // labelsLen
                                    Tensor({}, element::i32, std::vector<int> {2}),                                                               // blankIdx
                                    Tensor({2}, element::f16, std::vector<float16> {1.41156f, 13.2745f})),                                          // refOut
                      CTCLossParams(true, true, true,  // collapse repeated, merge repeated, unique
                                    Tensor({2, 3, 3}, element::f16, std::vector<float16> {0, 1, 8, 5, 5, 2, 0, 7, 7, 10, 4, 5, 9, 0, 0, 5, 7, 0}),  // logits
                                    Tensor({2}, element::i32, std::vector<int> {3, 3}),                                                           // logitsLen
                                    Tensor({2, 3}, element::i32, std::vector<int> {0, 1, 2, 1, 1, 1}),                                            // labels
                                    Tensor({2}, element::i32, std::vector<int> {2, 1}),                                                           // labelsLen
                                    Tensor({}, element::i32, std::vector<int> {2}),                                                               // blankIdx
                                    Tensor({2}, element::f16, std::vector<float16> {1.41223f, 13.2745f}))),                                         // refOut
    ReferenceCTCLossLayerTest::getTestCaseName);
}  // namespace

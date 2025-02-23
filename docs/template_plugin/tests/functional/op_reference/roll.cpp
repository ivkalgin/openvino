// Copyright (C) 2018-2022 Intel Corporation
// SPDX-License-Identifier: Apache-2.0
//

#include <gtest/gtest.h>

#include "openvino/opsets/opset1.hpp"
#include "openvino/opsets/opset7.hpp"
#include "base_reference_test.hpp"

using namespace reference_tests;
using namespace ov;

namespace {
struct RollParams {
    RollParams(const Tensor& dataTensor, const Tensor& shiftTensor, const Tensor& axesTensor,
               const Tensor& expectedTensor, const std::string& testcaseName = "") :
               dataTensor(dataTensor), shiftTensor(shiftTensor), axesTensor(axesTensor),
               expectedTensor(expectedTensor), testcaseName(testcaseName) {}

    Tensor dataTensor;
    Tensor shiftTensor;
    Tensor axesTensor;
    Tensor expectedTensor;
    std::string testcaseName;
};

class ReferenceRollLayerTest : public testing::TestWithParam<RollParams>, public CommonReferenceTest {
public:
    void SetUp() override {
        auto params = GetParam();
        function = CreateFunction(params);
        inputData = {params.dataTensor.data};
        refOutData = {params.expectedTensor.data};
    }

    static std::string getTestCaseName(const testing::TestParamInfo<RollParams>& obj) {
        auto param = obj.param;
        std::ostringstream result;
        result << "dType=" << param.dataTensor.type;
        result << "_dShape=" << param.dataTensor.shape;
        result << "_sType=" << param.shiftTensor.type;
        result << "_sShape=" << param.shiftTensor.shape;
        result << "_aType=" << param.axesTensor.type;
        result << "_aShape=" << param.axesTensor.shape;
        result << "_eType=" << param.expectedTensor.type;
        if (param.testcaseName != "") {
            result << "_eShape=" << param.expectedTensor.shape;
            result << "_=" << param.testcaseName;
        } else {
            result << "_eShape=" << param.expectedTensor.shape;
        }
        return result.str();
    }

private:
    static std::shared_ptr<Model> CreateFunction(const RollParams& params) {
        const auto data = std::make_shared<opset1::Parameter>(params.dataTensor.type, params.dataTensor.shape);
        const auto shift = std::make_shared<opset1::Constant>(params.shiftTensor.type,
                                                              params.shiftTensor.shape,
                                                              params.shiftTensor.data.data());
        const auto axes = std::make_shared<opset1::Constant>(params.axesTensor.type,
                                                             params.axesTensor.shape,
                                                             params.axesTensor.data.data());
        const auto roll = std::make_shared<opset7::Roll>(data, shift, axes);
        return std::make_shared<Model>(NodeVector {roll}, ParameterVector {data});
    }
};

TEST_P(ReferenceRollLayerTest, CompareWithRefs) {
    Exec();
}

template <element::Type_t IN_ET>
std::vector<RollParams> generateRollParams() {
    using T = typename element_type_traits<IN_ET>::value_type;
    std::vector<RollParams> rollParams {
        // roll_repeated_axes
        RollParams(
            Tensor({4, 3}, IN_ET, std::vector<T>{1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12}),
            Tensor({3}, element::i64, std::vector<int64_t>{1, 2, 1}),
            Tensor({3}, element::i64, std::vector<int64_t>{0, 1, 0}),
            Tensor({4, 3}, IN_ET, std::vector<T>{8, 9, 7, 11, 12, 10, 2, 3, 1, 5, 6, 4}),
            "roll_repeated_axes"),

        // roll_negative_axes
        RollParams(
            Tensor({4, 2, 3}, IN_ET, std::vector<T>{1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24}),
            Tensor({3}, element::i64, std::vector<int64_t>{2, -1, -7}),
            Tensor({3}, element::i64, std::vector<int64_t>{-1, -1, -2}),
            Tensor({4, 2, 3}, IN_ET, std::vector<T>{6, 4, 5, 3, 1, 2, 12, 10, 11, 9, 7, 8, 18, 16, 17, 15, 13, 14, 24, 22, 23, 21, 19, 20}),
            "roll_negative_axes"),
    };
    return rollParams;
}

std::vector<RollParams> generateRollFloatingPointParams() {
    std::vector<RollParams> rollParams {
        // roll_2d_input
        RollParams(
            Tensor({4, 3}, element::f32, std::vector<float>{50.2907,
                                                            70.8054,
                                                            -68.3403,
                                                            62.6444,
                                                            4.9748,
                                                            -18.5551,
                                                            40.5383,
                                                            -15.3859,
                                                            -4.5881,
                                                            -43.3479,
                                                            94.1676,
                                                            -95.7097}),
            Tensor({1}, element::i64, std::vector<int64_t>{1}),
            Tensor({1}, element::i64, std::vector<int64_t>{0}),
            Tensor({4, 3}, element::f32, std::vector<float>{-43.3479,
                                                            94.1676,
                                                            -95.7097,
                                                            50.2907,
                                                            70.8054,
                                                            -68.3403,
                                                            62.6444,
                                                            4.9748,
                                                            -18.5551,
                                                            40.5383,
                                                            -15.3859,
                                                            -4.5881}),
            "roll_2d_input"),

        // roll_2d_input_negative_shift
        RollParams(
            Tensor({4, 3}, element::f32, std::vector<float>{50.2907,
                                                            70.8054,
                                                            -68.3403,
                                                            62.6444,
                                                            4.9748,
                                                            -18.5551,
                                                            40.5383,
                                                            -15.3859,
                                                            -4.5881,
                                                            -43.3479,
                                                            94.1676,
                                                            -95.7097}),
            Tensor({2}, element::i64, std::vector<int64_t>{-1, 2}),
            Tensor({2}, element::i64, std::vector<int64_t>{0, 1}),
            Tensor({4, 3}, element::f32, std::vector<float>{4.9748,
                                                            -18.5551,
                                                            62.6444,
                                                            -15.3859,
                                                            -4.5881,
                                                            40.5383,
                                                            94.1676,
                                                            -95.7097,
                                                            -43.3479,
                                                            70.8054,
                                                            -68.3403,
                                                            50.2907}),
            "roll_2d_input_negative_shift"),

        // roll_3d_input
        RollParams(
            Tensor({4, 2, 3}, element::f32, std::vector<float>{94.0773,  33.0599, 58.1724,  -20.3640, 54.5372, -54.3023, 10.4662, 11.7532,
                                                               -11.7692, 56.4223, -95.3774, 8.8978,   1.9305,  13.8025,  12.0827, 81.4669,
                                                               19.5321,  -8.9553, -75.3226, 20.8033,  20.7660, 62.7361,  14.9372, -33.0825}),
            Tensor({3}, element::i64, std::vector<int64_t>{2, 1, 3}),
            Tensor({3}, element::i64, std::vector<int64_t>{0, 1, 2}),
            Tensor({4, 2, 3}, element::f32, std::vector<float>{81.4669,  19.5321,  -8.9553, 1.9305,   13.8025,  12.0827, 62.7361,  14.9372,
                                                               -33.0825, -75.3226, 20.8033, 20.7660,  -20.3640, 54.5372, -54.3023, 94.0773,
                                                               33.0599,  58.1724,  56.4223, -95.3774, 8.8978,   10.4662, 11.7532,  -11.7692}),
            "roll_3d_input"),

        // roll_3d_input_negative_shift
        RollParams(
            Tensor({4, 2, 3}, element::f32, std::vector<float>{94.0773,  33.0599, 58.1724,  -20.3640, 54.5372, -54.3023, 10.4662, 11.7532,
                                                               -11.7692, 56.4223, -95.3774, 8.8978,   1.9305,  13.8025,  12.0827, 81.4669,
                                                               19.5321,  -8.9553, -75.3226, 20.8033,  20.7660, 62.7361,  14.9372, -33.0825}),
            Tensor({3}, element::i64, std::vector<int64_t>{-5, 1, 3}),
            Tensor({3}, element::i64, std::vector<int64_t>{0, 1, 1}),
            Tensor({4, 2, 3}, element::f32, std::vector<float>{10.4662, 11.7532,  -11.7692, 56.4223, -95.3774, 8.8978,   1.9305,  13.8025,
                                                               12.0827, 81.4669,  19.5321,  -8.9553, -75.3226, 20.8033,  20.7660, 62.7361,
                                                               14.9372, -33.0825, 94.0773,  33.0599, 58.1724,  -20.3640, 54.5372, -54.3023}),
            "roll_3d_input_negative_shift"),
    };
    return rollParams;
}

std::vector<RollParams> generateRollCombinedParams() {
    const std::vector<std::vector<RollParams>> rollTypeParams {
        generateRollParams<element::Type_t::i8>(),
        generateRollParams<element::Type_t::i16>(),
        generateRollParams<element::Type_t::i32>(),
        generateRollParams<element::Type_t::i64>(),
        generateRollParams<element::Type_t::u8>(),
        generateRollParams<element::Type_t::u16>(),
        generateRollParams<element::Type_t::u32>(),
        generateRollParams<element::Type_t::u64>(),
        generateRollParams<element::Type_t::bf16>(),
        generateRollParams<element::Type_t::f16>(),
        generateRollParams<element::Type_t::f32>(),
        generateRollParams<element::Type_t::f64>(),
        generateRollFloatingPointParams(),
    };
    std::vector<RollParams> combinedParams;

    for (const auto& params : rollTypeParams) {
        combinedParams.insert(combinedParams.end(), params.begin(), params.end());
    }
    return combinedParams;
}

INSTANTIATE_TEST_SUITE_P(smoke_Roll_With_Hardcoded_Refs, ReferenceRollLayerTest,
    testing::ValuesIn(generateRollCombinedParams()), ReferenceRollLayerTest::getTestCaseName);
} // namespace

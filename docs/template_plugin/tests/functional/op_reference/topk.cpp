// Copyright (C) 2018-2022 Intel Corporation
// SPDX-License-Identifier: Apache-2.0
//

#include <gtest/gtest.h>

#include "openvino/opsets/opset3.hpp"
#include "openvino/opsets/opset1.hpp"
#include "base_reference_test.hpp"

using namespace reference_tests;
using namespace ov;

namespace {
struct TopKParams {
    TopKParams(
        const Tensor& A, const Tensor& k, const int64_t axis,
        const opset1::TopK::Mode mode, const opset1::TopK::SortType sort,
        const Tensor& result0, const Tensor& result1, const size_t outIdx,
        const std::string& testcaseName = "") :
        A(A), k(k), axis(axis), mode(mode), sort(sort),
        result0(result0), result1(result1), outIdx(outIdx),
        testcaseName(testcaseName) {}

    Tensor A;
    Tensor k;
    int64_t axis;
    opset1::TopK::Mode mode;
    opset1::TopK::SortType sort;
    Tensor result0;
    Tensor result1;
    size_t outIdx;
    std::string testcaseName;
};

class ReferenceTopKTest : public testing::TestWithParam<TopKParams>, public CommonReferenceTest {
public:
    static std::string getTestCaseName(const testing::TestParamInfo<TopKParams>& obj) {
        auto param = obj.param;
        std::ostringstream result;
        result << "aType=" << param.A.type;
        result << "_aShape=" << param.A.shape;
        result << "_kType=" << param.k.type;
        result << "_kShape=" << param.k.shape;
        result << "_axis=" << param.axis;
        result << "_r0Type=" << param.result0.type;
        result << "_r0Shape=" << param.result0.shape;
        result << "_r1Type=" << param.result1.type;
        result << "_r1Shape=" << param.result1.shape;
        result << "_=" << param.testcaseName;
        return result.str();
    }
};

struct TopKParamsResnet50 {
    TopKParamsResnet50(
        const Tensor& A,
        const Tensor& result5Value, const Tensor& result5Index,
        const Tensor& result1Value, const Tensor& result1Index,
        const std::string& testcaseName = "") :
        A(A),
        result5Value(result5Value), result5Index(result5Index),
        result1Value(result1Value), result1Index(result1Index),
        testcaseName(testcaseName) {}

    Tensor A;
    Tensor result5Value;
    Tensor result5Index;
    Tensor result1Value;
    Tensor result1Index;
    std::string testcaseName;
};

class ReferenceTopKTestResnet50 : public testing::TestWithParam<TopKParamsResnet50>, public CommonReferenceTest {
public:
    void SetUp() override {
        auto params = GetParam();
        function = CreateFunction(params);
        inputData = {params.A.data};
        refOutData = {params.result5Value.data, params.result5Index.data,
                      params.result1Value.data, params.result1Index.data};
    }

    static std::string getTestCaseName(const testing::TestParamInfo<TopKParamsResnet50>& obj) {
        auto param = obj.param;
        std::ostringstream result;
        result << "aType=" << param.A.type;
        result << "_aShape=" << param.A.shape;
        result << "_r5vType=" << param.result5Value.type;
        result << "_r5vShape=" << param.result5Value.shape;
        result << "_r5iType=" << param.result5Index.type;
        result << "_r5iShape=" << param.result5Index.shape;
        result << "_r1vType=" << param.result1Value.type;
        result << "_r1vShape=" << param.result1Value.shape;
        result << "_r1iType=" << param.result1Index.type;
        result << "_r1iShape=" << param.result1Index.shape;
        result << "_=" << param.testcaseName;
        return result.str();
    }

private:
    static std::shared_ptr<Model> CreateFunction(const TopKParamsResnet50& params) {
        const auto A = std::make_shared<opset1::Parameter>(params.A.type,
                                                           params.A.shape);
        const auto B = std::make_shared<opset1::TopK>(A,
                                                      opset1::Constant::create(element::i64, {}, {5}),
                                                      1,
                                                      opset1::TopK::Mode::MAX,
                                                      opset1::TopK::SortType::SORT_VALUES);
        const auto C = std::make_shared<opset1::TopK>(A,
                                                      opset1::Constant::create(element::i64, {}, {1}),
                                                      1,
                                                      opset1::TopK::Mode::MAX,
                                                      opset1::TopK::SortType::SORT_VALUES);

        const auto out5_value = B->output(0);
        const auto out5_index = B->output(1);
        const auto out1_value = C->output(0);
        const auto out1_index = C->output(1);
        const auto f = std::make_shared<Model>(OutputVector{out5_value, out5_index, out1_value, out1_index}, ParameterVector{A});
        return f;
    }
};

TEST_P(ReferenceTopKTestResnet50, CompareWithRefs) {
    Exec();
}

template <element::Type_t ET, element::Type_t ET_OUT>
std::vector<TopKParamsResnet50> generateParamsResnet50() {
    using T = typename element_type_traits<ET>::value_type;
    using T_OUT = typename element_type_traits<ET_OUT>::value_type;
    std::vector<TopKParamsResnet50> params {
        TopKParamsResnet50(
            Tensor(ET, {128, 1000}, [](std::vector<size_t> shape) -> std::vector<T> {
                std::vector<T> data;
                for (size_t i = 0; i < shape[0]; i++) {
                    for (size_t j = 0; j < shape[1]; j++) {
                        data.push_back(j);
                    }
                }
                return data;
            }({128, 1000})),
            Tensor(ET, {128, 5}, [](std::vector<size_t> rshape, std::vector<size_t> shape) -> std::vector<T>{
                std::vector<T> expected_value;
                for (size_t i = 0; i < rshape[0]; i++) {
                    for (size_t j = 0; j < rshape[1]; j++) {
                        expected_value.push_back(shape[1] - j - 1);
                    }
                }
                return expected_value;
            }({128, 5}, {128, 1000})),
            Tensor(ET_OUT, {128, 5}, [](std::vector<size_t> rshape, std::vector<size_t> shape) -> std::vector<T_OUT>{
                std::vector<T_OUT> expected_index;
                for (size_t i = 0; i < rshape[0]; i++) {
                    for (size_t j = 0; j < rshape[1]; j++) {
                        expected_index.push_back(shape[1] - j - 1);
                    }
                }
                return expected_index;
            }({128, 5}, {128, 1000})),
            Tensor(ET, {128, 1}, [](std::vector<size_t> rshape, std::vector<size_t> shape) -> std::vector<T>{
                std::vector<T> expected_value;
                for (size_t i = 0; i < rshape[0]; i++) {
                    for (size_t j = 0; j < rshape[1]; j++) {
                        expected_value.push_back(shape[1] - j - 1);
                    }
                }
                return expected_value;
            }({128, 1}, {128, 1000})),
            Tensor(ET_OUT, {128, 1}, [](std::vector<size_t> rshape, std::vector<size_t> shape) -> std::vector<T_OUT>{
                std::vector<T_OUT> expected_index;
                for (size_t i = 0; i < rshape[0]; i++) {
                    for (size_t j = 0; j < rshape[1]; j++) {
                        expected_index.push_back(shape[1] - j - 1);
                    }
                }
                return expected_index;
            }({128, 1}, {128, 1000})),
            "topk_resnet50"),
    };
    return params;
}

std::vector<TopKParamsResnet50> generateCombinedParamsResnet50() {
    const std::vector<std::vector<TopKParamsResnet50>> generatedParams {
        generateParamsResnet50<element::Type_t::i8, element::Type_t::i32>(),
        generateParamsResnet50<element::Type_t::i16, element::Type_t::i32>(),
        generateParamsResnet50<element::Type_t::i32, element::Type_t::i32>(),
        generateParamsResnet50<element::Type_t::i64, element::Type_t::i32>(),
        generateParamsResnet50<element::Type_t::u8, element::Type_t::i32>(),
        generateParamsResnet50<element::Type_t::u16, element::Type_t::i32>(),
        generateParamsResnet50<element::Type_t::u32, element::Type_t::i32>(),
        generateParamsResnet50<element::Type_t::u64, element::Type_t::i32>(),
        generateParamsResnet50<element::Type_t::bf16, element::Type_t::i32>(),
        generateParamsResnet50<element::Type_t::f16, element::Type_t::i32>(),
        generateParamsResnet50<element::Type_t::f32, element::Type_t::i32>(),
        generateParamsResnet50<element::Type_t::f64, element::Type_t::i32>(),
    };
    std::vector<TopKParamsResnet50> combinedParams;

    for (const auto& params : generatedParams) {
        combinedParams.insert(combinedParams.end(), params.begin(), params.end());
    }
    return combinedParams;
}

INSTANTIATE_TEST_SUITE_P(smoke_TopK_With_Hardcoded_Refs, ReferenceTopKTestResnet50,
    testing::ValuesIn(generateCombinedParamsResnet50()), ReferenceTopKTestResnet50::getTestCaseName);

class ReferenceTopKTestMaxMinSort : public ReferenceTopKTest {
public:
    void SetUp() override {
        auto params = GetParam();
        function = CreateFunction(params);
        inputData = {params.A.data};
        refOutData = {params.result0.data, params.result1.data};
    }

private:
    static std::shared_ptr<Model> CreateFunction(const TopKParams& params) {
        const auto A = std::make_shared<opset1::Parameter>(params.A.type,
                                                           params.A.shape);
        const auto k = opset1::Constant::create(params.k.type,
                                                params.k.shape,
                                                params.k.data.data());
        const auto B = std::make_shared<opset1::TopK>(A, k, params.axis, params.mode, params.sort);
        const auto f = std::make_shared<Model>(B->outputs(), ParameterVector{A});
        return f;
    }
};

TEST_P(ReferenceTopKTestMaxMinSort, CompareWithRefs) {
    Exec();
}

template <element::Type_t ET, element::Type_t ET2, element::Type_t ET_OUT>
std::vector<TopKParams> generateParamsMaxMinSort() {
    using T = typename element_type_traits<ET>::value_type;
    using T2 = typename element_type_traits<ET2>::value_type;
    using T_OUT = typename element_type_traits<ET_OUT>::value_type;
    std::vector<TopKParams> params {
        TopKParams(
            Tensor(ET, {128, 1000}, [](std::vector<size_t> shape) -> std::vector<T> {
                std::vector<T> data;
                for (size_t i = 0; i < shape[0]; i++) {
                    for (size_t j = 0; j < shape[1]; j++) {
                        data.push_back(j);
                    }
                }
                return data;
            }({128, 1000})),
            Tensor(ET2, {}, std::vector<T2>{5}),
            1,
            opset1::TopK::Mode::MAX,
            opset1::TopK::SortType::NONE,
            Tensor(ET, {128, 5}, [](std::vector<size_t> rshape, std::vector<size_t> shape) -> std::vector<T>{
                std::vector<T> expected_value;
                for (size_t i = 0; i < rshape[0]; i++) {
                    expected_value.push_back(shape[1] - 3);
                    expected_value.push_back(shape[1] - 1);
                    expected_value.push_back(shape[1] - 2);
                    expected_value.push_back(shape[1] - 5);
                    expected_value.push_back(shape[1] - 4);
                }
                return expected_value;
            }({128, 5}, {128, 1000})),
            Tensor(ET_OUT, {128, 5}, [](std::vector<size_t> rshape, std::vector<size_t> shape) -> std::vector<T_OUT>{
                std::vector<T_OUT> expected_index;
                for (size_t i = 0; i < rshape[0]; i++) {
                    expected_index.push_back(shape[1] - 3);
                    expected_index.push_back(shape[1] - 1);
                    expected_index.push_back(shape[1] - 2);
                    expected_index.push_back(shape[1] - 5);
                    expected_index.push_back(shape[1] - 4);
                }
                return expected_index;
            }({128, 5}, {128, 1000})),
            0,
            "topk_max_sort_none"),

        TopKParams(
            Tensor(ET, {128, 1000}, [](std::vector<size_t> shape) -> std::vector<T> {
                std::vector<T> data;
                for (size_t i = 0; i < shape[0]; i++) {
                    for (size_t j = 0; j < shape[1]; j++) {
                        data.push_back(j);
                    }
                }
                return data;
            }({128, 1000})),
            Tensor(ET2, {}, std::vector<T2>{5}),
            1,
            opset1::TopK::Mode::MIN,
            opset1::TopK::SortType::NONE,
            Tensor(ET, {128, 5}, [](std::vector<size_t> rshape) -> std::vector<T>{
                std::vector<T> expected_value;
                for (size_t i = 0; i < rshape[0]; i++) {
                    expected_value.push_back(1);
                    expected_value.push_back(0);
                    expected_value.push_back(3);
                    expected_value.push_back(2);
                    expected_value.push_back(4);
                }
                return expected_value;
            }({128, 5})),
            Tensor(ET_OUT, {128, 5}, [](std::vector<size_t> rshape) -> std::vector<T_OUT>{
                std::vector<T_OUT> expected_index;
                for (size_t i = 0; i < rshape[0]; i++) {
                    expected_index.push_back(1);
                    expected_index.push_back(0);
                    expected_index.push_back(3);
                    expected_index.push_back(2);
                    expected_index.push_back(4);
                }
                return expected_index;
            }({128, 5})),
            0,
            "topk_min_sort_none"),

        TopKParams(
            Tensor(ET, {128, 1000}, [](std::vector<size_t> shape) -> std::vector<T> {
                std::vector<T> data;
                for (size_t i = 0; i < shape[0]; i++) {
                    for (size_t j = 0; j < shape[1]; j++) {
                        data.push_back(j);
                    }
                }
                return data;
            }({128, 1000})),
            Tensor(ET2, {}, std::vector<T2>{5}),
            1,
            opset1::TopK::Mode::MAX,
            opset1::TopK::SortType::SORT_VALUES,
            Tensor(ET, {128, 5}, [](std::vector<size_t> rshape, std::vector<size_t> shape) -> std::vector<T>{
                std::vector<T> expected_value;
                for (size_t i = 0; i < rshape[0]; i++) {
                    for (size_t j = 0; j < rshape[1]; j++) {
                        expected_value.push_back(shape[1] - j - 1);
                    }
                }
                return expected_value;
            }({128, 5}, {128, 1000})),
            Tensor(ET_OUT, {128, 5}, [](std::vector<size_t> rshape, std::vector<size_t> shape) -> std::vector<T_OUT>{
                std::vector<T_OUT> expected_index;
                for (size_t i = 0; i < rshape[0]; i++) {
                    for (size_t j = 0; j < rshape[1]; j++) {
                        expected_index.push_back(shape[1] - j - 1);
                    }
                }
                return expected_index;
            }({128, 5}, {128, 1000})),
            0,
            "topk_max_sort_value"),

        TopKParams(
            Tensor(ET, {128, 1000}, [](std::vector<size_t> shape) -> std::vector<T> {
                std::vector<T> data;
                for (size_t i = 0; i < shape[0]; i++) {
                    for (size_t j = 0; j < shape[1]; j++) {
                        data.push_back(j);
                    }
                }
                return data;
            }({128, 1000})),
            Tensor(ET2, {}, std::vector<T2>{5}),
            1,
            opset1::TopK::Mode::MIN,
            opset1::TopK::SortType::SORT_VALUES,
            Tensor(ET, {128, 5}, [](std::vector<size_t> rshape) -> std::vector<T>{
                std::vector<T> expected_value;
                for (size_t i = 0; i < rshape[0]; i++) {
                    for (size_t j = 0; j < rshape[1]; j++) {
                        expected_value.push_back(j);
                    }
                }
                return expected_value;
            }({128, 5})),
            Tensor(ET_OUT, {128, 5}, [](std::vector<size_t> rshape) -> std::vector<T_OUT>{
                std::vector<T_OUT> expected_index;
                for (size_t i = 0; i < rshape[0]; i++) {
                    for (size_t j = 0; j < rshape[1]; j++) {
                        expected_index.push_back(j);
                    }
                }
                return expected_index;
            }({128, 5})),
            0,
            "topk_min_sort_value"),

        TopKParams(
            Tensor(ET, {128, 1000}, [](std::vector<size_t> shape) -> std::vector<T> {
                std::vector<T> data;
                for (size_t i = 0; i < shape[0]; i++) {
                    for (size_t j = 0; j < shape[1]; j++) {
                        data.push_back(j);
                    }
                }
                return data;
            }({128, 1000})),
            Tensor(ET2, {}, std::vector<T2>{5}),
            1,
            opset1::TopK::Mode::MAX,
            opset1::TopK::SortType::SORT_INDICES,
            Tensor(ET, {128, 5}, [](std::vector<size_t> rshape, std::vector<size_t> shape) -> std::vector<T>{
                std::vector<T> expected_value;
                for (size_t i = 0; i < rshape[0]; i++) {
                    expected_value.push_back(shape[1] - 5);
                    expected_value.push_back(shape[1] - 4);
                    expected_value.push_back(shape[1] - 3);
                    expected_value.push_back(shape[1] - 2);
                    expected_value.push_back(shape[1] - 1);
                }
                return expected_value;
            }({128, 5}, {128, 1000})),
            Tensor(ET_OUT, {128, 5}, [](std::vector<size_t> rshape, std::vector<size_t> shape) -> std::vector<T_OUT>{
                std::vector<T_OUT> expected_index;
                for (size_t i = 0; i < rshape[0]; i++) {
                    expected_index.push_back(shape[1] - 5);
                    expected_index.push_back(shape[1] - 4);
                    expected_index.push_back(shape[1] - 3);
                    expected_index.push_back(shape[1] - 2);
                    expected_index.push_back(shape[1] - 1);
                }
                return expected_index;
            }({128, 5}, {128, 1000})),
            0,
            "topk_max_sort_index"),

        TopKParams(
            Tensor(ET, {128, 1000}, [](std::vector<size_t> shape) -> std::vector<T> {
                std::vector<T> data;
                for (size_t i = 0; i < shape[0]; i++) {
                    for (size_t j = 0; j < shape[1]; j++) {
                        data.push_back(j);
                    }
                }
                return data;
            }({128, 1000})),
            Tensor(ET2, {}, std::vector<T2>{5}),
            1,
            opset1::TopK::Mode::MIN,
            opset1::TopK::SortType::SORT_INDICES,
            Tensor(ET, {128, 5}, [](std::vector<size_t> rshape) -> std::vector<T>{
                std::vector<T> expected_value;
                for (size_t i = 0; i < rshape[0]; i++) {
                    for (size_t j = 0; j < rshape[1]; j++) {
                        expected_value.push_back(j);
                    }
                }
                return expected_value;
            }({128, 5})),
            Tensor(ET_OUT, {128, 5}, [](std::vector<size_t> rshape) -> std::vector<T_OUT>{
                std::vector<T_OUT> expected_index;
                for (size_t i = 0; i < rshape[0]; i++) {
                    for (size_t j = 0; j < rshape[1]; j++) {
                        expected_index.push_back(j);
                    }
                }
                return expected_index;
            }({128, 5})),
            0,
            "topk_min_sort_index"),

        TopKParams(
            Tensor(ET, {5}, std::vector<T>{3, 1, 2, 5, 4}),
            Tensor(ET2, {}, std::vector<T2>{3}),
            0,
            opset1::TopK::Mode::MAX,
            opset1::TopK::SortType::SORT_VALUES,
            Tensor(ET, {3}, std::vector<T>{5, 4, 3}),
            Tensor(ET_OUT, {3}, std::vector<T_OUT>{3, 4, 0}),
            0,
            "topk_mode_sort_order"),

        TopKParams(
            Tensor(ET, {5}, std::vector<T>{3, 1, 2, 5, 4}),
            Tensor(ET2, {}, std::vector<T2>{3}),
            0,
            opset1::TopK::Mode::MAX,
            opset1::TopK::SortType::SORT_INDICES,
            Tensor(ET, {3}, std::vector<T>{3, 5, 4}),
            Tensor(ET_OUT, {3}, std::vector<T_OUT>{0, 3, 4}),
            0,
            "topk_mode_sort_order_1"),

        TopKParams(
            Tensor(ET, {5}, std::vector<T>{3, 1, 2, 5, 4}),
            Tensor(ET2, {}, std::vector<T2>{3}),
            0,
            opset1::TopK::Mode::MIN,
            opset1::TopK::SortType::SORT_VALUES,
            Tensor(ET, {3}, std::vector<T>{1, 2, 3}),
            Tensor(ET_OUT, {3}, std::vector<T_OUT>{1, 2, 0}),
            0,
            "topk_mode_sort_order_2"),

        TopKParams(
            Tensor(ET, {5}, std::vector<T>{3, 1, 2, 5, 4}),
            Tensor(ET2, {}, std::vector<T2>{3}),
            0,
            opset1::TopK::Mode::MIN,
            opset1::TopK::SortType::SORT_INDICES,
            Tensor(ET, {3}, std::vector<T>{3, 1, 2}),
            Tensor(ET_OUT, {3}, std::vector<T_OUT>{0, 1, 2}),
            0,
            "topk_mode_sort_order_3"),
    };
    return params;
}

std::vector<TopKParams> generateCombinedParamsMaxMinSort() {
    const std::vector<std::vector<TopKParams>> generatedParams {
        generateParamsMaxMinSort<element::Type_t::i8, element::Type_t::i64, element::Type_t::i32>(),
        generateParamsMaxMinSort<element::Type_t::i16, element::Type_t::i64, element::Type_t::i32>(),
        generateParamsMaxMinSort<element::Type_t::i32, element::Type_t::i64, element::Type_t::i32>(),
        generateParamsMaxMinSort<element::Type_t::i64, element::Type_t::i64, element::Type_t::i32>(),
        generateParamsMaxMinSort<element::Type_t::u8, element::Type_t::i64, element::Type_t::i32>(),
        generateParamsMaxMinSort<element::Type_t::u16, element::Type_t::i64, element::Type_t::i32>(),
        generateParamsMaxMinSort<element::Type_t::u32, element::Type_t::i64, element::Type_t::i32>(),
        generateParamsMaxMinSort<element::Type_t::u64, element::Type_t::i64, element::Type_t::i32>(),
        generateParamsMaxMinSort<element::Type_t::bf16, element::Type_t::i64, element::Type_t::i32>(),
        generateParamsMaxMinSort<element::Type_t::f16, element::Type_t::i64, element::Type_t::i32>(),
        generateParamsMaxMinSort<element::Type_t::f32, element::Type_t::i64, element::Type_t::i32>(),
        generateParamsMaxMinSort<element::Type_t::f64, element::Type_t::i64, element::Type_t::i32>(),
    };
    std::vector<TopKParams> combinedParams;

    for (const auto& params : generatedParams) {
        combinedParams.insert(combinedParams.end(), params.begin(), params.end());
    }
    return combinedParams;
}

INSTANTIATE_TEST_SUITE_P(smoke_TopK_With_Hardcoded_Refs, ReferenceTopKTestMaxMinSort,
    testing::ValuesIn(generateCombinedParamsMaxMinSort()), ReferenceTopKTest::getTestCaseName);

class ReferenceTopKTestBackend : public ReferenceTopKTest {
public:
    void SetUp() override {
        auto params = GetParam();
        function = CreateFunction(params);
        inputData = {params.A.data};
        refOutData = {params.result0.data, params.result1.data};
    }

private:
    static std::shared_ptr<Model> CreateFunction(const TopKParams& params) {
        const auto A = std::make_shared<opset1::Parameter>(params.A.type,
                                                           params.A.shape);
        const auto k = opset1::Constant::create(params.k.type,
                                                params.k.shape,
                                                params.k.data.data());
        const auto B = std::make_shared<opset1::TopK>(A, k, params.axis, params.mode, params.sort);
        const auto f = std::make_shared<Model>(B->outputs(), ParameterVector{A});
        return f;
    }
};

TEST_P(ReferenceTopKTestBackend, CompareWithRefs) {
    Exec();
}

template <element::Type_t ET, element::Type_t ET2, element::Type_t ET_OUT>
std::vector<TopKParams> generateParamsV3() {
    using T = typename element_type_traits<ET>::value_type;
    using T2 = typename element_type_traits<ET2>::value_type;
    using T_OUT = typename element_type_traits<ET_OUT>::value_type;
    std::vector<TopKParams> params {
        TopKParams(
            Tensor(ET, {5}, std::vector<T>{3, 1, 2, 5, 4}),
            Tensor(ET2, {}, std::vector<T2>{3}),
            0,
            opset1::TopK::Mode::MAX,
            opset1::TopK::SortType::SORT_VALUES,
            Tensor(ET, {3}, std::vector<T>{5, 4, 3}),
            Tensor(ET_OUT, {3}, std::vector<T_OUT>{3, 4, 0}),
            0,
            "topk_mode_sort_order"),

        TopKParams(
            Tensor(ET, {5}, std::vector<T>{3, 1, 2, 5, 4}),
            Tensor(ET2, {}, std::vector<T2>{3}),
            0,
            opset1::TopK::Mode::MAX,
            opset1::TopK::SortType::SORT_INDICES,
            Tensor(ET, {3}, std::vector<T>{3, 5, 4}),
            Tensor(ET_OUT, {3}, std::vector<T_OUT>{0, 3, 4}),
            0,
            "topk_mode_sort_order_1"),

        TopKParams(
            Tensor(ET, {5}, std::vector<T>{3, 1, 2, 5, 4}),
            Tensor(ET2, {}, std::vector<T2>{3}),
            0,
            opset1::TopK::Mode::MIN,
            opset1::TopK::SortType::SORT_VALUES,
            Tensor(ET, {3}, std::vector<T>{1, 2, 3}),
            Tensor(ET_OUT, {3}, std::vector<T_OUT>{1, 2, 0}),
            0,
            "topk_mode_sort_order_2"),

        TopKParams(
            Tensor(ET, {5}, std::vector<T>{3, 1, 2, 5, 4}),
            Tensor(ET2, {}, std::vector<T2>{3}),
            0,
            opset1::TopK::Mode::MIN,
            opset1::TopK::SortType::SORT_INDICES,
            Tensor(ET, {3}, std::vector<T>{3, 1, 2}),
            Tensor(ET_OUT, {3}, std::vector<T_OUT>{0, 1, 2}),
            0,
            "topk_mode_sort_order_3"),
    };
    return params;
}

std::vector<TopKParams> generateCombinedParamsBackend() {
    const std::vector<std::vector<TopKParams>> generatedParams {
        generateParamsMaxMinSort<element::Type_t::i8, element::Type_t::i64, element::Type_t::i32>(),
        generateParamsMaxMinSort<element::Type_t::i16, element::Type_t::i64, element::Type_t::i32>(),
        generateParamsMaxMinSort<element::Type_t::i32, element::Type_t::i64, element::Type_t::i32>(),
        generateParamsMaxMinSort<element::Type_t::i64, element::Type_t::i64, element::Type_t::i32>(),
        generateParamsMaxMinSort<element::Type_t::u8, element::Type_t::i64, element::Type_t::i32>(),
        generateParamsMaxMinSort<element::Type_t::u16, element::Type_t::i64, element::Type_t::i32>(),
        generateParamsMaxMinSort<element::Type_t::u32, element::Type_t::i64, element::Type_t::i32>(),
        generateParamsMaxMinSort<element::Type_t::u64, element::Type_t::i64, element::Type_t::i32>(),
        generateParamsMaxMinSort<element::Type_t::bf16, element::Type_t::i64, element::Type_t::i32>(),
        generateParamsMaxMinSort<element::Type_t::f16, element::Type_t::i64, element::Type_t::i32>(),
        generateParamsMaxMinSort<element::Type_t::f32, element::Type_t::i64, element::Type_t::i32>(),
        generateParamsMaxMinSort<element::Type_t::f64, element::Type_t::i64, element::Type_t::i32>(),
    };
    std::vector<TopKParams> combinedParams;

    for (const auto& params : generatedParams) {
        combinedParams.insert(combinedParams.end(), params.begin(), params.end());
    }
    return combinedParams;
}

INSTANTIATE_TEST_SUITE_P(smoke_TopK_With_Hardcoded_Refs, ReferenceTopKTestBackend,
    testing::ValuesIn(generateCombinedParamsBackend()), ReferenceTopKTest::getTestCaseName);

class ReferenceTopKTest1dMaxMin : public ReferenceTopKTest {
public:
    void SetUp() override {
        auto params = GetParam();
        function = CreateFunction(params, params.outIdx);
        inputData = {params.A.data};
        if (params.outIdx != 0) {
            refOutData = {params.result1.data};
        } else {
            refOutData = {params.result0.data};
        }
    }

    static std::string getTestCaseName(const testing::TestParamInfo<TopKParams>& obj) {
        auto param = obj.param;
        std::ostringstream result;
        result << "aType=" << param.A.type;
        result << "_aShape=" << param.A.shape;
        result << "_kType=" << param.k.type;
        result << "_kShape=" << param.k.shape;
        result << "_axis=" << param.axis;
        result << "_r0Type=" << param.result0.type;
        result << "_r0Shape=" << param.result0.shape;
        result << "_r1Type=" << param.result1.type;
        result << "_r1Shape=" << param.result1.shape;
        result << "_outIdx=" << param.outIdx;
        result << "_=" << param.testcaseName;
        return result.str();
    }

private:
    static std::shared_ptr<Model> CreateFunction(const TopKParams& params, size_t out_idx) {
        const auto A = std::make_shared<opset1::Parameter>(params.A.type,
                                                           params.A.shape);
        const auto k = opset1::Constant::create(params.k.type,
                                                params.k.shape,
                                                params.k.data.data());
        const auto B = std::make_shared<opset1::TopK>(A, k, params.axis, params.mode, params.sort);
        const auto f = std::make_shared<Model>(OutputVector{B->output(out_idx)}, ParameterVector{A});
        return f;
    }
};

TEST_P(ReferenceTopKTest1dMaxMin, CompareWithRefs) {
    Exec();
}

template <element::Type_t ET, element::Type_t ET2, element::Type_t ET_OUT>
std::vector<TopKParams> generateParams1dMaxMin() {
    using T = typename element_type_traits<ET>::value_type;
    using T2 = typename element_type_traits<ET2>::value_type;
    using T_OUT = typename element_type_traits<ET_OUT>::value_type;
    std::vector<TopKParams> params {
        TopKParams(
            Tensor(ET, {6}, std::vector<T>{1, 2, 3, 4, 5, 6}),
            Tensor(ET2, {}, std::vector<T2>{6}),
            0,
            opset1::TopK::Mode::MAX,
            opset1::TopK::SortType::SORT_VALUES,
            Tensor(ET, {6}, std::vector<T>{6, 5, 4, 3, 2, 1}),
            Tensor(ET_OUT, {6}, std::vector<T_OUT>{5, 4, 3, 2, 1, 0}),
            0,
            "topk_1d_max_all"),

        TopKParams(
            Tensor(ET, {6}, std::vector<T>{1, 2, 3, 4, 5, 6}),
            Tensor(ET2, {}, std::vector<T2>{6}),
            0,
            opset1::TopK::Mode::MAX,
            opset1::TopK::SortType::SORT_VALUES,
            Tensor(ET, {6}, std::vector<T>{6, 5, 4, 3, 2, 1}),
            Tensor(ET_OUT, {6}, std::vector<T_OUT>{5, 4, 3, 2, 1, 0}),
            1,
            "topk_1d_max_all"),

        TopKParams(
            Tensor(ET, {6}, std::vector<T>{1, 2, 3, 4, 5, 6}),
            Tensor(ET2, {}, std::vector<T2>{3}),
            0,
            opset1::TopK::Mode::MAX,
            opset1::TopK::SortType::SORT_VALUES,
            Tensor(ET, {3}, std::vector<T>{6, 5, 4}),
            Tensor(ET_OUT, {3}, std::vector<T_OUT>{5, 4, 3}),
            0,
            "topk_1d_max_partial"),

        TopKParams(
            Tensor(ET, {6}, std::vector<T>{1, 2, 3, 4, 5, 6}),
            Tensor(ET2, {}, std::vector<T2>{3}),
            0,
            opset1::TopK::Mode::MAX,
            opset1::TopK::SortType::SORT_VALUES,
            Tensor(ET, {3}, std::vector<T>{6, 5, 4}),
            Tensor(ET_OUT, {3}, std::vector<T_OUT>{5, 4, 3}),
            1,
            "topk_1d_max_partial"),

        TopKParams(
            Tensor(ET, {6}, std::vector<T>{1, 2, 3, 4, 5, 6}),
            Tensor(ET2, {}, std::vector<T2>{1}),
            0,
            opset1::TopK::Mode::MAX,
            opset1::TopK::SortType::SORT_VALUES,
            Tensor(ET, {1}, std::vector<T>{6}),
            Tensor(ET_OUT, {1}, std::vector<T_OUT>{5}),
            0,
            "topk_1d_max_one"),

        TopKParams(
            Tensor(ET, {6}, std::vector<T>{1, 2, 3, 4, 5, 6}),
            Tensor(ET2, {}, std::vector<T2>{1}),
            0,
            opset1::TopK::Mode::MAX,
            opset1::TopK::SortType::SORT_VALUES,
            Tensor(ET, {1}, std::vector<T>{6}),
            Tensor(ET_OUT, {1}, std::vector<T_OUT>{5}),
            1,
            "topk_1d_max_one"),

        TopKParams(
            Tensor(ET, {6}, std::vector<T>{6, 5, 4, 3, 2, 1}),
            Tensor(ET2, {}, std::vector<T2>{6}),
            0,
            opset1::TopK::Mode::MIN,
            opset1::TopK::SortType::SORT_VALUES,
            Tensor(ET, {6}, std::vector<T>{1, 2, 3, 4, 5, 6}),
            Tensor(ET_OUT, {6}, std::vector<T_OUT>{5, 4, 3, 2, 1, 0}),
            0,
            "topk_1d_min_all"),

        TopKParams(
            Tensor(ET, {6}, std::vector<T>{6, 5, 4, 3, 2, 1}),
            Tensor(ET2, {}, std::vector<T2>{6}),
            0,
            opset1::TopK::Mode::MIN,
            opset1::TopK::SortType::SORT_VALUES,
            Tensor(ET, {6}, std::vector<T>{1, 2, 3, 4, 5, 6}),
            Tensor(ET_OUT, {6}, std::vector<T_OUT>{5, 4, 3, 2, 1, 0}),
            1,
            "topk_1d_min_all"),

        TopKParams(
            Tensor(ET, {6}, std::vector<T>{6, 5, 4, 3, 2, 1}),
            Tensor(ET2, {}, std::vector<T2>{3}),
            0,
            opset1::TopK::Mode::MIN,
            opset1::TopK::SortType::SORT_VALUES,
            Tensor(ET, {3}, std::vector<T>{1, 2, 3}),
            Tensor(ET_OUT, {3}, std::vector<T_OUT>{5, 4, 3}),
            0,
            "topk_1d_min_partial"),

        TopKParams(
            Tensor(ET, {6}, std::vector<T>{6, 5, 4, 3, 2, 1}),
            Tensor(ET2, {}, std::vector<T2>{3}),
            0,
            opset1::TopK::Mode::MIN,
            opset1::TopK::SortType::SORT_VALUES,
            Tensor(ET, {3}, std::vector<T>{1, 2, 3}),
            Tensor(ET_OUT, {3}, std::vector<T_OUT>{5, 4, 3}),
            1,
            "topk_1d_min_partial"),

        TopKParams(
            Tensor(ET, {6}, std::vector<T>{6, 5, 4, 3, 2, 1}),
            Tensor(ET2, {}, std::vector<T2>{1}),
            0,
            opset1::TopK::Mode::MIN,
            opset1::TopK::SortType::SORT_VALUES,
            Tensor(ET, {1}, std::vector<T>{1}),
            Tensor(ET_OUT, {1}, std::vector<T_OUT>{5}),
            0,
            "topk_1d_min_one"),

        TopKParams(
            Tensor(ET, {6}, std::vector<T>{6, 5, 4, 3, 2, 1}),
            Tensor(ET2, {}, std::vector<T2>{1}),
            0,
            opset1::TopK::Mode::MIN,
            opset1::TopK::SortType::SORT_VALUES,
            Tensor(ET, {1}, std::vector<T>{1}),
            Tensor(ET_OUT, {1}, std::vector<T_OUT>{5}),
            1,
            "topk_1d_min_one"),

        TopKParams(
            Tensor(ET, {2, 3, 2}, std::vector<T>{
                9, 2, 10, 12, 8, 4, 6, 1, 5, 3, 11, 7
            }),
            Tensor(ET2, {}, std::vector<T2>{3}),
            1,
            opset1::TopK::Mode::MAX,
            opset1::TopK::SortType::SORT_VALUES,
            Tensor(ET, {2, 3, 2}, std::vector<T>{
                10, 12, 9, 4, 8, 2, 11, 7, 6, 3, 5, 1
            }),
            Tensor(ET_OUT, {2, 3, 2}, std::vector<T_OUT>{
                1, 1, 0, 2, 2, 0, 2, 2, 0, 1, 1, 0
            }),
            0,
            "topk_3d_max_all"),

        TopKParams(
            Tensor(ET, {2, 3, 2}, std::vector<T>{
                9, 2, 10, 12, 8, 4, 6, 1, 5, 3, 11, 7
            }),
            Tensor(ET2, {}, std::vector<T2>{3}),
            1,
            opset1::TopK::Mode::MAX,
            opset1::TopK::SortType::SORT_VALUES,
            Tensor(ET, {2, 3, 2}, std::vector<T>{
                10, 12, 9, 4, 8, 2, 11, 7, 6, 3, 5, 1
            }),
            Tensor(ET_OUT, {2, 3, 2}, std::vector<T_OUT>{
                1, 1, 0, 2, 2, 0, 2, 2, 0, 1, 1, 0
            }),
            1,
            "topk_3d_max_all"),

        TopKParams(
            Tensor(ET, {2, 6, 3, 2, 4}, std::vector<T>{
                  1,   73,  9,   81,  17,  89,  2,   74,  10,  82,  18,  90,  3,   75,  11,  83,  19,
                  91,  4,   76,  12,  84,  20,  92,  145, 217, 153, 225, 161, 233, 146, 218, 154, 226,
                  162, 234, 147, 219, 155, 227, 163, 235, 148, 220, 156, 228, 164, 236, 5,   77,  13,
                  85,  21,  93,  6,   78,  14,  86,  22,  94,  7,   79,  15,  87,  23,  95,  8,   80,
                  16,  88,  24,  96,  149, 221, 157, 229, 165, 27,  150, 222, 158, 230, 166, 23,  151,
                  223, 159, 231, 17,  39,  2,   224, 160, 232, 168, 240, 25,  97,  33,  105, 41,  113,
                  26,  98,  34,  106, 42,  114, 27,  99,  35,  107, 43,  115, 28,  100, 36,  108, 44,
                  116, 169, 241, 177, 249, 185, 25,  170, 242, 178, 250, 186, 258, 171, 243, 179, 251,
                  187, 259, 172, 24,  180, 252, 188, 260, 29,  101, 37,  109, 45,  117, 30,  102, 38,
                  10,  46,  118, 31,  103, 39,  111, 47,  119, 32,  104, 40,  112, 48,  20,  173, 245,
                  181, 253, 189, 261, 174, 246, 182, 254, 190, 262, 175, 27,  183, 255, 191, 263, 176,
                  248, 184, 256, 192, 264, 49,  121, 57,  129, 65,  137, 50,  122, 58,  130, 66,  138,
                  51,  123, 59,  131, 67,  139, 52,  124, 60,  132, 68,  140, 193, 265, 201, 273, 209,
                  281, 194, 266, 202, 274, 210, 43,  115, 28,  100, 36,  108, 44,  116, 169, 241, 177,
                  212, 284, 53,  125, 61,  133, 69,  141, 54,  126, 62,  134, 70,  142, 55,  127, 63,
                  135, 71,  143, 56,  128, 64,  136, 72,  144, 197, 269, 205, 277, 213, 285, 198, 270,
                  206, 278, 214, 286, 199, 271, 207, 279, 215, 287, 200, 272, 208, 280, 216, 288
            }),
            Tensor(ET2, {}, std::vector<T2>{2}),
            1,
            opset1::TopK::Mode::MAX,
            opset1::TopK::SortType::SORT_VALUES,
            Tensor(ET, {2, 2, 3, 2, 4}, std::vector<T>{
                169, 241, 177, 249, 185, 233, 170, 242, 178, 250, 186, 258, 171, 243, 179, 251,
                187, 259, 172, 224, 180, 252, 188, 260, 149, 221, 157, 229, 165, 113, 150, 222,
                158, 230, 166, 234, 151, 223, 159, 231, 163, 235, 148, 220, 160, 232, 168, 240,
                197, 269, 205, 277, 213, 285, 198, 270, 206, 278, 214, 286, 199, 271, 207, 279,
                215, 287, 200, 272, 241, 280, 216, 288, 193, 265, 201, 273, 209, 281, 194, 266,
                202, 274, 210, 262, 175, 127, 183, 255, 191, 263, 176, 248, 208, 256, 212, 284
            }),
            Tensor(ET_OUT, {2, 2, 3, 2, 4}, std::vector<T_OUT>{
                5, 5, 5, 5, 5, 1, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 3, 5, 5, 5, 5,
                3, 3, 3, 3, 3, 4, 3, 3, 3, 3, 3, 1, 3, 3, 3, 3, 1, 1, 1, 1, 3, 3, 3, 3,
                5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 3, 5, 5, 5,
                3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 1, 1, 4, 1, 1, 1, 1, 1, 1, 5, 1, 3, 3
            }),
            0,
            "topk_5d_max_partial"),

        TopKParams(
            Tensor(ET, {2, 6, 3, 2, 4}, std::vector<T>{
                  1,   73,  9,   81,  17,  89,  2,   74,  10,  82,  18,  90,  3,   75,  11,  83,  19,
                  91,  4,   76,  12,  84,  20,  92,  145, 217, 153, 225, 161, 233, 146, 218, 154, 226,
                  162, 234, 147, 219, 155, 227, 163, 235, 148, 220, 156, 228, 164, 236, 5,   77,  13,
                  85,  21,  93,  6,   78,  14,  86,  22,  94,  7,   79,  15,  87,  23,  95,  8,   80,
                  16,  88,  24,  96,  149, 221, 157, 229, 165, 27,  150, 222, 158, 230, 166, 23,  151,
                  223, 159, 231, 17,  39,  2,   224, 160, 232, 168, 240, 25,  97,  33,  105, 41,  113,
                  26,  98,  34,  106, 42,  114, 27,  99,  35,  107, 43,  115, 28,  100, 36,  108, 44,
                  116, 169, 241, 177, 249, 185, 25,  170, 242, 178, 250, 186, 258, 171, 243, 179, 251,
                  187, 259, 172, 24,  180, 252, 188, 260, 29,  101, 37,  109, 45,  117, 30,  102, 38,
                  10,  46,  118, 31,  103, 39,  111, 47,  119, 32,  104, 40,  112, 48,  20,  173, 245,
                  181, 253, 189, 261, 174, 246, 182, 254, 190, 262, 175, 27,  183, 255, 191, 263, 176,
                  248, 184, 256, 192, 264, 49,  121, 57,  129, 65,  137, 50,  122, 58,  130, 66,  138,
                  51,  123, 59,  131, 67,  139, 52,  124, 60,  132, 68,  140, 193, 265, 201, 273, 209,
                  281, 194, 266, 202, 274, 210, 43,  115, 28,  100, 36,  108, 44,  116, 169, 241, 177,
                  212, 284, 53,  125, 61,  133, 69,  141, 54,  126, 62,  134, 70,  142, 55,  127, 63,
                  135, 71,  143, 56,  128, 64,  136, 72,  144, 197, 269, 205, 277, 213, 285, 198, 270,
                  206, 278, 214, 286, 199, 271, 207, 279, 215, 287, 200, 272, 208, 280, 216, 288
            }),
            Tensor(ET2, {}, std::vector<T2>{2}),
            1,
            opset1::TopK::Mode::MAX,
            opset1::TopK::SortType::SORT_VALUES,
            Tensor(ET, {2, 2, 3, 2, 4}, std::vector<T>{
                169, 241, 177, 249, 185, 233, 170, 242, 178, 250, 186, 258, 171, 243, 179, 251,
                187, 259, 172, 224, 180, 252, 188, 260, 149, 221, 157, 229, 165, 113, 150, 222,
                158, 230, 166, 234, 151, 223, 159, 231, 163, 235, 148, 220, 160, 232, 168, 240,
                197, 269, 205, 277, 213, 285, 198, 270, 206, 278, 214, 286, 199, 271, 207, 279,
                215, 287, 200, 272, 241, 280, 216, 288, 193, 265, 201, 273, 209, 281, 194, 266,
                202, 274, 210, 262, 175, 127, 183, 255, 191, 263, 176, 248, 208, 256, 212, 284
            }),
            Tensor(ET_OUT, {2, 2, 3, 2, 4}, std::vector<T_OUT>{
                5, 5, 5, 5, 5, 1, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 3, 5, 5, 5, 5,
                3, 3, 3, 3, 3, 4, 3, 3, 3, 3, 3, 1, 3, 3, 3, 3, 1, 1, 1, 1, 3, 3, 3, 3,
                5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 3, 5, 5, 5,
                3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 1, 1, 4, 1, 1, 1, 1, 1, 1, 5, 1, 3, 3
            }),
            1,
            "topk_5d_max_partial"),

        TopKParams(
            Tensor(ET, {2, 3, 2}, std::vector<T>{
                9, 2, 10, 12, 8, 4, 6, 1, 5, 3, 11, 7
            }),
            Tensor(ET2, {}, std::vector<T2>{2}),
            1,
            opset1::TopK::Mode::MAX,
            opset1::TopK::SortType::SORT_VALUES,
            Tensor(ET, {2, 2, 2}, std::vector<T>{
                10, 12, 9, 4, 11, 7, 6, 3
            }),
            Tensor(ET_OUT, {2, 2, 2}, std::vector<T_OUT>{
                1, 1, 0, 2, 2, 2, 0, 1
            }),
            0,
            "topk_3d_max_partial"),

        TopKParams(
            Tensor(ET, {2, 3, 2}, std::vector<T>{
                9, 2, 10, 12, 8, 4, 6, 1, 5, 3, 11, 7
            }),
            Tensor(ET2, {}, std::vector<T2>{2}),
            1,
            opset1::TopK::Mode::MAX,
            opset1::TopK::SortType::SORT_VALUES,
            Tensor(ET, {2, 2, 2}, std::vector<T>{
                10, 12, 9, 4, 11, 7, 6, 3
            }),
            Tensor(ET_OUT, {2, 2, 2}, std::vector<T_OUT>{
                1, 1, 0, 2, 2, 2, 0, 1
            }),
            1,
            "topk_3d_max_partial"),

        TopKParams(
            Tensor(ET, {2, 3, 2}, std::vector<T>{
                9, 2, 10, 12, 8, 4, 6, 1, 5, 3, 11, 7
            }),
            Tensor(ET2, {}, std::vector<T2>{1}),
            1,
            opset1::TopK::Mode::MAX,
            opset1::TopK::SortType::SORT_VALUES,
            Tensor(ET, {2, 1, 2}, std::vector<T>{
                10, 12, 11, 7
            }),
            Tensor(ET_OUT, {2, 1, 2}, std::vector<T_OUT>{
                1, 1, 2, 2
            }),
            0,
            "topk_3d_max_one"),

        TopKParams(
            Tensor(ET, {2, 3, 2}, std::vector<T>{
                9, 2, 10, 12, 8, 4, 6, 1, 5, 3, 11, 7
            }),
            Tensor(ET2, {}, std::vector<T2>{1}),
            1,
            opset1::TopK::Mode::MAX,
            opset1::TopK::SortType::SORT_VALUES,
            Tensor(ET, {2, 1, 2}, std::vector<T>{
                10, 12, 11, 7
            }),
            Tensor(ET_OUT, {2, 1, 2}, std::vector<T_OUT>{
                1, 1, 2, 2
            }),
            1,
            "topk_3d_max_one"),

        TopKParams(
            Tensor(ET, {2, 3, 2}, std::vector<T>{
                12, 2, 10, 9, 8, 4, 6, 1, 5, 3, 11, 7
            }),
            Tensor(ET2, {}, std::vector<T2>{3}),
            1,
            opset1::TopK::Mode::MIN,
            opset1::TopK::SortType::SORT_VALUES,
            Tensor(ET, {2, 3, 2}, std::vector<T>{
                8, 2, 10, 4, 12, 9, 5, 1, 6, 3, 11, 7
            }),
            Tensor(ET_OUT, {2, 3, 2}, std::vector<T_OUT>{
                2, 0, 1, 2, 0, 1, 1, 0, 0, 1, 2, 2
            }),
            0,
            "topk_3d_min_all"),

        TopKParams(
            Tensor(ET, {2, 3, 2}, std::vector<T>{
                12, 2, 10, 9, 8, 4, 6, 1, 5, 3, 11, 7
            }),
            Tensor(ET2, {}, std::vector<T2>{3}),
            1,
            opset1::TopK::Mode::MIN,
            opset1::TopK::SortType::SORT_VALUES,
            Tensor(ET, {2, 3, 2}, std::vector<T>{
                8, 2, 10, 4, 12, 9, 5, 1, 6, 3, 11, 7
            }),
            Tensor(ET_OUT, {2, 3, 2}, std::vector<T_OUT>{
                2, 0, 1, 2, 0, 1, 1, 0, 0, 1, 2, 2
            }),
            1,
            "topk_3d_min_all"),

        TopKParams(
            Tensor(ET, {2, 3, 2}, std::vector<T>{
                12, 2, 10, 9, 8, 4, 6, 1, 5, 3, 11, 7
            }),
            Tensor(ET2, {}, std::vector<T2>{2}),
            1,
            opset1::TopK::Mode::MIN,
            opset1::TopK::SortType::SORT_VALUES,
            Tensor(ET, {2, 2, 2}, std::vector<T>{
                8, 2, 10, 4, 5, 1, 6, 3
            }),
            Tensor(ET_OUT, {2, 2, 2}, std::vector<T_OUT>{
                2, 0, 1, 2, 1, 0, 0, 1
            }),
            0,
            "topk_3d_min_partial"),

        TopKParams(
            Tensor(ET, {2, 3, 2}, std::vector<T>{
                12, 2, 10, 9, 8, 4, 6, 1, 5, 3, 11, 7
            }),
            Tensor(ET2, {}, std::vector<T2>{2}),
            1,
            opset1::TopK::Mode::MIN,
            opset1::TopK::SortType::SORT_VALUES,
            Tensor(ET, {2, 2, 2}, std::vector<T>{
                8, 2, 10, 4, 5, 1, 6, 3
            }),
            Tensor(ET_OUT, {2, 2, 2}, std::vector<T_OUT>{
                2, 0, 1, 2, 1, 0, 0, 1
            }),
            1,
            "topk_3d_min_partial"),

        TopKParams(
            Tensor(ET, {2, 3, 2}, std::vector<T>{
                12, 2, 10, 9, 8, 4, 6, 1, 5, 3, 11, 7
            }),
            Tensor(ET2, {}, std::vector<T2>{1}),
            1,
            opset1::TopK::Mode::MIN,
            opset1::TopK::SortType::SORT_VALUES,
            Tensor(ET, {2, 1, 2}, std::vector<T>{
                8, 2, 5, 1
            }),
            Tensor(ET_OUT, {2, 1, 2}, std::vector<T_OUT>{
                2, 0, 1, 0
            }),
            0,
            "topk_3d_min_one"),

        TopKParams(
            Tensor(ET, {2, 3, 2}, std::vector<T>{
                12, 2, 10, 9, 8, 4, 6, 1, 5, 3, 11, 7
            }),
            Tensor(ET2, {}, std::vector<T2>{1}),
            1,
            opset1::TopK::Mode::MIN,
            opset1::TopK::SortType::SORT_VALUES,
            Tensor(ET, {2, 1, 2}, std::vector<T>{
                8, 2, 5, 1
            }),
            Tensor(ET_OUT, {2, 1, 2}, std::vector<T_OUT>{
                2, 0, 1, 0
            }),
            1,
            "topk_3d_min_one"),

        TopKParams(
            Tensor(ET, {4, 3}, std::vector<T>{
                9, 2, 10, 12, 8, 4, 6, 1, 5, 3, 11, 7
            }),
            Tensor(ET2, {}, std::vector<T2>{4}),
            0,
            opset1::TopK::Mode::MAX,
            opset1::TopK::SortType::SORT_VALUES,
            Tensor(ET, {4, 3}, std::vector<T>{
                12, 11, 10, 9, 8, 7, 6, 2, 5, 3, 1, 4
            }),
            Tensor(ET_OUT, {4, 3}, std::vector<T_OUT>{
                1, 3, 0, 0, 1, 3, 2, 0, 2, 3, 2, 1
            }),
            0,
            "topk_2d_max_all"),

        TopKParams(
            Tensor(ET, {4, 3}, std::vector<T>{
                9, 2, 10, 12, 8, 4, 6, 1, 5, 3, 11, 7
            }),
            Tensor(ET2, {}, std::vector<T2>{4}),
            0,
            opset1::TopK::Mode::MAX,
            opset1::TopK::SortType::SORT_VALUES,
            Tensor(ET, {4, 3}, std::vector<T>{
                12, 11, 10, 9, 8, 7, 6, 2, 5, 3, 1, 4
            }),
            Tensor(ET_OUT, {4, 3}, std::vector<T_OUT>{
                1, 3, 0, 0, 1, 3, 2, 0, 2, 3, 2, 1
            }),
            1,
            "topk_2d_max_all"),

        TopKParams(
            Tensor(ET, {4, 3}, std::vector<T>{
                9, 2, 10, 12, 8, 4, 6, 1, 5, 3, 11, 7
            }),
            Tensor(ET2, {}, std::vector<T2>{2}),
            0,
            opset1::TopK::Mode::MAX,
            opset1::TopK::SortType::SORT_VALUES,
            Tensor(ET, {2, 3}, std::vector<T>{
                12, 11, 10, 9, 8, 7
            }),
            Tensor(ET_OUT, {2, 3}, std::vector<T_OUT>{
                1, 3, 0, 0, 1, 3
            }),
            0,
            "topk_2d_max_partial"),

        TopKParams(
            Tensor(ET, {4, 3}, std::vector<T>{
                9, 2, 10, 12, 8, 4, 6, 1, 5, 3, 11, 7
            }),
            Tensor(ET2, {}, std::vector<T2>{2}),
            0,
            opset1::TopK::Mode::MAX,
            opset1::TopK::SortType::SORT_VALUES,
            Tensor(ET, {2, 3}, std::vector<T>{
                12, 11, 10, 9, 8, 7
            }),
            Tensor(ET_OUT, {2, 3}, std::vector<T_OUT>{
                1, 3, 0, 0, 1, 3
            }),
            1,
            "topk_2d_max_partial"),

        TopKParams(
            Tensor(ET, {4, 3}, std::vector<T>{
                9, 2, 10, 12, 8, 4, 6, 1, 5, 3, 11, 7
            }),
            Tensor(ET2, {}, std::vector<T2>{1}),
            0,
            opset1::TopK::Mode::MAX,
            opset1::TopK::SortType::SORT_VALUES,
            Tensor(ET, {1, 3}, std::vector<T>{
                12, 11, 10
            }),
            Tensor(ET_OUT, {1, 3}, std::vector<T_OUT>{
                1, 3, 0
            }),
            0,
            "topk_2d_max_one"),

        TopKParams(
            Tensor(ET, {4, 3}, std::vector<T>{
                9, 2, 10, 12, 8, 4, 6, 1, 5, 3, 11, 7
            }),
            Tensor(ET2, {}, std::vector<T2>{1}),
            0,
            opset1::TopK::Mode::MAX,
            opset1::TopK::SortType::SORT_VALUES,
            Tensor(ET, {1, 3}, std::vector<T>{
                12, 11, 10
            }),
            Tensor(ET_OUT, {1, 3}, std::vector<T_OUT>{
                1, 3, 0
            }),
            1,
            "topk_2d_max_one"),

        TopKParams(
            Tensor(ET, {2, 4}, std::vector<T>{
                1, 3, 2, 4, 1, 3, 3, 2
            }),
            Tensor(ET2, {}, std::vector<T2>{1}),
            1,
            opset1::TopK::Mode::MAX,
            opset1::TopK::SortType::SORT_VALUES,
            Tensor(ET, {2, 1}, std::vector<T>{
                4, 3
            }),
            Tensor(ET_OUT, {2, 1}, std::vector<T_OUT>{
                3, 1
            }),
            0,
            "topk_2d_max_one_with_equal_values"),

        TopKParams(
            Tensor(ET, {2, 4}, std::vector<T>{
                1, 3, 2, 4, 1, 3, 3, 2
            }),
            Tensor(ET2, {}, std::vector<T2>{1}),
            1,
            opset1::TopK::Mode::MAX,
            opset1::TopK::SortType::SORT_VALUES,
            Tensor(ET, {2, 1}, std::vector<T>{
                4, 3
            }),
            Tensor(ET_OUT, {2, 1}, std::vector<T_OUT>{
                3, 1
            }),
            1,
            "topk_2d_max_one_with_equal_values"),

        TopKParams(
            Tensor(ET, {4, 3}, std::vector<T>{
                12, 2, 10, 9, 8, 4, 6, 1, 5, 3, 11, 7
            }),
            Tensor(ET2, {}, std::vector<T2>{4}),
            0,
            opset1::TopK::Mode::MIN,
            opset1::TopK::SortType::SORT_VALUES,
            Tensor(ET, {4, 3}, std::vector<T>{
                3, 1, 4, 6, 2, 5, 9, 8, 7, 12, 11, 10
            }),
            Tensor(ET_OUT, {4, 3}, std::vector<T_OUT>{
                3, 2, 1, 2, 0, 2, 1, 1, 3, 0, 3, 0
            }),
            0,
            "topk_2d_min_all"),

        TopKParams(
            Tensor(ET, {4, 3}, std::vector<T>{
                12, 2, 10, 9, 8, 4, 6, 1, 5, 3, 11, 7
            }),
            Tensor(ET2, {}, std::vector<T2>{4}),
            0,
            opset1::TopK::Mode::MIN,
            opset1::TopK::SortType::SORT_VALUES,
            Tensor(ET, {4, 3}, std::vector<T>{
                3, 1, 4, 6, 2, 5, 9, 8, 7, 12, 11, 10
            }),
            Tensor(ET_OUT, {4, 3}, std::vector<T_OUT>{
                3, 2, 1, 2, 0, 2, 1, 1, 3, 0, 3, 0
            }),
            1,
            "topk_2d_min_all"),

        TopKParams(
            Tensor(ET, {4, 3}, std::vector<T>{
                12, 2, 10, 9, 8, 4, 6, 1, 5, 3, 11, 7
            }),
            Tensor(ET2, {}, std::vector<T2>{2}),
            0,
            opset1::TopK::Mode::MIN,
            opset1::TopK::SortType::SORT_VALUES,
            Tensor(ET, {2, 3}, std::vector<T>{
                3, 1, 4, 6, 2, 5
            }),
            Tensor(ET_OUT, {2, 3}, std::vector<T_OUT>{
                3, 2, 1, 2, 0, 2
            }),
            0,
            "topk_2d_min_partial"),

        TopKParams(
            Tensor(ET, {4, 3}, std::vector<T>{
                12, 2, 10, 9, 8, 4, 6, 1, 5, 3, 11, 7
            }),
            Tensor(ET2, {}, std::vector<T2>{2}),
            0,
            opset1::TopK::Mode::MIN,
            opset1::TopK::SortType::SORT_VALUES,
            Tensor(ET, {2, 3}, std::vector<T>{
                3, 1, 4, 6, 2, 5
            }),
            Tensor(ET_OUT, {2, 3}, std::vector<T_OUT>{
                3, 2, 1, 2, 0, 2
            }),
            1,
            "topk_2d_min_partial"),

        TopKParams(
            Tensor(ET, {4, 3}, std::vector<T>{
                12, 2, 10, 9, 8, 4, 6, 1, 5, 3, 11, 7
            }),
            Tensor(ET2, {}, std::vector<T2>{1}),
            0,
            opset1::TopK::Mode::MIN,
            opset1::TopK::SortType::NONE,
            Tensor(ET, {1, 3}, std::vector<T>{
                3, 1, 4
            }),
            Tensor(ET_OUT, {1, 3}, std::vector<T_OUT>{
                3, 2, 1
            }),
            0,
            "topk_2d_min_one"),

        TopKParams(
            Tensor(ET, {4, 3}, std::vector<T>{
                12, 2, 10, 9, 8, 4, 6, 1, 5, 3, 11, 7
            }),
            Tensor(ET2, {}, std::vector<T2>{1}),
            0,
            opset1::TopK::Mode::MIN,
            opset1::TopK::SortType::NONE,
            Tensor(ET, {1, 3}, std::vector<T>{
                3, 1, 4
            }),
            Tensor(ET_OUT, {1, 3}, std::vector<T_OUT>{
                3, 2, 1
            }),
            1,
            "topk_2d_min_one"),
    };
    return params;
}

std::vector<TopKParams> generateCombinedParams1dMaxMin() {
    const std::vector<std::vector<TopKParams>> generatedParams {
        generateParams1dMaxMin<element::Type_t::i16, element::Type_t::i64, element::Type_t::i32>(),
        generateParams1dMaxMin<element::Type_t::i32, element::Type_t::i64, element::Type_t::i32>(),
        generateParams1dMaxMin<element::Type_t::i64, element::Type_t::i64, element::Type_t::i32>(),
        generateParams1dMaxMin<element::Type_t::u16, element::Type_t::i64, element::Type_t::i32>(),
        generateParams1dMaxMin<element::Type_t::u32, element::Type_t::i64, element::Type_t::i32>(),
        generateParams1dMaxMin<element::Type_t::u64, element::Type_t::i64, element::Type_t::i32>(),
        generateParams1dMaxMin<element::Type_t::bf16, element::Type_t::i64, element::Type_t::i32>(),
        generateParams1dMaxMin<element::Type_t::f16, element::Type_t::i64, element::Type_t::i32>(),
        generateParams1dMaxMin<element::Type_t::f32, element::Type_t::i64, element::Type_t::i32>(),
        generateParams1dMaxMin<element::Type_t::f64, element::Type_t::i64, element::Type_t::i32>(),
    };
    std::vector<TopKParams> combinedParams;

    for (const auto& params : generatedParams) {
        combinedParams.insert(combinedParams.end(), params.begin(), params.end());
    }
    return combinedParams;
}

INSTANTIATE_TEST_SUITE_P(smoke_TopK_With_Hardcoded_Refs, ReferenceTopKTest1dMaxMin,
    testing::ValuesIn(generateCombinedParams1dMaxMin()), ReferenceTopKTest1dMaxMin::getTestCaseName);

class ReferenceTopKTestInt64 : public ReferenceTopKTest1dMaxMin {
private:
    static std::shared_ptr<Model> CreateFunction(const TopKParams& params, size_t out_idx) {
        const auto A = std::make_shared<opset1::Parameter>(params.A.type,
                                                           params.A.shape);
        const auto k = opset1::Constant::create(params.k.type,
                                                params.k.shape,
                                                params.k.data.data());
        const auto B = std::make_shared<opset1::TopK>(A,
                                                      k,
                                                      params.axis,
                                                      params.mode,
                                                      params.sort,
                                                      element::i64);
        const auto f = std::make_shared<Model>(OutputVector{B->output(out_idx)}, ParameterVector{A});
        return f;
    }
};

TEST_P(ReferenceTopKTestInt64, CompareWithRefs) {
    Exec();
}

template <element::Type_t ET, element::Type_t ET2, element::Type_t ET_OUT>
std::vector<TopKParams> generateParamsInt64() {
    using T = typename element_type_traits<ET>::value_type;
    using T2 = typename element_type_traits<ET2>::value_type;
    using T_OUT = typename element_type_traits<ET_OUT>::value_type;
    std::vector<TopKParams> params {
        TopKParams(
            Tensor(ET, {2, 3, 2}, std::vector<T>{
                9, 2, 10, 12, 8, 4, 6, 1, 5, 3, 11, 7
            }),
            Tensor(ET2, {}, std::vector<T2>{3}),
            1,
            opset1::TopK::Mode::MAX,
            opset1::TopK::SortType::SORT_VALUES,
            Tensor(ET, {2, 3, 2}, std::vector<T>{
                10, 12, 9, 4, 8, 2, 11, 7, 6, 3, 5, 1
            }),
            Tensor(ET_OUT, {2, 3, 2}, std::vector<T_OUT>{
                1, 1, 0, 2, 2, 0, 2, 2, 0, 1, 1, 0
            }),
            0,
            "topk_int64"),
        TopKParams(
            Tensor(ET, {2, 3, 2}, std::vector<T>{
                9, 2, 10, 12, 8, 4, 6, 1, 5, 3, 11, 7
            }),
            Tensor(ET2, {}, std::vector<T2>{3}),
            1,
            opset1::TopK::Mode::MAX,
            opset1::TopK::SortType::SORT_VALUES,
            Tensor(ET, {2, 3, 2}, std::vector<T>{
                10, 12, 9, 4, 8, 2, 11, 7, 6, 3, 5, 1
            }),
            Tensor(ET_OUT, {2, 3, 2}, std::vector<T_OUT>{
                1, 1, 0, 2, 2, 0, 2, 2, 0, 1, 1, 0
            }),
            1,
            "topk_int64"),
    };
    return params;
}

std::vector<TopKParams> generateCombinedParamsInt64() {
    const std::vector<std::vector<TopKParams>> generatedParams {
        generateParamsInt64<element::Type_t::f32, element::Type_t::i64, element::Type_t::i32>(),
    };
    std::vector<TopKParams> combinedParams;

    for (const auto& params : generatedParams) {
        combinedParams.insert(combinedParams.end(), params.begin(), params.end());
    }
    return combinedParams;
}

INSTANTIATE_TEST_SUITE_P(smoke_TopK_With_Hardcoded_Refs, ReferenceTopKTestInt64,
    testing::ValuesIn(generateCombinedParamsInt64()), ReferenceTopKTest1dMaxMin::getTestCaseName);

class ReferenceTopKTestSingleOutput : public ReferenceTopKTest {
public:
    void SetUp() override {
        auto params = GetParam();
        function = CreateFunction(params);
        inputData = {params.A.data};
        refOutData = {params.result1.data};
    }

private:
    static std::shared_ptr<Model> CreateFunction(const TopKParams& params) {
        const auto A = std::make_shared<opset1::Parameter>(params.A.type,
                                                           params.A.shape);
        const auto k = opset1::Constant::create(params.k.type,
                                                params.k.shape,
                                                params.k.data.data());
        const auto B = std::make_shared<opset1::TopK>(A, k, params.axis, params.mode, params.sort);
        const auto f = std::make_shared<Model>(OutputVector{B->output(1)}, ParameterVector{A});
        return f;
    }
};

TEST_P(ReferenceTopKTestSingleOutput, CompareWithRefs) {
    Exec();
}

template <element::Type_t ET, element::Type_t ET2, element::Type_t ET_OUT>
std::vector<TopKParams> generateParamsSingleOutput() {
    using T = typename element_type_traits<ET>::value_type;
    using T2 = typename element_type_traits<ET2>::value_type;
    using T_OUT = typename element_type_traits<ET_OUT>::value_type;
    std::vector<TopKParams> params {
        TopKParams(
            Tensor(ET, {2, 3, 2}, std::vector<T>{12, 2, 10, 9, 8, 4, 6, 1, 5, 3, 11, 7}),
            Tensor(ET2, {}, std::vector<T2>{2}),
            1,
            opset1::TopK::Mode::MIN,
            opset1::TopK::SortType::SORT_VALUES,
            Tensor(ET, {2, 2, 2}, std::vector<T>{}),
            Tensor(ET_OUT, {2, 2, 2}, std::vector<T_OUT>{2, 0, 1, 2, 1, 0, 0, 1}),
            0,
            "topk_3d_single_output"),
    };
    return params;
}

std::vector<TopKParams> generateCombinedParamsSingleOutput() {
    const std::vector<std::vector<TopKParams>> generatedParams {
        generateParamsSingleOutput<element::Type_t::i8, element::Type_t::i64, element::Type_t::i32>(),
        generateParamsSingleOutput<element::Type_t::i16, element::Type_t::i64, element::Type_t::i32>(),
        generateParamsSingleOutput<element::Type_t::i32, element::Type_t::i64, element::Type_t::i32>(),
        generateParamsSingleOutput<element::Type_t::i64, element::Type_t::i64, element::Type_t::i32>(),
        generateParamsSingleOutput<element::Type_t::u8, element::Type_t::i64, element::Type_t::i32>(),
        generateParamsSingleOutput<element::Type_t::u16, element::Type_t::i64, element::Type_t::i32>(),
        generateParamsSingleOutput<element::Type_t::u32, element::Type_t::i64, element::Type_t::i32>(),
        generateParamsSingleOutput<element::Type_t::u64, element::Type_t::i64, element::Type_t::i32>(),
        generateParamsSingleOutput<element::Type_t::bf16, element::Type_t::i64, element::Type_t::i32>(),
        generateParamsSingleOutput<element::Type_t::f16, element::Type_t::i64, element::Type_t::i32>(),
        generateParamsSingleOutput<element::Type_t::f32, element::Type_t::i64, element::Type_t::i32>(),
        generateParamsSingleOutput<element::Type_t::f64, element::Type_t::i64, element::Type_t::i32>(),
    };
    std::vector<TopKParams> combinedParams;

    for (const auto& params : generatedParams) {
        combinedParams.insert(combinedParams.end(), params.begin(), params.end());
    }
    return combinedParams;
}

INSTANTIATE_TEST_SUITE_P(smoke_TopK_With_Hardcoded_Refs, ReferenceTopKTestSingleOutput,
    testing::ValuesIn(generateCombinedParamsSingleOutput()), ReferenceTopKTest::getTestCaseName);

TEST(ReferenceTopKTestInvalid, topk_v1_invalid_strings) {
    const auto data = std::make_shared<opset1::Parameter>(element::f32, Shape{1, 2, 3});
    const auto k = opset1::Constant::create(element::i64, Shape{}, {1});
    EXPECT_THROW(opset1::TopK(data, k, 0, "max", "invalid_mode"), ngraph::CheckFailure);
    EXPECT_THROW(opset1::TopK(data, k, 0, "invalid_sort", "index"), ngraph::CheckFailure);
}

TEST(ReferenceTopKTestInvalid, topk_v1_invalid_k) {
    const auto data = std::make_shared<opset1::Parameter>(element::f32, Shape{1, 2, 3});
    const auto k_non_scalar = opset1::Constant::create(element::i64, Shape{2}, {1, 2});
    EXPECT_THROW(opset1::TopK(data, k_non_scalar, 0, "max", "index"), ngraph::NodeValidationFailure);
    const auto k_float = opset1::Constant::create(element::f32, Shape{}, {1.0f});
    EXPECT_THROW(opset1::TopK(data, k_float, 0, "max", "index"), ngraph::NodeValidationFailure);
    const auto k_negative = opset1::Constant::create(element::i8, Shape{}, {-1});
    EXPECT_THROW(opset1::TopK(data, k_negative, 0, "max", "index"), ngraph::NodeValidationFailure);
}




class ReferenceTopKTestResnet50V3 : public ReferenceTopKTestResnet50 {
private:
    static std::shared_ptr<Model> CreateFunction(const TopKParamsResnet50& params) {
        const auto A = std::make_shared<opset1::Parameter>(params.A.type,
                                                           params.A.shape);
        const auto B = std::make_shared<opset3::TopK>(A,
                                                      opset1::Constant::create(element::i64, {}, {5}),
                                                      1,
                                                      opset1::TopK::Mode::MAX,
                                                      opset1::TopK::SortType::SORT_VALUES);
        const auto C = std::make_shared<opset3::TopK>(A,
                                                      opset1::Constant::create(element::i64, {}, {1}),
                                                      1,
                                                      opset1::TopK::Mode::MAX,
                                                      opset1::TopK::SortType::SORT_VALUES);

        const auto out5_value = B->output(0);
        const auto out5_index = B->output(1);
        const auto out1_value = C->output(0);
        const auto out1_index = C->output(1);
        const auto f = std::make_shared<Model>(OutputVector{out5_value, out5_index, out1_value, out1_index}, ParameterVector{A});
        return f;
    }
};

TEST_P(ReferenceTopKTestResnet50V3, CompareWithRefs) {
    Exec();
}

INSTANTIATE_TEST_SUITE_P(smoke_TopK_With_Hardcoded_Refs, ReferenceTopKTestResnet50V3,
    testing::ValuesIn(generateCombinedParamsResnet50()), ReferenceTopKTestResnet50V3::getTestCaseName);

class ReferenceTopKTestMaxMinSortV3 : public ReferenceTopKTestMaxMinSort {
private:
    static std::shared_ptr<Model> CreateFunction(const TopKParams& params) {
        const auto A = std::make_shared<opset1::Parameter>(params.A.type,
                                                           params.A.shape);
        const auto k = opset1::Constant::create(params.k.type,
                                                params.k.shape,
                                                params.k.data.data());
        const auto B = std::make_shared<opset3::TopK>(A, k, params.axis, params.mode, params.sort);
        const auto f = std::make_shared<Model>(B->outputs(), ParameterVector{A});
        return f;
    }
};

TEST_P(ReferenceTopKTestMaxMinSortV3, CompareWithRefs) {
    Exec();
}

INSTANTIATE_TEST_SUITE_P(smoke_TopK_With_Hardcoded_Refs, ReferenceTopKTestMaxMinSortV3,
    testing::ValuesIn(generateCombinedParamsMaxMinSort()), ReferenceTopKTestMaxMinSortV3::getTestCaseName);

class ReferenceTopKTestBackendV3 : public ReferenceTopKTestBackend {
private:
    static std::shared_ptr<Model> CreateFunction(const TopKParams& params) {
        const auto A = std::make_shared<opset1::Parameter>(params.A.type,
                                                           params.A.shape);
        const auto k = opset1::Constant::create(params.k.type,
                                                params.k.shape,
                                                params.k.data.data());
        const auto B = std::make_shared<opset3::TopK>(A, k, params.axis, params.mode, params.sort);
        const auto f = std::make_shared<Model>(B->outputs(), ParameterVector{A});
        return f;
    }
};

TEST_P(ReferenceTopKTestBackendV3, CompareWithRefs) {
    Exec();
}

INSTANTIATE_TEST_SUITE_P(smoke_TopK_With_Hardcoded_Refs, ReferenceTopKTestBackendV3,
    testing::ValuesIn(generateCombinedParamsBackend()), ReferenceTopKTestBackendV3::getTestCaseName);

class ReferenceTopKTest1dMaxMinV3 : public ReferenceTopKTest1dMaxMin {
private:
    static std::shared_ptr<Model> CreateFunction(const TopKParams& params, size_t out_idx) {
        const auto A = std::make_shared<opset1::Parameter>(params.A.type,
                                                           params.A.shape);
        const auto k = opset1::Constant::create(params.k.type,
                                                params.k.shape,
                                                params.k.data.data());
        const auto B = std::make_shared<opset3::TopK>(A, k, params.axis, params.mode, params.sort);
        const auto f = std::make_shared<Model>(OutputVector{B->output(out_idx)}, ParameterVector{A});
        return f;
    }
};

TEST_P(ReferenceTopKTest1dMaxMinV3, CompareWithRefs) {
    Exec();
}

INSTANTIATE_TEST_SUITE_P(smoke_TopK_With_Hardcoded_Refs, ReferenceTopKTest1dMaxMinV3,
    testing::ValuesIn(generateCombinedParams1dMaxMin()), ReferenceTopKTest1dMaxMinV3::getTestCaseName);

class ReferenceTopKTestInt64V3 : public ReferenceTopKTestInt64 {
private:
    static std::shared_ptr<Model> CreateFunction(const TopKParams& params, size_t out_idx) {
        const auto A = std::make_shared<opset1::Parameter>(params.A.type,
                                                           params.A.shape);
        const auto k = opset1::Constant::create(params.k.type,
                                                params.k.shape,
                                                params.k.data.data());
        const auto B = std::make_shared<opset3::TopK>(A,
                                                      k,
                                                      params.axis,
                                                      params.mode,
                                                      params.sort,
                                                      element::i64);
        const auto f = std::make_shared<Model>(OutputVector{B->output(out_idx)}, ParameterVector{A});
        return f;
    }
};

TEST_P(ReferenceTopKTestInt64V3, CompareWithRefs) {
    Exec();
}

INSTANTIATE_TEST_SUITE_P(smoke_TopK_With_Hardcoded_Refs, ReferenceTopKTestInt64V3,
    testing::ValuesIn(generateCombinedParamsInt64()), ReferenceTopKTestInt64V3::getTestCaseName);

class ReferenceTopKTestSingleOutputV3 : public ReferenceTopKTestSingleOutput {
private:
    static std::shared_ptr<Model> CreateFunction(const TopKParams& params) {
        const auto A = std::make_shared<opset1::Parameter>(params.A.type,
                                                           params.A.shape);
        const auto k = opset1::Constant::create(params.k.type,
                                                params.k.shape,
                                                params.k.data.data());
        const auto B = std::make_shared<opset3::TopK>(A, k, params.axis, params.mode, params.sort);
        const auto f = std::make_shared<Model>(OutputVector{B->output(1)}, ParameterVector{A});
        return f;
    }
};

TEST_P(ReferenceTopKTestSingleOutputV3, CompareWithRefs) {
    Exec();
}

INSTANTIATE_TEST_SUITE_P(smoke_TopK_With_Hardcoded_Refs, ReferenceTopKTestSingleOutputV3,
    testing::ValuesIn(generateCombinedParamsSingleOutput()), ReferenceTopKTestSingleOutputV3::getTestCaseName);

TEST(ReferenceTopKTestInvalidV3, topk_v3_invalid_strings) {
    const auto data = std::make_shared<opset1::Parameter>(element::f32, Shape{1, 2, 3});
    const auto k = opset1::Constant::create(element::i64, Shape{}, {1});
    EXPECT_THROW(opset3::TopK(data, k, 0, "max", "invalid_mode"), ngraph::CheckFailure);
    EXPECT_THROW(opset3::TopK(data, k, 0, "invalid_sort", "index"), ngraph::CheckFailure);
}

TEST(ReferenceTopKTestInvalidV3, topk_v3_invalid_k) {
    const auto data = std::make_shared<opset1::Parameter>(element::f32, Shape{1, 2, 3});
    const auto k_non_scalar = opset1::Constant::create(element::i64, Shape{2}, {1, 2});
    EXPECT_THROW(opset3::TopK(data, k_non_scalar, 0, "max", "index"), ngraph::NodeValidationFailure);
    const auto k_float = opset1::Constant::create(element::f32, Shape{}, {1.0f});
    EXPECT_THROW(opset3::TopK(data, k_float, 0, "max", "index"), ngraph::NodeValidationFailure);
    const auto k_negative = opset1::Constant::create(element::i8, Shape{}, {-1});
    EXPECT_THROW(opset3::TopK(data, k_negative, 0, "max", "index"), ngraph::NodeValidationFailure);
}
} // namespace

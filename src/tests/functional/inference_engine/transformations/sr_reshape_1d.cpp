// Copyright (C) 2018-2022 Intel Corporation
// SPDX-License-Identifier: Apache-2.0
//

#include <gtest/gtest.h>

#include <ngraph/function.hpp>
#include <ngraph/opsets/opset5.hpp>
#include <cpp/ie_cnn_network.h>


TEST(SmartReshapeTests, Reshape1d) {
    std::shared_ptr<ngraph::Function> f(nullptr);
    {
        auto input = std::make_shared<ngraph::opset5::Parameter>(ngraph::element::f32, ngraph::PartialShape::dynamic());
        input->set_friendly_name("input");
        auto reshape = std::make_shared<ngraph::opset5::Reshape>(input, ngraph::opset5::Constant::create(ngraph::element::i64, {1}, {5}), true);
        f = std::make_shared<ngraph::Function>(ngraph::NodeVector{reshape}, ngraph::ParameterVector{input});
    }

    InferenceEngine::CNNNetwork network(f);

    ASSERT_TRUE(network.getFunction()->get_results()[0]->get_output_partial_shape(0).compatible(ngraph::PartialShape::dynamic()));
    ASSERT_TRUE(network.getFunction()->get_parameters()[0]->get_partial_shape().compatible({5}));

    ASSERT_NO_THROW(network.reshape(InferenceEngine::ICNNNetwork::InputShapes{{"input", {1, 3, 300, 300}}}));

    ASSERT_TRUE(network.getFunction()->get_results()[0]->get_output_partial_shape(0).compatible({270000}));
    ASSERT_TRUE(network.getFunction()->get_parameters()[0]->get_partial_shape().compatible({1, 3, 300, 300}));
}

TEST(SmartReshapeTests, Reshape1d_negative) {
    std::shared_ptr<ngraph::Function> f(nullptr);
    {
        auto input = std::make_shared<ngraph::opset5::Parameter>(ngraph::element::f32, ngraph::PartialShape::dynamic());
        auto pattern = std::make_shared<ngraph::opset5::Parameter>(ngraph::element::i64, ngraph::Shape{1});
        input->set_friendly_name("input");
        auto reshape = std::make_shared<ngraph::opset5::Reshape>(input, pattern, false);
        f = std::make_shared<ngraph::Function>(ngraph::NodeVector{reshape}, ngraph::ParameterVector{input, pattern});
    }

    InferenceEngine::CNNNetwork network(f);

    ASSERT_TRUE(network.getFunction()->get_results()[0]->get_output_partial_shape(0).compatible(ngraph::PartialShape::dynamic()));
    ASSERT_TRUE(network.getFunction()->get_parameters()[0]->get_partial_shape().is_dynamic());

    ASSERT_NO_THROW(network.reshape(InferenceEngine::ICNNNetwork::InputShapes{{"input", {1, 3, 300, 300}}}));

    ASSERT_TRUE(network.getFunction()->get_results()[0]->get_output_partial_shape(0).compatible({270000}));
    ASSERT_TRUE(network.getFunction()->get_parameters()[0]->get_partial_shape().compatible({1, 3, 300, 300}));
    ASSERT_FALSE(network.getFunction()->get_parameters()[1]->get_output_target_inputs(0).empty());
}

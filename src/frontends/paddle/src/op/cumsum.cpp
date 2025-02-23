// Copyright (C) 2018-2022 Intel Corporation
// SPDX-License-Identifier: Apache-2.0
//

#include <node_context.hpp>

#include "default_opset.hpp"

namespace ov {
namespace frontend {
namespace paddle {
namespace op {
NamedOutputs cumsum(const NodeContext& node) {
    const auto x = node.get_ng_input("X");
    const auto axis = node.get_attribute<int32_t>("axis", -1);
    const auto flatten = node.get_attribute<bool>("flatten", false);
    const auto reverse = node.get_attribute<bool>("reverse", false);
    const auto exclusive = node.get_attribute<bool>("exclusive", false);

    std::shared_ptr<ov::Node> input = x.get_node_shared_ptr();
    if (flatten) {
        // convert to 1-d tensor
        input = std::make_shared<default_opset::Reshape>(x,
                                                         default_opset::Constant::create(element::i64, {1}, {-1}),
                                                         false);
    }

    const auto axis_node = default_opset::Constant::create(element::i64, {}, {axis});
    return node.default_single_output_mapping(
        {std::make_shared<default_opset::CumSum>(input, axis_node, exclusive, reverse)},
        {"Out"});
}

}  // namespace op
}  // namespace paddle
}  // namespace frontend
}  // namespace ov

// Copyright (C) 2018-2022 Intel Corporation
// SPDX-License-Identifier: Apache-2.0
//

#include <node_context.hpp>

#include "openvino/opsets/opset6.hpp"

namespace ov {
namespace frontend {
namespace paddle {
namespace op {
NamedOutputs cast(const NodeContext& node) {
    auto data = node.get_ng_input("X");
    auto out_dtype = node.get_attribute<ov::element::Type>("out_dtype");

    return node.default_single_output_mapping({std::make_shared<ov::opset6::Convert>(data, out_dtype)}, {"Out"});
}

}  // namespace op
}  // namespace paddle
}  // namespace frontend
}  // namespace ov

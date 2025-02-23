// Copyright (C) 2018-2022 Intel Corporation
// SPDX-License-Identifier: Apache-2.0
//

#include "exceptions.hpp"

#include "node_context.hpp"

namespace ov {
namespace frontend {
namespace tensorflow {
std::string OpValidationFailure::get_error_msg_prefix_tf(const tensorflow::NodeContext& node) {
    std::stringstream ss;
    ss << "While validating node '" << node.get_op_type() << '\'';
    return ss.str();
}
}  // namespace tensorflow
}  // namespace frontend
}  // namespace ov

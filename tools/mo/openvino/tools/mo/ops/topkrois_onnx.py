# Copyright (C) 2018-2022 Intel Corporation
# SPDX-License-Identifier: Apache-2.0

from openvino.tools.mo.ops.op import Op


class ExperimentalDetectronTopKROIs(Op):
    op = 'ExperimentalDetectronTopKROIs'

    def __init__(self, graph, attrs):
        mandatory_props = dict(
            type=self.op,
            op=self.op,
            version='experimental',
            infer=self.infer
        )
        super().__init__(graph, mandatory_props, attrs)

    def backend_attrs(self):
        return ['max_rois', ]

    @staticmethod
    def infer(node):
        node.out_port(0).data.set_shape([node.max_rois, 4])

# Copyright (C) 2018-2022 Intel Corporation
# SPDX-License-Identifier: Apache-2.0

from openvino.tools.mo.front.common.layout import get_batch_dim, shape_for_layout
from openvino.tools.mo.graph.graph import Node, Graph
from openvino.tools.mo.ops.op import Op


class PSROIPoolingOp(Op):
    op = 'PSROIPooling'
    enabled = False

    def __init__(self, graph: Graph, attrs: dict):
        mandatory_props = {
            'type': self.op,
            'op': self.op,
            'version': 'opset2',
            'mode': 'average',
            'in_ports_count': 2,
            'out_ports_count': 1,
            'trans_std': 0,
            'no_trans': True,
            'infer': PSROIPoolingOp.psroipooling_infer
        }
        super().__init__(graph, mandatory_props, attrs)

    def supported_attrs(self):
        return [
            'spatial_scale',
            'output_dim',
            ('group_size', lambda node: int(node.group_size)),
            'mode',
            'spatial_bins_x',
            'spatial_bins_y',
        ]

    @staticmethod
    def psroipooling_infer(node: Node):
        """
        Sets shape of output node according specified parameters input blobs and node
        Sets number from the first input blob, channels from the second one, height and width are specified
        Parameters
        ----------
        node
        """
        shapes = [node.in_node(i).shape for i in range(len(node.in_nodes()))]
        if any(s is None for s in shapes):
            return
        layout = node.graph.graph['layout']
        assert len(layout) == 4
        assert node.has_valid('group_size')
        assert node.group_size == int(node.group_size)
        node['group_size'] = int(node['group_size'])
        node.out_node().shape = shape_for_layout(layout,
                                                 batch=shapes[1][get_batch_dim(layout, 4)],
                                                 features=node.output_dim,
                                                 height=node.group_size,
                                                 width=node.group_size)


class DeformablePSROIPoolingOp(PSROIPoolingOp):
    op = 'DeformablePSROIPooling'
    enabled = False

    def __init__(self, graph: Graph, attrs: dict):
        updated_attrs = {
            'type': self.op,
            'op': self.op,
            'version': 'opset1',
            'mode': 'bilinear_deformable',
            'in_ports_count': 3,
            'trans_std': 0,
        }
        updated_attrs.update(attrs)
        super().__init__(graph, updated_attrs)

    def supported_attrs(self):
        return super().supported_attrs() + ['trans_std', 'part_size']

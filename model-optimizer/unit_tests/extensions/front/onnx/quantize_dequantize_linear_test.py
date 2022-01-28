# Copyright (C) 2018-2021 Intel Corporation
# SPDX-License-Identifier: Apache-2.0

import unittest

import numpy as np
import onnx

from extensions.front.onnx.quantize_dequantize_linear import QuantizeDequantizeLinear
from mo.utils.ir_engine.compare_graphs import compare_graphs
from unit_tests.utils.graph import build_graph

# quantize and dequantize share tensors with scale/zp
nodes0_attributes = {
    'input': {'kind': 'op', 'op': 'AnyOp'},
    'quantize': {'kind': 'op', 'op': 'QuantizeLinear'},
    'dequantize': {'kind': 'op', 'op': 'DequantizeLinear'},
    'scale_param': {'kind': 'op', 'type': 'Const', 'op': 'Const'},
    'zerop_param': {'kind': 'op', 'type': 'Const', 'op': 'Const'},
    'out': {'kind': 'op', 'op': 'AnyOp'},
}

# quantize and dequantize do not share tensors with scale/zp
nodes1_attributes = {
    'input': {'kind': 'op', 'op': 'AnyOp'},
    'quantize': {'kind': 'op', 'op': 'QuantizeLinear'},
    'dequantize': {'kind': 'op', 'op': 'DequantizeLinear'},
    'scale_param_q': {'kind': 'op', 'type': 'Const', 'op': 'Const'},
    'zerop_param_q': {'kind': 'op', 'type': 'Const', 'op': 'Const'},
    'scale_param_dq': {'kind': 'op', 'type': 'Const', 'op': 'Const'},
    'zerop_param_dq': {'kind': 'op', 'type': 'Const', 'op': 'Const'},
    'out': {'kind': 'op', 'op': 'AnyOp'},
}

nodes_ref_attributes = {
    'input': {'kind': 'op', 'op': 'AnyOp'},
    'fq': {'kind': 'op', 'op': 'FakeQuantize'},
    'min_param': {'kind': 'op', 'type': 'Const', 'op': 'Const'},
    'max_param': {'kind': 'op', 'type': 'Const', 'op': 'Const'},
    'out': {'kind': 'op', 'op': 'AnyOp'},
}


class TestQuantizeDeQuantize2FakeQuantize(unittest.TestCase):

    def test_quantizedequantize2fakequantize_0(self):
        # testing the code path with uint8 zero-point
        graph = build_graph(nodes1_attributes,
                            [('input', 'quantize'),
                             ('quantize', 'dequantize'),
                             ('scale_param_q', 'quantize'),
                             ('zerop_param_q', 'quantize'),
                             ('scale_param_dq', 'dequantize'),
                             ('zerop_param_dq', 'dequantize'),
                             ('dequantize', 'out'),
                             ],
                            {'scale_param_q': {'shape': np.array([1]), 'value': np.float32(1.0 / 255)},
                             'zerop_param_q': {'shape': np.array([1]), 'value': np.uint8(0)},
                             'scale_param_dq': {'shape': np.array([1]), 'value': np.float32(1.0 / 255)},
                             'zerop_param_dq': {'shape': np.array([1]), 'value': np.uint8(0)},
                             }, nodes_with_edges_only=True)

        graph_ref = build_graph(nodes_ref_attributes,
                                [('input', 'fq', {'in': 0}),
                                 ('min_param', 'fq', {'out': 0, 'in': 1}),
                                 ('min_param', 'fq', {'out': 0, 'in': 3}),
                                 ('max_param', 'fq', {'out': 0, 'in': 2}),
                                 ('max_param', 'fq', {'out': 0, 'in': 4}),
                                 ('fq', 'out'),
                                 ],
                                {'fq': {'levels': 256},
                                 'min_param': {'value': np.float32(0.0)},
                                 'max_param': {'value': np.float32(1.0)},
                                 }, nodes_with_edges_only=True)

        graph.stage = 'front'
        tested_class = QuantizeDequantizeLinear()
        tested_class.find_and_replace_pattern(graph)

        (flag, resp) = compare_graphs(graph, graph_ref, 'out', check_op_attrs=True)
        self.assertTrue(flag, resp)

    def test_quantizedequantize2fakequantize_1(self):
        # testing the code path with int8 zero-point
        graph = build_graph(nodes0_attributes,
                            [('input', 'quantize'),
                             ('quantize', 'dequantize'),
                             ('scale_param', 'quantize'),
                             ('zerop_param', 'quantize'),
                             ('scale_param', 'dequantize'),
                             ('zerop_param', 'dequantize'),
                             ('dequantize', 'out'),
                             ],
                            {'scale_param': {'shape': np.array([1]), 'value': np.float32(1.0 / 255)},
                             'zerop_param': {'shape': np.array([1]), 'value': np.int8(0)},
                             }, nodes_with_edges_only=True)

        graph_ref = build_graph(nodes_ref_attributes,
                                [('input', 'fq', {'in': 0}),
                                 ('min_param', 'fq', {'out': 0, 'in': 1}),
                                 ('min_param', 'fq', {'out': 0, 'in': 3}),
                                 ('max_param', 'fq', {'out': 0, 'in': 2}),
                                 ('max_param', 'fq', {'out': 0, 'in': 4}),
                                 ('fq', 'out'),
                                 ],
                                {'fq': {'levels': 256},
                                 'min_param': {'value': np.float32(-128.0 / 255)},
                                 'max_param': {'value': np.float32(127.0 / 255)},
                                 }, nodes_with_edges_only=True)

        graph.stage = 'front'
        tested_class = QuantizeDequantizeLinear()
        tested_class.find_and_replace_pattern(graph)

        (flag, resp) = compare_graphs(graph, graph_ref, 'out', check_op_attrs=True)
        self.assertTrue(flag, resp)

    def test_quantizedequantize2fakequantize_2(self):
        # testing the code path with channelwise quantization
        q_axis = 0
        n_channels = 16
        input_shape = np.array([n_channels, 8, 3, 3])

        q_node_pb = onnx.helper.make_node('QuantizeLinear', ['s', 'zp'], ['q'], axis=q_axis)
        dq_node_pb = onnx.helper.make_node('DequantizeLinear', ['s', 'zp'], ['dq'], axis=q_axis)
        graph = build_graph(nodes0_attributes,
                            [('input', 'quantize'),
                             ('quantize', 'dequantize'),
                             ('scale_param', 'quantize'),
                             ('zerop_param', 'quantize'),
                             ('scale_param', 'dequantize'),
                             ('zerop_param', 'dequantize'),
                             ('dequantize', 'out'),
                             ],
                            {'input': {'shape': input_shape},
                             'scale_param': {'shape': np.array([n_channels]),
                                             'value': np.array([1.0 / 255] * n_channels, dtype=np.float32),
                                             },
                             'zerop_param': {'shape': np.array([n_channels]),
                                             'value': np.array([0] * n_channels, dtype=np.int8),
                                             },
                             'quantize': {'pb': q_node_pb},
                             'dequantize': {'pb': dq_node_pb},
                             }, nodes_with_edges_only=True)

        min_max_shape = [1] * len(input_shape)
        min_max_shape[q_axis] = n_channels
        graph_ref = build_graph(nodes_ref_attributes,
                                [('input', 'fq', {'in': 0}),
                                 ('min_param', 'fq', {'out': 0, 'in': 1}),
                                 ('min_param', 'fq', {'out': 0, 'in': 3}),
                                 ('max_param', 'fq', {'out': 0, 'in': 2}),
                                 ('max_param', 'fq', {'out': 0, 'in': 4}),
                                 ('fq', 'out'),
                                 ],
                                {'fq': {'levels': 256},
                                 'min_param': {'value': np.full(min_max_shape, -128.0 / 255, dtype=np.float32)},
                                 'max_param': {'value': np.full(min_max_shape, 127.0 / 255, dtype=np.float32)},
                                 }, nodes_with_edges_only=True)

        graph.stage = 'front'
        tested_class = QuantizeDequantizeLinear()
        tested_class.find_and_replace_pattern(graph)

        (flag, resp) = compare_graphs(graph, graph_ref, 'out', check_op_attrs=True)
        self.assertTrue(flag, resp)

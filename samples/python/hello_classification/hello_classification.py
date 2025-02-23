#!/usr/bin/env python3
# -*- coding: utf-8 -*-
# Copyright (C) 2018-2022 Intel Corporation
# SPDX-License-Identifier: Apache-2.0

import logging as log
import sys

import cv2
import numpy as np
from openvino.preprocess import PrePostProcessor, ResizeAlgorithm
from openvino.runtime import Core, Layout, Type


def main():
    log.basicConfig(format='[ %(levelname)s ] %(message)s', level=log.INFO, stream=sys.stdout)

    # Parsing and validation of input arguments
    if len(sys.argv) != 4:
        log.info('Usage: <path_to_model> <path_to_image> <device_name>')
        return 1

    model_path = sys.argv[1]
    image_path = sys.argv[2]
    device_name = sys.argv[3]

# --------------------------- Step 1. Initialize OpenVINO Runtime Core ------------------------------------------------
    log.info('Creating OpenVINO Runtime Core')
    core = Core()

# --------------------------- Step 2. Read a model --------------------------------------------------------------------
    log.info(f'Reading the network: {model_path}')
    # (.xml and .bin files) or (.onnx file)
    model = core.read_model(model_path)

    if len(model.inputs) != 1:
        log.error('Sample supports only single input topologies')
        return -1

    if len(model.outputs) != 1:
        log.error('Sample supports only single output topologies')
        return -1

# --------------------------- Step 3. Set up input --------------------------------------------------------------------
    # Read input image
    image = cv2.imread(image_path)
    # Add N dimension
    input_tensor = np.expand_dims(image, 0)

# --------------------------- Step 4. Apply preprocessing -------------------------------------------------------------
    ppp = PrePostProcessor(model)

    _, h, w, _ = input_tensor.shape

    # 1) Set input tensor information:
    # - input() provides information about a single model input
    # - precision of tensor is supposed to be 'u8'
    # - layout of data is 'NHWC'
    # - set static spatial dimensions to input tensor to resize from
    ppp.input().tensor() \
        .set_element_type(Type.u8) \
        .set_layout(Layout('NHWC')) \
        .set_spatial_static_shape(h, w)  # noqa: ECE001, N400

    # 2) Adding explicit preprocessing steps:
    # - apply linear resize from tensor spatial dims to model spatial dims
    ppp.input().preprocess().resize(ResizeAlgorithm.RESIZE_LINEAR)

    # 3) Here we suppose model has 'NCHW' layout for input
    ppp.input().model().set_layout(Layout('NCHW'))

    # 4) Set output tensor information:
    # - precision of tensor is supposed to be 'f32'
    ppp.output().tensor().set_element_type(Type.f32)

    # 5) Apply preprocessing modifing the original 'model'
    model = ppp.build()

# --------------------------- Step 5. Loading model to the device -----------------------------------------------------
    log.info('Loading the model to the plugin')
    compiled_model = core.compile_model(model, device_name)

# --------------------------- Step 6. Create infer request and do inference synchronously -----------------------------
    log.info('Starting inference in synchronous mode')
    results = compiled_model.infer_new_request({0: input_tensor})

# --------------------------- Step 7. Process output ------------------------------------------------------------------
    predictions = next(iter(results.values()))

    # Change a shape of a numpy.ndarray with results to get another one with one dimension
    probs = predictions.reshape(-1)

    # Get an array of 10 class IDs in descending order of probability
    top_10 = np.argsort(probs)[-10:][::-1]

    header = 'class_id probability'

    log.info(f'Image path: {image_path}')
    log.info('Top 10 results: ')
    log.info(header)
    log.info('-' * len(header))

    for class_id in top_10:
        probability_indent = ' ' * (len('class_id') - len(str(class_id)) + 1)
        log.info(f'{class_id}{probability_indent}{probs[class_id]:.7f}')

    log.info('')

# ----------------------------------------------------------------------------------------------------------------------
    log.info('This sample is an API example, for any performance measurements please use the dedicated benchmark_app tool\n')
    return 0


if __name__ == '__main__':
    sys.exit(main())

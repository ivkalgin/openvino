// Copyright (C) 2018-2022 Intel Corporation
// SPDX-License-Identifier: Apache-2.0
//

#pragma once

#if defined(HAVE_GPU_DEVICE_MEM_SUPPORT)
#    define HAVE_DEVICE_MEM_SUPPORT
#    include <gpu/gpu_context_api_ocl.hpp>
#endif
#include "utils.hpp"

namespace gpu {

#ifdef HAVE_DEVICE_MEM_SUPPORT
using BufferType = cl::Buffer;

struct OpenCL {
    cl::Context _context;
    cl::Device _device;
    cl::CommandQueue _queue;

    explicit OpenCL(std::shared_ptr<std::vector<cl_context_properties>> media_api_context_properties = nullptr) {
        // get Intel GPU OCL device, create context and queue
        {
            std::vector<cl::Device> devices;
            std::vector<cl::Platform> platforms;
            const unsigned int refVendorID = 0x8086;

            cl::Platform::get(&platforms);
            for (auto& p : platforms) {
                p.getDevices(CL_DEVICE_TYPE_GPU, &devices);
                for (auto& d : devices) {
                    if (refVendorID == d.getInfo<CL_DEVICE_VENDOR_ID>()) {
                        _device = d;
                        _context = cl::Context(_device);
                        break;
                    }
                }
            }

            cl_command_queue_properties props = CL_QUEUE_OUT_OF_ORDER_EXEC_MODE_ENABLE;
            _queue = cl::CommandQueue(_context, _device, props);
        }
    }

    explicit OpenCL(cl_context context) {
        // user-supplied context handle
        _context = cl::Context(context, true);
        _device = cl::Device(_context.getInfo<CL_CONTEXT_DEVICES>()[0].get(), true);

        cl_command_queue_properties props = CL_QUEUE_OUT_OF_ORDER_EXEC_MODE_ENABLE;
        _queue = cl::CommandQueue(_context, _device, props);
    }
};
#else
using BufferType = void*;
#endif

std::map<std::string, ov::runtime::TensorVector> get_remote_input_tensors(
    const std::map<std::string, std::vector<std::string>>& inputFiles,
    const std::vector<benchmark_app::InputsInfo>& app_inputs_info,
    const ov::runtime::CompiledModel& compiledModel,
    std::vector<BufferType>& clBuffer);

std::map<std::string, ov::runtime::Tensor> get_remote_output_tensors(
    const ov::runtime::CompiledModel& compiledModel,
    std::map<std::string, ::gpu::BufferType>& clBuffer);
}  // namespace gpu

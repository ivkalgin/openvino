# Copyright (C) 2018-2022 Intel Corporation
# SPDX-License-Identifier: Apache-2.0
#

set(VPU_DEPENDENCIES
    vpu_copy_firmware)

if (ENABLE_INTEL_GPU)
    list(APPEND VPU_DEPENDENCIES ov_intel_gpu_plugin)
    if(ENABLE_HETERO)
        list(APPEND VPU_DEPENDENCIES ov_hetero_plugin)
    endif()
endif()

addIeTarget(
        NAME myriadTestData
        DEVELOPER_PACKAGE
            tests
        TYPE STATIC
        ROOT ${CMAKE_CURRENT_SOURCE_DIR}/test_data
        LINK_LIBRARIES
            ieTestHelpers
        DEFINES
            PUBLIC
            DATA_PATH=\"${DATA_PATH}\"
            MODELS_PATH=\"${MODELS_PATH}\"
)

addIeTarget(
    NAME VPUCommonTests
    DEVELOPER_PACKAGE
        tests
    TYPE STATIC
    ROOT ${CMAKE_CURRENT_SOURCE_DIR}/common
    ADDITIONAL_SOURCE_DIRS
        ${CMAKE_CURRENT_SOURCE_DIR}/shared_tests_instance
        ${CMAKE_CURRENT_SOURCE_DIR}/vpu_base
    INCLUDES
        ${CMAKE_CURRENT_SOURCE_DIR}/vpu_base
        ${CMAKE_CURRENT_SOURCE_DIR}/common/regression/helpers
    LINK_LIBRARIES
        IESharedTests
        ieTestHelpers
        funcTestUtils
        format_reader
        vpu_graph_transformer
        vpu_custom_kernels
)

target_include_directories(VPUCommonTests INTERFACE
    ${CMAKE_CURRENT_SOURCE_DIR}/vpu_base
    ${CMAKE_CURRENT_SOURCE_DIR}/common/regression/helpers
    $<TARGET_PROPERTY:funcTestUtils,INTERFACE_INCLUDE_DIRECTORIES>
    $<TARGET_PROPERTY:vpu_graph_transformer,INTERFACE_INCLUDE_DIRECTORIES>
    )

openvino_developer_export_targets(COMPONENT vpu TARGETS vpu_custom_kernels)

addIeTargetTest(
    NAME MyriadFunctionalTests
    ROOT ${CMAKE_CURRENT_SOURCE_DIR}/myriad_tests
    ADDITIONAL_SOURCE_DIRS
        ${CMAKE_CURRENT_SOURCE_DIR}/graph_transformer
    INCLUDES
        $<TARGET_PROPERTY:vpu_graph_transformer,INTERFACE_INCLUDE_DIRECTORIES>
        ${CMAKE_CURRENT_BINARY_DIR}
    LINK_LIBRARIES
        IESharedTests
        vpu_custom_kernels
    LINK_LIBRARIES_WHOLE_ARCHIVE
        myriadTestData
        VPUCommonTests
    DEPENDENCIES
        ${VPU_DEPENDENCIES}
        ov_intel_vpu_plugin
    LABELS  # Must be the last parameter. Reason: see description comment for addIeTargetTest().
        VPU
        MYRIAD
    )
# because vpu_graph_transformer is compiled with LTO
if(CMAKE_COMPILER_IS_GNUCXX AND CMAKE_CXX_COMPILER_VERSION VERSION_GREATER_EQUAL 9.0)
    set_target_properties(VPUCommonTests PROPERTIES INTERPROCEDURAL_OPTIMIZATION_RELEASE ${ENABLE_LTO})
endif()

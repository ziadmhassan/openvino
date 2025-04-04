// Copyright (C) 2024 Intel Corporation
// SPDX-License-Identifier: Apache-2.0
//

#pragma once

#include "openvino/pass/graph_rewrite.hpp"

namespace ov::intel_gpu {

// Before:
//          ┌─────────┐                                                                  ┌─────────┐
//          │ReadValue│                                                                  │ReadValue│
//          └────┬────┘                                                                  └────┬────┘
//               │                                 ┌───────────┐                              │
//               │         ┌───────────────────────┼ LoraInput ┼───────────────────┐          │
//               │         │                       └─────┬─────┘                   │          │
//               │    ┌────▼───┐                         │                    ┌────▼───┐      │
//               └────►  Gemm  │                         │                    │  Gemm  ◄──────┘
// ┌─────────┐        └────┬───┘                         │                    └────┬───┘       ┌─────────┐
// │ReadValue│             │                             │                         │           │ReadValue│
// └────┬────┘             │                 ┌───────────▼────────────┐            │           └────┬────┘
//      │             ┌────▼───┐             │FullyConnectedCompressed│       ┌────▼───┐            │
//      └─────────────►Multiply│             └───────────┬────────────┘       │Multiply◄────────────┘
//                    └────┬───┘                         │                    └────────┘
// ┌─────────┐             │                             │                         │               ┌─────────┐
// │ReadValue│             │                             │                         │               │ReadValue│
// └────┬────┘             │                             │                         │               └────┬────┘
//      │             ┌────▼───┐                  ┌──────▼──────┐             ┌────▼───┐                │
//      └─────────────►  Gemm  │      ┌───────────┼VariadicSplit┼──────────┐  │  Gemm  ◄────────────────┘
//                    └────┬───┘      │           └──────┬──────┘          │  └────┬───┘
//                         │          │                  │                 │       │
//                         │          │                  │                 │       │
//                         │          │                  │                 │       │
//                         │       ┌──▼──┐               ▼              ┌──▼──┐    │
//                         └───────► Add │              ...             │ Add ◄────┘
//                                 └─────┘                              └─────┘
// After:
//                                                                                   ┌─────────┐
//                                                                              ┌────┼ReadValue│
//        ┌──────────┐                                             ┌──────┐     │    └─────────┘
//        │LoRA_Input┼────────────────────────────┐  ┌─────────────┼Concat◄─────┤     ...
//        └────┬─────┘                            │  │             └──────┘     │    ┌─────────┐
//             │                                  │  │                          └────┼ReadValue│
//             │                                  │  │                               └─────────┘
//             │                             ┌────▼──▼───┐
//             │                             │MatMulFused│
//             │                             └───────────┘
//             │                                   │                                 ┌─────────┐
//             │                                   │                            ┌────┼ReadValue│
//             │                                   │               ┌──────┐     │    └─────────┘
//             │                                   │      ┌────────┼Concat◄─────┤     ...
//             │                                   │      │        └──────┘     │    ┌─────────┐
//             │                                   │      │                     └────┼ReadValue│
// ┌───────────▼────────────┐                  ┌───▼──────▼──┐                       └─────────┘
// │FullyConnectedCompressed│                  │MultiplyFused│
// └───────────┬────────────┘                  └──────┬──────┘
//             │                                      │
//             │                     ┌─────────┐      │      ┌─────────┐
//             │                     │ReadValue│   ┌──▼──┐   │ReadValue│
//             │                     └────┬────┘   │Split│   └────┬────┘
//             │                          │        └──┬──┘        │
//             │                          │           │           │
//             │                          │  ┌────────┼────────┐  │
//             │                          │  │                 │  │
//             │                       ┌──▼──▼──┐           ┌──▼──▼──┐
//             │                       │ MatMul │    ...    │ MatMul │
//             │                       └────┬───┘           └────┬───┘
//             │                            └──────┐    ┌────────┘
//             │                                   │    │
//             │             ┌─────┐             ┌─▼────▼─┐
//             └─────────────► Add ◄─────────────┼ Concat │
//                           └──┬──┘             └────────┘
//                              │
//                              │
//                       ┌──────▼──────┐
//                       │VariadicSplit│
//                       └─────────────┘

class LoRAHorizontalFusion: public ov::pass::MatcherPass {
public:
    OPENVINO_MATCHER_PASS_RTTI("LoRAHorizontalFusion");
    LoRAHorizontalFusion();
};

}   // namespace ov::intel_gpu

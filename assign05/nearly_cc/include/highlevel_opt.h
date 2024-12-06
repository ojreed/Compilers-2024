// Copyright (c) 2021-2023, David H. Hovemeyer <david.hovemeyer@gmail.com>
//
// Permission is hereby granted, free of charge, to any person obtaining a
// copy of this software and associated documentation files (the "Software"),
// to deal in the Software without restriction, including without limitation
// the rights to use, copy, modify, merge, publish, distribute, sublicense,
// and/or sell copies of the Software, and to permit persons to whom the
// Software is furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included
// in all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
// THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR
// OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
// ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
// OTHER DEALINGS IN THE SOFTWARE.

#ifndef HIGHLEVEL_OPT_H
#define HIGHLEVEL_OPT_H

#include <memory>
#include <vector>
#include <algorithm>
#include <functional>
#include <stdexcept>
#include "options.h"
#include "function.h"
#include "cfg.h"
#include "cfg_builder.h"
#include "cfg_printer.h"
#include "cfg_transform.h"
#include "live_vregs.h"

//! HighLevelOpt is responsible for doing optimizations
//! on the high-level IR for a Function.
class HighLevelOpt {
private:
  // command line options
  const Options &m_options;

  // helper functions can use this to access the Function
  std::shared_ptr<Function> m_function;

  // no value semantics
  HighLevelOpt(const HighLevelOpt &);
  HighLevelOpt &operator=(const HighLevelOpt &);

public:
  //! Constructor.
  //! @param options Reference to the command-line Options
  HighLevelOpt(const Options &options);
  ~HighLevelOpt();

  //! Optimize the high-level IR for given Function.
  //! The implementation should store an optimized version of the
  //! high-level InstructionSequence in the Function object,
  //! based on the original (unoptimized) high-level InstructionSequence.
  //!
  //! @param function the Function whose high-level InstructionSequence
  //!                 should be optimized
  void optimize(std::shared_ptr<Function> function);
};

#endif // HIGHLEVEL_OPT_H

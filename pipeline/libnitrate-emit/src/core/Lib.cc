////////////////////////////////////////////////////////////////////////////////
///                                                                          ///
///     .-----------------.    .----------------.     .----------------.     ///
///    | .--------------. |   | .--------------. |   | .--------------. |    ///
///    | | ____  _____  | |   | |     ____     | |   | |    ______    | |    ///
///    | ||_   _|_   _| | |   | |   .'    `.   | |   | |   / ____ `.  | |    ///
///    | |  |   \ | |   | |   | |  /  .--.  \  | |   | |   `'  __) |  | |    ///
///    | |  | |\ \| |   | |   | |  | |    | |  | |   | |   _  |__ '.  | |    ///
///    | | _| |_\   |_  | |   | |  \  `--'  /  | |   | |  | \____) |  | |    ///
///    | ||_____|\____| | |   | |   `.____.'   | |   | |   \______.'  | |    ///
///    | |              | |   | |              | |   | |              | |    ///
///    | '--------------' |   | '--------------' |   | '--------------' |    ///
///     '----------------'     '----------------'     '----------------'     ///
///                                                                          ///
///   * NITRATE TOOLCHAIN - The official toolchain for the Nitrate language. ///
///   * Copyright (C) 2024 Wesley C. Jones                                   ///
///                                                                          ///
///   The Nitrate Toolchain is free software; you can redistribute it or     ///
///   modify it under the terms of the GNU Lesser General Public             ///
///   License as published by the Free Software Foundation; either           ///
///   version 2.1 of the License, or (at your option) any later version.     ///
///                                                                          ///
///   The Nitrate Toolcain is distributed in the hope that it will be        ///
///   useful, but WITHOUT ANY WARRANTY; without even the implied warranty of ///
///   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU      ///
///   Lesser General Public License for more details.                        ///
///                                                                          ///
///   You should have received a copy of the GNU Lesser General Public       ///
///   License along with the Nitrate Toolchain; if not, see                  ///
///   <https://www.gnu.org/licenses/>.                                       ///
///                                                                          ///
////////////////////////////////////////////////////////////////////////////////

#include <llvm-18/llvm/MC/TargetRegistry.h>
#include <llvm-18/llvm/Support/ManagedStatic.h>
#include <llvm-18/llvm/Support/TargetSelect.h>
#include <nitrate-emit/Lib.h>

#include <atomic>
#include <iostream>
#include <nitrate-core/Init.hh>
#include <nitrate-core/Logger.hh>
#include <nitrate-core/Macro.hh>

static std::atomic<size_t> QcodeLibRefCount = 0;

static auto InitializeLLVM() -> bool {
#ifdef LLVM_SUUPORT_ALL_TARGETS
  llvm::InitializeAllTargetInfos();
  llvm::InitializeAllTargets();
  llvm::InitializeAllTargetMCs();
  llvm::InitializeAllAsmParsers();
  llvm::InitializeAllAsmPrinters();

  /* Check if LLVM is initialized */
  if (llvm::TargetRegistry::targets().empty()) {
    ncc::Log << "LLVM initialization failed";
    return false;
  }
#else
#warning "Building LIBNITRATE without support for ANY LLVM targets!!"
#endif

  return true;
}

static void DoinitializeLLVM() { llvm::llvm_shutdown(); }

static auto DoInit() -> bool {
  if (!InitializeLLVM()) {
    return false;
  }

  return true;
}

static void DoDeinit() { DoinitializeLLVM(); }

extern "C" NCC_EXPORT auto QcodeLibInit() -> bool {
  if (QcodeLibRefCount++ > 1) {
    return true;
  }

  if (!ncc::CoreLibrary.InitRC()) {
    return false;
  }

  return DoInit();
}

extern "C" NCC_EXPORT void QcodeLibDeinit() {
  if (--QcodeLibRefCount > 0) {
    return;
  }

  DoDeinit();

  ncc::CoreLibrary.DeinitRC();
}

extern "C" NCC_EXPORT auto QcodeStrerror() -> const char* { return ""; }

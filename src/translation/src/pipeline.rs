//! Type-state pipeline builder for the Nitrate compiler.
//!
//! This module centralizes all pipeline construction and execution, ensuring
//! only valid stage transitions can occur at compile time through Rust's type
//! system. Each pipeline stage wraps the output of a compilation pass, and
//! the only way to reach a subsequent stage is through a typed transition method.
//!
//! The `Store` (TLS-backed HIR storage) is NOT owned by pipeline stages.
//! The caller must set up TLS via `using_storage(&store, || { ... })` before
//! invoking any HIR stage methods (those after `lower_hir`).
//!
//! # Stage flow
//!
//! ```text
//! Source ──lex()──► Tokenized ──parse()──► Parsed ──lower_hir()──► HirLowered
//!                                                                      │
//!                                                                optimize_hir()
//!                                                                      │
//!                                                                      ▼
//!                                                               HirOptimized
//!                                                                      │
//!                                                                validate()
//!                                                                      │
//!                                                                      ▼
//!                                                              HirValidated
//!                                                             /     │
//!                                                     dump_hir()   mangle()
//!                                                                  │
//!                                                                  ▼
//!                                                             HirMangled
//!                                                                  │
//!                                                             lower_mir()
//!                                                                  │
//!                                                                  ▼
//!                                                             MirLowered
//!                                                                  │
//!                                                           codegen()
//!                                                                  │
//!                                                                  ▼
//!                                                          LlvmGenerated
//!                                                        /     |       \
//!                                            dump_llvm_ir()  dump_asm() emit_obj()
//! ```

use crate::hir_dump::Dump;
use crate::hir_validate::ValidateHirItem;
use crate::parse::Parser;
use nitrate_diagnosis::{CompilerLog, FileId, intern_file_id};
use nitrate_hir::prelude as hir;
use nitrate_hir_from_tree::{Ast2HirCtx, convert_ast_to_hir};
use nitrate_hir_mangle::mangle_symbols;
use nitrate_llvm::{LLVMContext, OptLevel};
use nitrate_llvm_from_mir::generate_llvmir_from_mir;
use nitrate_mir::prelude as mir;
use nitrate_mir_from_hir::lower_hir_to_mir;
use nitrate_token_lexer::{Lexer, LexerError};
use nitrate_tree::ast;
use nitrate_tree_resolve::ImportContext;
use std::num::NonZero;
use std::path::{Path, PathBuf};

// ────────────────────────────────────────────────────────────────────
// Pipeline configuration
// ────────────────────────────────────────────────────────────────────

pub struct PipelineConfig {
    pub package_name: String,
    pub target_triple: Option<String>,
    pub target_cpu: Option<String>,
    pub opt_level: OptLevel,
    pub hir_passes: Vec<Box<dyn crate::hir_optimize::HirOptimization + Send>>,
    pub hir_module_passes: Vec<Box<dyn crate::hir_optimize::HirModuleOptimization + Send>>,
    pub mir_passes: Vec<Box<dyn crate::mir_optimize::MirOptimization + Send>>,
    pub mir_module_passes: Vec<Box<dyn crate::mir_optimize::MirModuleOptimization + Send>>,
    pub no_default_passes: bool,
    pub thread_count: NonZero<usize>,
    pub log: CompilerLog,
}

impl Default for PipelineConfig {
    fn default() -> Self {
        Self {
            package_name: String::new(),
            target_triple: None,
            target_cpu: None,
            opt_level: OptLevel::None,
            hir_passes: Vec::new(),
            hir_module_passes: Vec::new(),
            mir_passes: Vec::new(),
            mir_module_passes: Vec::new(),
            no_default_passes: false,
            thread_count: NonZero::new(1).unwrap(),
            log: CompilerLog::default(),
        }
    }
}

// ────────────────────────────────────────────────────────────────────
// Pipeline — the top-level entry point
// ────────────────────────────────────────────────────────────────────

pub struct Pipeline {
    config: PipelineConfig,
}

impl Pipeline {
    pub fn new(config: PipelineConfig) -> Self {
        Self { config }
    }

    pub fn load_source(self, source_path: &Path) -> Result<Source, PipelineError> {
        let source_bytes = std::fs::read(source_path).map_err(|e| PipelineError::Io(source_path.to_path_buf(), e))?;

        let file_id = intern_file_id(source_path.to_string_lossy().as_ref()).ok_or(PipelineError::FileIdOverflow)?;

        Ok(Source {
            config: self.config,
            source_bytes,
            source_file_id: file_id,
            source_path: Some(source_path.to_path_buf()),
        })
    }
}

// ────────────────────────────────────────────────────────────────────
// Source stage
// ────────────────────────────────────────────────────────────────────

pub struct Source {
    pub(crate) config: PipelineConfig,
    pub(crate) source_bytes: Vec<u8>,
    pub(crate) source_file_id: FileId,
    pub(crate) source_path: Option<PathBuf>,
}

impl Source {
    pub fn lex(self) -> Result<Tokenized, PipelineError> {
        if self.config.log.error_bit() {
            return Err(PipelineError::CompilationFailed);
        }

        Ok(Tokenized {
            config: self.config,
            source_bytes: self.source_bytes,
            source_file_id: self.source_file_id,
            source_path: self.source_path,
        })
    }
}

// ────────────────────────────────────────────────────────────────────
// Tokenized stage
// ────────────────────────────────────────────────────────────────────

pub struct Tokenized {
    config: PipelineConfig,
    source_bytes: Vec<u8>,
    source_file_id: FileId,
    source_path: Option<PathBuf>,
}

impl Tokenized {
    pub fn parse(self) -> Result<Parsed, PipelineError> {
        let lexer = Lexer::new(&self.source_bytes, Some(self.source_file_id))
            .map_err(|LexerError::SourceTooBig| PipelineError::SourceTooLarge)?;

        let mut parser = Parser::new(lexer, &self.config.log);
        let module = parser.parse_source(self.config.package_name.clone().into());

        if self.config.log.error_bit() {
            return Err(PipelineError::CompilationFailed);
        }

        Ok(Parsed {
            config: self.config,
            module,
            source_path: self.source_path,
        })
    }
}

// ────────────────────────────────────────────────────────────────────
// Parsed stage
// ────────────────────────────────────────────────────────────────────

pub struct Parsed {
    config: PipelineConfig,
    module: ast::Module,
    source_path: Option<PathBuf>,
}

impl Parsed {
    pub fn dump_ast(self, pretty: bool) {
        if pretty {
            serde_json::to_writer_pretty(&mut std::io::stdout(), &self.module).expect("Failed to write AST to stdout");
        } else {
            serde_json::to_writer(&mut std::io::stdout(), &self.module).expect("Failed to write AST to stdout");
        }
    }

    /// Lower the AST to HIR.
    ///
    /// **Caller must ensure HIR Store TLS is active** via `using_storage`.
    pub fn lower_hir(self, ptr_size: u32) -> Result<HirLowered, PipelineError> {
        let ptr_size = match ptr_size {
            4 => hir::PtrSize::U32,
            8 => hir::PtrSize::U64,
            _ => return Err(PipelineError::UnsupportedPtrSize(ptr_size)),
        };

        let import_ctx = ImportContext::new(
            self.config.package_name.clone().into(),
            self.source_path
                .as_ref()
                .map(|p| p.to_string_lossy().to_string())
                .unwrap_or_default()
                .into(),
        );

        let mut ctx = Ast2HirCtx::new(ptr_size, import_ctx);
        let module = convert_ast_to_hir(self.module, &mut ctx, &self.config.log)
            .map_err(|_| PipelineError::CompilationFailed)?;

        Ok(HirLowered {
            config: self.config,
            module,
            symbol_tab: ctx.tab,
        })
    }
}

// ────────────────────────────────────────────────────────────────────
// HirLowered stage
// ────────────────────────────────────────────────────────────────────

pub struct HirLowered {
    config: PipelineConfig,
    module: hir::Module,
    symbol_tab: hir::SymbolTab,
}

impl HirLowered {
    pub fn dump_hir(self) {
        println!("{}", self.module.to_string());
    }

    /// Run HIR optimization passes. TLS must be active.
    pub fn optimize_hir(mut self) -> Result<HirOptimized, PipelineError> {
        // Clone the log to avoid simultaneous mutable/immutable borrows on self.config
        let log = self.config.log.clone();

        // Function-level passes
        if !self.config.no_default_passes {
            // Default passes would go here
        }
        for pass in self.config.hir_passes.iter_mut() {
            for item in &self.module.items {
                if let hir::Item::Function(func_id) = item {
                    let mut func = func_id.borrow_mut();
                    pass.optimize(&mut *func, &log);
                }
            }
        }

        // Module-level passes
        if !self.config.no_default_passes {
            // Default passes would go here
        }
        for pass in self.config.hir_module_passes.iter_mut() {
            pass.optimize(&mut self.module, &mut self.symbol_tab, &log);
        }

        if log.error_bit() {
            return Err(PipelineError::CompilationFailed);
        }

        Ok(HirOptimized {
            config: self.config,
            module: self.module,
            symbol_tab: self.symbol_tab,
        })
    }

    /// Validate HIR directly (skip optimization). TLS must be active.
    pub fn validate(self) -> Result<HirValidated, PipelineError> {
        // Clone the log so the ValidateCtx borrow doesn't conflict with later access
        let log = self.config.log.clone();
        let mut hir_verifier = crate::hir_validate::ValidateCtx::new(&self.symbol_tab, &log);
        let valid_module = self
            .module
            .clone()
            .validate(&mut hir_verifier)
            .map_err(|_| PipelineError::HirValidationFailed)?;

        if log.error_bit() {
            return Err(PipelineError::CompilationFailed);
        }

        Ok(HirValidated {
            config: self.config,
            module: valid_module,
            symbol_tab: self.symbol_tab,
        })
    }
}

// ────────────────────────────────────────────────────────────────────
// HirOptimized stage
// ────────────────────────────────────────────────────────────────────

pub struct HirOptimized {
    config: PipelineConfig,
    module: hir::Module,
    symbol_tab: hir::SymbolTab,
}

impl HirOptimized {
    /// Validate the optimized HIR. TLS must be active.
    pub fn validate(self) -> Result<HirValidated, PipelineError> {
        let mut hir_verifier = crate::hir_validate::ValidateCtx::new(&self.symbol_tab, &self.config.log);
        let valid_module = self
            .module
            .clone()
            .validate(&mut hir_verifier)
            .map_err(|_| PipelineError::HirValidationFailed)?;

        if self.config.log.error_bit() {
            return Err(PipelineError::CompilationFailed);
        }

        Ok(HirValidated {
            config: self.config,
            module: valid_module,
            symbol_tab: self.symbol_tab,
        })
    }
}

// ────────────────────────────────────────────────────────────────────
// HirValidated stage
// ────────────────────────────────────────────────────────────────────

pub struct HirValidated {
    config: PipelineConfig,
    module: crate::hir_validate::ValidHir<hir::Module>,
    symbol_tab: hir::SymbolTab,
}

impl HirValidated {
    pub fn dump_hir(self) {
        println!("{}", self.module.into_inner().to_string());
    }

    pub fn finish_check(self) {
        // Consumes the pipeline — check complete
    }

    /// Apply name mangling. TLS must be active.
    pub fn mangle(mut self) -> HirMangled {
        mangle_symbols(&self.config.package_name, &mut self.symbol_tab);

        HirMangled {
            config: self.config,
            module: self.module,
            symbol_tab: self.symbol_tab,
        }
    }
}

// ────────────────────────────────────────────────────────────────────
// HirMangled stage
// ────────────────────────────────────────────────────────────────────

pub struct HirMangled {
    config: PipelineConfig,
    module: crate::hir_validate::ValidHir<hir::Module>,
    symbol_tab: hir::SymbolTab,
}

impl HirMangled {
    /// Lower HIR to MIR. TLS must be active.
    pub fn lower_mir(self) -> MirLowered {
        let hir_module = self.module.into_inner();
        let mir_module = lower_hir_to_mir(&hir_module, &self.symbol_tab);

        MirLowered {
            config: self.config,
            mir_module,
        }
    }
}

// ────────────────────────────────────────────────────────────────────
// MirLowered stage
// ────────────────────────────────────────────────────────────────────

pub struct MirLowered {
    config: PipelineConfig,
    mir_module: mir::MirModule,
}

impl MirLowered {
    /// Run MIR optimization passes.
    pub fn optimize_mir(mut self) -> Result<MirOptimized, PipelineError> {
        if !self.config.no_default_passes {
            // Default passes
        }
        for pass in self.config.mir_passes.iter_mut() {
            for func_id in self.mir_module.functions.iter() {
                let mut func = func_id.borrow_mut();
                pass.optimize(&mut *func, &self.config.log);
            }
        }

        if !self.config.no_default_passes {
            // Default module passes
        }
        for pass in self.config.mir_module_passes.iter_mut() {
            pass.optimize(&mut self.mir_module, &self.config.log);
        }

        if self.config.log.error_bit() {
            return Err(PipelineError::CompilationFailed);
        }

        Ok(MirOptimized {
            config: self.config,
            mir_module: self.mir_module,
        })
    }

    /// Write the MIR control-flow graph as a Graphviz DOT file.
    pub fn dump_mir_dot(&self, output_path: &Path) -> Result<(), std::io::Error> {
        let dot = self.mir_module.emit_dot();
        std::fs::write(output_path, dot)
    }

    /// Codegen (skip MIR optimization).
    pub fn codegen(self) -> Result<LlvmGenerated, PipelineError> {
        let llvm_ctx = create_llvm_context(self.config.target_triple.as_deref(), self.config.opt_level)?;

        Ok(LlvmGenerated {
            config: self.config,
            mir_module: self.mir_module,
            llvm_ctx,
        })
    }
}

// ────────────────────────────────────────────────────────────────────
// MirOptimized stage
// ────────────────────────────────────────────────────────────────────

pub struct MirOptimized {
    config: PipelineConfig,
    mir_module: mir::MirModule,
}

impl MirOptimized {
    pub fn codegen(self) -> Result<LlvmGenerated, PipelineError> {
        let llvm_ctx = create_llvm_context(self.config.target_triple.as_deref(), self.config.opt_level)?;

        Ok(LlvmGenerated {
            config: self.config,
            mir_module: self.mir_module,
            llvm_ctx,
        })
    }
}

// ────────────────────────────────────────────────────────────────────
// LlvmGenerated stage
// ────────────────────────────────────────────────────────────────────

pub struct LlvmGenerated {
    config: PipelineConfig,
    mir_module: mir::MirModule,
    llvm_ctx: LLVMContext,
}

impl LlvmGenerated {
    pub fn optimize_llvm(self) -> LlvmOptimized {
        LlvmOptimized {
            config: self.config,
            mir_module: self.mir_module,
            llvm_ctx: self.llvm_ctx,
        }
    }

    pub fn dump_llvm_ir(self) {
        let llvm_module = generate_llvmir_from_mir(&self.config.package_name, &self.mir_module, &self.llvm_ctx);
        println!("{}", llvm_module.print_to_string().to_string());
    }

    pub fn dump_asm(self) -> Result<(), PipelineError> {
        let mut llvm_module = generate_llvmir_from_mir(&self.config.package_name, &self.mir_module, &self.llvm_ctx);
        self.llvm_ctx
            .write_asm(&mut llvm_module, &mut std::io::stdout())
            .map_err(|e| PipelineError::LlvmEmitError(e.to_string()))
    }

    pub fn emit_obj(self, output_path: &Path) -> Result<Emitted, PipelineError> {
        let mut llvm_module = generate_llvmir_from_mir(&self.config.package_name, &self.mir_module, &self.llvm_ctx);
        self.llvm_ctx
            .write_object_file(&mut llvm_module, output_path)
            .map_err(|e| PipelineError::LlvmEmitError(e.to_string()))?;

        Ok(Emitted {
            output_path: output_path.to_path_buf(),
        })
    }
}

// ────────────────────────────────────────────────────────────────────
// LlvmOptimized stage
// ────────────────────────────────────────────────────────────────────

pub struct LlvmOptimized {
    config: PipelineConfig,
    mir_module: mir::MirModule,
    llvm_ctx: LLVMContext,
}

impl LlvmOptimized {
    pub fn dump_llvm_ir(self) {
        let mut llvm_module = generate_llvmir_from_mir(&self.config.package_name, &self.mir_module, &self.llvm_ctx);
        self.llvm_ctx.optimize_module(&mut llvm_module);
        println!("{}", llvm_module.print_to_string().to_string());
    }

    pub fn dump_asm(self) -> Result<(), PipelineError> {
        let mut llvm_module = generate_llvmir_from_mir(&self.config.package_name, &self.mir_module, &self.llvm_ctx);
        self.llvm_ctx.optimize_module(&mut llvm_module);
        self.llvm_ctx
            .write_asm(&mut llvm_module, &mut std::io::stdout())
            .map_err(|e| PipelineError::LlvmEmitError(e.to_string()))
    }

    pub fn emit_obj(self, output_path: &Path) -> Result<Emitted, PipelineError> {
        let mut llvm_module = generate_llvmir_from_mir(&self.config.package_name, &self.mir_module, &self.llvm_ctx);
        self.llvm_ctx.optimize_module(&mut llvm_module);
        self.llvm_ctx
            .write_object_file(&mut llvm_module, output_path)
            .map_err(|e| PipelineError::LlvmEmitError(e.to_string()))?;

        Ok(Emitted {
            output_path: output_path.to_path_buf(),
        })
    }
}

// ────────────────────────────────────────────────────────────────────
// Emitted stage
// ────────────────────────────────────────────────────────────────────

pub struct Emitted {
    output_path: PathBuf,
}

impl Emitted {
    pub fn path(&self) -> &Path {
        &self.output_path
    }
}

// ────────────────────────────────────────────────────────────────────
// Error type
// ────────────────────────────────────────────────────────────────────

#[derive(Debug, thiserror::Error)]
pub enum PipelineError {
    #[error("I/O error accessing '{0}': {1}")]
    Io(PathBuf, std::io::Error),

    #[error("source file is too large to be processed")]
    SourceTooLarge,

    #[error("FileId overflow — too many source files")]
    FileIdOverflow,

    #[error("compilation failed (see diagnostics for details)")]
    CompilationFailed,

    #[error("unsupported pointer size: {0} bytes")]
    UnsupportedPtrSize(u32),

    #[error("HIR validation failed")]
    HirValidationFailed,

    #[error("LLVM context creation failed: {0}")]
    LlvmContextError(String),

    #[error("LLVM emission failed: {0}")]
    LlvmEmitError(String),
}

// ────────────────────────────────────────────────────────────────────
// Helpers
// ────────────────────────────────────────────────────────────────────

fn create_llvm_context(target_triple: Option<&str>, opt_level: OptLevel) -> Result<LLVMContext, PipelineError> {
    let triple = match target_triple {
        Some(t) => t.to_string(),
        None => LLVMContext::default_target_triple(),
    };

    LLVMContext::new(&triple, opt_level).map_err(|e| PipelineError::LlvmContextError(e.to_string()))
}

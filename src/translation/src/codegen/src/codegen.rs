use cranelift::module::{FuncId, Linkage, Module};
use log::{debug, trace};
use nitrate_parsetree::kind;
use std::sync::Arc;
use std::{collections::HashMap, str::FromStr};

use cranelift::{
    object::{ObjectBuilder, ObjectModule},
    prelude::*,
};

use target_lexicon::Triple;

#[derive(Debug)]
pub enum CodegenError {
    IoError(std::io::Error),
    TargetTripleParseError(target_lexicon::ParseError),
    UnsupportedTargetTriple(Triple),
    UnsupportedISAConfiguration(String),
    UnsupportedISAConfigurationFlag((String, String)),
    ModuleCreationError(cranelift::module::ModuleError),
    ObjectFinalizationError(cranelift::object::object::write::Error),
    Other(String),
}

#[derive(Default)]
pub struct Codegen {
    target_triple_string: String,
    isa_config: HashMap<String, String>,
}

impl Codegen {
    pub fn new(target_triple_string: String, isa_config: HashMap<String, String>) -> Self {
        Self {
            target_triple_string,
            isa_config,
        }
    }

    fn create_shared_flags() -> cranelift::codegen::settings::Flags {
        let builder = cranelift::codegen::settings::builder();
        let flags = cranelift::codegen::settings::Flags::new(builder);

        debug!(
            "Shared flags: {:?}",
            flags
                .iter()
                .map(|flag| (flag.name, flag.value_string()))
                .collect::<Vec<_>>()
        );

        flags
    }

    fn create_target_triple(triple_str: &str) -> Result<Triple, CodegenError> {
        trace!("Creating target triple from string: {}", triple_str);

        let triple = Triple::from_str(triple_str).map_err(CodegenError::TargetTripleParseError)?;
        debug!("Compiling for target: {:?}", triple);

        Ok(triple)
    }

    fn create_isa(
        shared_flags: cranelift::codegen::settings::Flags,
        target_triple: Triple,
        isa_config: &HashMap<String, String>,
    ) -> Result<Arc<dyn cranelift::codegen::isa::TargetIsa>, CodegenError> {
        let mut isa_builder = match cranelift::codegen::isa::lookup(target_triple.clone()) {
            Ok(isa) => Ok(isa),
            Err(_) => Err(CodegenError::UnsupportedTargetTriple(target_triple)),
        }?;

        for (key, value) in isa_config {
            if isa_builder.set(key, value).is_err() {
                return Err(CodegenError::UnsupportedISAConfigurationFlag((
                    key.to_owned(),
                    value.to_owned(),
                )));
            };

            debug!("ISA config flag set: {} = {}", key, value);
        }

        match isa_builder.finish(shared_flags) {
            Ok(isa) => Ok(isa),
            Err(e) => Err(CodegenError::UnsupportedISAConfiguration(e.to_string())),
        }
    }

    fn create_module(
        isa: Arc<dyn isa::TargetIsa>,
        module_name: &str,
    ) -> Result<ObjectModule, CodegenError> {
        let libcall_names = cranelift::module::default_libcall_names();

        let mut builder = ObjectBuilder::new(isa, module_name, libcall_names)
            .map_err(|e| CodegenError::ModuleCreationError(e))?;

        // Put each function and data object in its own section.
        // This supposedly helps the linker to reduce code and data bloat by
        // garbage collecting unused sections.
        builder
            .per_data_object_section(true)
            .per_function_section(true);

        debug!("Created module with name: {}", module_name);

        Ok(ObjectModule::new(builder))
    }

    fn create_global_variable(
        variable: &kind::StaticVariable,
        module: &mut ObjectModule,
    ) -> Result<(), CodegenError> {
        let name = &variable.name;

        // TODO: Determine the correct linkage, and TLS status
        let linkage = Linkage::Local;
        let is_writable = true;
        let is_tls = false;

        let data_id = module
            .declare_data(name, linkage, is_writable, is_tls)
            .map_err(CodegenError::ModuleCreationError)?;

        // TODO: Determine the correct size
        let size = match None {
            Some(size) => size,

            None => {
                return Err(CodegenError::Other(
                    "Global variable type does not have a known size".to_string(),
                ));
            }
        };

        let mut description = cranelift::module::DataDescription::new();
        description.set_align(1);
        description.define_zeroinit(size);

        module
            .define_data(data_id, &description)
            .map_err(CodegenError::ModuleCreationError)
    }

    fn create_global_function(
        _function: &kind::NamedFunction,
        _module: &mut ObjectModule,
    ) -> Result<FuncId, CodegenError> {
        // TODO: Generate code for the function

        // let name = function.name();

        Err(CodegenError::Other(
            "Global function generation not yet implemented".to_string(),
        ))
    }

    pub fn generate(
        self,
        package: &kind::Package,
        output: &mut dyn std::io::Write,
    ) -> Result<(), CodegenError> {
        let shared_flags = Self::create_shared_flags();
        let target_triple = Self::create_target_triple(&self.target_triple_string)?;
        let isa = Self::create_isa(shared_flags, target_triple, &self.isa_config)?;

        let mut obj_module = Self::create_module(isa, &package.name)?;

        for expression in &package.root.items {
            match expression {
                kind::Item::NamedFunction(function) => {
                    Self::create_global_function(function, &mut obj_module)?;
                }

                kind::Item::StaticVariable(global_variable) => {
                    Self::create_global_variable(global_variable, &mut obj_module)?;
                }

                _ => {
                    return Err(CodegenError::Other(
                        "Top-level expression is not a function or global variable".to_string(),
                    ));
                }
            }
        }

        match obj_module.finish().emit() {
            Ok(object_file_bytes) => output
                .write_all(&object_file_bytes)
                .map_err(CodegenError::IoError),

            Err(e) => Err(CodegenError::ObjectFinalizationError(e)),
        }
    }
}

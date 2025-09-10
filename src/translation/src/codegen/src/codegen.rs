use std::str::FromStr;
use std::sync::Arc;

use cranelift::{
    module::Module,
    object::{ObjectBuilder, ObjectModule},
    prelude::isa::TargetIsa,
};

use nitrate_structure::SourceModel;
use target_lexicon::Triple;

#[derive(Debug)]
pub enum CodegenError {
    IoError(std::io::Error),
    TargetTripleParseError(target_lexicon::ParseError),
    UnsupportedTargetTriple(Triple),
    UnsupportedISAConfiguration(String),
    ModuleCreationError(cranelift::module::ModuleError),
    ObjectFinalizationError(cranelift::object::object::write::Error),
    Other(String),
}

#[derive(Default)]
pub struct Codegen {}

impl Codegen {
    fn create_cpu_feature_configuration() -> cranelift::codegen::settings::Flags {
        let builder = cranelift::codegen::settings::builder();
        let flags = cranelift::codegen::settings::Flags::new(builder);

        flags
    }

    fn create_target_triple(triple_str: &str) -> Result<Triple, CodegenError> {
        Triple::from_str(triple_str).map_err(CodegenError::TargetTripleParseError)
    }

    fn create_isa(
        target_triple: Triple,
        cpu_flags: cranelift::codegen::settings::Flags,
    ) -> Result<Arc<dyn cranelift::codegen::isa::TargetIsa>, CodegenError> {
        let isa_builder = match cranelift::codegen::isa::lookup(target_triple.clone()) {
            Ok(isa) => Ok(isa),
            Err(_) => Err(CodegenError::UnsupportedTargetTriple(target_triple)),
        }?;

        match isa_builder.finish(cpu_flags) {
            Ok(isa) => Ok(isa),
            Err(e) => Err(CodegenError::UnsupportedISAConfiguration(e.to_string())),
        }
    }

    fn create_module(
        isa: Arc<dyn TargetIsa>,
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

        Ok(ObjectModule::new(builder))
    }

    pub fn generate<'a>(
        self,
        _model: &SourceModel<'a>,
        output: &mut dyn std::io::Write,
    ) -> Result<(), CodegenError> {
        let target_triple_string = "x86_64"; // Example target ISA
        let module_name = "example_module";

        let cpu_flags = Self::create_cpu_feature_configuration();
        let target_triple = Self::create_target_triple(target_triple_string)?;
        let isa = Self::create_isa(target_triple, cpu_flags)?;

        let module = Self::create_module(isa, module_name)?;

        // TODO: Implement code generation logic

        match module.finish().emit() {
            Ok(object_file_bytes) => output
                .write_all(&object_file_bytes)
                .map_err(CodegenError::IoError),

            Err(e) => Err(CodegenError::ObjectFinalizationError(e)),
        }
    }
}

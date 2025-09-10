use nitrate_structure::SourceModel;

#[derive(Debug)]
pub enum CodegenError {
    IoError(std::io::Error),
    Other(String),
}

#[derive(Debug, Default)]
pub struct Codegen {}

impl Codegen {
    pub fn generate<'a>(
        self,
        _model: &SourceModel<'a>,
        object: &mut dyn std::io::Write,
    ) -> Result<(), CodegenError> {
        // TODO: Implement code generation logic

        writeln!(object, "Generating code...").map_err(CodegenError::IoError)
    }
}

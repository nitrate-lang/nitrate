#[derive(Debug)]
pub struct DiagnosticDrain {
    _logger: slog::Logger,
}

impl DiagnosticDrain {
    pub fn new(logger: slog::Logger) -> Self {
        Self { _logger: logger }
    }

    pub fn emit(&self, _msg: &str) {
        // TODO: Implement actual emission logic
    }
}

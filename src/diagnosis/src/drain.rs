#[derive(Debug)]
pub struct DiagnosticDrain {
    _logger: slog::Logger,
}

impl Default for DiagnosticDrain {
    fn default() -> Self {
        Self {
            _logger: slog::Logger::root(slog::Discard, slog::o!()),
        }
    }
}

impl DiagnosticDrain {
    pub fn new(logger: slog::Logger) -> Self {
        Self { _logger: logger }
    }

    pub fn emit(&self, _msg: &str) {
        // TODO: Implement actual emission logic
    }
}

#[derive(Debug)]
pub struct DiagnosticDrain {
    _logger: slog::Logger,
    any_errors: bool,
}

impl Default for DiagnosticDrain {
    fn default() -> Self {
        Self {
            _logger: slog::Logger::root(slog::Discard, slog::o!()),
            any_errors: false,
        }
    }
}

impl DiagnosticDrain {
    pub fn new(logger: slog::Logger) -> Self {
        Self {
            _logger: logger,
            any_errors: false,
        }
    }

    pub fn emit(&self, _msg: &str) {
        // TODO: Implement actual emission logic
    }

    pub fn any_errors(&self) -> bool {
        self.any_errors
    }
}

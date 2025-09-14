use std::{
    collections::HashMap,
    sync::atomic::{AtomicBool, Ordering},
};

use slog::{error, info, warn};

use crate::{DiagnosticId, FormattableDiagnosticGroup, Origin};

#[derive(Debug, Clone, Copy, PartialEq, Eq)]
pub enum Severity {
    Discard,
    Info,
    Warning,
    Error,
}

#[derive(Debug)]
pub struct DiagnosticCollector {
    log: slog::Logger,
    code_map: HashMap<DiagnosticId, Severity>,
    info_bit: AtomicBool,
    warning_bit: AtomicBool,
    error_bit: AtomicBool,
}

impl Default for DiagnosticCollector {
    fn default() -> Self {
        Self::new(slog::Logger::root(slog::Discard, slog::o!()))
    }
}

impl DiagnosticCollector {
    pub fn new(log: slog::Logger) -> Self {
        Self {
            log,
            code_map: HashMap::new(),
            info_bit: AtomicBool::new(false),
            warning_bit: AtomicBool::new(false),
            error_bit: AtomicBool::new(false),
        }
    }

    pub fn set_severity(&mut self, id: DiagnosticId, severity: Severity) {
        self.code_map.insert(id, severity);
    }

    fn get_severity(&self, id: &DiagnosticId) -> Severity {
        if let Some(sev) = self.code_map.get(id) {
            return *sev;
        }

        Severity::Error
    }

    fn emit_info(&self, message: &str, id: &DiagnosticId, origin: &Origin) {
        self.info_bit.store(true, Ordering::Release);

        match origin {
            Origin::None => {
                info!(self.log, "info[E{:04X}]: {}", id.0, message);
            }

            Origin::Unknown => {
                info!(self.log, "info[E{:04X}]: {}\n--> ???", id.0, message);
            }

            Origin::Point(pos) => {
                info!(self.log, "info[E{:04X}]: {}\n--> {}", id.0, message, pos);
            }

            Origin::Span(span) => {
                // TODO: Print the span nicely

                info!(
                    self.log,
                    "info[E{:04X}]: {}\n--> {}\n--> {}", id.0, message, span.start, span.end
                );
            }
        }
    }

    fn emit_warning(&self, message: &str, id: &DiagnosticId, origin: &Origin) {
        self.warning_bit.store(true, Ordering::Release);

        match origin {
            Origin::None => {
                warn!(self.log, "warning[E{:04X}]: {}", id.0, message);
            }

            Origin::Unknown => {
                warn!(self.log, "warning[E{:04X}]: {}\n--> ???", id.0, message);
            }

            Origin::Point(pos) => {
                warn!(self.log, "warning[E{:04X}]: {}\n--> {}", id.0, message, pos);
            }

            Origin::Span(span) => {
                // TODO: Print the span nicely

                warn!(
                    self.log,
                    "warning[E{:04X}]: {}\n--> {}\n--> {}", id.0, message, span.start, span.end
                );
            }
        }
    }

    fn emit_error(&self, message: &str, id: &DiagnosticId, origin: &Origin) {
        self.error_bit.store(true, Ordering::Release);

        match origin {
            Origin::None => {
                error!(self.log, "error[E{:04X}]: {}", id.0, message);
            }

            Origin::Unknown => {
                error!(self.log, "error[E{:04X}]: {}\n--> ???", id.0, message);
            }

            Origin::Point(pos) => {
                error!(self.log, "error[E{:04X}]: {}\n--> {}", id.0, message, pos);
            }

            Origin::Span(span) => {
                // TODO: Print the span nicely

                error!(
                    self.log,
                    "error[E{:04X}]: {}\n--> {}\n--> {}", id.0, message, span.start, span.end
                );
            }
        }
    }

    pub fn emit(&self, diag: &dyn FormattableDiagnosticGroup) {
        let id = match DiagnosticId::new(diag.group_id(), diag.variant_id()) {
            Some(id) => id,
            None => DiagnosticId::UNKNOWN,
        };

        match self.get_severity(&id) {
            Severity::Discard => {}

            Severity::Info => {
                let fmt = diag.format();
                self.emit_info(&fmt.message, &id, &fmt.origin);
            }

            Severity::Warning => {
                let fmt = diag.format();
                self.emit_warning(&fmt.message, &id, &fmt.origin);
            }

            Severity::Error => {
                let fmt = diag.format();
                self.emit_error(&fmt.message, &id, &fmt.origin);
            }
        }
    }

    pub fn info_bit(&self) -> bool {
        self.info_bit.load(Ordering::Acquire)
    }

    pub fn clear_info_bit(&mut self) {
        self.info_bit.store(false, Ordering::Release);
    }

    pub fn warning_bit(&self) -> bool {
        self.warning_bit.load(Ordering::Acquire)
    }

    pub fn clear_warnings_bit(&mut self) {
        self.warning_bit.store(false, Ordering::Release);
    }

    pub fn error_bit(&self) -> bool {
        self.error_bit.load(Ordering::Acquire)
    }

    pub fn clear_errors_bit(&mut self) {
        self.error_bit.store(false, Ordering::Release);
    }
}

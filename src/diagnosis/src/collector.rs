use crate::{DiagnosticId, FormattableDiagnosticGroup, Origin};
use slog::{Drain, error, info, warn};
use std::cell::RefCell;
use std::collections::HashMap;
use std::rc::Rc;
use std::sync::atomic::{AtomicBool, Ordering};

#[derive(Debug, Clone, Copy, PartialEq, Eq)]
pub enum Severity {
    Discard,
    Info,
    Warning,
    Error,
}

#[derive(Debug, Clone)]
pub struct CompilerLog {
    log: slog::Logger,
    code_map: Rc<RefCell<HashMap<DiagnosticId, Severity>>>,
    info_bit: Rc<AtomicBool>,
    warning_bit: Rc<AtomicBool>,
    error_bit: Rc<AtomicBool>,
}

impl Default for CompilerLog {
    fn default() -> Self {
        Self::new(slog::Logger::root(slog::Discard, slog::o!()))
    }
}

impl CompilerLog {
    pub fn new(log: slog::Logger) -> Self {
        Self {
            log,
            code_map: Rc::new(RefCell::new(HashMap::new())),
            info_bit: Rc::new(AtomicBool::new(false)),
            warning_bit: Rc::new(AtomicBool::new(false)),
            error_bit: Rc::new(AtomicBool::new(false)),
        }
    }

    pub fn default_stdout() -> Self {
        let decorator = slog_term::TermDecorator::new().stdout().build();
        let drain = slog_term::FullFormat::new(decorator).build().fuse();
        let drain = slog_async::Async::new(drain).build().fuse();
        let slog_stdout = slog::Logger::root(drain, slog::o!());
        Self::new(slog_stdout)
    }

    pub fn default_stderr() -> Self {
        let decorator = slog_term::TermDecorator::new().stderr().build();
        let drain = slog_term::FullFormat::new(decorator).build().fuse();
        let drain = slog_async::Async::new(drain).build().fuse();
        let slog_stderr = slog::Logger::root(drain, slog::o!());
        Self::new(slog_stderr)
    }

    pub fn set_severity(&mut self, id: DiagnosticId, severity: Severity) {
        self.code_map.borrow_mut().insert(id, severity);
    }

    fn get_severity(&self, id: &DiagnosticId) -> Severity {
        if let Some(sev) = self.code_map.borrow().get(id) {
            return *sev;
        }

        Severity::Error
    }

    fn emit_info(&self, message: &str, id: &DiagnosticId, origin: &Origin, group: &str) {
        self.info_bit.store(true, Ordering::Release);

        match origin {
            Origin::None => {
                info!(self.log, "info[E{:04X}]: {}: {}", id.0, group, message);
            }

            Origin::Point(pos) => {
                info!(self.log, "info[E{:04X}]: {}: {}\n--> {}", id.0, group, message, pos);
            }

            Origin::Span(span) => {
                info!(
                    self.log,
                    "info[E{:04X}]: {}: {}\n--> {}\n--> {}", id.0, group, message, span.start, span.end
                );
            }
        }
    }

    fn emit_warning(&self, message: &str, id: &DiagnosticId, origin: &Origin, group: &str) {
        self.warning_bit.store(true, Ordering::Release);

        match origin {
            Origin::None => {
                warn!(self.log, "warning[E{:04X}]: {}: {}", id.0, group, message);
            }

            Origin::Point(pos) => {
                warn!(self.log, "warning[E{:04X}]: {}: {}\n--> {}", id.0, group, message, pos);
            }

            Origin::Span(span) => {
                warn!(
                    self.log,
                    "warning[E{:04X}]: {}: {}\n--> {}\n--> {}", id.0, group, message, span.start, span.end
                );
            }
        }
    }

    fn emit_error(&self, message: &str, id: &DiagnosticId, origin: &Origin, group: &str) {
        self.error_bit.store(true, Ordering::Release);

        match origin {
            Origin::None => {
                error!(self.log, "error[E{:04X}]: {}: {}", id.0, group, message);
            }

            Origin::Point(pos) => {
                error!(self.log, "error[E{:04X}]: {}: {}\n--> {}", id.0, group, message, pos);
            }

            Origin::Span(span) => {
                error!(
                    self.log,
                    "error[E{:04X}]: {}: {}\n--> {}\n--> {}", id.0, group, message, span.start, span.end
                );
            }
        }
    }

    pub fn report(&self, diag: &dyn FormattableDiagnosticGroup) {
        let gid = diag.group_id();
        let id = match DiagnosticId::new(gid, diag.variant_id()) {
            Some(id) => id,
            None => DiagnosticId::UNKNOWN,
        };

        match self.get_severity(&id) {
            Severity::Discard => {}

            Severity::Info => {
                let fmt = diag.format();
                self.emit_info(&fmt.message, &id, &fmt.origin, &gid.to_string());
            }

            Severity::Warning => {
                let fmt = diag.format();
                self.emit_warning(&fmt.message, &id, &fmt.origin, &gid.to_string());
            }

            Severity::Error => {
                let fmt = diag.format();
                self.emit_error(&fmt.message, &id, &fmt.origin, &gid.to_string());
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

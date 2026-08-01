use crate::Interpreter;
use crate::commands::build::{CompileOptions, resolve_manifest, target_dir_for};
use clap::Parser;
use nitrate_diagnosis::CompilerLog;
use slog::{info, warn};
use std::path::PathBuf;

#[derive(Parser, Debug)]
#[command(about, long_about = None)]
pub(crate) struct DocArgs {
    /// Opens the docs in a browser after the operation
    #[arg(long)]
    pub(crate) open: bool,

    /// Don't build documentation for dependencies
    #[arg(long)]
    pub(crate) no_deps: bool,

    /// Document private items
    #[arg(long)]
    pub(crate) document_private_items: bool,

    /// Build artifacts in release mode, with optimizations
    #[arg(long, short = 'r')]
    pub(crate) release: bool,

    /// Build artifacts with the specified profile
    #[arg(long, value_name = "PROFILE-NAME")]
    pub(crate) profile: Option<String>,

    /// Build for the target triple
    #[arg(long, value_name = "TRIPLE")]
    pub(crate) target: Option<String>,

    /// Directory for all generated artifacts
    #[arg(long, value_name = "DIRECTORY")]
    pub(crate) target_dir: Option<PathBuf>,

    /// Path to Cargo.toml
    #[arg(long, value_name = "PATH")]
    pub(crate) manifest_path: Option<PathBuf>,

    /// Package to document
    #[arg(long, short = 'p', value_name = "SPEC")]
    pub(crate) package: Option<String>,

    /// Document all packages in the workspace
    #[arg(long)]
    pub(crate) workspace: bool,

    /// Space or comma separated list of features to activate
    #[arg(long, short = 'F', value_name = "FEATURES")]
    pub(crate) features: Vec<String>,

    /// Activate all available features
    #[arg(long)]
    pub(crate) all_features: bool,

    /// Do not activate the `default` feature
    #[arg(long)]
    pub(crate) no_default_features: bool,

    /// Number of parallel jobs, defaults to # of CPUs
    #[arg(long, short = 'j', value_name = "N")]
    pub(crate) jobs: Option<usize>,
}

fn html_escape(input: &str) -> String {
    input
        .replace('&', "\u{26}amp;")
        .replace('<', "\u{26}lt;")
        .replace('>', "\u{26}gt;")
        .replace('"', "\u{26}quot;")
}

impl Interpreter<'_> {
    fn gen_doc_page(&self, manifest: &crate::package::Manifest) -> Vec<(String, String)> {
        // For each .nit file in src/, build an HTML page listing public items.
        let src_dir = manifest.manifest_dir.join("src");
        let mut pages = Vec::new();
        let log = CompilerLog::new(self.log.clone());

        let entries = match std::fs::read_dir(&src_dir) {
            Ok(entries) => entries,
            Err(_) => return pages,
        };

        for entry in entries.flatten() {
            let path = entry.path();
            if path.extension().and_then(|e| e.to_str()) != Some("nit") {
                continue;
            }
            let file_name = path
                .file_name()
                .and_then(|f| f.to_str())
                .unwrap_or("entry.nit")
                .to_string();

            let source = match std::fs::read_to_string(&path) {
                Ok(s) => s,
                Err(_) => continue,
            };

            let source_bytes = source.as_bytes();
            let source_code_file =
                nitrate_diagnosis::intern_file_id(path.to_string_lossy().as_ref()).expect("FileId overflow");
            let lexer = match nitrate_translation::token_lexer::Lexer::new(source_bytes, Some(source_code_file)) {
                Ok(lexer) => lexer,
                Err(_) => continue,
            };
            let mut parser = nitrate_translation::parse::Parser::new(lexer, &log);
            let ast = parser.parse_source(manifest.package.name.clone().into());

            let mut items_html = String::new();
            for item in &ast.items {
                let (tag, name) = match item {
                    nitrate_translation::parsetree::ast::Item::Function(f) => ("fn", f.name.to_string()),
                    nitrate_translation::parsetree::ast::Item::Struct(s) => ("struct", s.name.to_string()),
                    nitrate_translation::parsetree::ast::Item::Enum(e) => ("enum", e.name.to_string()),
                    nitrate_translation::parsetree::ast::Item::Trait(t) => ("trait", t.name.to_string()),
                    nitrate_translation::parsetree::ast::Item::TypeAlias(t) => ("type", t.name.to_string()),
                    nitrate_translation::parsetree::ast::Item::Variable(v) => ("static", v.name.to_string()),
                    _ => continue,
                };
                items_html.push_str(&format!(
                    "<div class=\"item\"><span class=\"item-kind\">{}</span> <code>{}</code></div>\n",
                    tag,
                    html_escape(&name)
                ));
            }

            let page = format!(
                "<!DOCTYPE html>\n<html lang=\"en\">\n<head>\n<meta charset=\"utf-8\">\n\
                 <title>{pkg} - {file}</title>\n\
                 <style>body{{font-family:sans-serif;margin:2em;}} .item{{padding:.25em 0;}} \
                 .item-kind{{color:#888;font-weight:bold;}}</style>\n</head>\n<body>\n\
                 <h1>{pkg}</h1>\n<h2>{file}</h2>\n{items}\n</body>\n</html>\n",
                pkg = html_escape(&manifest.package.name),
                file = html_escape(&file_name),
                items = items_html
            );

            pages.push((file_name, page));
        }

        pages
    }

    pub(crate) fn sc_doc(&mut self, args: DocArgs) -> anyhow::Result<()> {
        let manifest = resolve_manifest(args.manifest_path.as_deref()).map_err(|e| anyhow::anyhow!("{e}"))?;
        let target_dir = target_dir_for(&manifest, args.target_dir.as_deref());
        let profile = match &args.profile {
            Some(p) => p.clone(),
            None if args.release => "release".to_string(),
            None => "debug".to_string(),
        };
        let doc_dir = target_dir.join(&profile).join("doc");

        std::fs::create_dir_all(&doc_dir).map_err(|e| anyhow::anyhow!("Failed to create doc dir: {e}"))?;

        let mut opts = CompileOptions::default();
        opts.release = args.release;
        opts.profile = args.profile;
        opts.target = args.target;
        opts.target_dir = args.target_dir;
        opts.manifest_path = args.manifest_path;
        opts.check_only = true;
        self.compile_package(&opts)?;

        let pages = self.gen_doc_page(&manifest);

        if pages.is_empty() {
            warn!(self.log, "no source files found to document");
        }

        for (file_name, html) in &pages {
            let out_path = doc_dir.join(file_name.replace(".nit", ".html"));
            std::fs::write(&out_path, html).map_err(|e| anyhow::anyhow!("Failed to write doc: {e}"))?;
            info!(self.log, "Documented {} -> {}", file_name, out_path.display());
        }

        let index_path = doc_dir.join("index.html");
        let index = format!(
            "<!DOCTYPE html>\n<html lang=\"en\">\n<head>\n<meta charset=\"utf-8\">\n\
             <title>{pkg} docs</title>\n</head>\n<body>\n<h1>{pkg}</h1>\n<ul>\n",
            pkg = html_escape(&manifest.package.name)
        );
        let mut index_body = index;
        for (file_name, _) in &pages {
            index_body.push_str(&format!(
                "<li><a href=\"{}\">{}</a></li>\n",
                file_name.replace(".nit", ".html"),
                html_escape(file_name)
            ));
        }
        index_body.push_str("</ul>\n</body>\n</html>\n");
        std::fs::write(&index_path, index_body).map_err(|e| anyhow::anyhow!("Failed to write index: {e}"))?;

        if args.open {
            if let Err(e) = std::process::Command::new("xdg-open").arg(&index_path).status() {
                info!(
                    self.log,
                    "Could not open browser ({}); docs are at {}",
                    e,
                    index_path.display()
                );
            }
        }

        Ok(())
    }
}

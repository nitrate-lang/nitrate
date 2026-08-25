use crate::{Interpreter, package::DEFAULT_REGISTRY};
use clap::Parser;
use slog::info;

#[derive(Parser, Debug)]
#[command(about, long_about = None)]
pub(crate) struct SearchArgs {
    /// Query to search for
    #[arg(value_name = "QUERY")]
    pub(crate) query: Vec<String>,

    /// Limit the number of results
    #[arg(long, default_value_t = 10)]
    pub(crate) limit: usize,

    /// Registry index URL to search packages in
    #[arg(long)]
    pub(crate) index: Option<String>,

    /// Registry to search packages in
    #[arg(long)]
    pub(crate) registry: Option<String>,
}

fn urlencode(input: &str) -> String {
    let mut out = String::with_capacity(input.len());
    for b in input.bytes() {
        match b {
            b'A'..=b'Z' | b'a'..=b'z' | b'0'..=b'9' | b'-' | b'_' | b'.' | b'~' => out.push(b as char),
            _ => out.push_str(&format!("%{:02X}", b)),
        }
    }
    out
}

impl Interpreter<'_> {
    pub(crate) fn sc_search(&mut self, args: SearchArgs) -> anyhow::Result<()> {
        let query = args.query.join(" ");
        if query.is_empty() {
            return Err(anyhow::anyhow!("no search query provided"));
        }

        let registry = args
            .registry
            .as_deref()
            .or(args.index.as_deref())
            .unwrap_or(DEFAULT_REGISTRY);

        let url = format!(
            "{}/api/v1/crates?q={}&limit={}",
            registry.trim_end_matches('/'),
            urlencode(&query),
            args.limit.min(100)
        );

        let response = ureq::get(&url).call().map_err(|e| anyhow::anyhow!("{e}"))?;
        let json: serde_json::Value = response.into_json().map_err(|e| anyhow::anyhow!("{e}"))?;
        let crates = json
            .get("crates")
            .and_then(|v| v.as_array())
            .ok_or_else(|| anyhow::anyhow!("registry returned no results"))?;

        info!(
            self.log,
            "Searching for `{}` (up to {} results)",
            query,
            args.limit.min(100)
        );

        if crates.is_empty() {
            info!(self.log, "no crates found matching `{}`", query);
            return Ok(());
        }

        for c in crates {
            let name = c.get("name").and_then(|n| n.as_str()).unwrap_or("?");
            let version = c.get("max_version").and_then(|n| n.as_str()).unwrap_or("?");
            let description = c.get("description").and_then(|n| n.as_str()).unwrap_or("");
            println!("{:<30} v{}", name, version);
            if !description.is_empty() {
                println!("{}{}", " ".repeat(31), description);
            }
            println!();
        }

        Ok(())
    }
}

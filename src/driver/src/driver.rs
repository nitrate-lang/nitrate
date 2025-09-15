use anstream::println;
use anstyle::{AnsiColor, Color, Style};
use log::error;

use clap::{Command, CommandFactory, Parser};

pub fn get_styles() -> clap::builder::Styles {
    clap::builder::Styles::styled()
        .usage(
            anstyle::Style::new()
                .bold()
                .underline()
                .fg_color(Some(anstyle::Color::Ansi(anstyle::AnsiColor::Green))),
        )
        .header(
            anstyle::Style::new()
                .bold()
                .underline()
                .fg_color(Some(anstyle::Color::Ansi(anstyle::AnsiColor::Green))),
        )
        .literal(
            anstyle::Style::new()
                .fg_color(Some(anstyle::Color::Ansi(anstyle::AnsiColor::Cyan)))
                .bold(),
        )
        .invalid(
            anstyle::Style::new()
                .bold()
                .fg_color(Some(anstyle::Color::Ansi(anstyle::AnsiColor::Red))),
        )
        .error(
            anstyle::Style::new()
                .bold()
                .fg_color(Some(anstyle::Color::Ansi(anstyle::AnsiColor::Red))),
        )
        .valid(
            anstyle::Style::new()
                .bold()
                .underline()
                .fg_color(Some(anstyle::Color::Ansi(anstyle::AnsiColor::Green))),
        )
        .placeholder(
            anstyle::Style::new().fg_color(Some(anstyle::Color::Ansi(anstyle::AnsiColor::Cyan))),
        )
}

#[derive(Parser, Debug)]
#[command(about, long_about = None)]
struct BuildArgs {}

#[derive(Parser, Debug)]
#[command(about, long_about = None)]
struct CheckArgs {}

#[derive(Parser, Debug)]
#[command(about, long_about = None)]
struct CleanArgs {}

#[derive(Parser, Debug)]
#[command(about, long_about = None)]
struct DocArgs {}

#[derive(Parser, Debug)]
#[command(about, long_about = None)]
struct NewDocs {}

#[derive(Parser, Debug)]
#[command(about, long_about = None)]
struct InitArgs {}

#[derive(Parser, Debug)]
#[command(about, long_about = None)]
struct AddArgs {}

#[derive(Parser, Debug)]
#[command(about, long_about = None)]
struct RemoveArgs {}

#[derive(Parser, Debug)]
#[command(about, long_about = None)]
struct RunArgs {}

#[derive(Parser, Debug)]
#[command(about, long_about = None)]
struct TestArgs {}

#[derive(Parser, Debug)]
#[command(about, long_about = None)]
struct BenchArgs {}

#[derive(Parser, Debug)]
#[command(about, long_about = None)]
struct UpdateArgs {}

#[derive(Parser, Debug)]
#[command(about, long_about = None)]
struct SearchArgs {}

#[derive(Parser, Debug)]
#[command(about, long_about = None)]
struct PublishArgs {}

#[derive(Parser, Debug)]
#[command(about, long_about = None)]
struct InstallArgs {}

#[derive(Parser, Debug)]
#[command(about, long_about = None)]
struct UninstallArgs {}

#[derive(Parser, Debug)]
enum Commands {
    /// Compile the current package
    Build(BuildArgs),

    /// Analyze the current package and report errors, but don't build object files
    Check(CheckArgs),

    /// Clear the current package's caches
    Clean(CleanArgs),

    /// Build this package's and its dependencies' documentation
    Doc(DocArgs),

    /// Create a new no3 package
    New(NewDocs),

    /// Create a new no3 package in an existing directory
    Init(InitArgs),

    /// Add dependencies to a manifest file
    Add(AddArgs),

    /// Remove dependencies from a manifest file
    Remove(RemoveArgs),

    /// Run a binary or example of the local package
    Run(RunArgs),

    /// Run the tests
    Test(TestArgs),

    /// Run the benchmarks
    Bench(BenchArgs),

    /// Update dependencies listed in no3.lock
    Update(UpdateArgs),

    /// Search registry for packages
    Search(SearchArgs),

    /// Package and upload this package to the registry
    Publish(PublishArgs),

    /// Install a Nitrate binary
    Install(InstallArgs),

    /// Uninstall a Nitrate binary
    Uninstall(UninstallArgs),
}

/// Nitrate's package manager
#[derive(Parser, Debug)]
#[command(about, long_about = None)]
#[command(styles=get_styles())]
struct Args {
    /// Print version info and exit
    #[arg(short = 'V', long)]
    version: bool,

    /// List installed commands
    #[arg(long)]
    #[clap(action)]
    list: bool,

    /// Provide a detailed explanation of a nitc error message
    #[arg(long, value_name = "CODE")]
    explain: Option<String>,

    /// Use verbose output (-vv very verbose output)
    #[arg(short, long, action = clap::ArgAction::Count)]
    verbose: u8,

    /// Do not print no3 log messages
    #[arg(short, long)]
    quiet: bool,

    /// Coloring
    #[arg(long, value_parser = ["auto", "always", "never"], value_name = "WHEN")]
    color: Option<String>,

    /// Change to DIRECTORY before doing anything
    #[arg(short = 'C', value_name = "DIRECTORY")]
    change_dir: Option<String>,

    /// Assert that `no3.lock` will remain unchanged
    #[arg(long)]
    locked: bool,

    /// Run without accessing the network
    #[arg(long)]
    offline: bool,

    /// Equivalent to specifying both --locked and --offline
    #[arg(long)]
    frozen: bool,

    /// Override a configuration value
    #[arg(long, value_name = "KEY=VALUE|PATH")]
    config: Vec<String>,

    /// Unstable flags to no3, see 'no3 -Z help' for details
    #[arg(short = 'Z', value_name = "FLAG")]
    unstable_flags: Vec<String>,

    #[command(subcommand)]
    command: Option<Commands>,
}

#[derive(Debug, Default)]
pub struct Interpreter {}

impl Interpreter {
    fn list_commands() {
        let fg = Style::new()
            .bold()
            .fg_color(Some(Color::Ansi(AnsiColor::Green)));
        let reset = fg.render_reset();
        println!("{fg}Installed commands:{reset}");

        let mut commands = Args::command()
            .get_subcommands()
            .cloned()
            .collect::<Vec<_>>();

        // Help is a special case
        let help =
            Command::new("help").about("Print this message or the help of the given subcommand(s)");
        commands.push(help);

        for cmd in commands {
            let name = cmd.get_name();
            let about = cmd.get_about().unwrap_or_default();

            let fg = Style::new()
                .bold()
                .fg_color(Some(Color::Ansi(AnsiColor::Cyan)));

            let reset = fg.render_reset();

            println!("    {fg}{:<21}{reset}{}", name, about);
        }
    }

    pub fn run(&self, args: &[String]) -> ! {
        let args = Args::parse_from(args);

        match args.color.as_deref() {
            Some("auto") => {}
            Some("always") => unsafe { std::env::remove_var("NO_COLOR") },
            Some("never") => unsafe { std::env::set_var("NO_COLOR", "1") },

            Some(_) | None => {}
        }

        if args.version {
            println!("no3 {}", env!("CARGO_PKG_VERSION"));
            std::process::exit(0);
        }

        if let Some(change_dir) = &args.change_dir {
            if let Err(e) = std::env::set_current_dir(change_dir) {
                error!("failed to change directory: {}", e);
                std::process::exit(1);
            }
        }

        if args.list {
            Self::list_commands();
            std::process::exit(0);
        }

        println!("Executing with args: {:?}", args);
        std::process::exit(0);
    }
}

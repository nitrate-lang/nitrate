use crate::commands::*;
use anstream::println;
use anstyle::{AnsiColor, Color, Style};
use clap::{Command, CommandFactory, Parser};
use slog::Logger;
use slog::error;

fn get_styles() -> clap::builder::Styles {
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
    New(NewArgs),

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

pub enum InterpreterError {
    UnknownCommand,
    CLISemanticError,

    IoError(std::io::Error),
    OperationalError,
}

impl From<std::io::Error> for InterpreterError {
    fn from(err: std::io::Error) -> Self {
        InterpreterError::IoError(err)
    }
}

pub struct Interpreter<'log> {
    pub(crate) log: &'log Logger,
}

impl<'log> Interpreter<'log> {
    pub fn new(log: &'log Logger) -> Interpreter<'log> {
        Interpreter { log }
    }

    fn list_commands() -> Result<(), InterpreterError> {
        let fg = Style::new()
            .bold()
            .fg_color(Some(Color::Ansi(AnsiColor::Green)));

        let reset = fg.render_reset();

        println!("{fg}Installed commands:{reset}");

        let mut commands = Args::command()
            .get_subcommands()
            .cloned()
            .collect::<Vec<_>>();

        let help = Command::new("help") // Help is a special case
            .about("Print this message or the help of the given subcommand(s)");
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

        Ok(())
    }

    pub fn run(&mut self, args: &[String]) -> Result<(), InterpreterError> {
        let args = Args::parse_from(args);

        /* Output color environment configuration override */
        if let Some(color_config) = &args.color {
            match color_config.as_str() {
                "always" => unsafe { std::env::remove_var("NO_COLOR") },
                "never" => unsafe { std::env::set_var("NO_COLOR", "1") },
                "auto" | _ => {}
            }
        }

        if args.version {
            println!("no3 {}", env!("CARGO_PKG_VERSION"));
            return Ok(());
        }

        if let Some(change_dir_path) = &args.change_dir {
            if let Err(e) = std::env::set_current_dir(change_dir_path) {
                error!(self.log, "failed to change directory: {}", e);
                return Err(InterpreterError::IoError(e));
            }
        }

        if args.list {
            return Self::list_commands();
        }

        if let Some(explain_code) = &args.explain {
            return self.explain_error_code(explain_code);
        }

        /* TODO: Configure logger level */
        let _verbosity_level = match (args.quiet, args.verbose) {
            (true, _) => slog::Level::Critical,
            (false, 0) => slog::Level::Info,
            (false, 1) => slog::Level::Debug,
            (false, 2..) => slog::Level::Trace,
        };

        let (_no_internet, _no_lockfile_change) = match (args.frozen, args.offline, args.locked) {
            (true, _, _) => (true, true),
            (false, true, _) => (true, false),
            (false, false, true) => (false, true),
            (false, false, false) => (false, false),
        };

        if let Some(subcommand) = args.command {
            return match subcommand {
                Commands::Build(build_args) => self.sc_build(build_args),
                Commands::Check(check_args) => self.sc_check(check_args),
                Commands::Clean(clean_args) => self.sc_clean(clean_args),
                Commands::Doc(doc_args) => self.sc_doc(doc_args),
                Commands::New(new_args) => self.sc_new(new_args),
                Commands::Init(init_args) => self.sc_init(init_args),
                Commands::Add(add_args) => self.sc_add(add_args),
                Commands::Remove(remove_args) => self.sc_remove(remove_args),
                Commands::Run(run_args) => self.sc_run(run_args),
                Commands::Test(test_args) => self.sc_test(test_args),
                Commands::Bench(bench_args) => self.sc_bench(bench_args),
                Commands::Update(update_args) => self.sc_update(update_args),
                Commands::Search(search_args) => self.sc_search(search_args),
                Commands::Publish(publish_args) => self.sc_publish(publish_args),
                Commands::Install(install_args) => self.sc_install(install_args),
                Commands::Uninstall(uninstall_args) => self.sc_uninstall(uninstall_args),
            };
        }

        let mut cmd = Args::command();
        cmd.print_help().unwrap();

        Err(InterpreterError::UnknownCommand)
    }
}

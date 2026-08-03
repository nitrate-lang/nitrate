use clap::Parser;
use nitrate_generate::Gen;

#[derive(Parser, Debug)]
#[clap(name = "nitrate_gen", version = "0.1.0", author = "Your Name")]
struct Args {
    #[clap(long, default_value = "1")]
    min_cyclomatic_complexity: u32,

    #[clap(long, default_value = "1000")]
    max_budget: u32,

    #[clap(long, default_value = "1")]
    function_count: u32,

    #[clap(long)]
    seed: Option<u64>,
}

#[tokio::main]
async fn main() {
    env_logger::init();

    let args = Args::parse();

    if args.min_cyclomatic_complexity == 0 {
        eprintln!("Error: min_cyclomatic_complexity must be greater than 0");
        std::process::exit(1);
    }

    if args.max_budget == 0 {
        eprintln!("Error: max_budget must be greater than 0");
        std::process::exit(1);
    }

    if args.function_count == 0 {
        eprintln!("Error: function_count must be greater than 0");
        std::process::exit(1);
    }

    if args.max_budget < args.min_cyclomatic_complexity {
        eprintln!("Error: max_budget must be greater than or equal to min_cyclomatic_complexity");
        std::process::exit(1);
    }

    if args.max_budget < args.function_count {
        eprintln!("Error: max_budget must be greater than or equal to function_count");
        std::process::exit(1);
    }

    let seed = if let Some(seed) = args.seed {
        eprintln!("Using provided seed: {}", seed);
        seed
    } else {
        eprintln!("No seed provided, using random seed.");
        rand::random()
    };

    let config = nitrate_generate::GenConfig {
        min_cyclomatic_complexity: args.min_cyclomatic_complexity,
        max_budget: args.max_budget,
        function_count: args.function_count,
        seed,
    };

    let mut generator = Gen::new(config);
    let program = generator.gen_program();
    println!("{}", program);
}

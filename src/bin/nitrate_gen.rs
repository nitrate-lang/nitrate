use nitrate_generate::Gen;

#[tokio::main]
async fn main() {
    env_logger::init();

    let config = nitrate_generate::GenConfig {
        min_cyclomatic_complexity: 1,
        max_budget: 1000,
        function_count: 1,
    };
    let mut generator = Gen::new(config);
    let program = generator.gen_program();
    println!("{}", program);
}

#[derive(Debug, Default)]
pub struct Interpreter {}

impl Interpreter {
    pub fn execute(&self, args: &[String]) -> Result<(), ()> {
        println!("Executing with args: {:?}", args);
        Ok(())
    }
}

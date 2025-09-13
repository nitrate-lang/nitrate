#[test]
fn refinement_type_evaluation_basic() {
    use nitrate_evaluate::AbstractMachine;
    use nitrate_parsetree::Builder;

    let mut m = AbstractMachine::new();

    let base = Builder::get_u8();
    let width = Builder::create_u32(7);
    let min = Builder::create_u32(10);
    let max = Builder::create_u32(50);

    let test = Builder::create_refinement_type()
        .with_base(base)
        .with_width(Some(width))
        .with_minimum(Some(min))
        .with_maximum(Some(max))
        .build();

    let result = m
        .evaluate_type(&test)
        .expect("Failed to evaluate refinement type");

    assert_eq!(result, test);
}

#[test]
fn refinement_type_evaluation_ordering() {
    use nitrate_evaluate::{AbstractMachine, Unwind};
    use nitrate_parsetree::Builder;
    use std::sync::Arc;
    use std::sync::Mutex;

    // The type: `+{emit(o: 6); type(u8)}(): emit(o: 7): [emit(o: 8): emit(o: 9)]`

    let input_type = Builder::create_refinement_type()
        .with_base(Builder::create_latent_type(
            Builder::create_block()
                .add_statement(
                    Builder::create_call()
                        .with_callee_name("emit")
                        .add_argument(Some("o"), Builder::create_u8(6))
                        .build(),
                )
                .add_element(Builder::create_type_envelop(Builder::get_u8()))
                .build(),
        ))
        .with_width(Some(
            Builder::create_call()
                .with_callee_name("emit")
                .add_argument(Some("o"), Builder::create_u8(7))
                .build(),
        ))
        .with_minimum(Some(
            Builder::create_call()
                .with_callee_name("emit")
                .add_argument(Some("o"), Builder::create_u8(8))
                .build(),
        ))
        .with_maximum(Some(
            Builder::create_call()
                .with_callee_name("emit")
                .add_argument(Some("o"), Builder::create_u8(9))
                .build(),
        ))
        .build();

    let expected_output_type = Builder::create_refinement_type()
        .with_base(Builder::get_u8())
        .with_width(Some(Builder::create_u8(7)))
        .with_minimum(Some(Builder::create_u8(8)))
        .with_maximum(Some(Builder::create_u8(9)))
        .build();

    let expected_observable_behavioral_ordering = [
        Builder::create_u8(6),
        Builder::create_u8(7),
        Builder::create_u8(8),
        Builder::create_u8(9),
    ];

    let mut machine = AbstractMachine::new();
    let observations = Arc::new(Mutex::new(Vec::new()));

    let observation_rc = observations.clone();
    machine.provide_function("emit", move |vm| {
        let value = vm
            .get_parameter("o")
            .cloned()
            .ok_or(Unwind::MissingArgument)?;

        observation_rc.lock().unwrap().push(value.clone());

        Ok(value)
    });

    let result = machine
        .evaluate_type(&input_type)
        .expect("Failed to evaluate refinement type");

    assert_eq!(result, expected_output_type);

    assert_eq!(
        *observations.lock().unwrap(),
        &expected_observable_behavioral_ordering
    );
}

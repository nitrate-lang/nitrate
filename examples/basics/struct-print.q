import "c" {
    fn printf(fmt: string, ...): i32;
}

group Address {
    street: string,
    city: string,
    state: string,
    postal_code: string,
}

group Person {
    name: string,
    age: u8,
    address: Address,
}

fn init_person(person: %*Person) {
    person.name = "John Doe";
    person.age = 30;
    person.address.street = "123 Main St";
    person.address.city = "Springfield";
    person.address.state = "IL";
    person.address.postal_code = "62701";
    ret;
}

fn print_person(person: *Person) {
    printf("Name: %s\n", person.name);
    printf("Age: %d\n", person.age);
    printf("Address: %s\n", person.address.street);
    printf("City: %s\n", person.address.city);
    printf("State: %s\n", person.address.state);
    printf("Postal Code: %s\n", person.address.postal_code);
    ret;
}

pub "c" fn main(): i32 {
    let p: Person;

    init_person(&p);
    print_person(&p);

    printf("&p: %p\n", &p);

    ret;
}
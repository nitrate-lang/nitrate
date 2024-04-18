import "c" {
    fn printf(fmt: *i8, ...): i32;
}

group Address {
    street: string = "123 Main St",
    city: string = "Springfield",
    state: string = "IL",
    postal_code: string = "62701",
}

group Person {
    name: string = "John Doe",
    age: u8 = 30,
    address: Address,
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

fn main() {
    let p: Person;

    /*
    let p: Person;     /// uninitialized
    _ZPerson_init(&p); /// assigns default values


    */

    print_person(&p);

    printf("&p: %p\n", &p);

    ret;
}
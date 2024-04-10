union Person {
    name: string,
    age: u8 ,
}

union Gathering {
    event_name: string,
    location: string,
    timestamp: u64,
    attendees: [Person; 10],
}

pub let x: Gathering;
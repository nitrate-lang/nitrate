union Person {
    name: string = "John smith",
    age: u8 = (30+1)>>1,
}

union Gathering {
    event_name: string,
    location: string,
    timestamp: u64,
    attendees: [Person; 10],
}
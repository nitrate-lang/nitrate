@lang v1
@author Wesley Jones
@license none

subsystem std.crypto.ecc.eddh.X25519: std.crypto.random {
	struct PublicKey {
        key: [u8; 32];
    };

    struct PrivateKey {
        key: [u8; 32];
    };

    struct RawKeyPair {
        pubkey: std.crypto.ecc.eddh.X25519.PublicKey;
        pkey: std.crypto.ecc.eddh.X25519.PrivateKey;
    };

    struct Key {
        kp: std.crypto.ecc.eddh.X25519.RawKeyPair;
        is_generated: bool;
        entropy: i16;
        name: [u8; 8];
    };
        
};

pub var keypair: std.crypto.ecc.eddh.X25519.Key;

pub fn _start() {}
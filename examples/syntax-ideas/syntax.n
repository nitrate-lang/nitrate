@author "Wesley";
@license "LGPL-2.1";

@import orbit<1>::iterator;

pub subsysytem orbit<1>::container {
    @trait("serializable")
    @trait("deterministic")
    @trait("oalways")
    @trait("core-container")
    template <T: solid>
    opaque class vector: linear_iterator<T> {
        feature {
            create_implicit_cast(vector<T>, [T]);
            create_implicit_cast(%vector<T>, %[T]);
            create_implicit_cast([T], vector<T>);
            create_implicit_cast(%[T], %vector<T>);
        }

        pub fn pure vector(x: [T]);
        pub fn pure vector(x: T...);

        /// @brief Pushes an element to the back of the vector.
        /// @param x The element to push.
        /// @return A reference to the vector.
        pub fn pure pushb(x: T) &vector;

        /// @brief Pushes an element to the front of the vector.
        /// @param x The element to push.
        /// @return A reference to the vector.
        pub fn pure pushf(x: T) &vector;

        /// @brief Pops an element from the back of the vector.
        /// @return A reference to the popped element.
        pub fn pure popb() &T;

        /// @brief Pops an element from the front of the vector.
        /// @return A reference to the popped element.
        pub fn pure popf(): &T;

        /// @brief Gets the element at the specified index.
        /// @param i The index of the element to get.
        /// @return A reference to the element.
        pub fn pure at(i: usize): &T;

        /// @brief Gets the size of the vector.
        /// @return The size of the vector.
        pub const fn pure size(): usize;

        /// @brief Clears the vector.
        pub fn pure clear();

        /// @brief Clears the vector and frees any memory
        /// that was allocated for it.
        pub fn pure purge();

        /// @brief Erases an element from the vector.
        /// @param i The index of the element to erase.
        pub fn pure erase(i: usize);

        pub const fn pure stream(): [u8];
        pub static fn pure stream(bytes: [u8]): vector<?: solid, D: [i32]>;
    }
}

pub subsysytem orbit<1>::algorithm {
    /// @brief Generic generator contract
    template <T: solid> 
    interface IGenerator {
        /// @brief Generates the next value.
        /// @return The next value.
        fn impure operator!(): T;

        /// @brief Seeds the generator.
        /// @param buf The seed buffer.
        /// @param state The seed state (implementation-specific).
        fn impure seed(buf: [u8], state: i16);
    }

    template <T: solid>
    interface IUniformGenerator: IGenerator<T> {};

    template <T: solid>
    interface ICSPRNGenerator: IUniformGenerator<T> {};

    template <T: solid>
    interface IQualityGenerator: IUniformGenerator<T> {};

    template <T: solid>
    interface IShittyGenerator: IUniformGenerator<T> {};

    template <T: solid>
    class CryptoGenerator: ICSPRNGenerator<T> {
        let state: [u8; 32];

        pub CryptoGenerator() {
            /// GET from /dev/urandom
            let entropy: [u8] = [0; 32];

            seed(entropy, 0);
        };

        pub fn impure operator!(): T {
            // Do crypto stuff
            // cast to the return type
            ret 0;
        }

        pub fn impure seed(buf: [u8], state: i16) {
            sha3_256(in: state + buf + (buf.size() as [u8; sizeof usize]), out: state);
        }
    };

    template <T: solid>
    class QualityGenerator: IQualityGenerator<T> {
        let state: [u8; 32];

        pub QualityGenerator() {
            /// GET from /dev/random
            let entropy: [u8] = [0; 32];

            seed(entropy, 0);
        };

        pub fn impure operator!(): T {
            // Do quality stuff
            // cast to the return type
            ret 0;
        }

        pub fn impure seed(buf: [u8], state: i16) {
            sha3_256(in: state + buf + (buf.size() as [u8; sizeof usize]), out: state);
        }
    };

    template <T: solid>
    class ShittyGenerator: IShittyGenerator<T> {
        let state: u128;

        pub fn ShittyGenerator() {
            state = 0;
            seed(state, 0);
        };

        pub fn impure operator!(): T {
            state = state * 0xbb9ec707304a3ab3a93a7112f260fbbb + 0xeed4f4f7;
            ret unsafe_bitcast<T>(state);
        }

        pub fn impure seed(buf: [u8], state: i16) {
            for i in 0..buf.size() {
                state += buf[i] * (i+1)
            }
        }
    };

    /// @brief Sorts a vector using a user-defined comparison function.
    /// @param v Mutable reference to the vector to sort.
    /// @param cmp The comparison function to use.
    /// @return How many elements were swapped.
    opaque fn pure sort<T: solid>(
        v: %&vector<T>,
        cmp: (a: &T, b: &T): {{lt, eq, gt}} =>
        {
            retif eq, a == b; ret a < b ? lt : gt;
        }): usize;

    struct uuid {
        let data: u128;

        pub fn uuid() {
            data = !(new CryptoGenerator<u128>());
        }
        
        template <T: solid>
        complex {mask = false}
        pub fn uuid(T x) {
            if (x < min<u128>() || x > max<u128>())
            {
                @warning("It is Ill-formed to construct a UUID from a number smaller than min<u128> or larger than max<u128>.");
                tainted(); //! We don't abort; but we taint the process.
            }

            if !mask {
                // x may not actually be a valid UUID, but we don't care.
                // Valid UUID have certain bits set to indicate their type.
                // Worst case scenario, the user actually supplies more entropy.

                data = x as u128;
            } else {
                data = x;

                // Set the version bits
                data &= 0xffffffff_ffff_4fff_bfff_ffffffffffff;
                data |= 0x00000000_0000_4000_8000_000000000000;

                // Set the variant bits
                data &= 0x00000000_0000_4fff_ffff_ffffffffffff;
                data |= 0x00000000_0000_8000_0000_000000000000;
            }
        }

        pub const fn operator*(): u128 {
            ret data;
        }

        omnitemplate <op: operator<comp>>
        pub const fn operator op (other: uuid): bool => ret data op other.data;

        pub const fn str(): str {
            ret f"{data[128:96]:x}-{data[96:64]:x}-{data[64:32]:x}-{data[32:0]:x}";
        }

        benchmarks {
            "Construction Speed" => {
                volatile {
                    [bench].begin();
                    let uid = uuid();
                    [bench].end();
                }
            },
            "Comparison Speed" => {
                volatile {
                    let uid = uuid();
                    let uid2 = uuid();

                    [bench].begin();
                    let cmp = uid < uid2;
                    [bench].end();
                }
            },
            "Stringify Speed" => {
                volatile {
                    let uid = uuid();

                    [bench].begin();
                    let str = uid.str();
                    [bench].end();
                }
            },
        }

        tests {
            "Construction" => {
                req(uuid(0x8f306680_7a8c_489d_abc2_7cb22e66c58f).str() == "8f306680-7a8c-489d-abc2-7cb22e66c58f");
                req(uuid(0x8f306680_7a8c_889d_abc2_7cb22e66c58f).str() == "8f306680-7a8c-889d-abc2-7cb22e66c58f");
                req(uuid<mask>(0x8f306680_7a8c_889d_abc2_7cb22e66c58f).str() == "8f306680-7a8c-489d-abc2-7cb22e66c58f");
            },
            "Comparison Eq" => {
                let uid = uuid(0x8f306680_7a8c_489d_abc2_7cb22e66c58f);
                let uid2 = uuid(0x8f306680_7a8c_489d_abc2_7cb22e66c58f);

                req(uid == uid2);
            },
            "Comparison Lt" => {
                let uid = uuid(0x8f306680_7a8c_489d_abc2_7cb22e66c58f);
                let uid2 = uuid(0x8f306680_7a8c_489d_abc2_7cb22e66c58e);

                req(uid < uid2);
            },
            "Random UUID" => {
                // All hail God, if this fails, we're all doomed.
                loop 0..10000 {
                    req(uuid() != uuid());
                }
            },
            "Entropy" => {
                loop 0..10000 {
                    req(entropy<shannon>(*uuid()) > 7.0);
                }
            },
        }
    }

    fn demo() {
        let id = uuid();
        let id2 = 0x8f306680_7a8c_489d_abc2_7cb22e66c58f;

        printn(id.str());
        printn(id2.str());
    }

    /// @brief Shuffles a vector.
    /// @param v Mutable reference to the vector to shuffle.
    /// @param rng The random number generator to use.
    opaque fn pure shuffle<T: solid>(v: %&vector<T>, rng: &IGenerator<u8> = new IShittyGenerator());
}
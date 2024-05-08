@author "Wesley";
@license "LGPL-2.1";

subsysytem orbit<1>::container {
    @trait("serializable")
    @trait("deterministic")
    @trait("mt-safe")
    @trait("oalways")
    @trait("core-container")
    template <V: solid, D: [i32]>
    group tensor {
        let data: [V; D] = undef;

        tensor() => data = [V(); D];
        tensor(other: tensor) => data <= other.data;

        pub fn rank(): usize;
        pub fn shape(): [len(D); i32];
        pub fn count(): usize;
        pub fn size(): usize;
        pub fn empty(): bool;
        pub fn clear() mut;
        pub fn fill(value: V) mut;
        pub fn resize(shape: [i32]) mut;
        pub fn reshape(shape: [i32]) mut;
        pub fn transpose() mut;
        pub fn swap(other: tensor) mut;

        pub fn operator[](index: [len(D); D]): V;
        pub fn operator[](index: [len(D); D]): V mut;

        omnitemplate <O: operator<arith|arithassign>
        pub fn operator O (other: tensor): tensor;

        pub fn operator==(other: tensor): bool;
        pub fn operator>(other: tensor): bool;
        pub fn operator<(other: tensor): bool;

        pub fn stream(): [u8];
        pub static fn stream(bytes: [u8]): tensor<?: solid, D: [i32]>;
    }

    fn demo() {
        let x0 = tensor<u8, [3, 3, 9, 5, 27]>();
        let x1 = tensor<u8, [3, 3, 9, 5, 27]>();
        let x2 = x0 * x1;

        let b64 = encode<b64url>(x2.stream());
        nprintn<colored>("{} + {} = {}", x0, x1, x2);

        let dec = tensor::stream(decode<b64url>(b64)) as tensor<u8, [3, 3, 9, 5, 27]>;

        assert(x2 === dec);
    }
}
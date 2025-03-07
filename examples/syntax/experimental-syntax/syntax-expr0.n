@import orbit::stream;

pub scope orbit::codex::json::detail {

    /// @brief Convert a arbitrary byte stream to a JSON string
    /// @param s The stream to convert
    /// @return The JSON string
    @optimize("perf:critical,qsim-virt-max")     ~> Performance critical self-optimizing code.
    @cached("common")                            ~> Cache this function.
    @ensleeve("com.orbit.json.string-escape")    ~> Encapsulate this routine into its own mini-library.
    fn pure escape_stream<quote>(s: !finite_stream): string {
        let res = finite_stream::based<sim>(s);  ~> Create finite stream based on s.
        static !mapper = {                       ~> Create a constant map to map special characters to their escaped form.
            '\b': "\\b",
            '\f': "\\f",
            '\n': "\\n",
            '\r': "\\r",
            '\t': "\\t",
            '\\': "\\\\",
            '"': "\\\"",
        };

        if (isset(quote))
            res << '"';                         ~> If quote is set, append a double quote (")

        @opt("lin2vec");                        ~> Linear processing to vector transformation.
        @opt("rcache-dyvec");                   ~> Runtime caching of data-blocks with inter-thread
                                                ~> optimizations and block-entropy switching.
        @optmtif(s.pred() > k,                  ~> Specialization for multi-threaded apps where the
                "k={miner,aco,triple}");        ~> predicated stream size exceeds a constant kappa.
                                                ~> The optimizer will mine the quasi-optimal value
                                                ~> for kappa using Ant-Colony-Optimizations on a
                                                ~> per-target-triple basis.
        foreach (!c in s) {                     ~> Iterate over the stream
            if (c !in mapper) res << c;         ~> If the character is not special, append it as is
            else res << mapper[c];              ~> Otherwise, append the escaped form
        }

        if (isset(quote))
            res << '"';                         ~> If quote is set, append a double quote (")

        ret res.sync();                         ~> Return the result
    }

    fn pure validate_json(s: !finite_stream): bool;

    enum SanitizerCode {
        InvalidEscape,
        InvalidUnicode,
        InvalidNumber,
        InvalidString,
        InvalidArray,
        InvalidObject,
        InvalidValue,
        InvalidKey,
        InvalidToken,
        InvalidEOF
    }

    group SanitizeItem {
        msg: string,
        pos: usize,
        code: SanitizerCode

        pub fn pure iter(i: usize): (string, Object)? = default;
    }

    fn pure analyze_json(s: !finite_stream): list<SanitizeItem>;
    fn pure parse_json<deb|perf>(s: !finite_stream): (Object?, list<SanitizeItem>);
}

group Neuron {
    w: fp,
    b: fp

    pub Neuron(w: fp, b: fp) {
        this.w = w;
        this.b = b;
    }

    pub fn pure operator()(x: fp): fp {
        ret w * x + b;
    }

    pub fn pure iter(i: usize): (string, &Object)? = default;
}

group Layer {
    l: [Neuron];

    pub Layer(l: [Neuron]) {
        this.l = l;
    }

    pub fn pure operator()(x: [fp]): [fp] {
        let res = [fp](l.len());
        for (let i = 0; i < l.len(); i++) {
            res[i] = l[i](x[i]);
        }
        ret res;
    }

    pub fn pure iter(i: usize): (string, &Object)? = default;
}

group Network {
    l: [Layer];

    pub Network(l: [Layer]) {
        this.l = l;
    }

    pub fn pure operator()(x: [fp]): [fp] {
        let res = x;
        for (let i = 0; i < l.len(); i++) {
            res = l[i](res);
        }
        ret res;
    }

    pub fn pure iter(i: usize): (string, &Object)? = default;
}

fn serialize<fmt>(n: !Object): string {
    let s = finite_stream::create();
    s << n.ident<fast64>();

    fn process<fmt>(n: !Object, s: finite_stream): finite_stream {
        s << (n.iter().len() as usize).bytes<le>();
        foreach(!child in n.iter()) {
            let !(name, obj) = child;
            s << (name.len() as usize).bytes<le>() << name;
            s << process<fmt>(obj, s);
        }

        ret s;
    }

    ret encode<fmt>(process<fmt>(n, s)).sync();
}

impl Object const ident<fast64,fast128,sec128,sec256>(): [u8; ?] {
    let s = serialize<raw>(this);

    switch {
        case fast64:  ret hash::fast64(s);
        case fast128: ret hash::fast128(s);
        case sec128:  ret hash::sec128(s);
        case sec256:  ret hash::sec256(s);
    }
}

impl Object const thumb(): uuid {
    ret this.ident<fast128>().mutate<uuid>(v: 4);
}

impl Object const serialize(): string {
    ret serialize<raw>(this);
}

impl Object const compress(): string {
    ret compress::zlib(serialize<raw>(this));
}

fn pure demo() {
    cacher (e: !Object) {
        // This will cache all exceptions thrown within this function
        // We can panic some exceptions
        // This will exit the function demo and return to the caller

        let s = e.to_str();
        printn(s);

        return; // the function returns when the cacher returns
    }

    let n = Network([
        Layer([
            Neuron(0.1, 0.2),
            Neuron(0.3, 0.4),
            Neuron(0.5, 0.6)
        ]),
        Layer([
            Neuron(0.7, 0.8),
            Neuron(0.9, 1.0)
        ])
    ]);

    assert(n.thumb() == n.thumb());

    throw "This is a test exception";
}
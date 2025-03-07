@author "Wesley";
@license "Unlicense";
@description "A simple web server using Nitrate";

@import orbit::http;
@import orbit::markdown;
@import orbit::log;

using orbit::http::* as .;
using orbit::markdown::* as .;
using orbit::log::* as .;

fn main() {
    let srv = HttpServerBuilder()
            .bind("0.0.0.0", 8443)          ~> Bind to 0.0.0.0:8443
            .certs<SnakeOil>(discard: true) ~> Generate SSL certificates
            .build<Perf>();                 ~> Optimize the HTTP server for performance
                                            ~> (yep, it's that easy! Configure threads
                                            ~> and other settings with the builder)

    srv.get("/hello", fn (req, res) {
        res.send(m("# Hello, World!").html());      ~> Convert markdown to HTML (done at compile-time)
    });

    srv.get("/api/v1/user/:id", fn (req, res) {
        res.send(m(f"**User ID**: {req.param("id")}").html());
    });

    srv.post("/echo", fn (req, res) {
        res.send(req.body());
    });

    ret srv.listen(fn (req, res) {      ~> Lets log each request inteligently
        log(info) <<< req << endl;      ~> Use the '<<<' overload to pretty-print the request
    }).code();                          ~> Generate a status code from the Result
}

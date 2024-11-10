# Syntax ideas:
# - `<<` operator overloading
# - `@import` for importing modules
# - ```` for metavalues. Where they serve as there own unique type and can be handled.
# - `cacher` cache all exceptions from within the function
# - `name<x>` function specialization. for syntax sugar for templates
# - anonymous functions
# - `(x0, x1, x2, ...).method()` call methods on a tuple of like objects


@import orbit::server;

fn nothrow main() {
    cacher (e: !Object) {
        panic(e.to_str());
    }

    let app = ServerBuilder<http>()
               .ssl(snakeoil: true)
               .backwards()
               .build<perf>();

    app.get("/hello", (req: Request, res: Response) {
        res << "Hello, World!" << `text/plain` << `utf-8` << `200 OK`;
    });

    app.get("/json", (req: Request, res: Response) {
        res << { "hello": "world" } << `application/json` << `utf-8` << `200 OK`;
    });

    app.get("/error", (req: Request, res: Response) {
        res << "Internal Server Error" << `text/plain` << `utf-8` << `500 Internal Server Error`;
    });

    app.get("/redirect", (req: Request, res: Response) {
        res << "Redirecting..." << `text/plain` << `utf-8` << `302 Found` << { "Location": "/hello" };
    });

    let r0 = app.listen<http>("0.0.0.0:8080", (req: Request) {
        printn(req.basics());
    }, bg: true);

    let r1 = app.listen<https>("0.0.0.0:8443", (req: Request) {
        printn(req.basics());
    }, bg: true);

    (r0, r1).join();

    ret 0;
}
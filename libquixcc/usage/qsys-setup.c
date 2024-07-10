/* --> This example demonstrates how to use the QSys API to create
 *     custom compiler system calls.
 * ------------------------------------------------------------------
 *
 *  The example defines two QSys calls:
 *  - `6969`: It takes a string argument and prints it to the
 *    console.
 *
 * - `7000`: It injects a C import statement and a printf call
 *    into the generated code. Thus, the generated code will
 *    include a C import statement and a printf call that prints
 *    a message at runtime.
 *
 * Key Takeaways:
 * 1.  The EngineAPI is used to interact with the preprocessor.
 * 2.  The `quixcc_cc_job_t` is the context over which the compilation
 * job is performed.
 * 3.  The `quixcc_cc_new` function is used to create a new compilation job.
 * 4.  The `quixcc_cc_option` function is used to set compiler options.
 * 5.  The `quixcc_qsys_add` function is used to register QSys calls.
 * 6.  The `quixcc_cc_source` function is used to set the input source
 *     stream.
 * 7.  The `quixcc_cc_output` function is used to set the output stream.
 * 8.  The `quixcc_run` function is used to run the compilation job.
 * 9.  The `quixcc_engine_t` is an API supplied opaque object
 *     enabling interaction with the preprocessor on a call-by-call basis.
 * 10. The `quixcc_engine_emit` function is used to emit tokens into the
 *     preprocessor.
 * 11. There are APIs create instances of `quixcc_tok_t` objects.
 * 12. Most memory is managed internally by the library.
 **/

#include <quixcc/Library.h>
#include <quixcc/plugin/EngineAPI.h>

#define QSYS_DEFINE(_name, _desc)                                      \
  bool qsys_##_name(quixcc_engine_t *e, uint32_t n, quixcc_expr_t **v, \
                    uint32_t c)

const char source[] =
    "@use \"v1.0\";\n"
    "\n"
    "fn main(): i32 {\n"
    "  @qsys(6969, \"We are gods among men!\");\n"
    "  @qsys(7000);\n"
    "  ret 0;\n"
    "}\n";

static bool make_job(quixcc_cc_job_t **job) {
  *job = quixcc_cc_new();
  if (!*job) {
    return false;
  }

  /*========= Output object file =========*/
  quixcc_cc_option(*job, "-c", true);
  return true;
}

QSYS_DEFINE(call_6969, "QSys Call 6969") {
  (void)n;
  QSYS_ARGASSERT(qsys_impl_6969, 1);
  QSYS_ARG_STRING(message, message, 0);

  printf("QSys Call 6969: %s\n", message);
  return true;
}

QSYS_DEFINE(call_inject, "Generate tokens example") {
  (void)n;
  (void)v;

  QSYS_ARGASSERT(qsys_impl_inject, 0);

  /*========= Emit 'import "C" fn printf(fmt: string, ...);' =========*/
  quixcc_engine_emit(e, quixcc_tok_new_kw(e, QUIXCC_KW_IMPORT));
  quixcc_engine_emit(e, quixcc_tok_new(e, QUIXCC_LEX_STR, "C"));
  quixcc_engine_emit(e, quixcc_tok_new_kw(e, QUIXCC_KW_FN));
  quixcc_engine_emit(e, quixcc_tok_new_ident(e, "printf"));
  quixcc_engine_emit(e, quixcc_tok_new_punct(e, QUIXCC_PUNCT_OPEN_PAREN));
  quixcc_engine_emit(e, quixcc_tok_new_ident(e, "fmt"));
  quixcc_engine_emit(e, quixcc_tok_new_punct(e, QUIXCC_PUNCT_COLON));
  quixcc_engine_emit(e, quixcc_tok_new_ident(e, "string"));
  quixcc_engine_emit(e, quixcc_tok_new_punct(e, QUIXCC_PUNCT_COMMA));
  quixcc_engine_emit(e, quixcc_tok_new_op(e, QUIXCC_OP_ELLIPSIS));
  quixcc_engine_emit(e, quixcc_tok_new_punct(e, QUIXCC_PUNCT_CLOSE_PAREN));
  quixcc_engine_emit(e, quixcc_tok_new_punct(e, QUIXCC_PUNCT_SEMICOLON));

  /*== Emit 'printf("We were **injected** at compile time!\n");' ==*/
  quixcc_engine_emit(e, quixcc_tok_new_ident(e, "printf"));
  quixcc_engine_emit(e, quixcc_tok_new_punct(e, QUIXCC_PUNCT_OPEN_PAREN));
  quixcc_engine_emit(e,
                     quixcc_tok_new(e, QUIXCC_LEX_STR,
                                    "We were **injected** at compile time!\n"));
  quixcc_engine_emit(e, quixcc_tok_new_punct(e, QUIXCC_PUNCT_CLOSE_PAREN));
  quixcc_engine_emit(e, quixcc_tok_new_punct(e, QUIXCC_PUNCT_SEMICOLON));

  return true;
}

static bool setup_qsys(quixcc_cc_job_t *job) {
  bool ok = true;

  /*========= Register QSys Calls =========*/
  ok &= quixcc_qsys_add(job, 6969, qsys_call_6969);
  ok &= quixcc_qsys_add(job, 7000, qsys_call_inject);
  // Add more QSys calls here...

  return ok;
}

static bool build_source(quixcc_cc_job_t *job) {
  /*============ Setup streams ============*/
  FILE *sin = fmemopen((void *)source, sizeof(source) - 1, "r");
  if (sin == NULL) {
    return false;
  }
  // fmemopen is a GNU extension, so it may not be available on all platforms.
  FILE *sout = fopen("qsys-setup-output.o", "w");
  if (sout == NULL) {
    fclose(sin);
    return false;
  }

  /*============ Setup input stream ============*/
  quixcc_cc_source(job, sin, "<stdin>");

  /*============ Setup output stream ============*/
  quixcc_cc_output(job, sout, NULL);

  /*============ Run the compiler ============*/
  if (!quixcc_run(job)) {
    fclose(sin);
    fclose(sout);

    /*============ Print status messages ============*/
    const quixcc_status_t *status = quixcc_status(job);
    for (size_t i = 0; i < status->m_count; i++) {
      fprintf(stderr, "quixcc: %s\n", status->m_messages[i]->message);
    }
    return false;
  }

  /*============ Cleanup ============*/
  fclose(sin);
  fclose(sout);
  return true;
}

int main() {
  quixcc_cc_job_t *job;
  int err;

  quixcc_lib_init();

  /*================ Setup ===============*/
  if (!make_job(&job)) {
    return 1;
  }

  /*========= Install QSys Calls =========*/
  if (!setup_qsys(job)) {
    err = 1;
    goto cleanup;
  };

  /*========= Build Source =========*/
  if (!build_source(job)) {
    err = 1;
    goto cleanup;
  };

  err = 0;

cleanup:
  quixcc_cc_dispose(job);
  return err;
}

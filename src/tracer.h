#ifndef TYPEDYNTRACER_TRACER_H
#define TYPEDYNTRACER_TRACER_H

#include <Rinternals.h>
#undef TRUE
#undef FALSE
#undef length
#undef eval
#undef error

#ifdef __cplusplus
extern "C" {
#endif

SEXP create_dyntracer(SEXP output_dirpath,
                      SEXP verbose,
                      SEXP truncate,
                      SEXP binary,
                      SEXP compression_level);

SEXP destroy_dyntracer(SEXP dyntracer_sexp);

#ifdef __cplusplus
}
#endif

#endif /* TYPEDYNTRACER_TRACER_H */

#include "utilities.h"

#include "base64.h"

#include <algorithm>

int get_file_size(std::ifstream& file) {
    int position = file.tellg();
    file.seekg(0, std::ios_base::end);
    int length = file.tellg();
    file.seekg(position, std::ios_base::beg);
    return length;
}

std::string readfile(std::ifstream& file) {
    std::string contents;
    file.seekg(0, std::ios::end);
    contents.reserve(file.tellg());
    file.seekg(0, std::ios::beg);
    contents.assign(std::istreambuf_iterator<char>(file),
                    std::istreambuf_iterator<char>());
    return contents;
}

bool file_exists(const std::string& filepath) {
    return std::ifstream(filepath).good();
}

std::string vector_logic(std::string vec_type, SEXP vec_sexp) {
    int len = LENGTH(vec_sexp);
    bool has_na = false;
    int i = 0;
    std::string ret_str = vec_type;

    // deal with the possiblity that its a matrix
    if (Rf_isMatrix(vec_sexp)) {
        int n_row = Rf_nrows(vec_sexp);
        int n_col = Rf_ncols(vec_sexp);

        ret_str.append("[" + std::to_string(n_row) + ", " + std::to_string(n_col) + "]");

        return ret_str;
    }

    switch(TYPEOF(vec_sexp)) {
        case STRSXP: {
            for (i = 0; i < len; ++i) {
                if (STRING_ELT(vec_sexp, i) == NA_STRING) {
                    has_na = true;
                    break;
                }
            }
            break;
        }
        case LGLSXP: {
            auto as_bool_vec = LOGICAL(vec_sexp);
            for (i = 0; i < len; ++i) {
                if (as_bool_vec[i] == NA_LOGICAL) {
                    has_na = true;
                    break;
                }
            }
            break;
        }
        case INTSXP: {
            auto as_int_vec = INTEGER(vec_sexp);
            for (i = 0; i < len; ++i) {
                if (as_int_vec[i] == NA_INTEGER) {
                    has_na = true;
                    break;
                }
            }
            break;
        }
        case REALSXP: {
            auto as_real_vec = REAL(vec_sexp);
            for (i = 0; i < len; ++i) {
                if (ISNA(as_real_vec[i])) {
                    has_na = true;
                    break;
                }
            }
            break;
        }
        case CPLXSXP: {
            auto as_cplx_vec = COMPLEX(vec_sexp);
            for (i = 0; i < len; ++i) {
                if (ISNA(as_cplx_vec[i].r) || ISNA(as_cplx_vec[i].i)) {
                    has_na = true;
                    break;
                }
            }
            break;
        }
        case RAWSXP: {
            /* there won't be NAs here b/c raws are just raw bytes */
            break;
        }
    }

    if (len == 1) {
        // scalar
        // we deal with them separately, just put the length in
    }

    std::string len_portion = "[" + std::to_string(len) + "]";
    ret_str.append(len_portion);

    if (!has_na) {
        // append NA-less tag 
        ret_str.append("@{NA-free}");
    }

    return ret_str;
}

// TODO it might be a data frame?
std::string list_logic(SEXP list_sxp) {

    // TODO complete data frame
    if (Rf_isFrame(list_sxp)) {
        SEXP col_names = Rf_GetColNames(list_sxp);
        // TODO do we want nrows? types of column elements?
        return "TODO";
    }

    std::string ret_str = "list<";
    std::string last_type = "";
    std::string new_type = "";
    int len = LENGTH(list_sxp);
    for(int i = 0; i < len; ++i) {
        if (i == 0) {
            last_type = TYPEOF(VECTOR_ELT(list_sxp, i));
        } else {
            new_type = TYPEOF(VECTOR_ELT(list_sxp, i));
            if (last_type.compare(new_type) != 0) {
                last_type = "any";
                break;
            }
        }
    }

    ret_str.append(last_type);
    ret_str.append(">");
    std::string len_str = "[" + std::to_string(len) + "]";
    ret_str.append(len_str);
    return ret_str;
}

std::string env_logic(SEXP env_sxp) {
    std::string ret_str = "environment";

    // TRUE is Rboolean
    // var_names will be a character vector
    SEXP var_names = R_lsInternal(env_sxp, TRUE);

    int len = LENGTH(var_names);
    std::string bindings = "{";
    for (int i = 0; i < len; ++i) {
        bindings.append(CHAR(STRING_ELT(var_names, i)));
        if (i != len - 1) {
            bindings.append(", ");
        }
    }
    bindings.append("}");
    ret_str.append(bindings);
    return ret_str;
}

/* typr */
std::string get_type_of_sexp(SEXP thing) {
    int len = 1;
    switch (TYPEOF(thing)) {
        case NILSXP:
            return "NULL";
        case SYMSXP:
            return "symbol";
        case LISTSXP:
            // NOT A LIST
            break;
        case CLOSXP:
            return "closure";
        case ENVSXP:
            // TODO reflect?
            break;
        case PROMSXP:
            return "unused";
        case LANGSXP:
            // ???
            break;
        case SPECIALSXP:
            return "special";
        case BUILTINSXP:
            return "builtin";
        case CHARSXP:
            // return vector_logic("character", thing);
            // not actually a character
            break;
        case LGLSXP:
            return vector_logic("logical", thing);
        case INTSXP:
            return vector_logic("integer", thing);
        case REALSXP:
            return vector_logic("double", thing);
        case CPLXSXP:
            return vector_logic("complex", thing);
        case STRSXP:
            // actually character
            return vector_logic("character", thing);
        case DOTSXP:
            return "list<any>";
        case ANYSXP:
            return "any";
            break;
        case VECSXP:
            return list_logic(thing);
        case EXPRSXP:
            return "expression";
        case BCODESXP:
            // ??
            break;
        case EXTPTRSXP:
            // ???
            break;
        case WEAKREFSXP:
            // ???
            break;
        case RAWSXP:
            return vector_logic("raw", thing);
        case S4SXP:
            // ??
            break;
        case NEWSXP:
            // ???
            break;
        case FREESXP:
            // ?
            break;
        case FUNSXP:
            // ?? will this happen
            // apparently we dont need this
            return "function";
    }

    return "TODO";
}

char* copy_string(char* destination, const char* source, size_t buffer_size) {
    size_t l = strlen(source);
    if (l >= buffer_size) {
        strncpy(destination, source, buffer_size - 1);
        destination[buffer_size - 1] = '\0';
    } else {
        strcpy(destination, source);
    }
    return destination;
}

bool sexp_to_bool(SEXP value) {
    return LOGICAL(value)[0] == TRUE;
}

int sexp_to_int(SEXP value) {
    return (int) *INTEGER(value);
}

std::string sexp_to_string(SEXP value) {
    return std::string(CHAR(STRING_ELT(value, 0)));
}

const char* get_name(SEXP sexp) {
    const char* s = NULL;

    switch (TYPEOF(sexp)) {
    case CHARSXP:
        s = CHAR(sexp);
        break;
    case LANGSXP:
        s = get_name(CAR(sexp));
        break;
    case BUILTINSXP:
    case SPECIALSXP:
        s = CHAR(PRIMNAME(sexp));
        break;
    case SYMSXP:
        s = CHAR(PRINTNAME(sexp));
        break;
    }

    return s == NULL ? "" : s;
}

#include <Rinternals.h>

std::string serialize_r_expression(SEXP e) {
    std::string expression;
    int linecount = 0;
    SEXP strvec = serialize_sexp(e, &linecount);
    for (int i = 0; i < linecount - 1; ++i) {
        expression.append(CHAR(STRING_ELT(strvec, i))).append("\n");
    }
    if (linecount >= 1) {
        expression.append(CHAR(STRING_ELT(strvec, linecount - 1)));
    }
    return expression;
}

std::string compute_hash(const char* data) {
    const EVP_MD* md = EVP_md5();
    unsigned char md_value[EVP_MAX_MD_SIZE];
    unsigned int md_len = 0;
#if OPENSSL_VERSION_NUMBER < 0x10100000L
    EVP_MD_CTX mdctx;
    EVP_MD_CTX_init(&mdctx);
    EVP_DigestInit_ex(&mdctx, md, NULL);
    EVP_DigestUpdate(&mdctx, data, strlen(data));
    EVP_DigestFinal_ex(&mdctx, md_value, &md_len);
    EVP_MD_CTX_cleanup(&mdctx);
#else
    EVP_MD_CTX* mdctx = EVP_MD_CTX_new();
    EVP_MD_CTX_init(mdctx);
    EVP_DigestInit_ex(mdctx, md, NULL);
    EVP_DigestUpdate(mdctx, data, strlen(data));
    EVP_DigestFinal_ex(mdctx, md_value, &md_len);
    EVP_MD_CTX_free(mdctx);
#endif
    std::string result{base64_encode(
        reinterpret_cast<const unsigned char*>(md_value), md_len)};

    // This replacement is done so that the hash can be directly used
    // as a filename. If this is not done, the / in the hash prevents
    // it from being used as the name of the file which contains the
    // function which is hashed.
    std::replace(result.begin(), result.end(), '/', '#');
    return result;
}

const char* remove_null(const char* value) {
    return value ? value : "";
}

std::string to_string(const char* str) {
    return str ? std::string(str) : std::string("");
}

std::string pos_seq_to_string(const pos_seq_t& pos_seq) {
    if (pos_seq.size() == 0) {
        return "()";
    }

    std::string str = "(" + std::to_string(pos_seq[0]);

    for (auto i = 1; i < pos_seq.size(); ++i) {
        str.append(" ").append(std::to_string(pos_seq[i]));
    }

    return str + ")";
}

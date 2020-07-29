/*
 * JMU CS 261 Example
 * Mike Lam
 *
 * Demonstrates floating-point representation and IEEE standards.
 *
 * Must be compiled with '-lm' on some platforms to link against the C standard
 * math library.
 */

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <math.h>

/*
 * print raw bitstrings
 */
void print_bytes (int value, int len)
{
    for (int i = 1 << (len-1); i > 0; i >>= 1) {
        printf("%c", (value & i) ? '1' : '0');
    }
}

/*
 * extract bit strings and sign/exponent/significand info from a 32-bit float
 */
void disect_float32 (float fnum)
{
    uint32_t fint = *(uint32_t*)&fnum;
    float    flog = logbf(fnum);
    printf("32-bit: %e  ",  fnum);
    for (uint32_t i = 1 << 31; i > 0; i >>= 1) {
        printf("%c", (fint & i) ? '1' : '0');
        if (i == 1 << (32-1) || i == 1 << (32-1-8)) {
            printf(" ");
        }
    }
    printf(" (0x%x)", *(uint32_t*)&fnum);
    printf("  sign=%c", (fint & 0x80000000) ? '1' : '0');
    printf(" exp=%f (%f)", flog, flog+127.0);
    printf("  value=%.11f", fnum);
    printf("\n");
}

/*
 * extract bit strings and sign/exponent/significand info from a 64-bit float
 */
void disect_float64 (double dnum)
{
    uint64_t dint = *(uint64_t*)&dnum;
    float    dlog = logb(dnum);
    printf("64-bit: %le  ", dnum);
    print_bytes(dint, 64);
    for (uint64_t i = (uint64_t)1 << 63; i > 0; i >>= 1) {
        printf("%c", (dint & i) ? '1' : '0');
        if (i == (uint64_t)1 << (64-1) || i == (uint64_t)1 << (64-1-11)) {
            printf(" ");
        }
    }
    printf(" (0x%lx)", *(uint64_t*)&dnum);
    printf(" sign=%c", (dint & 0x8000000000000000) ? '1' : '0');
    printf(" exp=%f (%f)", dlog, dlog+1023.0);
    printf("  value=%.19f", dnum);
    printf("\n");
}

/*
 * given a sign+exp+frac floating-point number, decode it and print all the
 * intermediate pieces of data
 */
void disect_float (int sign, int e, int exp_len, int f, int sig_len)
{
    double sign_val = sign == 1 ? -1 : 1;
    int bias = (1 << (exp_len-1)) - 1;

    // exponent and fraction

    if (e == (1 << exp_len) - 1) {

        if (f == 0) {
            printf("   special:  %sinfinity\n", (sign ? "-" : ""));
        } else {
            printf("   special:  NaN\n");
        }

    } else {

        // normal or denormal?
        bool normal = (e != 0);

        // exponent
        int E = (normal) ? (e - bias) : (1 - bias);
        int twoE_numer = (E < 0 ? 1         : 1 << E);
        int twoE_denom = (E < 0 ? 1 << (-E) : 1     );
        double exp_val = pow(2.0, (double)E);

        // significand
        int M = f;
        int denom = 1 << sig_len;       // denominator of f and M (in book)
        if (normal) {
            M += denom;
        }
        double sig_val = (double)M / (double)denom;

        // final value
        int val_numer = (E < 0 ?         1 : 1 << E) * M;
        int val_denom = (E < 0 ? 1 << (-E) : 1)      * denom;
        double value = sig_val * exp_val * sign_val;

        // print everything
        printf("  %8s:  sign=%d  e=%d  bias=%d  E=%d  2^E=%d",
                (normal ? "normal" : "denormal"), sign, e, bias, E, twoE_numer);
        if (twoE_denom > 1) {
            printf("/%d", twoE_denom);
        }
        printf("  f=%d/%d  M=%d/%d  2^E*M=%d/%d  val=%lf\n",
                f, denom, M, denom, val_numer, val_denom, value);
    }
}

int main (int argc, char **argv)
{
    // check command-line parameters
    if (argc < 2 || argc > 4) {
        printf("Usage: ./fp <number>\n");
        printf("       ./fp <exp_len> <sig_len>\n");
        printf("       ./fp <sign_bit> <exp_bits> <sig_bits>\n");
        return EXIT_FAILURE;
    }

    if (argc == 2) {

        // MODE 1: "./fp <number>"
        //   (print info for a single floating-point number)

        double   dnum = strtod(argv[1], NULL);

        disect_float32((float)dnum);
        disect_float64(dnum);

    } else if (argc == 3) {

        // MODE 2: "./fp <exp_len> <sig_len>"
        //   (print all floating-point numbers w/ given sizes)

        char *exp_str  = argv[1];
        char *sig_str  = argv[2];

        int exp_len = strtol(exp_str, NULL, 10);
        int sig_len = strtol(sig_str, NULL, 10);

        // positive numbers
        for (int e = 0; e < (1 << exp_len); e++) {
            for (int f = 0; f < (1 << sig_len); f++) {
                printf("0 "); print_bytes(e, exp_len);
                printf(" ");  print_bytes(f, sig_len);
                printf(" %8x", (e << sig_len) | f);
                disect_float(0, e, exp_len, f, sig_len);
            }
        }

        // negative numbers
        for (int e = 0; e < (1 << exp_len); e++) {
            for (int f = 0; f < (1 << sig_len); f++) {
                printf("1 "); print_bytes(e, exp_len);
                printf(" ");  print_bytes(f, sig_len);
                printf(" %8x", (1 << (sig_len+exp_len)) | (e << sig_len) | f);
                disect_float(1, e, exp_len, f, sig_len);
            }
        }


    } else if (argc == 4) {

        // MODE 3: "./fp <sign_bit> <exp_bits> <sig_bits>"
        //   (decode binary bitstrings as a floating-point number)

        char *sign_str = argv[1];
        char *exp_str  = argv[2];
        char *sig_str  = argv[3];

        disect_float(strtol (sign_str, NULL, 2),
                     strtol (exp_str,  NULL, 2),
                     strnlen(exp_str,  128),
                     strtol (sig_str,  NULL, 2),
                     strnlen(sig_str,  128));
    }

    return EXIT_SUCCESS;
}


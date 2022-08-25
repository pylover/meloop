#ifndef _TESTING_H
#define _TESTING_H


#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>


#define RED     "\e[0;31m"
#define GREEN   "\e[0;32m"
#define YELLOW  "\e[0;33m"
#define BLUE    "\e[0;34m"
#define MAGENTA "\e[0;35m"
#define CYAN    "\e[0;36m"
#define WHITE   "\e[0;37m"

#define CLR     "\33[0m"


#define pcolor(c_, f_, ...) \
    printf(c_); \
    printf((f_), ##__VA_ARGS__); \
    printf(CLR)


#define pok(f_, ...) pcolor(GREEN, f_, ##__VA_ARGS__)
#define perr(f_, ...) pcolor(RED, f_, ##__VA_ARGS__)


#define pokln(f_, ...) pok(f_, ##__VA_ARGS__); printf("\r\n")
#define perrln(f_, ...) perr(f_, ##__VA_ARGS__); printf("\r\n")
#define pcolorln(c_, f_, ...) pcolor(c_, f_, ##__VA_ARGS__); printf("\r\n")
#define pdataln(f_, ...) pcolorln(WHITE, f_, ##__VA_ARGS__);


#define SUCCESS(c) if (c) {pokln("%s Ok", __func__); return; }
#define FAILED() perrln("%s Failed", __func__)
#define EXPECTED() pcolor(BLUE, "Expected: ")
#define GIVEN() pcolor(YELLOW, "Given: ")


static void 
printbinary(const unsigned char *buf, int buflen) {
    int i;
    for (i = 0; i < buflen; i++){
        printf("\\%02X", buf[i]);
    }
    printf("\n");
}


void 
equalbin(const unsigned char *expected, const unsigned char *given, 
        uint32_t len) {
    SUCCESS(memcmp(given, expected, len) == 0);

    /* Error */
    FAILED();
    EXPECTED();
    printbinary(expected, len);

    GIVEN();
    printbinary(given, len);

    exit(EXIT_FAILURE);
}


void 
equalchr(const char expected, const char given) {
    SUCCESS(given == expected);

    /* Error */
    FAILED();
    EXPECTED();
    pdataln("%c", expected);

    GIVEN();
    pdataln("%c", given);

    exit(EXIT_FAILURE);
}


void 
equalstr(const char *expected, const char *given) {
    SUCCESS(strcmp(given, expected) == 0);

    /* Error */
    FAILED();
    EXPECTED();
    pdataln("%s", expected);

    GIVEN();
    pdataln("%s", given);

    exit(EXIT_FAILURE);
}


void 
equalnstr(const char *expected, const char *given, u_int32_t len) {
    SUCCESS(strncmp(given, expected, len) == 0);

    /* Error */
    FAILED();
    EXPECTED();
    pdataln("%.*s", len, expected);

    GIVEN();
    pdataln("%.*s", len, given);

    exit(EXIT_FAILURE);
}


void 
equalint(int expected, int given) {
    SUCCESS(given == expected);

    /* Error */
    FAILED();
    EXPECTED();
    pdataln("%d", expected);

    GIVEN();
    pdataln("%d", given);

    exit(EXIT_FAILURE);
}


void 
equalbool(bool expected, bool given) {
    SUCCESS(given == expected);

    /* Error */
    FAILED();
    EXPECTED();
    pdataln("%s", expected?"true":"false");

    GIVEN();
    pdataln("%s", given?"true":"false");

    exit(EXIT_FAILURE);
}


/* Asserts */

#define assert(f, ...) \
    pcolor(CYAN, "%s:%d", __FILE__, __LINE__); \
    pcolor(MAGENTA, " [%s] ", __func__); \
    f(__VA_ARGS__)

#define eqchr(...) assert(equalchr, __VA_ARGS__)
#define eqstr(...) assert(equalstr, __VA_ARGS__)
#define eqnstr(...) assert(equalnstr, __VA_ARGS__)
#define eqint(...) assert(equalint, __VA_ARGS__)
#define eqbool(...) assert(equalbool, __VA_ARGS__)
#define istrue(e) assert(equalbool, true, (e))
#define isfalse(e) assert(equalbool, false, (e))
#define eqbin(e, g, l) \
    assert(equalbin, (unsigned char*)g, (unsigned char*)e, l)


#endif

/* Task 5 - Complete the Code (TODO 1 completed).
 * Compile : gcc -std=c99 -Wall -Wextra q5_complete.c -o q5_complete */
#include <stdio.h>
 
/* Issues the next mess token number, starting from 1001. */
int next_token(void)
{
    /* TODO 1: static local variable - initialised once, keeps its value between calls */
    static int token = 1000;
    return ++token;
}
 
/* The same logic with an automatic variable, for comparison. */
int auto_token(void)
{
    int token = 1000;            /* created and re-initialised on EVERY call */
    return ++token;
}
 
int main(void)
{
    int s1 = next_token(), s2 = next_token(), s3 = next_token();
    int a1 = auto_token(), a2 = auto_token(), a3 = auto_token();
    printf("static: %d %d %d\n", s1, s2, s3);
    printf("auto : %d %d %d\n", a1, a2, a3);
    return 0;
}
File: q5_order.c  —  Task 5 – three calls in one printf (order of evaluation)
/* Task 5 - Order of evaluation of function arguments is UNSPECIFIED in C.
 * Compile : gcc -std=c99 -Wall -Wextra q5_order.c -o q5_order */
#include <stdio.h>
 
int next_token(void)
{
    static int token = 1000;
    return ++token;
}
 
int main(void)
{
    printf("Three calls inside ONE printf: ");
    printf("%d %d %d\n", next_token(), next_token(), next_token());
    /* The calls above already used 1001..1003, so a second round continues at 1004. */
    int a = next_token();
    int b = next_token();
    int c = next_token();
    printf("Three separate statements    : %d %d %d\n", a, b, c);
    return 0;
}
File: q5_v1_auto.c  —  Task 5.1 (a) – automatic local variable
/* Task 5.1 (a) automatic local variable. */
#include <stdio.h>
int next_token(void)
{
    int token = 1000;            /* automatic: re-created on each call */
    return ++token;
}
int main(void)
{
    int t1 = next_token();
    int t2 = next_token();
    int t3 = next_token();
    printf("(a) auto tokens   : %d %d %d\n", t1, t2, t3);
    return 0;
}
File: q5_v1_static.c  —  Task 5.1 (b) – static local variable
/* Task 5.1 (b) static local variable. */
#include <stdio.h>
int next_token(void)
{
    static int token = 1000;     /* static: initialised once, lives for the whole run */
    return ++token;
}
int main(void)
{
    int t1 = next_token();
    int t2 = next_token();
    int t3 = next_token();
    printf("(b) static tokens : %d %d %d\n", t1, t2, t3);
    return 0;
}
File: q5_v1_global_main.c  —  Task 5.1 (c) – extern global, file 1 of 2 (main)
/* Task 5.1 (c) file 1 of 2: uses the global counter through extern.
 * Compile : gcc -std=c99 -Wall -Wextra q5_v1_global_main.c q5_v1_global_counter.c -o q5_v1_global */
#include <stdio.h>
 
extern int g_token;              /* declaration only: the definition is in the other file */
int next_token(void);
 
int main(void)
{
    int t1 = next_token();
    int t2 = next_token();
    int t3 = next_token();
    printf("(c) global tokens : %d %d %d\n", t1, t2, t3);
    printf("    main file reads g_token directly = %d\n", g_token);
    return 0;
}
File: q5_v1_global_counter.c  —  Task 5.1 (c) – extern global, file 2 of 2 (defines g_token)
/* Task 5.1 (c) file 2 of 2: DEFINES the global counter. */
int g_token = 1000;              /* external linkage: visible to other files */
 
int next_token(void)
{
    return ++g_token;
}
File: q5_v1_register.c  —  Task 5.1 (d) – register loop variable
/* Task 5.1 (d) register loop variable in the printing loop. */
#include <stdio.h>
int next_token(void)
{
    static int token = 1000;
    return ++token;
}
int main(void)
{
    register int i;              /* hint: keep the loop counter in a CPU register */
    printf("(d) register loop tokens :");
    for (i = 0; i < 5; i++)
        printf(" %d", next_token());
    printf("\n");
    return 0;
}
File: q5_global_risk_main.c  —  Task 5.3 – risk of an extern global (compiled with q5_v1_global_counter.c)
/* Task 5.3 evidence: any file that can see an extern global can change it by accident.
 * Compile : gcc -std=c99 -Wall -Wextra q5_global_risk_main.c q5_v1_global_counter.c -o q5_global_risk */
#include <stdio.h>
 
extern int g_token;
int next_token(void);
 
int main(void)
{
    int t1 = next_token();
    int t2 = next_token();
    g_token = 5000;              /* accidental (or malicious) change from another file */
    int t3 = next_token();
    printf("Tokens after accidental write: %d %d %d\n", t1, t2, t3);
    return 0;
}
File: q5_register_addr.c  —  Task 5.3 – address of a register variable (expected to fail)
/* Task 5.3 evidence: the address of a register variable cannot be taken.
 * This file is EXPECTED to fail to compile. */
#include <stdio.h>
int main(void)
{
    register int i = 5;
    int *p = &i;                 /* error: address of register variable requested */
    printf("%d\n", *p);
    return 0;
}
File: q5_v2_tokens.h  —  Task 5.2 – shared header for the two-counter versions
/* shared definitions for the two-counter versions */
#define VEG    0
#define NONVEG 1
File: q5_v2_auto.c  —  Task 5.2 (a) – two counters, automatic variable
/* Task 5.2 (a) automatic variable with two counters: a parameter is added, but an
 * automatic variable still cannot remember anything between calls. */
#include <stdio.h>
#include "q5_v2_tokens.h"
int next_token(int kind)
{
    int token[2] = {1000, 1000};         /* re-created on every call */
    return ++token[kind];
}
int main(void)
{
    int v1 = next_token(VEG);
    int n1 = next_token(NONVEG);
    int v2 = next_token(VEG);
    int n2 = next_token(NONVEG);
    int v3 = next_token(VEG);
    printf("VEG    : %d %d %d\n", v1, v2, v3);
    printf("NONVEG : %d %d\n", n1, n2);
    return 0;
}
File: q5_v2_static.c  —  Task 5.2 (b) – two counters, static array
/* Task 5.2 (b) static local array: one hidden counter per kind. */
#include <stdio.h>
#include "q5_v2_tokens.h"
int next_token(int kind)
{
    static int token[2] = {1000, 1000};
    return ++token[kind];
}
int main(void)
{
    int v1 = next_token(VEG);
    int n1 = next_token(NONVEG);
    int v2 = next_token(VEG);
    int n2 = next_token(NONVEG);
    int v3 = next_token(VEG);
    printf("VEG    : %d %d %d\n", v1, v2, v3);
    printf("NONVEG : %d %d\n", n1, n2);
    return 0;
}
File: q5_v2_global_main.c  —  Task 5.2 (c) – two counters, extern global, file 1 of 2
/* Task 5.2 (c) file 1 of 2.
 * Compile : gcc -std=c99 -Wall -Wextra q5_v2_global_main.c q5_v2_global_counter.c -o q5_v2_global */
#include <stdio.h>
#include "q5_v2_tokens.h"
 
extern int g_token[2];
int next_token(int kind);
 
int main(void)
{
    int v1 = next_token(VEG);
    int n1 = next_token(NONVEG);
    int v2 = next_token(VEG);
    int n2 = next_token(NONVEG);
    int v3 = next_token(VEG);
    printf("VEG    : %d %d %d\n", v1, v2, v3);
    printf("NONVEG : %d %d\n", n1, n2);
    return 0;
}
File: q5_v2_global_counter.c  —  Task 5.2 (c) – two counters, extern global, file 2 of 2
/* Task 5.2 (c) file 2 of 2: two global counters. */
#include "q5_v2_tokens.h"
int g_token[2] = {1000, 1000};
 
int next_token(int kind)
{
    return ++g_token[kind];
}
File: q5_v2_register.c  —  Task 5.2 (d) – two counters with register loop variables
/* Task 5.2 (d) register loop variable with the two-counter static function. */
#include <stdio.h>
#include "q5_v2_tokens.h"
int next_token(int kind)
{
    static int token[2] = {1000, 1000};
    return ++token[kind];
}
int main(void)
{
    register int i;
    printf("VEG    :");
    for (i = 0; i < 3; i++) printf(" %d", next_token(VEG));
    printf("\nNONVEG :");
    for (i = 0; i < 2; i++) printf(" %d", next_token(NONVEG));
    printf("\n");
    return 0;
}
File: q5_pointer.c  —  Task 5 – alternative: counter owned by the caller, passed by pointer
/* Task 5 - Alternative: the counter lives in the caller and is passed by pointer.
 * Compile : gcc -std=c99 -Wall -Wextra q5_pointer.c -o q5_pointer */
#include <stdio.h>
 
/* returns the next token, or -1 if no counter was supplied */
int next_token(int *counter)
{
    if (counter == NULL) return -1;
    return ++*counter;
}
 
int main(void)
{
    int veg = 1000, nonveg = 1000;       /* two independent counters, no globals */
    int v1 = next_token(&veg);
    int n1 = next_token(&nonveg);
    int v2 = next_token(&veg);
    int n2 = next_token(&nonveg);
    int v3 = next_token(&veg);
    printf("VEG    : %d %d %d\n", v1, v2, v3);
    printf("NONVEG : %d %d\n", n1, n2);
    printf("NULL counter -> %d (rejected)\n", next_token(NULL));
    printf("Counters still alive in main: veg=%d nonveg=%d\n", veg, nonveg);
    return 0;
}

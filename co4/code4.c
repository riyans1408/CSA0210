/* Task 4 - the ORIGINAL faulty program, exactly as given (only #include <stdio.h> added
 * so that printf is declared). It is EXPECTED to fail to compile.
 * Compile : gcc -std=c99 -Wall -Wextra q4_original.c -o q4_original */
#include <stdio.h>
 
struct account { int number; float balance; };
void addInterest(struct account a, float rate) {
    a.balance = a.balance + a.balance * rate / 100;
}
int main(void) {
    struct account list[3] = {{101, 5000}, {102, 12000}, {103, 800}};
    struct account *p = list;
    for (int i = 0; i < 3; i++) addInterest(list[i], 4);
    printf("%d %f\n", p.number, p.balance);
    return 0;
}
File: q4_evidence.c  —  Task 4.2 – address evidence for defect 1
/* Task 4.2 - Evidence for defect 1 (pass by value). The compile error (p.number) is fixed
 * here with p->number ONLY so that the program can run; addInterest() is still faulty.
 * Compile : gcc -std=c99 -Wall -Wextra q4_evidence.c -o q4_evidence */
#include <stdio.h>
 
struct account { int number; float balance; };
 
static const void *copy_address = NULL;      /* remembers the address seen inside the function */
 
void addInterest(struct account a, float rate) {
    a.balance = a.balance + a.balance * rate / 100;
    copy_address = &a;
    printf("   inside addInterest(): &a = %p, a.balance = %.2f\n", (const void *)&a, a.balance);
}
 
int main(void) {
    struct account list[3] = {{101, 5000}, {102, 12000}, {103, 800}};
    struct account *p = list;
    int different = 0;
    for (int i = 0; i < 3; i++) {
        printf("main: &list[%d] = %p, balance before = %.2f\n", i, (const void *)&list[i], list[i].balance);
        addInterest(list[i], 4);
        if (copy_address != (const void *)&list[i]) different++;
        printf("main: balance after call = %.2f\n", list[i].balance);
    }
    printf("Addresses inside the function differed from main's in %d of 3 calls\n", different);
    printf("Balances: %.2f %.2f %.2f  (unchanged)\n", list[0].balance, list[1].balance, list[2].balance);
    printf("%d %f\n", p->number, p->balance);
    return 0;
}
File: q4_fixed.c  —  Task 4.3 – repaired program with pointer, arrow operator and tests
/* Task 4.3 - Repaired program: structure pointer + arrow operator.
 * Compile : gcc -std=c99 -Wall -Wextra q4_fixed.c -o q4_fixed */
#include <stdio.h>
 
struct account { int number; float balance; };
 
/* Modifies the caller's structure through a pointer. Returns 0 on success, -1 if the
 * pointer is NULL or the rate is negative (invalid input is rejected, balance unchanged). */
int addInterest(struct account *a, float rate)
{
    if (a == NULL || rate < 0) return -1;
    a->balance = a->balance + a->balance * rate / 100;
    return 0;
}
 
/* read-only access: const pointer, so the function cannot change the account */
void print_account(const struct account *a)
{
    printf("  account %d  balance %.2f\n", a->number, a->balance);
}
 
int main(void)
{
    struct account list[3] = {{101, 5000}, {102, 12000}, {103, 800}};
    struct account *p = list;
    struct account zero = {104, 0}, same = {105, 5000}, dbl = {106, 5000}, neg = {107, 5000};
 
    printf("Before interest (4%%):\n");
    for (int i = 0; i < 3; i++) print_account(p + i);
    for (int i = 0; i < 3; i++) addInterest(&list[i], 4);
    printf("After interest:\n");
    for (int i = 0; i < 3; i++) print_account(&p[i]);
    printf("%d %f\n", p->number, p->balance);
 
    printf("\n=== Test cases ===\n");
    printf("TC01: 5000 at 4%%    -> %.2f\n", list[0].balance);
    printf("TC02: 12000 at 4%%   -> %.2f\n", list[1].balance);
    printf("TC03: 800 at 4%%     -> %.2f\n", list[2].balance);
    addInterest(&same, 0);
    printf("TC04: 5000 at 0%%    -> %.2f\n", same.balance);
    addInterest(&zero, 4);
    printf("TC05: 0 at 4%%       -> %.2f\n", zero.balance);
    addInterest(&dbl, 100);
    printf("TC06: 5000 at 100%%  -> %.2f\n", dbl.balance);
    printf("TC07: 5000 at -4%%   -> status %d, balance %.2f (rejected)\n", addInterest(&neg, -4), neg.balance);
    printf("TC08: NULL pointer  -> status %d (rejected)\n", addInterest(NULL, 4));
    return 0;
}
File: q4_byvalue.c  —  Task 4 – alternative: pass by value and return the modified copy
/* Task 4 - Alternative: pass by value and RETURN the modified copy.
 * Compile : gcc -std=c99 -Wall -Wextra q4_byvalue.c -o q4_byvalue */
#include <stdio.h>
 
struct account { int number; float balance; };
 
struct account addInterest(struct account a, float rate)
{
    a.balance = a.balance + a.balance * rate / 100;
    return a;                                   /* a second copy travels back */
}
 
int main(void)
{
    struct account list[3] = {{101, 5000}, {102, 12000}, {103, 800}};
    for (int i = 0; i < 3; i++) list[i] = addInterest(list[i], 4);
    for (int i = 0; i < 3; i++) printf("account %d  balance %.2f\n", list[i].number, list[i].balance);
    printf("\nsizeof(struct account) = %zu bytes\n", sizeof(struct account));
    printf("Copied per call by value : %zu (argument) + %zu (return value) = %zu bytes\n",
           sizeof(struct account), sizeof(struct account), 2 * sizeof(struct account));
    printf("Passed per call by pointer: %zu bytes (one address)\n", sizeof(struct account *));
    return 0;
}

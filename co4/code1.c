/* ---------------------------------------------------------------------------
 * Task 1 - Data Design Review: primary health centre patient register
 * Standard C (C99).
 * Compile : gcc -std=c99 -Wall -Wextra q1_patient_register.c -o q1
 * Sections: A = safe demonstrations of the faults in the ORIGINAL design
 *           B = the REPAIRED design (nested structures + structure pointer)
 *           C = validation / boundary test cases (printed as TCnn lines)
 * ------------------------------------------------------------------------- */
#include <stdio.h>
#include <string.h>
#include <ctype.h>
 
#define MAX_PATIENTS 100
#define NAME_LEN      30
#define PHONE_LEN     10          /* digits; the array needs one more byte for '\0' */
 
/* ---- repaired data design ------------------------------------------------ */
struct Date          { int day, month, year; };
struct BloodPressure { int systolic, diastolic; };   /* e.g. 120 / 80 */
 
struct Patient {
    char name[NAME_LEN];
    int  age;
    char phone[PHONE_LEN + 1];        /* 10 digits + '\0' */
    struct BloodPressure bp;          /* nested structure */
    struct Date visit;                /* nested structure */
};
 
struct PatientRegister {              /* the register owns its data: no globals */
    struct Patient list[MAX_PATIENTS];
    int count;
};
 
enum AddStatus { ADD_OK = 0, ADD_FULL, ADD_BAD_NAME, ADD_BAD_AGE,
                 ADD_BAD_PHONE, ADD_BAD_BP, ADD_BAD_DATE, ADD_NULL };
 
static const char *status_text(int s)
{
    switch (s) {
    case ADD_OK:        return "OK";
    case ADD_FULL:      return "REJECTED: register full";
    case ADD_BAD_NAME:  return "REJECTED: invalid name";
    case ADD_BAD_AGE:   return "REJECTED: invalid age";
    case ADD_BAD_PHONE: return "REJECTED: invalid phone";
    case ADD_BAD_BP:    return "REJECTED: invalid blood pressure";
    case ADD_BAD_DATE:  return "REJECTED: invalid date";
    case ADD_NULL:      return "REJECTED: null pointer";
    }
    return "UNKNOWN";
}
 
/* Copies at most n-1 characters and always adds '\0' (models a too-small char array). */
static void copy_fit(char *dst, size_t n, const char *src)
{
    size_t len = strlen(src);
    if (len > n - 1) len = n - 1;
    memcpy(dst, src, len);
    dst[len] = '\0';
}
 
/* ---- small validation helpers ------------------------------------------- */
static int is_leap(int y) { return (y % 4 == 0 && y % 100 != 0) || (y % 400 == 0); }
 
static int days_in_month(int m, int y)
{
    static const int dm[] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
    return (m == 2 && is_leap(y)) ? 29 : dm[m - 1];
}
 
static int valid_date(const struct Date *d)
{
    if (d->year < 2000 || d->year > 2100) return 0;
    if (d->month < 1 || d->month > 12)    return 0;
    return d->day >= 1 && d->day <= days_in_month(d->month, d->year);
}
 
/* exactly 10 characters and every one of them a digit */
static int valid_phone(const char *s)
{
    size_t i;
    if (strlen(s) != PHONE_LEN) return 0;
    for (i = 0; i < PHONE_LEN; i++)
        if (!isdigit((unsigned char)s[i])) return 0;
    return 1;
}
 
static int validate_patient(const struct Patient *p)
{
    if (memchr(p->name, '\0', NAME_LEN) == NULL || p->name[0] == '\0') return ADD_BAD_NAME;
    if (p->age < 0 || p->age > 120)                                    return ADD_BAD_AGE;
    if (memchr(p->phone, '\0', sizeof p->phone) == NULL ||
        !valid_phone(p->phone))                                        return ADD_BAD_PHONE;
    if (p->bp.systolic  < 70 || p->bp.systolic  > 250 ||
        p->bp.diastolic < 40 || p->bp.diastolic > 150 ||
        p->bp.systolic <= p->bp.diastolic)                             return ADD_BAD_BP;
    if (!valid_date(&p->visit))                                        return ADD_BAD_DATE;
    return ADD_OK;
}
 
/* ---- Task 1.2: addPatient() rewritten to accept structure pointers -------- */
int addPatient(struct PatientRegister *reg, const struct Patient *p)
{
    int st;
    if (reg == NULL || p == NULL)    return ADD_NULL;
    if (reg->count >= MAX_PATIENTS)  return ADD_FULL;      /* limit check */
    st = validate_patient(p);
    if (st != ADD_OK)                return st;
    reg->list[reg->count] = *p;                            /* one whole-structure copy */
    reg->count++;
    return ADD_OK;
}
 
/* Builds a Patient from raw values and adds it; returns the status. */
static int try_add(struct PatientRegister *reg, const char *name, int age,
                   const char *phone, int sys, int dia, int d, int m, int y)
{
    struct Patient p;
    if (name == NULL || phone == NULL) return ADD_NULL;
    if (strlen(name) >= NAME_LEN)      return ADD_BAD_NAME;   /* would not fit */
    if (!valid_phone(phone))           return ADD_BAD_PHONE;  /* checked on raw text */
    memset(&p, 0, sizeof p);
    strcpy(p.name, name);
    p.age = age;
    strcpy(p.phone, phone);
    p.bp.systolic = sys;  p.bp.diastolic = dia;
    p.visit.day = d;  p.visit.month = m;  p.visit.year = y;
    return addPatient(reg, &p);
}
 
/* Selection sort by age: one whole-structure swap moves ONE record. */
static int sort_by_age(struct PatientRegister *reg)
{
    int i, j, min, swaps = 0;
    for (i = 0; i < reg->count - 1; i++) {
        min = i;
        for (j = i + 1; j < reg->count; j++)
            if (reg->list[j].age < reg->list[min].age) min = j;
        if (min != i) {
            struct Patient tmp = reg->list[i];
            reg->list[i] = reg->list[min];
            reg->list[min] = tmp;
            swaps++;
        }
    }
    return swaps;
}
 
static void print_register(const struct PatientRegister *reg)
{
    int i;
    for (i = 0; i < reg->count; i++) {
        const struct Patient *p = &reg->list[i];
        printf("  %d. %-8s age %3d  ph %s  BP %d/%d  visit %02d/%02d/%04d\n",
               i + 1, p->name, p->age, p->phone, p->bp.systolic, p->bp.diastolic,
               p->visit.day, p->visit.month, p->visit.year);
    }
}
 
static void print_ages(const struct PatientRegister *reg)
{
    int i;
    for (i = 0; i < reg->count; i++) printf("%s%d", i ? " " : "", reg->list[i].age);
}
 
int main(void)
{
    /* ===== A. faults of the ORIGINAL design, shown safely ===== */
    char o_name[100][20];  int o_age[100];  char o_phone[100][10];
    float o_bp[100];       char o_date[100][8];
    char phone_buf[10], date_buf[8];
    const char *phone_src = "9876543210", *date_src = "12/10/2026";
    float bp_f = 120 / 80;                           /* what "120/80" really becomes */
    size_t old_total, old_swap, new_swap;
 
    printf("=== A. Original design: demonstrated faults ===\n");
    copy_fit(phone_buf, sizeof phone_buf, phone_src);
    printf("A1 phone \"%s\" kept in char[10]  -> \"%s\" (only %zu digits survive)\n",
           phone_src, phone_buf, strlen(phone_buf));
    printf("A2 blood pressure 120/80 kept in one float -> %.2f (two readings lost)\n", bp_f);
    copy_fit(date_buf, sizeof date_buf, date_src);
    printf("A3 date \"%s\" kept in char[8]   -> \"%s\" (truncated)\n", date_src, date_buf);
 
    old_total = sizeof o_name + sizeof o_age + sizeof o_phone + sizeof o_bp + sizeof o_date;
    old_swap  = sizeof o_name[0] + sizeof o_age[0] + sizeof o_phone[0] +
                sizeof o_bp[0] + sizeof o_date[0];
    new_swap  = sizeof(struct Patient);
    printf("A4 original: 5 parallel arrays = %zu bytes for 100 patients\n", old_total);
    printf("A5 original: sorting by age must swap 5 arrays; one record = %zu bytes (5 swaps)\n",
           old_swap);
 
    /* ===== B. repaired design ===== */
    {
        struct PatientRegister reg;
        int swaps;
        memset(&reg, 0, sizeof reg);
        printf("\n=== B. Repaired design: array of structures ===\n");
        printf("sizeof(struct Patient) = %zu bytes, register of %d = %zu bytes\n",
               sizeof(struct Patient), MAX_PATIENTS, sizeof(struct PatientRegister));
        try_add(&reg, "Meena",   45, "9840011223", 120, 80, 5, 11, 2026);
        try_add(&reg, "Arun",     7, "9840022334", 100, 65, 5, 11, 2026);
        try_add(&reg, "Kavitha", 63, "9840033445", 140, 90, 6, 11, 2026);
        try_add(&reg, "Joseph",  30, "9840044556", 118, 76, 6, 11, 2026);
        try_add(&reg, "Divya",    7, "9840055667",  98, 60, 7, 11, 2026);
        printf("Before sorting (%d patients):\n", reg.count);
        print_register(&reg);
        swaps = sort_by_age(&reg);
        printf("After sorting by age (%d whole-structure swaps, %zu bytes moved per swap):\n",
               swaps, new_swap);
        print_register(&reg);
        printf("Bytes moved per swap: structure design = %zu, parallel arrays = %zu\n",
               new_swap, old_swap);
    }
 
    /* ===== C. validation and boundary tests ===== */
    {
        struct PatientRegister reg, big, tiny;
        int n;
        memset(&reg, 0, sizeof reg);
        memset(&big, 0, sizeof big);
        memset(&tiny, 0, sizeof tiny);
        printf("\n=== C. Validation tests ===\n");
        printf("TC01: %s\n", status_text(try_add(&reg, "Asha",   34, "9876543210", 120, 80, 10, 11, 2026)));
        printf("TC02: %s\n", status_text(try_add(&reg, "Bala",   -5, "9876543211", 120, 80, 10, 11, 2026)));
        printf("TC03: %s\n", status_text(try_add(&reg, "Baby",    0, "9876543212", 90,  60, 10, 11, 2026)));
        printf("TC04: %s\n", status_text(try_add(&reg, "Elder", 120, "9876543213", 130, 85, 10, 11, 2026)));
        printf("TC05: %s\n", status_text(try_add(&reg, "Old",   121, "9876543214", 130, 85, 10, 11, 2026)));
        printf("TC06: %s\n", status_text(try_add(&reg, "Chitra", 40, "987654321",  120, 80, 10, 11, 2026)));
        printf("TC07: %s\n", status_text(try_add(&reg, "Dinesh", 40, "98765abcde", 120, 80, 10, 11, 2026)));
        printf("TC08: %s\n", status_text(try_add(&reg, "Eswar",  40, "9876543215", 80, 120, 10, 11, 2026)));
        printf("TC09: %s\n", status_text(try_add(&reg, "Farah",  40, "9876543216", 120, 80, 31,  2, 2026)));
        printf("TC10: %s\n", status_text(try_add(&reg, "Gopal",  40, "9876543217", 120, 80, 29,  2, 2028)));
        printf("TC11: %s\n", status_text(try_add(&reg, "Hema",   40, "9876543218", 120, 80, 29,  2, 2026)));
        printf("TC12: %s\n", status_text(try_add(&reg, "",       40, "9876543219", 120, 80, 10, 11, 2026)));
        printf("TC13: %s\n", status_text(addPatient(&reg, NULL)));
        printf("TC14: count = %d\n", reg.count);
        for (n = 0; n < MAX_PATIENTS; n++)
            try_add(&big, "Filler", 30, "9000000000", 120, 80, 1, 1, 2026);
        printf("TC15: %s\n", status_text(try_add(&big, "Extra", 30, "9000000001", 120, 80, 1, 1, 2026)));
        printf("TC16: count = %d\n", big.count);
        printf("TC17: %s\n", status_text(try_add(&tiny, "Idris", 30, "987654321012", 120, 80, 1, 1, 2026)));
 
        /* sorting tests */
        memset(&reg, 0, sizeof reg);
        try_add(&reg, "P1", 45, "9000000001", 120, 80, 1, 1, 2026);
        try_add(&reg, "P2",  7, "9000000002", 120, 80, 1, 1, 2026);
        try_add(&reg, "P3", 63, "9000000003", 120, 80, 1, 1, 2026);
        try_add(&reg, "P4", 30, "9000000004", 120, 80, 1, 1, 2026);
        try_add(&reg, "P5",  7, "9000000005", 120, 80, 1, 1, 2026);
        n = sort_by_age(&reg);
        printf("TC18: ages after sort = ");  print_ages(&reg);  printf(" (%d swaps)\n", n);
        n = sort_by_age(&reg);
        printf("TC19: sorting an already sorted register = %d swaps\n", n);
        memset(&tiny, 0, sizeof tiny);
        n = sort_by_age(&tiny);
        printf("TC20: sorting an empty register = %d swaps, count %d\n", n, tiny.count);
        try_add(&tiny, "Solo", 50, "9000000009", 120, 80, 1, 1, 2026);
        n = sort_by_age(&tiny);
        printf("TC21: sorting one patient = %d swaps, count %d\n", n, tiny.count);
    }
    return 0;
}

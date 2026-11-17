/* Task 2 - Complete the Code (TODO 1 and TODO 2 completed).
 * Compile : gcc -std=c99 -Wall -Wextra q2_complete.c -o q2_complete */
#include <stdio.h>
 
struct date { int day, month, year; };
struct address { char city[20]; int pincode; };
struct parcel { int id; float weight; struct address to; struct date booked; };
 
static int swaps_done = 0;          /* counts whole-structure swaps (file-scope static) */
 
void print_parcel(const struct parcel *p)
{
    /* TODO 1: id, weight (1 decimal), city-pincode and booking date dd/mm/yyyy, using -> */
    printf("%d %.1f kg %s-%d %02d/%02d/%04d\n",
           p->id, p->weight, p->to.city, p->to.pincode,
           p->booked.day, p->booked.month, p->booked.year);
}
 
/* Selection sort of the parcels in ascending order of weight. */
void sort_by_weight(struct parcel p[], int n)
{
    /* TODO 2: for each position i find the lightest parcel in i..n-1 and swap the
     * WHOLE structure into position i. */
    for (int i = 0; i < n - 1; i++) {
        int min = i;
        for (int j = i + 1; j < n; j++)
            if (p[j].weight < p[min].weight) min = j;
        if (min != i) {
            struct parcel tmp = p[i];   /* copy 1 */
            p[i] = p[min];              /* copy 2 */
            p[min] = tmp;               /* copy 3 */
            swaps_done++;
        }
    }
}
 
int main(void)
{
    struct parcel list[3] = {
        {501, 4.5f, {"Madurai", 625001},    {12, 10, 2026}},
        {502, 1.2f, {"Chennai", 600077},    {12, 10, 2026}},
        {503, 2.8f, {"Coimbatore", 641001}, {13, 10, 2026}}
    };
    sort_by_weight(list, 3);
    for (int i = 0; i < 3; i++) print_parcel(&list[i]);
 
    /* bytes copied by the swaps */
    printf("\nsizeof(struct parcel) = %zu bytes\n", sizeof(struct parcel));
    printf("Bytes copied per swap (3 whole-structure copies) = %zu\n", 3 * sizeof(struct parcel));
    printf("Swaps performed = %d, total bytes copied = %zu\n",
           swaps_done, (size_t)swaps_done * 3 * sizeof(struct parcel));
    return 0;
}
File: q2_design.c  —  Task 2 – full parcel model, operations, access paths and flat-structure comparison
/* Task 2 - Courier parcel tracking: full data model, operations, access paths and the
 * flat-structure comparison.
 * Compile : gcc -std=c99 -Wall -Wextra q2_design.c -o q2_design */
#include <stdio.h>
#include <string.h>
#include <ctype.h>
 
#define MAX_BOOKINGS 100
 
/* ---- Task 2.2: nested structure definitions using typedef ---- */
typedef struct { int day, month, year; } Date;
typedef struct { char name[30]; char city[20]; int pincode; } Party;   /* sender / receiver */
typedef enum { NORMAL = 0, EXPRESS = 1 } Service;
typedef struct {
    int     id;
    float   weight;          /* kg */
    Date    booked;          /* nested */
    Party   sender;          /* nested */
    Party   receiver;        /* nested */
    Service service;
    float   charge;          /* rupees */
} Parcel;
 
/* The same information written as a FLAT structure (for the alternative analysis) */
typedef struct {
    int id; float weight;
    int b_day, b_month, b_year;
    char s_name[30], s_city[20]; int s_pincode;
    char r_name[30], r_city[20]; int r_pincode;
    Service service; float charge;
} ParcelFlat;
 
/* ---- Task 2.3 (a): selection sort by weight, ascending ---- */
static void sort_by_weight(Parcel *p, int n)
{
    int i, j, min;
    for (i = 0; i < n - 1; i++) {
        min = i;
        for (j = i + 1; j < n; j++)
            if ((p + j)->weight < (p + min)->weight) min = j;
        if (min != i) { Parcel t = p[i]; p[i] = p[min]; p[min] = t; }
    }
}
 
static int is_sorted(const Parcel *p, int n)
{
    for (int i = 1; i < n; i++) if (p[i].weight < p[i - 1].weight) return 0;
    return 1;
}
 
/* case-insensitive text comparison (strcasecmp is not standard C) */
static int same_text(const char *a, const char *b)
{
    while (*a && *b) {
        if (tolower((unsigned char)*a) != tolower((unsigned char)*b)) return 0;
        a++; b++;
    }
    return *a == *b;
}
 
/* ---- Task 2.3 (b): list parcels for a receiver city; returns how many, -1 if invalid ---- */
static int list_by_city(const Parcel *p, int n, const char *city, int verbose)
{
    int found = 0;
    if (p == NULL || city == NULL || city[0] == '\0') return -1;
    for (int i = 0; i < n; i++)
        if (same_text(p[i].receiver.city, city)) {
            if (verbose)
                printf("  #%d  %.2f kg  to %s (%s-%d)  %s  Rs %.2f\n", p[i].id, p[i].weight,
                       p[i].receiver.name, p[i].receiver.city, p[i].receiver.pincode,
                       p[i].service == EXPRESS ? "express" : "normal", p[i].charge);
            found++;
        }
    return found;
}
 
static void print_ids_for_city(const Parcel *p, int n, const char *city)
{
    int first = 1;
    for (int i = 0; i < n; i++)
        if (same_text(p[i].receiver.city, city)) { printf("%s%d", first ? "" : " ", p[i].id); first = 0; }
}
 
static void print_weights(const Parcel *p, int n)
{
    for (int i = 0; i < n; i++) printf("%s%.2f", i ? " " : "", p[i].weight);
}
 
int main(void)
{
    Parcel bookings[MAX_BOOKINGS] = {      /* the day's bookings: array of structures */
        {601, 4.50f, {12,10,2026}, {"Anitha","Chennai",600017},  {"Karthik","Madurai",625001},    NORMAL, 180.0f},
        {602, 1.20f, {12,10,2026}, {"Suresh","Salem",636001},    {"Priya","Chennai",600077},      EXPRESS,150.0f},
        {603, 2.80f, {13,10,2026}, {"Mohan","Trichy",620001},    {"Divya","Coimbatore",641001},   NORMAL,  95.0f},
        {604, 0.50f, {13,10,2026}, {"Latha","Madurai",625002},   {"Ganesh","Chennai",600042},     EXPRESS, 80.0f},
        {605, 7.25f, {13,10,2026}, {"Ravi","Chennai",600001},    {"Selvi","Madurai",625010},      NORMAL, 260.0f},
        {606, 2.80f, {13,10,2026}, {"Nisha","Erode",638001},     {"Hari","Chennai",600001},       NORMAL, 110.0f}
    };
    int n = 6;
    Parcel *ptr = bookings;
    Parcel one[1] = { {701, 3.0f, {14,10,2026}, {"A","X",1}, {"B","Y",2}, NORMAL, 50.0f} };
 
    printf("=== Entity sizes ===\n");
    printf("sizeof(Date)=%zu  sizeof(Party)=%zu  sizeof(Parcel)=%zu\n",
           sizeof(Date), sizeof(Party), sizeof(Parcel));
    printf("sizeof(ParcelFlat)=%zu\n", sizeof(ParcelFlat));
 
    printf("\n=== Task 2.4: access paths to the 3rd parcel's receiver PIN code ===\n");
    printf("(i)  bookings[2].receiver.pincode        = %d\n", bookings[2].receiver.pincode);
    printf("(ii) (ptr + 2)->receiver.pincode          = %d\n", (ptr + 2)->receiver.pincode);
 
    printf("\n=== Sorting by weight ===\n");
    sort_by_weight(bookings, n);
    for (int i = 0; i < n; i++)
        printf("  #%d  %.2f kg  -> %s\n", bookings[i].id, bookings[i].weight, bookings[i].receiver.city);
 
    printf("\n=== Parcels for receiver city Chennai ===\n");
    list_by_city(bookings, n, "Chennai", 1);
 
    printf("\n=== Test cases ===\n");
    printf("TC01: sorted weights = "); print_weights(bookings, n); printf("\n");
    printf("TC02: Chennai -> %d parcels: ", list_by_city(bookings, n, "Chennai", 0));
    print_ids_for_city(bookings, n, "Chennai"); printf("\n");
    printf("TC03: CHENNAI (upper case) -> %d parcels\n", list_by_city(bookings, n, "CHENNAI", 0));
    printf("TC04: Salem (sender city only) -> %d parcels\n", list_by_city(bookings, n, "Salem", 0));
    printf("TC05: empty list (n = 0) -> %d parcels\n", list_by_city(bookings, 0, "Chennai", 0));
    sort_by_weight(one, 1);
    printf("TC06: one parcel sorted -> weight %.2f, is_sorted = %d\n", one[0].weight, is_sorted(one, 1));
    printf("TC07: array with equal weights (2.80, 2.80) sorted -> is_sorted = %d\n", is_sorted(bookings, n));
    printf("TC08: city = NULL -> %s\n", list_by_city(bookings, n, NULL, 0) < 0 ? "rejected (-1)" : "accepted");
    printf("TC09: empty city text -> %s\n", list_by_city(bookings, n, "", 0) < 0 ? "rejected (-1)" : "accepted");
    printf("TC10: sizeof(Parcel) = %zu, sizeof(ParcelFlat) = %zu\n", sizeof(Parcel), sizeof(ParcelFlat));
    return 0;
}

/* Task 3 - Complete the Code (TODO 1 completed).
 * Compile : gcc -std=c99 -Wall -Wextra q3_complete.c -o q3_complete */
#include <stdio.h>
 
enum kind { TEMPERATURE, HUMIDITY, STATUS };
 
struct reading {
    enum kind type;          /* tag: tells which union member is valid */
    union {
        float celsius;
        int   percent;
        char  status[8];
    } value;
};
 
void show(const struct reading *r)
{
    /* TODO 1: print ONLY the member that the tag says is valid */
    switch (r->type) {
    case TEMPERATURE: printf("Temp %.1f C\n",     r->value.celsius); break;
    case HUMIDITY:    printf("Humidity %d %%\n",  r->value.percent); break;
    case STATUS:      printf("Status %s\n",       r->value.status);  break;
    default:          printf("Invalid packet\n");                    break;
    }
}
 
int main(void)
{
    struct reading log[3] = {
        {TEMPERATURE, {.celsius = 31.5f}},
        {HUMIDITY,    {.percent = 68}},
        {STATUS,      {.status = "OK"}}
    };
    printf("sizeof union = %zu, sizeof struct reading = %zu\n",
           sizeof log[0].value, sizeof log[0]);
    for (int i = 0; i < 3; i++) show(&log[i]);
    return 0;
}
File: q3_design.c  —  Task 3 – sizes, validating constructors, misuse experiment and test cases
/* Task 3 - Both designs, predicted vs measured sizes, safe constructors, the misuse
 * experiment and the pointer-based alternative.
 * Compile : gcc -std=c99 -Wall -Wextra q3_design.c -o q3_design */
#include <stdio.h>
#include <string.h>
#include <stddef.h>
 
enum kind { TEMPERATURE, HUMIDITY, STATUS };
 
/* Design 1: a structure that holds ALL three fields at once */
struct packet_all {
    float celsius;
    int   percent;
    char  status[8];
};
 
/* Design 2: tagged union - a tag plus ONE of the three values */
struct reading {
    enum kind type;
    union { float celsius; int percent; char status[8]; } value;
};
 
/* Alternative: tag + void pointer to separately allocated data */
struct reading_ptr { enum kind type; void *data; };
 
enum make_status { MK_OK = 0, MK_BAD_RANGE, MK_BAD_TEXT, MK_NULL };
 
static const char *mk_text(int s)
{
    switch (s) {
    case MK_OK:        return "OK";
    case MK_BAD_RANGE: return "REJECTED: value out of range";
    case MK_BAD_TEXT:  return "REJECTED: status text empty or longer than 7 characters";
    default:           return "REJECTED: null pointer";
    }
}
 
/* Constructors validate before storing (assumed sensor limits: -90..60 C, 0..100 %) */
static int make_temperature(struct reading *r, float c)
{
    if (r == NULL) return MK_NULL;
    if (c < -90.0f || c > 60.0f) return MK_BAD_RANGE;
    r->type = TEMPERATURE;  r->value.celsius = c;
    return MK_OK;
}
static int make_humidity(struct reading *r, int pct)
{
    if (r == NULL) return MK_NULL;
    if (pct < 0 || pct > 100) return MK_BAD_RANGE;
    r->type = HUMIDITY;  r->value.percent = pct;
    return MK_OK;
}
static int make_status(struct reading *r, const char *text)
{
    size_t len;
    if (r == NULL || text == NULL) return MK_NULL;
    len = strlen(text);
    if (len == 0 || len > 7) return MK_BAD_TEXT;      /* 7 characters + '\0' = 8 bytes */
    r->type = STATUS;
    memset(r->value.status, 0, sizeof r->value.status);
    memcpy(r->value.status, text, len);
    return MK_OK;
}
 
static void show(const struct reading *r)
{
    switch (r->type) {
    case TEMPERATURE: printf("Temp %.1f C\n",    r->value.celsius); break;
    case HUMIDITY:    printf("Humidity %d %%\n", r->value.percent); break;
    case STATUS:      printf("Status %s\n",      r->value.status);  break;
    default:          printf("Invalid packet\n");                   break;
    }
}
 
int main(void)
{
    struct reading r;
    unsigned char bytes[sizeof r.value];
 
    printf("=== Predicted and measured sizes (64-bit GCC) ===\n");
    printf("Prediction  struct of all three: 4 (float) + 4 (int) + 8 (char[8]) = 16\n");
    printf("Prediction  tagged union: 4 (tag) + 8 (union: largest member 8, align 4) = 12\n");
    printf("Measured    sizeof(struct packet_all) = %zu\n", sizeof(struct packet_all));
    printf("Measured    sizeof(union in reading)  = %zu\n", sizeof r.value);
    printf("Measured    sizeof(struct reading)    = %zu\n", sizeof(struct reading));
    printf("offsetof(struct reading, type)  = %zu\n", offsetof(struct reading, type));
    printf("offsetof(struct reading, value) = %zu\n", offsetof(struct reading, value));
    printf("For 10000 packets: all-fields = %zu bytes, tagged union = %zu bytes\n",
           10000 * sizeof(struct packet_all), 10000 * sizeof(struct reading));
    printf("Pointer alternative: sizeof(struct reading_ptr) = %zu bytes (+ the malloc'd block)\n",
           sizeof(struct reading_ptr));
 
    printf("\n=== Misuse experiment (store float, read int) ===\n");
    memset(&r, 0, sizeof r);
    r.type = TEMPERATURE;
    r.value.celsius = 31.5f;                      /* store a temperature ... */
    printf("Stored celsius = %.1f\n", r.value.celsius);
    printf("Read  percent  = %d   (reading the WRONG member)\n", r.value.percent);
    printf("Same value in hex = 0x%08X\n", (unsigned)r.value.percent);
    memcpy(bytes, &r.value, sizeof bytes);
    printf("Raw union bytes   =");
    for (size_t i = 0; i < sizeof bytes; i++) printf(" %02X", bytes[i]);
    printf("\n");
 
    printf("\n=== Tagged-union packets ===\n");
    make_temperature(&r, 31.5f);  show(&r);
    make_humidity(&r, 68);        show(&r);
    make_status(&r, "OK");        show(&r);
 
    printf("\n=== Test cases ===\n");
    printf("TC01: temperature 31.5   -> %s\n", mk_text(make_temperature(&r, 31.5f)));
    printf("TC02: temperature -90.0  -> %s\n", mk_text(make_temperature(&r, -90.0f)));
    printf("TC03: temperature 60.0   -> %s\n", mk_text(make_temperature(&r, 60.0f)));
    printf("TC04: temperature 60.5   -> %s\n", mk_text(make_temperature(&r, 60.5f)));
    printf("TC05: humidity 0         -> %s\n", mk_text(make_humidity(&r, 0)));
    printf("TC06: humidity 100       -> %s\n", mk_text(make_humidity(&r, 100)));
    printf("TC07: humidity 101       -> %s\n", mk_text(make_humidity(&r, 101)));
    printf("TC08: humidity -1        -> %s\n", mk_text(make_humidity(&r, -1)));
    printf("TC09: status \"OK\"        -> %s\n", mk_text(make_status(&r, "OK")));
    printf("TC10: status \"ABCDEFG\"   -> %s\n", mk_text(make_status(&r, "ABCDEFG")));
    printf("TC11: status \"ABCDEFGH\"  -> %s\n", mk_text(make_status(&r, "ABCDEFGH")));
    printf("TC12: status \"\"          -> %s\n", mk_text(make_status(&r, "")));
    printf("TC13: NULL packet        -> %s\n", mk_text(make_humidity(NULL, 50)));
    r.type = (enum kind)99;
    printf("TC14: unknown tag 99     -> ");  show(&r);
    return 0;
}

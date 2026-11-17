/* ---------------------------------------------------------------------------
 * Task 6 - College canteen inventory and billing
 * Standard C (C99).   Compile : gcc -std=c99 -Wall -Wextra q6_canteen.c -o q6_canteen
 *
 * Data model : nested structures (Date, Time), an array of structures (Canteen.items)
 *              and a tagged union (Item.kind + Item.info) for packaged / cooked / combo.
 * "Today" is passed in as a Date so that the demonstration is repeatable.
 * ------------------------------------------------------------------------- */
#include <stdio.h>
#include <string.h>
 
#define MAX_ITEMS  50
#define MAX_COMBO   3
#define NAME_LEN   30
#define WARN_DAYS   2          /* warn when an item expires within this many days */
#define MAX_QTY  1000          /* largest quantity allowed in one sale            */
 
typedef struct { int day, month, year; } Date;
typedef struct { int hour, minute; }     Time;
 
typedef enum { PACKAGED = 0, COOKED = 1, COMBO = 2 } ItemKind;   /* the union's tag */
 
typedef struct {
    int      id;
    char     name[NAME_LEN];
    ItemKind kind;                       /* tag: which union member is valid        */
    float    price;
    int      stock;                      /* not used for COMBO (derived from parts) */
    int      units_sold;
    float    revenue;
    union {
        struct { Date expiry; }                 packaged;
        struct { Time prepared; }               cooked;
        struct { int count; int item_id[MAX_COMBO]; } combo;
    } info;
} Item;
 
typedef struct { Item items[MAX_ITEMS]; int count; } Canteen;   /* no global item array */
 
enum Status { OK = 0, ERR_NO_ITEM, ERR_QTY, ERR_STOCK, ERR_EXPIRED,
              ERR_FULL, ERR_BAD_DATA, ERR_DUP, ERR_NULL };
 
/* file-scope static: bill numbers must survive between calls but stay private to this file */
static int g_bill_no = 0;
 
static const char *status_text(int s)
{
    switch (s) {
    case OK:           return "OK";
    case ERR_NO_ITEM:  return "REJECTED: unknown item";
    case ERR_QTY:      return "REJECTED: invalid quantity";
    case ERR_STOCK:    return "REJECTED: insufficient stock";
    case ERR_EXPIRED:  return "REJECTED: item expired";
    case ERR_FULL:     return "REJECTED: catalogue full";
    case ERR_BAD_DATA: return "REJECTED: invalid item data";
    case ERR_DUP:      return "REJECTED: duplicate item id";
    default:           return "REJECTED: null pointer";
    }
}
 
/* ------------------------------ date helpers ------------------------------ */
static int is_leap(int y) { return (y % 4 == 0 && y % 100 != 0) || (y % 400 == 0); }
 
static int valid_date(Date d)
{
    static const int dm[] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
    int days;
    if (d.year < 2000 || d.year > 2100 || d.month < 1 || d.month > 12) return 0;
    days = (d.month == 2 && is_leap(d.year)) ? 29 : dm[d.month - 1];
    return d.day >= 1 && d.day <= days;
}
 
/* number of days since 1970-01-01, so that two dates can simply be subtracted */
static long day_number(Date d)
{
    long y = d.year, m = d.month;
    long era, yoe, doy, doe;
    if (m <= 2) y -= 1;
    era = (y >= 0 ? y : y - 399) / 400;
    yoe = y - era * 400;
    doy = (153 * (m + (m > 2 ? -3 : 9)) + 2) / 5 + d.day - 1;
    doe = yoe * 365 + yoe / 4 - yoe / 100 + doy;
    return era * 146097 + doe - 719468;
}
 
/* ------------------------------ item builders ----------------------------- */
static void copy_name(char *dst, const char *src)
{
    size_t n = strlen(src);
    if (n > NAME_LEN - 1) n = NAME_LEN - 1;
    memcpy(dst, src, n);
    dst[n] = '\0';
}
 
static Item make_packaged(int id, const char *name, float price, int stock, Date expiry)
{
    Item it;  memset(&it, 0, sizeof it);
    it.id = id; copy_name(it.name, name); it.kind = PACKAGED;
    it.price = price; it.stock = stock; it.info.packaged.expiry = expiry;
    return it;
}
static Item make_cooked(int id, const char *name, float price, int stock, int hh, int mm)
{
    Item it;  memset(&it, 0, sizeof it);
    it.id = id; copy_name(it.name, name); it.kind = COOKED;
    it.price = price; it.stock = stock;
    it.info.cooked.prepared.hour = hh; it.info.cooked.prepared.minute = mm;
    return it;
}
static Item make_combo(int id, const char *name, float price, int count, int a, int b, int c)
{
    Item it;  memset(&it, 0, sizeof it);
    it.id = id; copy_name(it.name, name); it.kind = COMBO; it.price = price;
    it.info.combo.count = count;
    it.info.combo.item_id[0] = a; it.info.combo.item_id[1] = b; it.info.combo.item_id[2] = c;
    return it;
}
 
/* ------------------------------ lookup helpers ---------------------------- */
static Item *find_item(Canteen *c, int id)
{
    int i;
    for (i = 0; i < c->count; i++)
        if (c->items[i].id == id) return &c->items[i];
    return NULL;
}
 
static int is_expired(const Item *it, Date today)
{
    return it->kind == PACKAGED && day_number(it->info.packaged.expiry) < day_number(today);
}
 
static int occurrences(const Item *combo, int id)
{
    int k, n = 0;
    for (k = 0; k < combo->info.combo.count; k++)
        if (combo->info.combo.item_id[k] == id) n++;
    return n;
}
 
/* ------------------------------ add an item ------------------------------- */
int add_item(Canteen *c, const Item *it)
{
    int k;
    if (c == NULL || it == NULL)          return ERR_NULL;
    if (c->count >= MAX_ITEMS)            return ERR_FULL;
    if (it->id <= 0 || find_item(c, it->id) != NULL) return ERR_DUP;
    if (it->name[0] == '\0' || it->price <= 0 || it->stock < 0) return ERR_BAD_DATA;
    switch (it->kind) {
    case PACKAGED:
        if (!valid_date(it->info.packaged.expiry)) return ERR_BAD_DATA;
        break;
    case COOKED:
        if (it->info.cooked.prepared.hour < 0 || it->info.cooked.prepared.hour > 23 ||
            it->info.cooked.prepared.minute < 0 || it->info.cooked.prepared.minute > 59)
            return ERR_BAD_DATA;
        break;
    case COMBO:
        if (it->info.combo.count < 1 || it->info.combo.count > MAX_COMBO) return ERR_BAD_DATA;
        for (k = 0; k < it->info.combo.count; k++) {
            Item *part = find_item(c, it->info.combo.item_id[k]);
            if (part == NULL || part->kind == COMBO) return ERR_BAD_DATA;  /* parts must exist, no nesting */
        }
        break;
    default:
        return ERR_BAD_DATA;
    }
    c->items[c->count++] = *it;
    return OK;
}
 
/* ------------------------------ record a sale ----------------------------- */
/* Phase 1 checks everything; phase 2 changes stock. A rejected sale changes nothing. */
static int check_supply(Canteen *c, const Item *it, int qty, Date today)
{
    int k;
    if (it->kind != COMBO) {
        if (is_expired(it, today)) return ERR_EXPIRED;
        return it->stock >= qty ? OK : ERR_STOCK;
    }
    for (k = 0; k < it->info.combo.count; k++) {
        Item *part = find_item(c, it->info.combo.item_id[k]);
        if (part == NULL)            return ERR_NO_ITEM;
        if (is_expired(part, today)) return ERR_EXPIRED;
    }
    for (k = 0; k < it->info.combo.count; k++) {
        Item *part = find_item(c, it->info.combo.item_id[k]);
        if (part->stock < qty * occurrences(it, part->id)) return ERR_STOCK;
    }
    return OK;
}
 
int record_sale(Canteen *c, int id, int qty, Date today, float *bill)
{
    Item *it;
    int st, k;
    if (c == NULL || bill == NULL)  return ERR_NULL;
    if (qty < 1 || qty > MAX_QTY)   return ERR_QTY;
    it = find_item(c, id);
    if (it == NULL)                 return ERR_NO_ITEM;
    st = check_supply(c, it, qty, today);
    if (st != OK)                   return st;           /* nothing has been changed yet */
 
    if (it->kind == COMBO) {
        for (k = 0; k < it->info.combo.count; k++)
            find_item(c, it->info.combo.item_id[k])->stock -= qty;
    } else {
        it->stock -= qty;
    }
    it->units_sold += qty;
    it->revenue    += it->price * qty;
    *bill = it->price * qty;
    g_bill_no++;
    return OK;
}
 
/* ------------------------------ expiry warning ---------------------------- */
int expiry_warning(const Canteen *c, Date today, int within_days)
{
    int i, warnings = 0;
    for (i = 0; i < c->count; i++) {
        const Item *it = &c->items[i];
        long left;
        if (it->kind != PACKAGED || it->stock == 0) continue;   /* only items still in stock */
        left = day_number(it->info.packaged.expiry) - day_number(today);
        if (left < 0) {
            printf("  WARNING: %s (id %d) EXPIRED %ld day(s) ago - remove %d unit(s)\n",
                   it->name, it->id, -left, it->stock);
            warnings++;
        } else if (left == 0) {
            printf("  WARNING: %s (id %d) expires TODAY - %d unit(s) in stock\n",
                   it->name, it->id, it->stock);
            warnings++;
        } else if (left <= within_days) {
            printf("  WARNING: %s (id %d) expires in %ld day(s) - %d unit(s) in stock\n",
                   it->name, it->id, left, it->stock);
            warnings++;
        }
    }
    return warnings;
}
 
/* ------------------------------ sorted bill summary ----------------------- */
/* Sorts an array of POINTERS (cheap to swap) by revenue, highest first; ties by id. */
static void sort_by_revenue(const Item *order[], int n)
{
    int i, j;
    for (i = 0; i < n - 1; i++) {
        int best = i;
        for (j = i + 1; j < n; j++)
            if (order[j]->revenue > order[best]->revenue ||
                (order[j]->revenue == order[best]->revenue && order[j]->id < order[best]->id))
                best = j;
        if (best != i) { const Item *t = order[i]; order[i] = order[best]; order[best] = t; }
    }
}
 
void print_summary(const Canteen *c)
{
    const Item *order[MAX_ITEMS];
    float total = 0;
    int i;
    for (i = 0; i < c->count; i++) order[i] = &c->items[i];
    sort_by_revenue(order, c->count);
    printf("  Rank  Id   Item           Units   Revenue (Rs)\n");
    for (i = 0; i < c->count; i++) {
        printf("  %-5d %-4d %-14s %5d   %10.2f\n", i + 1, order[i]->id, order[i]->name,
               order[i]->units_sold, order[i]->revenue);
        total += order[i]->revenue;
    }
    printf("  Total revenue = Rs %.2f\n", total);
}
 
static void print_catalogue(const Canteen *c)
{
    int i, k;
    for (i = 0; i < c->count; i++) {
        const Item *it = &c->items[i];
        printf("  %-4d %-14s %-9s Rs %6.2f  ", it->id, it->name,
               it->kind == PACKAGED ? "packaged" : it->kind == COOKED ? "cooked" : "combo", it->price);
        switch (it->kind) {
        case PACKAGED:
            printf("stock %3d  expiry %02d/%02d/%04d\n", it->stock, it->info.packaged.expiry.day,
                   it->info.packaged.expiry.month, it->info.packaged.expiry.year);
            break;
        case COOKED:
            printf("stock %3d  prepared %02d:%02d\n", it->stock, it->info.cooked.prepared.hour,
                   it->info.cooked.prepared.minute);
            break;
        case COMBO:
            printf("parts:");
            for (k = 0; k < it->info.combo.count; k++) printf(" %d", it->info.combo.item_id[k]);
            printf("\n");
            break;
        }
    }
}
 
static void sale(Canteen *c, int no, int id, int qty, Date today)
{
    float bill = 0;
    int st = record_sale(c, id, qty, today, &bill);
    if (st == OK)
        printf("Sale #%02d: item %d x%d -> OK, bill Rs %.2f (bill no. %d)\n", no, id, qty, bill, g_bill_no);
    else
        printf("Sale #%02d: item %d x%d -> %s\n", no, id, qty, status_text(st));
}
 
int main(void)
{
    static Canteen canteen;                      /* static: too big for the stack, zero-initialised */
    Date today = {10, 11, 2026};
    Item bad;
 
    printf("=== Canteen demonstration, today = %02d/%02d/%04d ===\n", today.day, today.month, today.year);
    printf("sizeof(Item) = %zu bytes, sizeof(Canteen) = %zu bytes (room for %d items)\n",
           sizeof(Item), sizeof(Canteen), MAX_ITEMS);
 
    /* ---- load the catalogue ---- */
    {
        Item list[9];
        int i;
        list[0] = make_packaged(101, "Biscuit Pack", 20, 50, (Date){11, 11, 2026});
        list[1] = make_packaged(102, "Chips Pack",   25, 10, (Date){30, 11, 2026});
        list[2] = make_packaged(103, "Juice Box",    35, 30, (Date){12, 11, 2026});
        list[3] = make_packaged(104, "Cake Slice",   45,  5, (Date){ 8, 11, 2026});
        list[4] = make_packaged(105, "Wafers",       12, 25, (Date){13, 11, 2026});
        list[5] = make_packaged(106, "Candy",        10,  8, (Date){10, 11, 2026});
        list[6] = make_cooked  (201, "Samosa",       15, 30, 9, 30);
        list[7] = make_cooked  (202, "Tea",          10, 60, 8, 0);
        list[8] = make_combo   (301, "Snack Combo",  40, 3, 201, 202, 101);
        for (i = 0; i < 9; i++) add_item(&canteen, &list[i]);
    }
    printf("\n--- Catalogue (%d items) ---\n", canteen.count);
    print_catalogue(&canteen);
 
    /* ---- item validation ---- */
    printf("\n--- Adding invalid items ---\n");
    printf("Add V1: NULL item pointer           -> %s\n", status_text(add_item(&canteen, NULL)));
    bad = make_packaged(110, "Free Item", 0, 5, (Date){1, 12, 2026});
    printf("Add V2: price 0                     -> %s\n", status_text(add_item(&canteen, &bad)));
    bad = make_packaged(111, "Old Pack", 10, 5, (Date){31, 2, 2026});
    printf("Add V3: expiry date 31/02/2026      -> %s\n", status_text(add_item(&canteen, &bad)));
    bad = make_combo(112, "Ghost Combo", 30, 2, 201, 999, 0);
    printf("Add V4: combo with unknown part 999 -> %s\n", status_text(add_item(&canteen, &bad)));
    bad = make_combo(113, "Combo of Combo", 60, 2, 301, 202, 0);
    printf("Add V5: combo containing a combo    -> %s\n", status_text(add_item(&canteen, &bad)));
    bad = make_packaged(114, "Minus Stock", 10, -3, (Date){1, 12, 2026});
    printf("Add V6: negative stock              -> %s\n", status_text(add_item(&canteen, &bad)));
    bad = make_cooked(115, "Night Snack", 10, 5, 25, 0);
    printf("Add V7: prepared time 25:00         -> %s\n", status_text(add_item(&canteen, &bad)));
    bad = make_combo(116, "Big Combo", 90, 4, 201, 202, 101);
    printf("Add V8: combo with 4 parts          -> %s\n", status_text(add_item(&canteen, &bad)));
    bad = make_packaged(101, "Biscuit Again", 20, 5, (Date){1, 12, 2026});
    printf("Add V9: duplicate id 101            -> %s\n", status_text(add_item(&canteen, &bad)));
    {
        static Canteen full;                     /* a second catalogue filled to its limit */
        int i;
        for (i = 1; i <= MAX_ITEMS; i++) {
            Item f = make_packaged(i, "Filler", 5, 1, (Date){1, 12, 2026});
            add_item(&full, &f);
        }
        bad = make_packaged(900, "One Too Many", 5, 1, (Date){1, 12, 2026});
        printf("Add V10: 51st item into a full catalogue -> %s\n", status_text(add_item(&full, &bad)));
    }
    printf("Catalogue size is still %d\n", canteen.count);
 
    /* ---- sales ---- */
    printf("\n--- Sales ---\n");
    sale(&canteen,  1, 101,    5, today);   /* normal packaged sale                  */
    sale(&canteen,  2, 102,    6, today);   /* normal; leaves 4 chips                */
    sale(&canteen,  3, 201,   12, today);   /* normal cooked sale                    */
    sale(&canteen,  4, 202,   20, today);
    sale(&canteen,  5, 301,    3, today);   /* combo: reduces three component stocks */
    sale(&canteen,  6, 103,   35, today);   /* exceeds stock (30)                    */
    sale(&canteen,  7, 103,   10, today);
    sale(&canteen,  8, 102,    4, today);   /* exactly the remaining stock           */
    sale(&canteen,  9, 102,    1, today);   /* nothing left                          */
    sale(&canteen, 10, 104,    1, today);   /* expired cake                          */
    sale(&canteen, 11, 101,    0, today);   /* zero quantity                         */
    sale(&canteen, 12, 101,   -2, today);   /* negative quantity                     */
    sale(&canteen, 13, 999,    1, today);   /* unknown item                          */
    sale(&canteen, 14, 301,   20, today);   /* combo larger than stock of a part     */
    {
        Item *s = find_item(&canteen, 201), *t = find_item(&canteen, 202), *b = find_item(&canteen, 101);
        printf("Stock after sale #14: samosa %d, tea %d, biscuit %d (no partial deduction)\n",
               s->stock, t->stock, b->stock);
    }
    sale(&canteen, 15, 106,    2, today);   /* expires today: still sellable         */
    sale(&canteen, 16, 101, 5000, today);   /* above MAX_QTY                         */
 
    /* ---- expiry warning ---- */
    printf("\n--- Expiry warning (within %d days of %02d/%02d/%04d) ---\n", WARN_DAYS, today.day, today.month, today.year);
    printf("Warnings raised: %d\n", expiry_warning(&canteen, today, WARN_DAYS));
 
    /* ---- summary ---- */
    printf("\n--- Day's bill summary, sorted by revenue ---\n");
    print_summary(&canteen);
    return 0;
}

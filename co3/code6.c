#include <stdio.h>
#include <string.h>
 
#define MAX_NAMES 500
#define NAME_LEN 30
 
void removeNewline(char s[]) {
    s[strcspn(s, "\n")] = '\0';
}
 
void sortNames(char names[][NAME_LEN], int n) {
    char key[NAME_LEN];
    for (int i = 1; i < n; i++) {
        strcpy(key, names[i]);
        int j = i - 1;
        while (j >= 0 && strcmp(names[j], key) > 0) {
            strcpy(names[j + 1], names[j]);
            j--;
        }
        strcpy(names[j + 1], key);
    }
}
 
int binarySearch(char names[][NAME_LEN], int n, const char target[]) {
    int low = 0, high = n - 1;
    while (low <= high) {
        int mid = low + (high - low) / 2;
        int result = strcmp(names[mid], target);
        if (result == 0) return mid;
        if (result < 0) low = mid + 1;
        else high = mid - 1;
    }
    return -1;
}
 
int main(void) {
    char names[MAX_NAMES][NAME_LEN];
    char target[NAME_LEN];
    int n;
 
    printf("Number of residents (1-%d): ", MAX_NAMES);
    if (scanf("%d", &n) != 1 || n < 1 || n > MAX_NAMES) {
        printf("Invalid count.\n");
        return 1;
    }
    getchar(); /* consume newline after number */
 
    for (int i = 0; i < n; i++) {
        printf("Name %d: ", i + 1);
        if (fgets(names[i], NAME_LEN, stdin) == NULL) {
            printf("Input error.\n");
            return 1;
        }
        removeNewline(names[i]);
    }
 
    sortNames(names, n);
    printf("\nSorted directory:\n");
    for (int i = 0; i < n; i++)
        printf("%s\n", names[i]);
 
    printf("\nDuplicate entries:\n");
    int foundDuplicate = 0;
    for (int i = 1; i < n; i++) {
        if (strcmp(names[i - 1], names[i]) == 0) {
            printf("%s\n", names[i]);
            foundDuplicate = 1;
        }
    }
    if (!foundDuplicate) printf("None\n");
 
    printf("\nSearch name: ");
    if (fgets(target, NAME_LEN, stdin) != NULL) {
        removeNewline(target);
        int pos = binarySearch(names, n, target);
        if (pos >= 0) printf("Found at sorted index %d\n", pos);
        else printf("Name not found\n");
    }
    return 0;
}


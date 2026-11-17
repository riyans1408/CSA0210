#include <stdio.h>
#define DISTRICTS 38
#define MONTHS 12
 
void districtTotals(const float rain[DISTRICTS][MONTHS],
                    float totals[DISTRICTS]) {
    for (int i = 0; i < DISTRICTS; i++) {
        totals[i] = 0;
        for (int j = 0; j < MONTHS; j++)
            totals[i] += rain[i][j];
    }
}
 
void monthlyAverages(const float rain[DISTRICTS][MONTHS],
                     float avg[MONTHS]) {
    for (int j = 0; j < MONTHS; j++) {
        float sum = 0;
        for (int i = 0; i < DISTRICTS; i++)
            sum += rain[i][j];
        avg[j] = sum / DISTRICTS;
    }
}
 
void findMaximum(const float rain[DISTRICTS][MONTHS],
                 float *maximum, int *district, int *month) {
    *maximum = rain[0][0];
    *district = 0; *month = 0;
    for (int i = 0; i < DISTRICTS; i++)
        for (int j = 0; j < MONTHS; j++)
            if (rain[i][j] > *maximum) {
                *maximum = rain[i][j];
                *district = i; *month = j;
            }
}
 
int main(void) {
    float rain[DISTRICTS][MONTHS];
    float totals[DISTRICTS], averages[MONTHS];
    float totalAll = 0, maximum;
    int maxDistrict, maxMonth;
 
    /* Demonstration data; replace with user input for actual records. */
    for (int i = 0; i < DISTRICTS; i++)
        for (int j = 0; j < MONTHS; j++)
            rain[i][j] = (float)((i * 17 + j * 23) % 250);
 
    districtTotals(rain, totals);
    monthlyAverages(rain, averages);
    findMaximum(rain, &maximum, &maxDistrict, &maxMonth);
 
    for (int i = 0; i < DISTRICTS; i++) totalAll += totals[i];
 
    printf("Annual district totals:\n");
    for (int i = 0; i < DISTRICTS; i++)
        printf("District %d: %.1f mm\n", i + 1, totals[i]);
 
    printf("\nMonthly state averages:\n");
    for (int j = 0; j < MONTHS; j++)
        printf("Month %d: %.2f mm\n", j + 1, averages[j]);
 
    printf("\nWettest: %.1f mm at district %d, month %d\n",
           maximum, maxDistrict + 1, maxMonth + 1);
 
    printf("Deficient districts (below 60%% of mean annual total):\n");
    for (int i = 0; i < DISTRICTS; i++)
        if (totals[i] < 0.60f * (totalAll / DISTRICTS))
            printf("%d ", i + 1);
    printf("\n");
    return 0;
}

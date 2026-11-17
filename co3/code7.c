#include <stdio.h>
 
#define MAX_CANDIDATES 12
#define MAX_VOTES 500000
 
typedef struct {
    int candidate;
    int votes;
} CandidateResult;
 
void sortResults(CandidateResult a[], int n) {
    for (int i = 0; i < n - 1; i++) {
        int best = i;
        for (int j = i + 1; j < n; j++) {
            if (a[j].votes > a[best].votes ||
                (a[j].votes == a[best].votes &&
                 a[j].candidate < a[best].candidate))
                best = j;
        }
        if (best != i) {
            CandidateResult temp = a[i];
            a[i] = a[best];
            a[best] = temp;
        }
    }
}
 
int main(void) {
    int candidateCount, voteCount;
    int counts[MAX_CANDIDATES] = {0};
    int valid = 0, invalid = 0;
 
    printf("Enter number of candidates (1-%d): ", MAX_CANDIDATES);
    if (scanf("%d", &candidateCount) != 1 ||
        candidateCount < 1 || candidateCount > MAX_CANDIDATES) {
        printf("Invalid candidate count.\n");
        return 1;
    }
 
    printf("Enter number of votes (0-%d): ", MAX_VOTES);
    if (scanf("%d", &voteCount) != 1 ||
        voteCount < 0 || voteCount > MAX_VOTES) {
        printf("Invalid vote count.\n");
        return 1;
    }
 
    for (int i = 0; i < voteCount; i++) {
        int x;
        printf("Vote %d: ", i + 1);
        if (scanf("%d", &x) != 1) {
            printf("Invalid input format.\n");
            return 1;
        }
        if (x >= 1 && x <= candidateCount) {
            counts[x - 1]++;
            valid++;
        } else {
            invalid++;
        }
    }
 
    CandidateResult result[MAX_CANDIDATES];
    for (int i = 0; i < candidateCount; i++) {
        result[i].candidate = i + 1;
        result[i].votes = counts[i];
    }
 
    sortResults(result, candidateCount);
 
    printf("\nValid votes: %d\nInvalid votes: %d\n",
           valid, invalid);
    printf("\nResults (descending):\n");
    for (int i = 0; i < candidateCount; i++)
        printf("Candidate %d: %d votes\n",
               result[i].candidate, result[i].votes);
 
    if (candidateCount > 0) {
        int highest = result[0].votes;
        int tied = 0;
        for (int i = 1; i < candidateCount; i++)
            if (result[i].votes == highest) tied = 1;
 
        if (tied)
            printf("Result: Tie for the highest vote count (%d).\n",
                   highest);
        else
            printf("Winner: Candidate %d with %d votes.\n",
                   result[0].candidate, highest);
    }
    return 0;
}


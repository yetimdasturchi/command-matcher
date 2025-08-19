#include "jaro.h"

double jaro_distance(char *s1, char *s2) {
    int len1 = strlen(s1);
    int len2 = strlen(s2);

    if (len1 == 0 && len2 == 0) return 1.0;

    int matchDistance = (len1 > len2) ? len1 / 2 - 1 : len2 / 2 - 1;
    bool s1Matches[len1];
    bool s2Matches[len2];
    memset(s1Matches, false, sizeof(s1Matches));
    memset(s2Matches, false, sizeof(s2Matches));

    int matches = 0;
    for (int i = 0; i < len1; i++) {
        int start = (i >= matchDistance) ? i - matchDistance : 0;
        int end = (i + matchDistance <= len2 - 1) ? i + matchDistance : len2 - 1;
        for (int j = start; j <= end; j++) {
            if (!s2Matches[j] && s1[i] == s2[j]) {
                s1Matches[i] = true;
                s2Matches[j] = true;
                matches++;
                break;
            }
        }
    }

    if (matches == 0) return 0.0;

    double transpositions = 0;
    int k = 0;
    for (int i = 0; i < len1; i++) {
        if (s1Matches[i]) {
            while (!s2Matches[k]) k++;
            if (s1[i] != s2[k]) transpositions++;
            k++;
        }
    }

    double jaroSimilarity = (double)matches / len1;
    double jaroDistance = (jaroSimilarity + (matches - transpositions / 2) / matches) / 2;
    return jaroDistance;
}

double jaro_winkler_distance(char *s1, char *s2, double p) {
    double jaroDistance = jaro_distance(s1, s2);

    int prefixLength = 0;
    int maxPrefixLength = (p < 0.25) ? (int)(p * 4) : 4;
    while (prefixLength < maxPrefixLength && s1[prefixLength] == s2[prefixLength]) {
        prefixLength++;
    }

    double jaroWinklerDistance = jaroDistance + (prefixLength * 0.1 * (1 - jaroDistance));
    return jaroWinklerDistance;
}
#include "similarity.h"

void similar_str(const char *txt1, int len1, const char *txt2, int len2, int *pos1, int *pos2, int *max, int *count) {
    *max = 0;
    *count = 0;
    for (int i = 0; i < len1; i++) {
        for (int j = 0; j < len2; j++) {
            int l = 0;
            
            while ((i + l < len1) && (j + l < len2) && (txt1[i + l] == txt2[j + l])) {
                l++;
            }
            
            if (l > *max) {
                *max = l;
                *count += 1;
                *pos1 = i;
                *pos2 = j;
            }
        }
    }
}

int similar_char(const char *txt1, int len1, const char *txt2, int len2) {
    int sum = 0;
    int pos1 = 0;
    int pos2 = 0;
    int max = 0;
    int count = 0;

    similar_str(txt1, len1, txt2, len2, &pos1, &pos2, &max, &count);

    if ((sum = max)) {
        if (pos1 && pos2 && count > 1) {
            char *subTxt1 = (char *)malloc(pos1 + 1);
            strncpy(subTxt1, txt1, pos1);
            subTxt1[pos1] = '\0';

            char *subTxt2 = (char *)malloc(pos2 + 1);
            strncpy(subTxt2, txt2, pos2);
            subTxt2[pos2] = '\0';

            sum += similar_char(subTxt1, pos1, subTxt2, pos2);

            free(subTxt1);
            free(subTxt2);
        }

        if ((pos1 + max < len1) && (pos2 + max < len2)) {
            char *subTxt1 = (char *)malloc(len1 - pos1 - max + 1);
            strncpy(subTxt1, txt1 + pos1 + max, len1 - pos1 - max);
            subTxt1[len1 - pos1 - max] = '\0';

            char *subTxt2 = (char *)malloc(len2 - pos2 - max + 1);
            strncpy(subTxt2, txt2 + pos2 + max, len2 - pos2 - max);
            subTxt2[len2 - pos2 - max] = '\0';

            sum += similar_char(subTxt1, len1 - pos1 - max, subTxt2, len2 - pos2 - max);

            free(subTxt1);
            free(subTxt2);
        }
    }

    return sum;
}
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "coach.h"

int ajouter_coach(char *filename, coach c) {
    FILE *f = fopen(filename, "a");
    if (f != NULL) {
        fprintf(f, "%s|%s|%s|%s|%s|%s\n", c.firstname, c.lastname, c.gender, c.speciality, c.level, c.status);
        fclose(f);
        return 1;
    }
    return 0;
}

int modifier_coach(char *filename, char *firstname, char *lastname, coach nouv) {
    int tr = 0;
    coach c;
    FILE *f = fopen(filename, "r");
    FILE *f2 = fopen("nouv_coach.txt", "w");
    if (f != NULL && f2 != NULL) {
        while (fscanf(f, "%[^|]|%[^|]|%[^|]|%[^|]|%[^|]|%[^\n]\n", c.firstname, c.lastname, c.gender, c.speciality, c.level, c.status) != EOF) {
            if (strcmp(c.firstname, firstname) == 0 && strcmp(c.lastname, lastname) == 0) {
                fprintf(f2, "%s|%s|%s|%s|%s|%s\n", nouv.firstname, nouv.lastname, nouv.gender, nouv.speciality, nouv.level, nouv.status);
                tr = 1;
            } else {
                fprintf(f2, "%s|%s|%s|%s|%s|%s\n", c.firstname, c.lastname, c.gender, c.speciality, c.level, c.status);
            }
        }
        fclose(f);
        fclose(f2);
        remove(filename);
        rename("nouv_coach.txt", filename);
        return tr;
    }
    return 0;
}

int supprimer_coach(char *filename, char *firstname, char *lastname) {
    int tr = 0;
    coach c;
    FILE *f = fopen(filename, "r");
    FILE *f2 = fopen("nouv_coach.txt", "w");
    if (f != NULL && f2 != NULL) {
        while (fscanf(f, "%[^|]|%[^|]|%[^|]|%[^|]|%[^|]|%[^\n]\n", c.firstname, c.lastname, c.gender, c.speciality, c.level, c.status) != EOF) {
            if (strcmp(c.firstname, firstname) == 0 && strcmp(c.lastname, lastname) == 0) {
                tr = 1;
            } else {
                fprintf(f2, "%s|%s|%s|%s|%s|%s\n", c.firstname, c.lastname, c.gender, c.speciality, c.level, c.status);
            }
        }
        fclose(f);
        fclose(f2);
        remove(filename);
        rename("nouv_coach.txt", filename);
        return tr;
    }
    return 0;
}

coach chercher_coach(char *filename, char *firstname, char *lastname) {
    coach c;
    int tr = 0;
    FILE *f = fopen(filename, "r");
    if (f != NULL) {
        while (fscanf(f, "%[^|]|%[^|]|%[^|]|%[^|]|%[^|]|%[^\n]\n", c.firstname, c.lastname, c.gender, c.speciality, c.level, c.status) != EOF) {
            if (strcmp(c.firstname, firstname) == 0 && strcmp(c.lastname, lastname) == 0) {
                tr = 1;
                break;
            }
        }
        fclose(f);
    }
    if (tr == 0) {
        strcpy(c.firstname, "-1");
    }
    return c;
}

int coach_exists(char *filename, char *firstname, char *lastname) {
    coach c = chercher_coach(filename, firstname, lastname);
    if (strcmp(c.firstname, "-1") != 0) {
        return 1;
    }
    return 0;
}

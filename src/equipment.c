#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "equipment.h"

int ajouter_equipment(char *filename, equipment eq) {
    FILE *f = fopen(filename, "a");
    if (f != NULL) {
        fprintf(f, "%d|%s|%s|%d|%s\n", eq.id, eq.name, eq.category, eq.quantity, eq.status);
        fclose(f);
        return 1;
    }
    return 0;
}

int modifier_equipment(char *filename, int id, equipment nouv) {
    int tr = 0;
    equipment eq;
    FILE *f = fopen(filename, "r");
    FILE *f2 = fopen("nouv_equipment.txt", "w");
    if (f != NULL && f2 != NULL) {
        while (fscanf(f, "%d|%[^|]|%[^|]|%d|%[^\n]\n", &eq.id, eq.name, eq.category, &eq.quantity, eq.status) != EOF) {
            if (eq.id == id) {
                fprintf(f2, "%d|%s|%s|%d|%s\n", nouv.id, nouv.name, nouv.category, nouv.quantity, nouv.status);
                tr = 1;
            } else {
                fprintf(f2, "%d|%s|%s|%d|%s\n", eq.id, eq.name, eq.category, eq.quantity, eq.status);
            }
        }
        fclose(f);
        fclose(f2);
        remove(filename);
        rename("nouv_equipment.txt", filename);
        return tr;
    }
    return 0;
}

int supprimer_equipment(char *filename, int id) {
    int tr = 0;
    equipment eq;
    FILE *f = fopen(filename, "r");
    FILE *f2 = fopen("nouv_equipment.txt", "w");
    if (f != NULL && f2 != NULL) {
        while (fscanf(f, "%d|%[^|]|%[^|]|%d|%[^\n]\n", &eq.id, eq.name, eq.category, &eq.quantity, eq.status) != EOF) {
            if (eq.id == id) {
                tr = 1;
            } else {
                fprintf(f2, "%d|%s|%s|%d|%s\n", eq.id, eq.name, eq.category, eq.quantity, eq.status);
            }
        }
        fclose(f);
        fclose(f2);
        remove(filename);
        rename("nouv_equipment.txt", filename);
        return tr;
    }
    return 0;
}

equipment chercher_equipment(char *filename, int id) {
    equipment eq;
    int tr = 0;
    FILE *f = fopen(filename, "r");
    if (f != NULL) {
        while (fscanf(f, "%d|%[^|]|%[^|]|%d|%[^\n]\n", &eq.id, eq.name, eq.category, &eq.quantity, eq.status) != EOF) {
            if (eq.id == id) {
                tr = 1;
                break;
            }
        }
        fclose(f);
    }
    if (tr == 0) {
        eq.id = -1;
    }
    return eq;
}

int equipment_exists(char *filename, int id) {
    equipment eq = chercher_equipment(filename, id);
    if (eq.id != -1) {
        return 1;
    }
    return 0;
}

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "event.h"

int ajouter_event(char *filename, event e) {
    FILE *f = fopen(filename, "a");
    if (f != NULL) {
        fprintf(f, "%d|%s|%s|%s|%s|%s\n", e.id, e.name, e.date, e.time, e.location, e.description);
        fclose(f);
        return 1;
    }
    return 0;
}

int modifier_event(char *filename, int id, event nouv) {
    int tr = 0;
    event e;
    FILE *f = fopen(filename, "r");
    FILE *f2 = fopen("nouv_event.txt", "w");
    if (f != NULL && f2 != NULL) {
        while (fscanf(f, "%d|%[^|]|%[^|]|%[^|]|%[^|]|%[^\n]\n", &e.id, e.name, e.date, e.time, e.location, e.description) != EOF) {
            if (e.id == id) {
                fprintf(f2, "%d|%s|%s|%s|%s|%s\n", nouv.id, nouv.name, nouv.date, nouv.time, nouv.location, nouv.description);
                tr = 1;
            } else {
                fprintf(f2, "%d|%s|%s|%s|%s|%s\n", e.id, e.name, e.date, e.time, e.location, e.description);
            }
        }
        fclose(f);
        fclose(f2);
        remove(filename);
        rename("nouv_event.txt", filename);
        return tr;
    }
    return 0;
}

int supprimer_event(char *filename, int id) {
    int tr = 0;
    event e;
    FILE *f = fopen(filename, "r");
    FILE *f2 = fopen("nouv_event.txt", "w");
    if (f != NULL && f2 != NULL) {
        while (fscanf(f, "%d|%[^|]|%[^|]|%[^|]|%[^|]|%[^\n]\n", &e.id, e.name, e.date, e.time, e.location, e.description) != EOF) {
            if (e.id == id) {
                tr = 1;
            } else {
                fprintf(f2, "%d|%s|%s|%s|%s|%s\n", e.id, e.name, e.date, e.time, e.location, e.description);
            }
        }
        fclose(f);
        fclose(f2);
        remove(filename);
        rename("nouv_event.txt", filename);
        return tr;
    }
    return 0;
}

event chercher_event(char *filename, int id) {
    event e;
    int tr = 0;
    FILE *f = fopen(filename, "r");
    if (f != NULL) {
        while (fscanf(f, "%d|%[^|]|%[^|]|%[^|]|%[^|]|%[^\n]\n", &e.id, e.name, e.date, e.time, e.location, e.description) != EOF) {
            if (e.id == id) {
                tr = 1;
                break;
            }
        }
        fclose(f);
    }
    if (tr == 0) {
        e.id = -1;
    }
    return e;
}

int event_exists(char *filename, int id) {
    event e = chercher_event(filename, id);
    if (e.id != -1) {
        return 1;
    }
    return 0;
}

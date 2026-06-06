#ifndef EQUIPMENT_H
#define EQUIPMENT_H

typedef struct {
    int id;
    char name[50];
    char category[50];
    int quantity;
    char status[50];
} equipment;

// Function prototypes
int ajouter_equipment(char *filename, equipment eq);
int modifier_equipment(char *filename, int id, equipment nouv);
int supprimer_equipment(char *filename, int id);
equipment chercher_equipment(char *filename, int id);
int equipment_exists(char *filename, int id);

#endif // EQUIPMENT_H

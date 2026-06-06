#ifndef COACH_H
#define COACH_H

typedef struct {
    char firstname[50];
    char lastname[50];
    char gender[20];
    char speciality[50];
    char level[50];
    char status[50];
} coach;

// Function prototypes
int ajouter_coach(char *filename, coach c);
int modifier_coach(char *filename, char *firstname, char *lastname, coach nouv);
int supprimer_coach(char *filename, char *firstname, char *lastname);
coach chercher_coach(char *filename, char *firstname, char *lastname);
int coach_exists(char *filename, char *firstname, char *lastname);

#endif // COACH_H

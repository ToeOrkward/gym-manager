// course.h
#ifndef COURSE_H
#define COURSE_H

typedef struct {
    int id;
    char typecourse[50];
    char level[50];
    char gender[20];
    char date[20];
    int hour;
    int duration;
    char status[20];
    char rescoach[50];
    int room;
} course;

// Function prototypes
int ajouterEW(char *filename, course c);
int modifierEW(char *filename, int id, course nouv);
int supprimerEW(char *filename, int id);
course chercherEW(char *filename, int id);
int chercher_multi_criteriaEW(char *filename, int search_id, char *search_gender, course results[], int max_results);
int restaurerEW(char *restore_filename, char *main_filename, int id);
int course_id_exists(char *filename, int id);

#endif

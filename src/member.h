#ifndef MEMBER_H
#define MEMBER_H

#include <stdio.h>

typedef struct
{
    char firstname[30];    
    char lastname[30];
    char phonenumber[20];
    char gender[10];
    int cin;
    char startdate[20];
    float amountpaid;
    char username[30];
    char password[30];
    char monthsub[20];     
} member;
typedef struct{
char cname[35];
char cid[10];
char cemail[60];
int cphone_int;
char cadress[30];
int cnemp;
}center;

int ajouter(char *filename, member m);
int modifier(char *filename, int cin, member nouv);
int supprimer(char *filename, int cin);
member chercher(char *filename, int cin);
int chercher_multi_criteria(char *filename, int search_cin, char *search_phone, 
                           char *search_gender, member results[], int max_results);
int restaurer(char *restore_filename, char *main_filename, int cin);
int calculate_active_today();
int cin_exists(char *filename, int cin); 
int addcenter(center c);
int editcenter(char * filename, char * id, center nc);
int deletecenter(char * filename, char * id);
center searchcenter(char * filename, char * id);
int total_centres();
int total_employes();
center centre_max_employes();
center centre_min_employes();
void distribution_employes(int *moins_10, int *entre_10_20, int *plus_20);

//Eya
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
#endif

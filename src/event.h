#ifndef EVENT_H
#define EVENT_H

typedef struct {
    int id;
    char name[50];
    char date[20];
    char time[20];
    char location[50];
    char description[100];
} event;

// Function prototypes
int ajouter_event(char *filename, event e);
int modifier_event(char *filename, int id, event nouv);
int supprimer_event(char *filename, int id);
event chercher_event(char *filename, int id);
int event_exists(char *filename, int id);

#endif // EVENT_H

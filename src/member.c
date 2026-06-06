#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "member.h"

int ajouter(char *filename, member m)
{
    FILE *f = fopen(filename, "a");
    if (!f) return 0;
    
    // Save with PIPE separator
    fprintf(f, "%s|%s|%s|%s|%d|%s|%.2f|%s|%s|%s\n", 
            m.firstname, m.lastname, m.phonenumber, m.gender,
            m.cin, m.startdate, m.amountpaid, m.username, 
            m.password, m.monthsub);
    
    fclose(f);
    return 1;
}

int modifier(char *filename, int cin, member nouv)
{
    int tr = 0;
    FILE *f = fopen(filename, "r");
    FILE *f2 = fopen("nouv.txt", "w");
    char line[256];
    
    if(f == NULL || f2 == NULL) {
        if(f) fclose(f);
        if(f2) fclose(f2);
        return 0;
    }
    
    while(fgets(line, sizeof(line), f)) {
        // Remove newline
        line[strcspn(line, "\n")] = 0;
        
        // Parse with pipe separator
        member m;
        if (sscanf(line, "%[^|]|%[^|]|%[^|]|%[^|]|%d|%[^|]|%f|%[^|]|%[^|]|%[^\n]",
                   m.firstname, m.lastname, m.phonenumber, m.gender, 
                   &m.cin, m.startdate, &m.amountpaid, m.username, 
                   m.password, m.monthsub) == 10) {
            
            if(m.cin == cin) {
                // Write UPDATED member in PIPE format
                fprintf(f2, "%s|%s|%s|%s|%d|%s|%.2f|%s|%s|%s\n", 
                        nouv.firstname,
                        nouv.lastname,
                        nouv.phonenumber,
                        nouv.gender,
                        nouv.cin,
                        nouv.startdate,
                        nouv.amountpaid,
                        nouv.username,
                        nouv.password,
                        nouv.monthsub);
                tr = 1;
            } else {
                // Write UNCHANGED member (original line)
                fprintf(f2, "%s\n", line);
            }
        }
    }
    
    fclose(f);
    fclose(f2);
    
    if(tr) {
        remove(filename);
        rename("nouv.txt", filename);
    } else {
        remove("nouv.txt");
    }
    
    return tr;
}

#include <string.h>  // Add this at top if not already

int supprimer(char *filename, int cin)
{
    int tr = 0;
    FILE *f = fopen(filename, "r");
    FILE *f2 = fopen("nouv.txt", "w");
    FILE *restore_file = fopen("restore.txt", "a");  // Open restore file in append mode
    char line[256];
    
    if(f == NULL || f2 == NULL) {
        if(f) fclose(f);
        if(f2) fclose(f2);
        if(restore_file) fclose(restore_file);
        return 0;
    }
    
    if(restore_file == NULL) {
        fclose(f);
        fclose(f2);
        return 0;
    }
    
    while(fgets(line, sizeof(line), f)) {
        // Remove newline
        line[strcspn(line, "\n")] = 0;
        
        // Parse with pipe separator
        member m;
        if (sscanf(line, "%[^|]|%[^|]|%[^|]|%[^|]|%d|%[^|]|%f|%[^|]|%[^|]|%[^\n]",
                   m.firstname, m.lastname, m.phonenumber, m.gender, 
                   &m.cin, m.startdate, &m.amountpaid, m.username, 
                   m.password, m.monthsub) == 10) {
            
            if(m.cin == cin) {
                // Save deleted member to restore.txt
                fprintf(restore_file, "%s|%s|%s|%s|%d|%s|%.2f|%s|%s|%s\n", 
                        m.firstname, m.lastname, m.phonenumber, m.gender,
                        m.cin, m.startdate, m.amountpaid, m.username,
                        m.password, m.monthsub);
                tr = 1;
            } else {
                // Keep member in new file
                fprintf(f2, "%s\n", line);
            }
        }
    }
    
    fclose(f);
    fclose(f2);
    fclose(restore_file);
    
    if(tr) {
        remove(filename);
        rename("nouv.txt", filename);
    } else {
        remove("nouv.txt");
    }
    
    return tr;
}
member chercher(char *filename, int cin)
{
    member m;
    int tr = 0;
    
    FILE *f = fopen(filename, "r");
    if(f == NULL) {
        m.cin = -1;
        return m;
    }
    
    while(tr == 0 && fscanf(f, "%s %s %s %s %d %s %f %s %s %s",
                       m.firstname,
                       m.lastname,
                       m.phonenumber,
                       m.gender,
                       &m.cin,
                       m.startdate,
                       &m.amountpaid,
                       m.username,
                       m.password,
                       m.monthsub) != EOF)
    {
        if(m.cin == cin) {
            tr = 1;
        }
    }
    
    fclose(f);
    
    if(tr == 0) {
        m.cin = -1;
    }
    
    return m;
}
#include <string.h>  // Add this at the top
#include <ctype.h>   // Add this for tolower()

// Flexible search function
int chercher_multi_criteria(char *filename, 
                           int search_cin, 
                           char *search_phone, 
                           char *search_gender, 
                           member results[], 
                           int max_results)
{
    FILE *f = fopen(filename, "r");
    char line[256];
    member m;
    int count = 0;
    
    if (!f) return 0;
    
    while (fgets(line, sizeof(line), f) && count < max_results) {
        line[strcspn(line, "\n")] = 0;
        
        if (sscanf(line, "%[^|]|%[^|]|%[^|]|%[^|]|%d|%[^|]|%f|%[^|]|%[^|]|%[^\n]",
                   m.firstname, m.lastname, m.phonenumber, m.gender, 
                   &m.cin, m.startdate, &m.amountpaid, m.username, 
                   m.password, m.monthsub) == 10) {
            
            int match = 1; // Assume match
            
            // Search by CIN (if provided and not 0)
            if (search_cin != 0 && m.cin != search_cin) {
                match = 0;
 		
            }
            
            // Search by phone (if provided and not empty)
            if (search_phone != NULL && strlen(search_phone) > 0) {
                if (strstr(m.phonenumber, search_phone) == NULL) {
                    match = 0;
                }
            }
            
            // Search by gender (if provided and not empty)
            if (search_gender != NULL && strlen(search_gender) > 0  ) {
                // Case-insensitive comparison
                char gender1[20], gender2[20];
                strncpy(gender1, m.gender, sizeof(gender1)-1);
                gender1[sizeof(gender1)-1] = '\0';
                strncpy(gender2, search_gender, sizeof(gender2)-1);
                gender2[sizeof(gender2)-1] = '\0';
                
                // Convert both to lowercase
                for (int i = 0; gender1[i]; i++) gender1[i] = tolower(gender1[i]);
                for (int i = 0; gender2[i]; i++) gender2[i] = tolower(gender2[i]);
                
                if (strcmp(gender1, gender2) != 0 ) {
                    match = 0;
                }
            }
            
            // If all criteria match, add to results
            if (match) {
                results[count] = m;
                count++;
            }
        }
    }
    
    fclose(f);
    return count;
}
int restaurer(char *restore_filename, char *main_filename, int cin)
{
    FILE *restore_f = fopen(restore_filename, "r");
    FILE *temp_f = fopen("temp_restore.txt", "w");
    FILE *main_f = fopen(main_filename, "a");  // Append to main file
    char line[256];
    member m;
    int found = 0;
    
    if(!restore_f || !temp_f || !main_f) {
        if(restore_f) fclose(restore_f);
        if(temp_f) fclose(temp_f);
        if(main_f) fclose(main_f);
        return 0;
    }
    
    while(fgets(line, sizeof(line), restore_f)) {
        line[strcspn(line, "\n")] = 0;
        
        if (sscanf(line, "%[^|]|%[^|]|%[^|]|%[^|]|%d|%[^|]|%f|%[^|]|%[^|]|%[^\n]",
                   m.firstname, m.lastname, m.phonenumber, m.gender, 
                   &m.cin, m.startdate, &m.amountpaid, m.username, 
                   m.password, m.monthsub) == 10) {
            
            if(m.cin == cin) {
                // Write to main file
                fprintf(main_f, "%s|%s|%s|%s|%d|%s|%.2f|%s|%s|%s\n",
                        m.firstname, m.lastname, m.phonenumber, m.gender,
                        m.cin, m.startdate, m.amountpaid, m.username,
                        m.password, m.monthsub);
                found = 1;
            } else {
                // Keep in restore file
                fprintf(temp_f, "%s\n", line);
            }
        }
    }
    
    fclose(restore_f);
    fclose(temp_f);
    fclose(main_f);
    
    if(found) {
        remove(restore_filename);
        rename("temp_restore.txt", restore_filename);
        return 1;
    } else {
        remove("temp_restore.txt");
        return 0;
    }
}

int calculate_active_today() {
    FILE *file = fopen("doc.txt", "r");
    int count = 0;
    char line[256];
    
    if (!file) return 0;
    
    while (fgets(line, sizeof(line), file)) {
        count++; // Count all members as active (simplified)
    }
    
    fclose(file);
    return (int)(count * 0.7); // Return 70% as active
}
// Add to member.c
int cin_exists(char *filename, int cin)
{
    FILE *f = fopen(filename, "r");
    char line[256];
    member m;
    
    if (!f) return 0;  // File doesn't exist = no duplicates
    
    while (fgets(line, sizeof(line), f)) {
        line[strcspn(line, "\n")] = 0;
        
        if (sscanf(line, "%[^|]|%[^|]|%[^|]|%[^|]|%d|%[^|]|%f|%[^|]|%[^|]|%[^\n]",
                   m.firstname, m.lastname, m.phonenumber, m.gender, 
                   &m.cin, m.startdate, &m.amountpaid, m.username, 
                   m.password, m.monthsub) == 10) {
            
            if (m.cin == cin) {
                fclose(f);
                return 1;  // CIN exists
            }
        }
    }
    
    fclose(f);
    return 0;  // CIN doesn't exist
}
//Wissem

#define CENTRE_FILE "/home/wissem/Desktop/Abschlussprojekt (another copy)/Integrated project/centre.txt"
#define CENTRE_FILE2 "/home/wissem/Desktop/Abschlussprojekt (another copy)/Integrated project/centre2.txt"

/* ===================== AJOUT ===================== */
int addcenter(center c) {
    FILE *f = fopen(CENTRE_FILE, "a");
    if (f != NULL) {
        fprintf(f, "%s %s %s %s %d %d\n", 
                c.cid, c.cname, c.cadress, c.cemail, c.cphone_int, c.cnemp);
        fclose(f);
        return 1;
    } 
    return 0;
}

/* ===================== MODIFICATION ===================== */
int editcenter(char *filename, char *id, center nc) {
    int tr = 0;
    center c;
    FILE *f = fopen(CENTRE_FILE, "r");
    FILE *f2 = fopen(CENTRE_FILE2, "w");
    if (f != NULL && f2 != NULL) {
        while (fscanf(f, "%s %s %s %s %d %d\n", 
                      c.cid, c.cname, c.cadress, c.cemail, &c.cphone_int, &c.cnemp) != EOF) {
            if (strcmp(c.cid, id) == 0) {
                fprintf(f2, "%s %s %s %s %d %d\n",
                        nc.cid, nc.cname, nc.cadress, nc.cemail, nc.cphone_int, nc.cnemp);
                tr = 1;
            } else {
                fprintf(f2, "%s %s %s %s %d %d\n",
                        c.cid, c.cname, c.cadress, c.cemail, c.cphone_int, c.cnemp);
            }
        }
        fclose(f);
        fclose(f2);
        remove(CENTRE_FILE);
        rename(CENTRE_FILE2, CENTRE_FILE);
    }
    return tr;
}

/* ===================== SUPPRESSION ===================== */
int deletecenter(char *filename, char *id) {
    int tr = 0;
    center c;
    FILE *f = fopen(CENTRE_FILE, "r");
    FILE *f2 = fopen(CENTRE_FILE2, "w");
    if (f != NULL && f2 != NULL) {
        while (fscanf(f, "%s %s %s %s %d %d\n", 
                      c.cid, c.cname, c.cadress, c.cemail, &c.cphone_int, &c.cnemp) == 6) {
            if (strcmp(c.cid, id) == 0)
                tr = 1;
            else
                fprintf(f2, "%s %s %s %s %d %d\n",
                        c.cid, c.cname, c.cadress, c.cemail, c.cphone_int, c.cnemp);
        }
        fclose(f);
        fclose(f2);
        remove(CENTRE_FILE);
        rename(CENTRE_FILE2, CENTRE_FILE);
    }
    return tr;
}

/* ===================== RECHERCHE ===================== */
center searchcenter(char *filename, char *id) {
    center c;
    int tr = 0;
    FILE *f = fopen(CENTRE_FILE, "r");
    if (f != NULL) {
        while (tr == 0 && fscanf(f, "%s %s %s %s %d %d\n", 
                                c.cid, c.cname, c.cadress, c.cemail, &c.cphone_int, &c.cnemp) != EOF) {
            if (strcmp(c.cid, id) == 0)
                tr = 1;
        }
        fclose(f);
    }
    if (tr == 0)
        strcpy(c.cid, "-1");
    return c;
}

int total_centres() {
    FILE *f = fopen(CENTRE_FILE, "r");
    if (!f) return 0;
    int count = 0;
    center c;
    while (fscanf(f, "%s %s %s %s %d %d",
                  c.cid, c.cname, c.cadress, c.cemail, &c.cphone_int, &c.cnemp) == 6) {
        count++;
    }
    fclose(f);
    return count;
}

int total_employes() {
    FILE *f = fopen(CENTRE_FILE, "r");
    if (!f) return 0;
    int sum = 0;
    center c;
    while (fscanf(f, "%s %s %s %s %d %d",
                  c.cid, c.cname, c.cadress, c.cemail, &c.cphone_int, &c.cnemp) == 6) {
        sum += c.cnemp;
    }
    fclose(f);
    return sum;
}

center centre_max_employes() {
    FILE *f = fopen(CENTRE_FILE, "r");
    center max = {"", "", "", "", 0, 0};
    if (!f) return max;
    center c;
    while (fscanf(f, "%s %s %s %s %d %d",
                  c.cid, c.cname, c.cadress, c.cemail, &c.cphone_int, &c.cnemp) == 6) {
        if (c.cnemp > max.cnemp) max = c;
    }
    fclose(f);
    return max;
}

center centre_min_employes() {
    FILE *f = fopen(CENTRE_FILE, "r");
    center min = {"", "", "", "", 0, 1000000};
    if (!f) return min;
    center c;
    while (fscanf(f, "%s %s %s %s %d %d",
                  c.cid, c.cname, c.cadress, c.cemail, &c.cphone_int, &c.cnemp) == 6) {
        if (c.cnemp < min.cnemp) min = c;
    }
    fclose(f);
    return min;
}

void distribution_employes(int *moins_10, int *entre_10_20, int *plus_20) {
    *moins_10 = *entre_10_20 = *plus_20 = 0;
    FILE *f = fopen(CENTRE_FILE, "r");
    if (!f) return;
    center c;
    while (fscanf(f, "%s %s %s %s %d %d",
                  c.cid, c.cname, c.cadress, c.cemail, &c.cphone_int, &c.cnemp) == 6) {
        if (c.cnemp < 10) (*moins_10)++;
        else if (c.cnemp <= 20) (*entre_10_20)++;
        else (*plus_20)++;
    }
    fclose(f);
}
//Eya
#include "course.h"

int ajouterEW(char *filename, course c)
{
    FILE *f = fopen(filename, "a");
    if (!f) return 0;
    
    // Save with PIPE separator
    fprintf(f, "%d|%s|%s|%s|%s|%d|%d|%s|%s|%d\n", 
            c.id, c.typecourse, c.level, c.gender,
            c.date, c.hour, c.duration, c.status, 
            c.rescoach, c.room);
    
    fclose(f);
    return 1;
}

int modifierEW(char *filename, int id, course nouv)
{
    int tr = 0;
    FILE *f = fopen(filename, "r");
    FILE *f2 = fopen("nouv.txt", "w");
    char line[256];
    
    if(f == NULL || f2 == NULL) {
        if(f) fclose(f);
        if(f2) fclose(f2);
        return 0;
    }
    
    while(fgets(line, sizeof(line), f)) {
        // Remove newline
        line[strcspn(line, "\n")] = 0;
        
        // Parse with pipe separator
        course c;
        if (sscanf(line, "%d|%[^|]|%[^|]|%[^|]|%[^|]|%d|%d|%[^|]|%[^|]|%d",
                   &c.id, c.typecourse, c.level, c.gender, 
                   c.date, &c.hour, &c.duration, c.status, 
                   c.rescoach, &c.room) == 10) {
            
            if(c.id == id) {
                // Write UPDATED member in PIPE format
                fprintf(f2, "%d|%s|%s|%s|%s|%d|%d|%s|%s|%d\n", 
                        nouv.id,
                        nouv.typecourse,
                        nouv.level,
                        nouv.gender,
                        nouv.date,
                        nouv.hour,
                        nouv.duration,
                        nouv.status,
                        nouv.rescoach,
                        nouv.room);
                tr = 1;
            } else {
                // Write UNCHANGED course (original line)
                fprintf(f2, "%s\n", line);
            }
        }
    }
    
    fclose(f);
    fclose(f2);
    
    if(tr) {
        remove(filename);
        rename("nouv.txt", filename);
    } else {
        remove("nouv.txt");
    }
    
    return tr;
}

#include <string.h>  // Add this at top if not already

int supprimerEW(char *filename, int id)
{
    int tr = 0;
    FILE *f = fopen(filename, "r");
    FILE *f2 = fopen("nouv.txt", "w");
    FILE *restore_file = fopen("restoreEW.txt", "a");  // Open restore file in append mode
    char line[256];
    course c;
    
    if(f == NULL || f2 == NULL || restore_file == NULL) {
        if(f) fclose(f);
        if(f2) fclose(f2);
        if(restore_file) fclose(restore_file);
        return 0;
    }
    
    while(fgets(line, sizeof(line), f)) {
        line[strcspn(line, "\n")] = 0;
        
        if (sscanf(line, "%d|%[^|]|%[^|]|%[^|]|%[^|]|%d|%d|%[^|]|%[^|]|%d",
                   &c.id, c.typecourse, c.level, c.gender, 
                   c.date, &c.hour, &c.duration, c.status, 
                   c.rescoach, &c.room) == 10) {
            
            if(c.id == id) {
                // Save deleted course to restoreEW.txt
                fprintf(restore_file, "%d|%s|%s|%s|%s|%d|%d|%s|%s|%d\n", 
                        c.id, c.typecourse, c.level, c.gender,
                        c.date, c.hour, c.duration, c.status,
                        c.rescoach, c.room);
                tr = 1;
            } else {
                // Keep course in new file
                fprintf(f2, "%s\n", line);
            }
        }
    }
    
    fclose(f);
    fclose(f2);
    fclose(restore_file);
    
    if(tr) {
        remove(filename);
        rename("nouv.txt", filename);
    } else {
        remove("nouv.txt");
    }
    
    return tr;
}
course chercherEW(char *filename, int id)
{
    course c;
    int tr = 0;
    
    FILE *f = fopen(filename, "r");
    if(f == NULL) {
        c.id = -1;
        return c;
    }
    
    while(tr == 0 && fscanf(f, "%d %s %s %s %s %d %d %s %s %d",
                       &c.id,
                       c.typecourse,
                       c.level,
                       c.gender,
                       c.date,
                       &c.hour,
                       &c.duration,
                       c.status,
                       c.rescoach,
                       &c.room) != EOF)
    {
        if(c.id == id) {
            tr = 1;
        }
    }
    
    fclose(f);
    
    if(tr == 0) {
        c.id = -1;
    }
    
    return c;
}
#include <string.h>  // Add this at the top
#include <ctype.h>   // Add this for tolower()

// Flexible search function
int chercher_multi_criteriaEW(char *filename, 
                           int search_id, 
                           char *search_gender, 
                           course results[], 
                           int max_results)
{
    FILE *f = fopen(filename, "r");
    char line[256];
    course c;
    int count = 0;
    
    if (!f) return 0;
    
    while (fgets(line, sizeof(line), f) && count < max_results) {
        line[strcspn(line, "\n")] = 0;
        
        if (sscanf(line, "%d|%[^|]|%[^|]|%[^|]|%[^|]|%d|%d|%[^|]|%[^|]|%d",
                   &c.id, c.typecourse, c.level, c.gender, 
                   c.date, &c.hour, &c.duration, c.status, 
                   c.rescoach, &c.room) == 10) {
            
            int match = 1; // Assume match
            
            // Search by id (if provided and not 0)
            if (search_id != 0 && c.id != search_id) {
                match = 0;
            }
            
            // Search by gender (if provided and not empty)
            if (search_gender != NULL && strlen(search_gender) > 0) {
                // Case-insensitive comparison
                char gender1[20], gender2[20];
                strncpy(gender1, c.gender, sizeof(gender1)-1);
                gender1[sizeof(gender1)-1] = '\0';
                strncpy(gender2, search_gender, sizeof(gender2)-1);
                gender2[sizeof(gender2)-1] = '\0';
                
                // Convert both to lowercase
                for (int i = 0; gender1[i]; i++) gender1[i] = tolower(gender1[i]);
                for (int i = 0; gender2[i]; i++) gender2[i] = tolower(gender2[i]);
                
                if (strcmp(gender1, gender2) != 0) {
                    match = 0;
                }
            }
            
            // If all criteria match, add to results
            if (match) {
                results[count] = c;
                count++;
            }
        }
    }
    
    fclose(f);
    return count;
}
int restaurerEW(char *restore_filename, char *main_filename, int id)
{
    FILE *restore_f = fopen(restore_filename, "r");
    FILE *temp_f = fopen("temp_restoreEW.txt", "w");
    FILE *main_f = fopen(main_filename, "a");  // Append to main file
    char line[256];
    course c;
    int found = 0;
    
    if(!restore_f || !temp_f || !main_f) {
        if(restore_f) fclose(restore_f);
        if(temp_f) fclose(temp_f);
        if(main_f) fclose(main_f);
        return 0;
    }
    
    while(fgets(line, sizeof(line), restore_f)) {
        line[strcspn(line, "\n")] = 0;
        
        if (sscanf(line, "%d|%[^|]|%[^|]|%[^|]|%[^|]|%d|%d|%[^|]|%[^|]|%d",
                   &c.id, c.typecourse, c.level, c.gender, 
                   c.date, &c.hour, &c.duration, c.status, 
                   c.rescoach, &c.room) == 10) {
            
            if(c.id == id) {
                // Write to main file
                fprintf(main_f, "%d|%s|%s|%s|%s|%d|%d|%s|%s|%d\n", 
                        c.id, c.typecourse, c.level, c.gender,
                        c.date, c.hour, c.duration, c.status,
                        c.rescoach, c.room);
                found = 1;
            } else {
                // Keep in restore file
                fprintf(temp_f, "%s\n", line);
            }
        }
    }
    
    fclose(restore_f);
    fclose(temp_f);
    fclose(main_f);
    
    if(found) {
        remove(restore_filename);
        rename("temp_restoreEW.txt", restore_filename);
        return 1;
    } else {
        remove("temp_restoreEW.txt");
        return 0;
    }
}
int course_id_exists(char *filename, int id)
{
    FILE *f = fopen(filename, "r");
    char line[256];
    course c;
    
    if (!f) return 0;  // File doesn't exist = no duplicates
    
    while (fgets(line, sizeof(line), f)) {
        line[strcspn(line, "\n")] = 0;
        
        // FIXED: Use correct sscanf format (remove the extra |] and \n])
        if (sscanf(line, "%d|%[^|]|%[^|]|%[^|]|%[^|]|%d|%d|%[^|]|%[^|]|%d",
                   &c.id, c.typecourse, c.level, c.gender, 
                   c.date, &c.hour, &c.duration, c.status, 
                   c.rescoach, &c.room) == 10) {
            
            if (c.id == id) {
                fclose(f);
                return 1;  // ID exists
            }
        }
    }
    
    fclose(f);
    return 0;  // ID doesn't exist
}


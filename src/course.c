#include <stdio.h>
#include <stdlib.h>
#include <string.h>
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

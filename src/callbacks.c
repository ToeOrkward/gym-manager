#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif
#include <string.h>
#include <stdlib.h>
#include <gtk/gtk.h>
#include <curl/curl.h>
#include "callbacks.h"
#include "interface.h"
#include "support.h"
#include "member.h"
#include <ctype.h>
#include <time.h> 

void
on_return3EW_clicked                     (GtkWidget       *object,
                                        gpointer         user_data)
{
	GtkWidget *current = gtk_widget_get_toplevel(GTK_WIDGET(object));
    GtkWidget *w = create_CourseManag();
    gtk_widget_show(w);
if (current)
        gtk_widget_destroy(current);
}

/* --- Add this near the top of callbacks.c (after includes) --- */
static int send_coach_request_email_simple(const char *coach_firstname, 
                                          const char *coach_lastname,
                                          const char *coach_speciality,
                                          const char *date_str,
                                          const char *session_time,
                                          const char *comment,
                                          const char *member_name);

static char current_member_name[100] = "";
// Function to set current member when logged in
void set_current_member(const char *firstname, const char *lastname)
{
    if (firstname && lastname) {
        snprintf(current_member_name, sizeof(current_member_name), "%s %s", firstname, lastname);
        g_print("Current member set to: %s\n", current_member_name);
    } else {
        current_member_name[0] = '\0';
        g_print("Current member cleared\n");
    }
}


void setup_treeview_columns(GtkWidget *treeview)
{
    // Create ListStore with 10 columns (we still store all data)
    GtkListStore *store = gtk_list_store_new(10,
                              G_TYPE_STRING,  // 0: firstname
                              G_TYPE_STRING,  // 1: lastname
                              G_TYPE_STRING,  // 2: phonenumber
                              G_TYPE_STRING,  // 3: gender
                              G_TYPE_INT,     // 4: cin
                              G_TYPE_STRING,  // 5: startdate
                              G_TYPE_FLOAT,   // 6: amountpaid
                              G_TYPE_STRING,  // 7: username (stored but not displayed)
                              G_TYPE_STRING,  // 8: password (stored but not displayed)
                              G_TYPE_STRING); // 9: monthsub
    
    gtk_tree_view_set_model(GTK_TREE_VIEW(treeview), GTK_TREE_MODEL(store));
    
    // Create ONLY 8 visible columns (skip username and password)
    GtkCellRenderer *renderer;
    GtkTreeViewColumn *column;
    
    // Column 0: First Name
    renderer = gtk_cell_renderer_text_new();
    column = gtk_tree_view_column_new_with_attributes("First Name", renderer, "text", 0, NULL);
    gtk_tree_view_append_column(GTK_TREE_VIEW(treeview), column);
    
    // Column 1: Last Name
    renderer = gtk_cell_renderer_text_new();
    column = gtk_tree_view_column_new_with_attributes("Last Name", renderer, "text", 1, NULL);
    gtk_tree_view_append_column(GTK_TREE_VIEW(treeview), column);
    
    // Column 2: Phone
    renderer = gtk_cell_renderer_text_new();
    column = gtk_tree_view_column_new_with_attributes("Phone", renderer, "text", 2, NULL);
    gtk_tree_view_append_column(GTK_TREE_VIEW(treeview), column);
    
    // Column 3: Gender
    renderer = gtk_cell_renderer_text_new();
    column = gtk_tree_view_column_new_with_attributes("Gender", renderer, "text", 3, NULL);
    gtk_tree_view_append_column(GTK_TREE_VIEW(treeview), column);
    
    // Column 4: CIN
    renderer = gtk_cell_renderer_text_new();
    column = gtk_tree_view_column_new_with_attributes("CIN", renderer, "text", 4, NULL);
    gtk_tree_view_append_column(GTK_TREE_VIEW(treeview), column);
    
    // Column 5: Start Date
    renderer = gtk_cell_renderer_text_new();
    column = gtk_tree_view_column_new_with_attributes("Start Date", renderer, "text", 5, NULL);
    gtk_tree_view_append_column(GTK_TREE_VIEW(treeview), column);
    
    // Column 6: Amount Paid
    renderer = gtk_cell_renderer_text_new();
    column = gtk_tree_view_column_new_with_attributes("Amount Paid", renderer, "text", 6, NULL);
    gtk_tree_view_append_column(GTK_TREE_VIEW(treeview), column);
    
    // SKIP Column 7: Username (not displayed)
    // SKIP Column 8: Password (not displayed)
    
    // Column 9: Month Sub
    renderer = gtk_cell_renderer_text_new();
    column = gtk_tree_view_column_new_with_attributes("Month Sub", renderer, "text", 9, NULL);
    gtk_tree_view_append_column(GTK_TREE_VIEW(treeview), column);
    
    // Optional: Make columns resizable
    gtk_tree_view_set_rules_hint(GTK_TREE_VIEW(treeview), TRUE);
    gtk_tree_view_set_headers_clickable(GTK_TREE_VIEW(treeview), TRUE);
}

void
on_checkbutton1_toggled                (GtkToggleButton *togglebutton,
                                        gpointer         user_data)
{
    GtkWidget *entry_password = lookup_widget(gtk_widget_get_toplevel(GTK_WIDGET(togglebutton)), "entry2");
    
    if (entry_password) {
        gboolean active = gtk_toggle_button_get_active(togglebutton);
        gtk_entry_set_visibility(GTK_ENTRY(entry_password), active);
        
        // Optional: Change echo character when showing password
        if (active) {
            gtk_entry_set_invisible_char(GTK_ENTRY(entry_password), 0); // No echo char
        } else {
            gtk_entry_set_invisible_char(GTK_ENTRY(entry_password), 8226); // Bullet char
        }
    }
}

void
on_login_clicked                       (GtkWidget       *object,
                                        gpointer         user_data)
{
    GtkWidget *window = gtk_widget_get_toplevel(object);
    GtkWidget *entry_username;
    GtkWidget *entry_password;
    
    if (!window || !GTK_IS_WINDOW(window)) {
        g_print("Error: Could not find window\n");
        return;
    }
    
    // Get the entry widgets
    entry_username = lookup_widget(window, "entry1");
    entry_password = lookup_widget(window, "entry2");
    
    if (!entry_username || !entry_password) {
        GtkWidget *dialog = gtk_message_dialog_new(GTK_WINDOW(window),
                                                  GTK_DIALOG_MODAL,
                                                  GTK_MESSAGE_ERROR,
                                                  GTK_BUTTONS_OK,
                                                  "Error: Could not find login fields!");
        gtk_dialog_run(GTK_DIALOG(dialog));
        gtk_widget_destroy(dialog);
        return;
    }
    
    // Get the entered values
    const gchar *username = gtk_entry_get_text(GTK_ENTRY(entry_username));
    const gchar *password = gtk_entry_get_text(GTK_ENTRY(entry_password));
    
    // Check 1: Admin login
    if (strcmp(username, "admin") == 0 && strcmp(password, "admin") == 0) {
        // Admin login successful
        gtk_widget_destroy(window);
        
        GtkWidget *admin_dashboard = create_AdminDashboard();
        gtk_window_set_position(GTK_WINDOW(admin_dashboard), GTK_WIN_POS_CENTER);
        gtk_widget_show_all(admin_dashboard);
        
        g_print("Admin login successful!\n");
        return;
    }
    
    // Check 2: Member login from doc.txt
    FILE *file = fopen("doc.txt", "r");
    if (file) {
        char line[256];
        member m;
        int member_found = 0;
        
        while (fgets(line, sizeof(line), file)) {
            line[strcspn(line, "\n")] = 0;
            
            // Parse with pipe separator
            if (sscanf(line, "%[^|]|%[^|]|%[^|]|%[^|]|%d|%[^|]|%f|%[^|]|%[^|]|%[^\n]",
                       m.firstname, m.lastname, m.phonenumber, m.gender, 
                       &m.cin, m.startdate, &m.amountpaid, m.username, 
                       m.password, m.monthsub) == 10) {
                
                // Check if username and password match
                if (strcmp(username, m.username) == 0 && strcmp(password, m.password) == 0) {
                    member_found = 1;
set_current_member(m.firstname, m.lastname);
                    break;
                }
            }
        }
        fclose(file);
        
        if (member_found) {
            // Member login successful
            gtk_widget_destroy(window);
            
            // Create member dashboard
            GtkWidget *member_dashboard = create_MemberDashboard();
            
            // Pass member data to dashboard (optional)
            // You can store member info in global variable or pass via user_data
            gtk_window_set_position(GTK_WINDOW(member_dashboard), GTK_WIN_POS_CENTER);
            gtk_widget_show_all(member_dashboard);
            
            g_print("Member login successful: %s %s\n", m.firstname, m.lastname);
            return;
        }
    }
    
    // Login failed - neither admin nor member
    GtkWidget *dialog = gtk_message_dialog_new(GTK_WINDOW(window),
                                              GTK_DIALOG_MODAL,
                                              GTK_MESSAGE_ERROR,
                                              GTK_BUTTONS_OK,
                                              "Invalid username or password!\n\n");
    gtk_dialog_run(GTK_DIALOG(dialog));
    gtk_widget_destroy(dialog);
    
    // Clear password field for security
    gtk_entry_set_text(GTK_ENTRY(entry_password), "");
    gtk_entry_set_text(GTK_ENTRY(entry_username), "");
    // Set focus back to username
    gtk_widget_grab_focus(entry_username);
    
    g_print("Login failed: username='%s'\n", username);
}


void
on_members_clicked                     (GtkWidget       *object,
                                        gpointer         user_data)
{
    GtkWidget *admin_dashboard;
    GtkWidget *member_manage;
    
    
    admin_dashboard = lookup_widget(object, "AdminDashboard");
    if (admin_dashboard != NULL) {
        gtk_widget_destroy(admin_dashboard);
    }
    
    
    member_manage = create_MemberManage();
    gtk_widget_show_all(member_manage);
}


void
on_button15_clicked                    (GtkWidget       *object,
                                        gpointer         user_data)
{
    GtkWidget *member_dashboard;
    GtkWidget *my_coach;
    
    
    member_dashboard = lookup_widget(object, "MemberDashboard");
    if (member_dashboard != NULL) {
        gtk_widget_destroy(member_dashboard);
    }
    
    
    my_coach = create_Mycoach();
    gtk_widget_show_all(my_coach);

}


void
on_add_clicked                         (GtkWidget       *object,
                                        gpointer         user_data)
{
    GtkWidget *add_member;
    GtkWidget *member_manage;
    
    
    member_manage = lookup_widget(object, "MemberManage");
    if (member_manage != NULL) {
        gtk_widget_destroy(member_manage);
    }
    
   
    add_member = create_addmember();
    gtk_widget_show_all(add_member);

}


void
on_list_clicked                        (GtkWidget       *object,
                                        gpointer         user_data)
{
    GtkWidget *recherche_member;
    GtkWidget *member_manage;
    
    
    member_manage = lookup_widget(object, "MemberManage");
    if (member_manage != NULL) {
        gtk_widget_destroy(member_manage);
    }
    
   
    recherche_member = create_recherchemember();
    gtk_widget_show_all(recherche_member);
}




void
on_delete_clicked                      (GtkWidget       *object,
                                        gpointer         user_data)
{
    GtkWidget *delete_member;
    GtkWidget *member_manage;
    
    
    member_manage = lookup_widget(object, "MemberManage");
    if (member_manage != NULL) {
        gtk_widget_destroy(member_manage);
    }
    
   
    delete_member = create_deletemember();
    gtk_widget_show_all(delete_member);
}


void
on_restore_clicked                     (GtkWidget       *object,
                                        gpointer         user_data)
{
    GtkWidget *restore_member;
    GtkWidget *member_manage;
    
    
    member_manage = lookup_widget(object, "MemberManage");
    if (member_manage != NULL) {
        gtk_widget_destroy(member_manage);
    }
    
   
    restore_member = create_restoremember();
    gtk_widget_show_all(restore_member);
}


void
on_stats_clicked                       (GtkWidget       *object,
                                        gpointer         user_data)
{
    GtkWidget *stats_window;
    GtkWidget *member_manage;
    
    member_manage = lookup_widget(object, "MemberManage");
    if (member_manage != NULL) {
        gtk_widget_destroy(member_manage);
    }
    
    stats_window = create_Statsview();
    gtk_window_set_position(GTK_WINDOW(stats_window), GTK_WIN_POS_CENTER);
    gtk_widget_show_all(stats_window);
}


void
on_home2admindash_clicked              (GtkWidget       *object,
                                        gpointer         user_data)
{
    GtkWidget *admin_dashboard;
    GtkWidget *member_manage;
    
    
    member_manage = lookup_widget(object, "MemberManage");
    if (member_manage != NULL) {
        gtk_widget_destroy(member_manage);
    }
    
   
    admin_dashboard = create_AdminDashboard();
    gtk_widget_show_all(admin_dashboard);
}

void
on_admin_coaches_clicked               (GtkWidget       *object,
                                        gpointer         user_data)
{
    GtkWidget *dashboard = gtk_widget_get_toplevel(object);
    GtkWidget *coach_manag = create_CoachManagAdmin();
    gtk_widget_show_all(coach_manag);
    gtk_widget_hide(dashboard);
}

void
on_admin_events_clicked                (GtkWidget       *object,
                                        gpointer         user_data)
{
    GtkWidget *dashboard = gtk_widget_get_toplevel(object);
    GtkWidget *event_manag = create_EventManagAdmin();
    gtk_widget_show_all(event_manag);
    gtk_widget_hide(dashboard);
}

void
on_admin_equipments_clicked            (GtkWidget       *object,
                                        gpointer         user_data)
{
    GtkWidget *dashboard = gtk_widget_get_toplevel(object);
    GtkWidget *equip_manag = create_EquipManagAdmin();
    gtk_widget_show_all(equip_manag);
    gtk_widget_hide(dashboard);
}

void
on_req_clicked                         (GtkWidget       *object,
                                        gpointer         user_data)
{
    GtkWidget *my_coach;
    GtkWidget *req_coach;
    
    
    my_coach = lookup_widget(object, "Mycoach");
    if (my_coach != NULL) {
        gtk_widget_destroy(my_coach);
    }
    
   
    req_coach = create_reqcoach();
    gtk_widget_show_all(req_coach);
}


void
on_return1_clicked                     (GtkWidget       *object,
                                        gpointer         user_data)
{
    GtkWidget *add_member;
    GtkWidget *member_manage;
    
    
    add_member = lookup_widget(object, "addmember");
    if (add_member != NULL) {
        gtk_widget_destroy(add_member);
    }
    
    
    member_manage = create_MemberManage();
    gtk_widget_show_all(member_manage);
}


void
on_saveaddition_clicked                (GtkWidget       *object,
                                        gpointer         user_data)
{

    GtkWidget *window = gtk_widget_get_toplevel(object);
    
    if (window == NULL || !GTK_IS_WINDOW(window)) {
        g_print("Error: Could not find window!\n");
        return;
    }
    
    // Get all widgets - ADJUST NAMES AS NEEDED
    GtkWidget *entry1 = lookup_widget(window, "entry3");
    GtkWidget *entry2 = lookup_widget(window, "entry4");
    GtkWidget *entry3 = lookup_widget(window, "entry5");
    GtkWidget *entry5 = lookup_widget(window, "entry6");
    GtkWidget *entry6 = lookup_widget(window, "entry7");
    GtkWidget *entry7 = lookup_widget(window, "entry8");
    GtkWidget *entry8 = lookup_widget(window, "entry9");
    GtkWidget *entry9 = lookup_widget(window, "entry10");
    GtkWidget *spinbutton1 = lookup_widget(window, "spinbutton1");
    GtkWidget *radiobutton1 = lookup_widget(window, "radiobutton1");
    GtkWidget *radiobutton2 = lookup_widget(window, "radiobutton2");
    
    // Check if widgets were found
    if (!entry1 || !entry2 || !entry3 || !entry5 || !entry6 || 
        !entry7 || !entry8 || !entry9 || !spinbutton1 ||
        !radiobutton1 || !radiobutton2) {
        GtkWidget *dialog = gtk_message_dialog_new(GTK_WINDOW(window),
                                                  GTK_DIALOG_MODAL,
                                                  GTK_MESSAGE_ERROR,
                                                  GTK_BUTTONS_OK,
                                                  "Error: Could not find all form fields!");
        gtk_dialog_run(GTK_DIALOG(dialog));
        gtk_widget_destroy(dialog);
        return;
    }
    
    // Get text from entries
    const gchar *firstname = gtk_entry_get_text(GTK_ENTRY(entry1));
    const gchar *lastname = gtk_entry_get_text(GTK_ENTRY(entry2));
    const gchar *phonenumber = gtk_entry_get_text(GTK_ENTRY(entry3));
    const gchar *cin_str = gtk_entry_get_text(GTK_ENTRY(entry5));
    const gchar *startdate = gtk_entry_get_text(GTK_ENTRY(entry6));
    const gchar *amountpaid_str = gtk_entry_get_text(GTK_ENTRY(entry7));
    const gchar *username = gtk_entry_get_text(GTK_ENTRY(entry8));
    const gchar *password = gtk_entry_get_text(GTK_ENTRY(entry9));
    
    // Get value from spinbutton
    gint monthsub_value = (gint)gtk_spin_button_get_value(GTK_SPIN_BUTTON(spinbutton1));
    
    // Get selected gender
    gchar gender[10];
    if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(radiobutton1))) {
        strcpy(gender, "Male");
    } else if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(radiobutton2))) {
        strcpy(gender, "Female");
    } else {
        strcpy(gender, "");
    }
    
    // Validate
    if (strlen(firstname) == 0 || strlen(cin_str) == 0 || strlen(gender) == 0 || strlen(lastname) == 0 || strlen(phonenumber) == 0 || strlen(startdate) == 0 || strlen(amountpaid_str) == 0 || strlen(username) == 0 || strlen(password) == 0) {
        GtkWidget *dialog = gtk_message_dialog_new(GTK_WINDOW(window),
                                                  GTK_DIALOG_MODAL,
                                                  GTK_MESSAGE_ERROR,
                                                  GTK_BUTTONS_OK,
                                                  "Please complete the missing information to finish the registration!");
        gtk_dialog_run(GTK_DIALOG(dialog));
        gtk_widget_destroy(dialog);
        return;
    }
    
    // Validate CIN is a number
    for (int i = 0; cin_str[i] != '\0'; i++) {
        if (!isdigit(cin_str[i])) {
            GtkWidget *dialog = gtk_message_dialog_new(GTK_WINDOW(window),
                                                      GTK_DIALOG_MODAL,
                                                      GTK_MESSAGE_ERROR,
                                                      GTK_BUTTONS_OK,
                                                      "CIN must be a number!");
            gtk_dialog_run(GTK_DIALOG(dialog));
            gtk_widget_destroy(dialog);
            return;
        }
    }
    for (int i = 0; cin_str[i] != '\0'; i++) {
        if (!isdigit(cin_str[i])) {
            GtkWidget *dialog = gtk_message_dialog_new(GTK_WINDOW(window),
                                                      GTK_DIALOG_MODAL,
                                                      GTK_MESSAGE_ERROR,
                                                      GTK_BUTTONS_OK,
                                                      "CIN must be a number!");
            gtk_dialog_run(GTK_DIALOG(dialog));
            gtk_widget_destroy(dialog);
            return;
        }
    }
    
    // === ADD THIS CIN UNIQUENESS CHECK ===
    int cin = atoi(cin_str);
    
    // Check if CIN already exists
    if (cin_exists("doc.txt", cin)) {
        GtkWidget *dialog = gtk_message_dialog_new(GTK_WINDOW(window),
                                                  GTK_DIALOG_MODAL,
                                                  GTK_MESSAGE_ERROR,
                                                  GTK_BUTTONS_OK,
                                                  "Error: CIN %d already exists!\n"
                                                  "Please use a unique CIN number.",
                                                  cin);
        gtk_dialog_run(GTK_DIALOG(dialog));
        gtk_widget_destroy(dialog);
        return;
    }
    
    // Optional: Also check restore.txt
    if (cin_exists("restore.txt", cin)) {
        GtkWidget *dialog = gtk_message_dialog_new(GTK_WINDOW(window),
                                                  GTK_DIALOG_MODAL,
                                                  GTK_MESSAGE_WARNING,
                                                  GTK_BUTTONS_OK,
                                                  "Warning: CIN %d exists in deleted members.\n"
                                                  "Delete it permanently first or use different CIN.",
                                                  cin);
        gtk_dialog_run(GTK_DIALOG(dialog));
        gtk_widget_destroy(dialog);
        return;
    }
    
    // Create member structure
    member new_member;
    
    // Fill the structure
    strncpy(new_member.firstname, firstname, sizeof(new_member.firstname)-1);
    new_member.firstname[sizeof(new_member.firstname)-1] = '\0';
    
    strncpy(new_member.lastname, lastname, sizeof(new_member.lastname)-1);
    new_member.lastname[sizeof(new_member.lastname)-1] = '\0';
    
    strncpy(new_member.phonenumber, phonenumber, sizeof(new_member.phonenumber)-1);
    new_member.phonenumber[sizeof(new_member.phonenumber)-1] = '\0';
    
    strncpy(new_member.gender, gender, sizeof(new_member.gender)-1);
    new_member.gender[sizeof(new_member.gender)-1] = '\0';
    
    new_member.cin = atoi(cin_str);
    
    strncpy(new_member.startdate, startdate, sizeof(new_member.startdate)-1);
    new_member.startdate[sizeof(new_member.startdate)-1] = '\0';
    
    new_member.amountpaid = atof(amountpaid_str);
    
    strncpy(new_member.username, username, sizeof(new_member.username)-1);
    new_member.username[sizeof(new_member.username)-1] = '\0';
    
    strncpy(new_member.password, password, sizeof(new_member.password)-1);
    new_member.password[sizeof(new_member.password)-1] = '\0';
    
    char monthsub_str[20];
    snprintf(monthsub_str, sizeof(monthsub_str), "%d", monthsub_value);
    strncpy(new_member.monthsub, monthsub_str, sizeof(new_member.monthsub)-1);
    new_member.monthsub[sizeof(new_member.monthsub)-1] = '\0';
    
    // Save to file
    int result = ajouter("doc.txt", new_member);
    
    // Show result
    GtkWidget *dialog;
    if (result == 1) {
        dialog = gtk_message_dialog_new(GTK_WINDOW(window),
                                       GTK_DIALOG_MODAL,
                                       GTK_MESSAGE_INFO,
                                       GTK_BUTTONS_OK,
                                       "Member saved successfully!");
        
        // Clear fields
        gtk_entry_set_text(GTK_ENTRY(entry1), "");
        gtk_entry_set_text(GTK_ENTRY(entry2), "");
        gtk_entry_set_text(GTK_ENTRY(entry3), "");
        gtk_entry_set_text(GTK_ENTRY(entry5), "");
        gtk_entry_set_text(GTK_ENTRY(entry6), "");
        gtk_entry_set_text(GTK_ENTRY(entry7), "");
        gtk_entry_set_text(GTK_ENTRY(entry8), "");
        gtk_entry_set_text(GTK_ENTRY(entry9), "");
        gtk_spin_button_set_value(GTK_SPIN_BUTTON(spinbutton1), 1.0);
        gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(radiobutton1), FALSE);
        gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(radiobutton2), FALSE);
        
        gtk_dialog_run(GTK_DIALOG(dialog));
        gtk_widget_destroy(dialog);
    } else {
        dialog = gtk_message_dialog_new(GTK_WINDOW(window),
                                       GTK_DIALOG_MODAL,
                                       GTK_MESSAGE_ERROR,
                                       GTK_BUTTONS_OK,
                                       "Error saving member!");
        gtk_dialog_run(GTK_DIALOG(dialog));
        gtk_widget_destroy(dialog);
    }
}


void
on_searchlist_clicked                  (GtkWidget       *object,
                                        gpointer         user_data)
{
    GtkWidget *window = gtk_widget_get_toplevel(object);
    GtkWidget *treeview;
    GtkWidget *entry_cin;
    GtkWidget *entry_phone;
    GtkWidget *radiobutton_male;
    GtkWidget *radiobutton_female;
    GtkWidget *radiobutton_all;
    
    if (!window || !GTK_IS_WINDOW(window)) {
        g_print("Error: Could not find window\n");
        return;
    }
    
    // Get the treeview
    treeview = lookup_widget(window, "treeview1");
    if (!treeview) {
        g_print("Error: Could not find treeview widget\n");
        return;
    }
    
    // Get search widgets
    entry_cin = lookup_widget(window, "entry11");
    entry_phone = lookup_widget(window, "entry12");
    radiobutton_male = lookup_widget(window, "radiobutton3");
    radiobutton_female = lookup_widget(window, "radiobutton4");
    radiobutton_all = lookup_widget(window, "radiobutton14");
    
    // Check if widgets exist (make them optional)
    if (!entry_cin || !entry_phone) {
        GtkWidget *dialog = gtk_message_dialog_new(GTK_WINDOW(window),
                                                  GTK_DIALOG_MODAL,
                                                  GTK_MESSAGE_ERROR,
                                                  GTK_BUTTONS_OK,
                                                  "Error: Could not find search fields!");
        gtk_dialog_run(GTK_DIALOG(dialog));
        gtk_widget_destroy(dialog);
        return;
    }
    
    // Get search values
    const gchar *cin_str = gtk_entry_get_text(GTK_ENTRY(entry_cin));
    const gchar *phone = gtk_entry_get_text(GTK_ENTRY(entry_phone));
    
    // Get selected gender - FIXED: declare variable properly
    char gender_filter[20] = ""; // FIXED: Declare variable here
    
    if (radiobutton_male && gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(radiobutton_male))) {
        strcpy(gender_filter, "Male");
    } else if (radiobutton_female && gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(radiobutton_female))) {
        strcpy(gender_filter, "Female");
    } else if (radiobutton_all && gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(radiobutton_all))) {
        // "All" means empty string
        gender_filter[0] = '\0';
    }
    
    // Convert CIN string to int (0 if empty)
    int cin = 0;
    if (strlen(cin_str) > 0) {
        cin = atoi(cin_str);
    }
	
    
    // Setup treeview if needed
    if (!gtk_tree_view_get_model(GTK_TREE_VIEW(treeview))) {
        setup_treeview_columns(treeview);
    }
    
    // Get the store
    GtkListStore *store = GTK_LIST_STORE(gtk_tree_view_get_model(GTK_TREE_VIEW(treeview)));
    
    // Clear existing data
    gtk_list_store_clear(store);
    
    // Perform search
    member results[100];
    
    // Check if we should apply gender filter
    char *gender_to_search = NULL;
    if (strlen(gender_filter) > 0) {
        gender_to_search = gender_filter;
    }
    
    // Check if phone is empty or not
    char *phone_to_search = NULL;
    if (strlen(phone) > 0) {
        phone_to_search = (char*)phone;
    }
    
    int result_count = chercher_multi_criteria("doc.txt", cin, phone_to_search, 
                                              gender_to_search, results, 100);
    
    if (result_count == 0) {
        // Show no results message
        GtkWidget *dialog = gtk_message_dialog_new(GTK_WINDOW(window),
                                                  GTK_DIALOG_MODAL,
                                                  GTK_MESSAGE_INFO,
                                                  GTK_BUTTONS_OK,
                                                  "No members found matching your criteria.");
        gtk_dialog_run(GTK_DIALOG(dialog));
        gtk_widget_destroy(dialog);
        return;
    }
    
    // Display results
    GtkTreeIter iter;
    for (int i = 0; i < result_count; i++) {
        gtk_list_store_append(store, &iter);
        gtk_list_store_set(store, &iter,
                          0, results[i].firstname,
                          1, results[i].lastname,
                          2, results[i].phonenumber,
                          3, results[i].gender,
                          4, results[i].cin,
                          5, results[i].startdate,
                          6, results[i].amountpaid,
                          7, results[i].username,
                          8, results[i].password,
                          9, results[i].monthsub,
                          -1);
    }
    
    g_print("Search complete: Found %d members\n", result_count);

}


void
on_return2_clicked                     (GtkWidget       *object,
                                        gpointer         user_data)
{
    GtkWidget *recherche_member;
    GtkWidget *member_manage;
    
    
    recherche_member = lookup_widget(object, "recherchemember");
    if (recherche_member != NULL) {
        gtk_widget_destroy(recherche_member);
    }
    
    
    member_manage = create_MemberManage();
    gtk_widget_show_all(member_manage);
}



void
on_searchdelete_clicked                (GtkWidget       *object,
                                        gpointer         user_data)
{
 GtkWidget *window = gtk_widget_get_toplevel(object);
    GtkWidget *treeview;
    GtkWidget *entry_cin;
    GtkWidget *entry_phone;
    GtkWidget *radiobutton_male;
    GtkWidget *radiobutton_female;
    GtkWidget *radiobutton_all;
    
    if (!window || !GTK_IS_WINDOW(window)) {
        g_print("Error: Could not find window\n");
        return;
    }
    
    // Get the treeview
    treeview = lookup_widget(window, "treeview3");
    if (!treeview) {
        g_print("Error: Could not find treeview widget\n");
        return;
    }
    
    // Get search widgets
    entry_cin = lookup_widget(window, "entry15");
    entry_phone = lookup_widget(window, "entry16");
    radiobutton_male = lookup_widget(window, "radiobutton7");
    radiobutton_female = lookup_widget(window, "radiobutton8");
    radiobutton_all = lookup_widget(window, "radiobutton15");
    
    // Check if widgets exist (make them optional)
    if (!entry_cin || !entry_phone) {
        GtkWidget *dialog = gtk_message_dialog_new(GTK_WINDOW(window),
                                                  GTK_DIALOG_MODAL,
                                                  GTK_MESSAGE_ERROR,
                                                  GTK_BUTTONS_OK,
                                                  "Error: Could not find search fields!");
        gtk_dialog_run(GTK_DIALOG(dialog));
        gtk_widget_destroy(dialog);
        return;
    }
    
    // Get search values
    const gchar *cin_str = gtk_entry_get_text(GTK_ENTRY(entry_cin));
    const gchar *phone = gtk_entry_get_text(GTK_ENTRY(entry_phone));
    
    // Get selected gender - FIXED: declare variable properly
    char gender_filter[20] = ""; // FIXED: Declare variable here
    
    if (radiobutton_male && gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(radiobutton_male))) {
        strcpy(gender_filter, "Male");
    } else if (radiobutton_female && gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(radiobutton_female))) {
        strcpy(gender_filter, "Female");
    } else if (radiobutton_all && gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(radiobutton_all))) {
        // "All" means empty string
        gender_filter[0] = '\0';
    }
    
    // Convert CIN string to int (0 if empty)
    int cin = 0;
    if (strlen(cin_str) > 0) {
        cin = atoi(cin_str);
    }
	
    
    // Setup treeview if needed
    if (!gtk_tree_view_get_model(GTK_TREE_VIEW(treeview))) {
        setup_treeview_columns(treeview);
    }
    
    // Get the store
    GtkListStore *store = GTK_LIST_STORE(gtk_tree_view_get_model(GTK_TREE_VIEW(treeview)));
    
    // Clear existing data
    gtk_list_store_clear(store);
    
    // Perform search
    member results[100];
    
    // Check if we should apply gender filter
    char *gender_to_search = NULL;
    if (strlen(gender_filter) > 0) {
        gender_to_search = gender_filter;
    }
    
    // Check if phone is empty or not
    char *phone_to_search = NULL;
    if (strlen(phone) > 0) {
        phone_to_search = (char*)phone;
    }
    
    int result_count = chercher_multi_criteria("doc.txt", cin, phone_to_search, 
                                              gender_to_search, results, 100);
    
    if (result_count == 0) {
        // Show no results message
        GtkWidget *dialog = gtk_message_dialog_new(GTK_WINDOW(window),
                                                  GTK_DIALOG_MODAL,
                                                  GTK_MESSAGE_INFO,
                                                  GTK_BUTTONS_OK,
                                                  "No members found matching your criteria.");
        gtk_dialog_run(GTK_DIALOG(dialog));
        gtk_widget_destroy(dialog);
        return;
    }
    
    // Display results
    GtkTreeIter iter;
    for (int i = 0; i < result_count; i++) {
        gtk_list_store_append(store, &iter);
        gtk_list_store_set(store, &iter,
                          0, results[i].firstname,
                          1, results[i].lastname,
                          2, results[i].phonenumber,
                          3, results[i].gender,
                          4, results[i].cin,
                          5, results[i].startdate,
                          6, results[i].amountpaid,
                          7, results[i].username,
                          8, results[i].password,
                          9, results[i].monthsub,
                          -1);
    }
    
    g_print("Search complete: Found %d members\n", result_count);

}




void
on_return4_clicked                     (GtkWidget       *object,
                                        gpointer         user_data)
{
    GtkWidget *delete_member;
    GtkWidget *member_manage;
    
    
    delete_member = lookup_widget(object, "deletemember");
    if (delete_member != NULL) {
        gtk_widget_destroy(delete_member);
    }
    
    
    member_manage = create_MemberManage();
    gtk_widget_show_all(member_manage);

}


void
on_searchrestore_clicked               (GtkWidget       *object,
                                        gpointer         user_data)
{
GtkWidget *window = gtk_widget_get_toplevel(object);
    GtkWidget *treeview;
    GtkWidget *entry_cin;
    GtkWidget *entry_phone;
    GtkWidget *radiobutton_male;
    GtkWidget *radiobutton_female;
    GtkWidget *radiobutton_all;
    
    if (!window || !GTK_IS_WINDOW(window)) {
        g_print("Error: Could not find window\n");
        return;
    }
    
    // Get the treeview
    treeview = lookup_widget(window, "treeview4");
    if (!treeview) {
        g_print("Error: Could not find treeview widget\n");
        return;
    }
    
    // Get search widgets
    entry_cin = lookup_widget(window, "entry17");
    entry_phone = lookup_widget(window, "entry18");
    radiobutton_male = lookup_widget(window, "radiobutton9");
    radiobutton_female = lookup_widget(window, "radiobutton10");
    radiobutton_all = lookup_widget(window, "radiobutton16");
    
    // Check if widgets exist (make them optional)
    if (!entry_cin || !entry_phone) {
        GtkWidget *dialog = gtk_message_dialog_new(GTK_WINDOW(window),
                                                  GTK_DIALOG_MODAL,
                                                  GTK_MESSAGE_ERROR,
                                                  GTK_BUTTONS_OK,
                                                  "Error: Could not find search fields!");
        gtk_dialog_run(GTK_DIALOG(dialog));
        gtk_widget_destroy(dialog);
        return;
    }
    
    // Get search values
    const gchar *cin_str = gtk_entry_get_text(GTK_ENTRY(entry_cin));
    const gchar *phone = gtk_entry_get_text(GTK_ENTRY(entry_phone));
    
    // Get selected gender - FIXED: declare variable properly
    char gender_filter[20] = ""; // FIXED: Declare variable here
    
    if (radiobutton_male && gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(radiobutton_male))) {
        strcpy(gender_filter, "Male");
    } else if (radiobutton_female && gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(radiobutton_female))) {
        strcpy(gender_filter, "Female");
    } else if (radiobutton_all && gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(radiobutton_all))) {
        // "All" means empty string
        gender_filter[0] = '\0';
    }
    
    // Convert CIN string to int (0 if empty)
    int cin = 0;
    if (strlen(cin_str) > 0) {
        cin = atoi(cin_str);
    }
	
    
    // Setup treeview if needed
    if (!gtk_tree_view_get_model(GTK_TREE_VIEW(treeview))) {
        setup_treeview_columns(treeview);
    }
    
    // Get the store
    GtkListStore *store = GTK_LIST_STORE(gtk_tree_view_get_model(GTK_TREE_VIEW(treeview)));
    
    // Clear existing data
    gtk_list_store_clear(store);
    
    // Perform search
    member results[100];
    
    // Check if we should apply gender filter
    char *gender_to_search = NULL;
    if (strlen(gender_filter) > 0) {
        gender_to_search = gender_filter;
    }
    
    // Check if phone is empty or not
    char *phone_to_search = NULL;
    if (strlen(phone) > 0) {
        phone_to_search = (char*)phone;
    }
    
    int result_count = chercher_multi_criteria("restore.txt", cin, phone_to_search, 
                                              gender_to_search, results, 100);
    
    if (result_count == 0) {
        // Show no results message
        GtkWidget *dialog = gtk_message_dialog_new(GTK_WINDOW(window),
                                                  GTK_DIALOG_MODAL,
                                                  GTK_MESSAGE_INFO,
                                                  GTK_BUTTONS_OK,
                                                  "No members found matching your criteria.");
        gtk_dialog_run(GTK_DIALOG(dialog));
        gtk_widget_destroy(dialog);
        return;
    }
    
    // Display results
    GtkTreeIter iter;
    for (int i = 0; i < result_count; i++) {
        gtk_list_store_append(store, &iter);
        gtk_list_store_set(store, &iter,
                          0, results[i].firstname,
                          1, results[i].lastname,
                          2, results[i].phonenumber,
                          3, results[i].gender,
                          4, results[i].cin,
                          5, results[i].startdate,
                          6, results[i].amountpaid,
                          7, results[i].username,
                          8, results[i].password,
                          9, results[i].monthsub,
                          -1);
    }
    
    g_print("Search complete: Found %d members\n", result_count);

}



void
on_return5_clicked                     (GtkWidget       *object,
                                        gpointer         user_data)
{
    GtkWidget *restore_member;
    GtkWidget *member_manage;
    
    
    restore_member = lookup_widget(object, "restoremember");
    if (restore_member != NULL) {
        gtk_widget_destroy(restore_member);
    }
    
    
    member_manage = create_MemberManage();
    gtk_widget_show_all(member_manage);
}


void
on_returnC_clicked                     (GtkWidget       *object,
                                        gpointer         user_data)
{
    GtkWidget *req_coach;
    GtkWidget *my_coach;
    
    
    req_coach = lookup_widget(object, "reqcoach");
    if (req_coach != NULL) {
        gtk_widget_destroy(req_coach);
    }
    
    
    my_coach = create_Mycoach();
    gtk_widget_show_all(my_coach);
}
///////////////////////////////////////

void
on_sendreq_clicked                     (GtkWidget       *object,
                                        gpointer         user_data)
{
    GtkWidget *window = gtk_widget_get_toplevel(object);
    GtkWidget *calendar;
    GtkWidget *entry_time;
    GtkWidget *entry_comment;
    GtkWidget *treeview;
    GtkTreeSelection *selection;
    GtkTreeModel *model;
    GtkTreeIter iter;
    
    if (!window || !GTK_IS_WINDOW(window)) {
        g_print("Error: Could not find window\n");
        return;
    }
    
    // Get widgets
    calendar = lookup_widget(window, "calendar1");
    entry_time = lookup_widget(window, "entry20");
    entry_comment = lookup_widget(window, "entry21");
    treeview = lookup_widget(window, "treeview5");
    
    if (!calendar || !entry_time || !entry_comment || !treeview) {
        GtkWidget *dialog = gtk_message_dialog_new(GTK_WINDOW(window),
                                                  GTK_DIALOG_MODAL,
                                                  GTK_MESSAGE_ERROR,
                                                  GTK_BUTTONS_OK,
                                                  "Error: Could not find all required fields!");
        gtk_dialog_run(GTK_DIALOG(dialog));
        gtk_widget_destroy(dialog);
        return;
    }
    
    // Get selected coach from treeview
    selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(treeview));
    if (!gtk_tree_selection_get_selected(selection, &model, &iter)) {
        GtkWidget *dialog = gtk_message_dialog_new(GTK_WINDOW(window),
                                                  GTK_DIALOG_MODAL,
                                                  GTK_MESSAGE_ERROR,
                                                  GTK_BUTTONS_OK,
                                                  "Please select a coach first!");
        gtk_dialog_run(GTK_DIALOG(dialog));
        gtk_widget_destroy(dialog);
        return;
    }
    
    // Get coach information
    gchar *coach_firstname, *coach_lastname, *coach_speciality;
    gtk_tree_model_get(model, &iter,
                      0, &coach_firstname,  // First Name
                      1, &coach_lastname,   // Last Name
                      3, &coach_speciality, // Speciality
                      -1);
    
    // Get date from calendar
    guint year, month, day;
    gtk_calendar_get_date(GTK_CALENDAR(calendar), &year, &month, &day);
    month += 1; // GTK calendar months are 0-11
    
    char date_str[20];
    snprintf(date_str, sizeof(date_str), "%04d-%02d-%02d", year, month, day);
    
    // Get session time
    const gchar *session_time = gtk_entry_get_text(GTK_ENTRY(entry_time));
    
    // Get comment
    const gchar *comment = gtk_entry_get_text(GTK_ENTRY(entry_comment));
    
    // Get current member information
    char member_name[100];
    if (strlen(current_member_name) == 0) {
        // No member logged in (shouldn't happen, but just in case)
        strcpy(member_name, "Guest Member");
    } else {
        strcpy(member_name, current_member_name);
    }
    
    // Validate inputs
    if (strlen(session_time) == 0) {
        GtkWidget *dialog = gtk_message_dialog_new(GTK_WINDOW(window),
                                                  GTK_DIALOG_MODAL,
                                                  GTK_MESSAGE_ERROR,
                                                  GTK_BUTTONS_OK,
                                                  "Please enter session time!");
        gtk_dialog_run(GTK_DIALOG(dialog));
        gtk_widget_destroy(dialog);
        
        g_free(coach_firstname);
        g_free(coach_lastname);
        g_free(coach_speciality);
        return;
    }
    
    // Save to Requests.txt
    FILE *file = fopen("Requests.txt", "a");
    if (!file) {
        GtkWidget *dialog = gtk_message_dialog_new(GTK_WINDOW(window),
                                                  GTK_DIALOG_MODAL,
                                                  GTK_MESSAGE_ERROR,
                                                  GTK_BUTTONS_OK,
                                                  "Error: Cannot save request!");
        gtk_dialog_run(GTK_DIALOG(dialog));
        gtk_widget_destroy(dialog);
        
        g_free(coach_firstname);
        g_free(coach_lastname);
        g_free(coach_speciality);
        return;
    }
    
    // Format: member|coach_firstname|coach_lastname|speciality|date|time|comment
    fprintf(file, "%s|%s|%s|%s|%s|%s|%s\n",
            member_name,
            coach_firstname,
            coach_lastname,
            coach_speciality,
            date_str,
            session_time,
            comment);
    
    fclose(file);
    
    // Show success message
    GtkWidget *dialog = gtk_message_dialog_new(GTK_WINDOW(window),
                                              GTK_DIALOG_MODAL,
                                              GTK_MESSAGE_INFO,
                                              GTK_BUTTONS_OK,
                                              "✓ Request sent successfully!\n\n"
                                              "From: %s\n"
                                              "To Coach: %s %s\n"
                                              "Speciality: %s\n"
                                              "Date: %s\n"
                                              "Time: %s\n"
                                              "Comment: %s",
                                              member_name,
                                              coach_firstname, coach_lastname,
                                              coach_speciality,
                                              date_str,
                                              session_time,
                                              comment);
    gtk_dialog_run(GTK_DIALOG(dialog));
    gtk_widget_destroy(dialog);
    
    // Clear fields
    gtk_entry_set_text(GTK_ENTRY(entry_time), "");
    gtk_entry_set_text(GTK_ENTRY(entry_comment), "");
    
    // Free memory
    g_free(coach_firstname);
    g_free(coach_lastname);
    g_free(coach_speciality);
    
    g_print("Request saved: %s requested %s %s for %s at %s\n",
            member_name, coach_firstname, coach_lastname, date_str, session_time);
}


void
on_searchcoach_clicked                 (GtkWidget       *object,
                                        gpointer         user_data)
{
    GtkWidget *window = gtk_widget_get_toplevel(object);
    GtkWidget *treeview;
    GtkWidget *combobox_speciality;
    GtkWidget *combobox_level;
    GtkWidget *combobox_availability;
    GtkWidget *radiobutton_male;
    GtkWidget *radiobutton_female;
    GtkWidget *radiobutton_all;
    
    if (!window || !GTK_IS_WINDOW(window)) {
        g_print("Error: Could not find window\n");
        return;
    }
    
    // Get the treeview
    treeview = lookup_widget(window, "treeview5");
    if (!treeview) {
        g_print("Error: Could not find treeview widget\n");
        return;
    }
    
    // Get search widgets
    combobox_speciality = lookup_widget(window, "combobox1");
    combobox_level = lookup_widget(window, "combobox2");
    radiobutton_male = lookup_widget(window, "radiobutton12");
    radiobutton_female = lookup_widget(window, "radiobutton13");
    radiobutton_all = lookup_widget(window, "radiobutton18");
    
    // Check if treeview has model
    if (!gtk_tree_view_get_model(GTK_TREE_VIEW(treeview))) {
        g_print("Error: Treeview not initialized\n");
        return;
    }
    
    // Get the store
    GtkListStore *store = GTK_LIST_STORE(gtk_tree_view_get_model(GTK_TREE_VIEW(treeview)));
    gtk_list_store_clear(store);
    
    // Get selected speciality
    char speciality_filter[100] = "";
    if (combobox_speciality) {
        gint active = gtk_combo_box_get_active(GTK_COMBO_BOX(combobox_speciality));
        if (active > 0) { // Skip "Select" option
            gchar *speciality_text = gtk_combo_box_get_active_text(GTK_COMBO_BOX(combobox_speciality));
            if (speciality_text) {
                strncpy(speciality_filter, speciality_text, sizeof(speciality_filter)-1);
                g_free(speciality_text);
            }
        }
    }
    
    // Get selected level
    char level_filter[50] = "";
    if (combobox_level) {
        gint active = gtk_combo_box_get_active(GTK_COMBO_BOX(combobox_level));
        if (active >= 0) {
            gchar *level_text = gtk_combo_box_get_active_text(GTK_COMBO_BOX(combobox_level));
            if (level_text) {
                strncpy(level_filter, level_text, sizeof(level_filter)-1);
                g_free(level_text);
            }
        }
    }
    
    // Get selected gender
    char gender_filter[20] = "";
    if (radiobutton_male && gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(radiobutton_male))) {
        strcpy(gender_filter, "Male");
    } else if (radiobutton_female && gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(radiobutton_female))) {
        strcpy(gender_filter, "Female");
    } else if (radiobutton_all && gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(radiobutton_all))) {
        gender_filter[0] = '\0'; // "All" means empty
    }
    
    // Open coach.txt file
    FILE *file = fopen("coach.txt", "r");
    if (!file) {
        GtkWidget *dialog = gtk_message_dialog_new(GTK_WINDOW(window),
                                                  GTK_DIALOG_MODAL,
                                                  GTK_MESSAGE_ERROR,
                                                  GTK_BUTTONS_OK,
                                                  "Error: Could not open coach.txt!");
        gtk_dialog_run(GTK_DIALOG(dialog));
        gtk_widget_destroy(dialog);
        return;
    }
    
    char line[256];
    GtkTreeIter iter;
    int result_count = 0;
    
    // Read and filter coaches
    while (fgets(line, sizeof(line), file)) {
        line[strcspn(line, "\n")] = 0;
        
        // NEW FORMAT with gender: firstname|lastname|gender|speciality|level|availability
        char firstname[50], lastname[50], gender[20], speciality[50], level[50], availability[50];
        
        if (sscanf(line, "%[^|]|%[^|]|%[^|]|%[^|]|%[^|]|%[^\n]",
                   firstname, lastname, gender, speciality, level, availability) == 6) {
            
            int match = 1; // Assume match
            
            // Filter by gender (if selected)
            if (strlen(gender_filter) > 0) {
                if (strcmp(gender, gender_filter) != 0) {
                    match = 0;
                }
            }
            
            // Filter by speciality (if selected)
            if (match && strlen(speciality_filter) > 0) {
                if (strcmp(speciality, speciality_filter) != 0) {
                    match = 0;
                }
            }
            
            // Filter by level (if selected)
            if (match && strlen(level_filter) > 0) {
                if (strcmp(level, level_filter) != 0) {
                    match = 0;
                }
            }
            
            
            // If all filters match, add to treeview
            if (match) {
                gtk_list_store_append(store, &iter);
                gtk_list_store_set(store, &iter,
                                  0, firstname,
                                  1, lastname,
                                  2, gender,
                                  3, speciality,
                                  4, level,
                                  5, availability,
                                  -1);
                result_count++;
                g_print("Found: %s %s (%s - %s)\n", firstname, lastname, gender, speciality);
            }
        }
    }
    
    fclose(file);
    
    // Show results message
    if (result_count == 0) {
        GtkWidget *dialog = gtk_message_dialog_new(GTK_WINDOW(window),
                                                  GTK_DIALOG_MODAL,
                                                  GTK_MESSAGE_INFO,
                                                  GTK_BUTTONS_OK,
                                                  "No coaches found matching your criteria.");
        gtk_dialog_run(GTK_DIALOG(dialog));
        gtk_widget_destroy(dialog);
    } else {
        g_print("Search complete: Found %d coaches\n", result_count);
    }
}



void
on_deletmember_clicked                 (GtkWidget       *object,
                                        gpointer         user_data)
{
    GtkWidget *window = gtk_widget_get_toplevel(object);
    GtkWidget *treeview;
    GtkTreeSelection *selection;
    GtkTreeModel *model;
    GtkTreeIter iter;
    
    if (!window || !GTK_IS_WINDOW(window)) {
        g_print("Error: Could not find window\n");
        return;
    }
    
    // Get the treeview (treeview3 for delete window)
    treeview = lookup_widget(window, "treeview3");
    if (!treeview) {
        g_print("Error: Could not find treeview3 widget\n");
        return;
    }
    
    // Get the selection
    selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(treeview));
    if (!gtk_tree_selection_get_selected(selection, &model, &iter)) {
        GtkWidget *dialog = gtk_message_dialog_new(GTK_WINDOW(window),
                                                  GTK_DIALOG_MODAL,
                                                  GTK_MESSAGE_ERROR,
                                                  GTK_BUTTONS_OK,
                                                  "Please select a member to delete!");
        gtk_dialog_run(GTK_DIALOG(dialog));
        gtk_widget_destroy(dialog);
        return;
    }
    
    // Get the CIN of selected member
    gint cin;
    gchar *firstname, *lastname;
    
    gtk_tree_model_get(model, &iter,
                      0, &firstname,  // First Name
                      1, &lastname,   // Last Name
                      4, &cin,        // CIN
                      -1);
    
    // Ask for confirmation
    GtkWidget *confirm_dialog = gtk_message_dialog_new(GTK_WINDOW(window),
                                                      GTK_DIALOG_MODAL,
                                                      GTK_MESSAGE_QUESTION,
                                                      GTK_BUTTONS_YES_NO,
                                                      "Are you sure you want to delete %s %s (CIN: %d)?\n\nMember will be moved to restore.txt.",
                                                      firstname, lastname, cin);
    
    gint response = gtk_dialog_run(GTK_DIALOG(confirm_dialog));
    gtk_widget_destroy(confirm_dialog);
    
    if (response == GTK_RESPONSE_YES) {
        // Delete the member
        int result = supprimer("doc.txt", cin);
        
        GtkWidget *result_dialog;
        if (result == 1) {
            result_dialog = gtk_message_dialog_new(GTK_WINDOW(window),
                                                  GTK_DIALOG_MODAL,
                                                  GTK_MESSAGE_INFO,
                                                  GTK_BUTTONS_OK,
                                                  "Member %s %s deleted successfully!\nMember saved to restore.txt.",
                                                  firstname, lastname);
            
            // Remove the row from treeview
            GtkListStore *store = GTK_LIST_STORE(model);
            gtk_list_store_remove(store, &iter);
            
            // Refresh the treeview
            on_deletemember_show(object, NULL);
            
        } else {
            result_dialog = gtk_message_dialog_new(GTK_WINDOW(window),
                                                  GTK_DIALOG_MODAL,
                                                  GTK_MESSAGE_ERROR,
                                                  GTK_BUTTONS_OK,
                                                  "Error deleting member %s %s!",
                                                  firstname, lastname);
        }
        
        gtk_dialog_run(GTK_DIALOG(result_dialog));
        gtk_widget_destroy(result_dialog);
    }
    
    // Free memory
    g_free(firstname);
    g_free(lastname);
}


void
on_restormember_clicked                (GtkWidget       *object,
                                        gpointer         user_data)
{
    GtkWidget *window = gtk_widget_get_toplevel(object);
    GtkWidget *treeview;
    GtkTreeSelection *selection;
    GtkTreeModel *model;
    GtkTreeIter iter;
    
    if (!window || !GTK_IS_WINDOW(window)) {
        g_print("Error: Could not find window\n");
        return;
    }
    
    // Get the treeview (assuming treeview for restore window)
    treeview = lookup_widget(window, "treeview4"); // Change to your restore treeview name
    if (!treeview) {
        g_print("Error: Could not find treeview widget for restore\n");
        return;
    }
    
    // Get the selection
    selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(treeview));
    if (!gtk_tree_selection_get_selected(selection, &model, &iter)) {
        GtkWidget *dialog = gtk_message_dialog_new(GTK_WINDOW(window),
                                                  GTK_DIALOG_MODAL,
                                                  GTK_MESSAGE_ERROR,
                                                  GTK_BUTTONS_OK,
                                                  "Please select a member to restore!");
        gtk_dialog_run(GTK_DIALOG(dialog));
        gtk_widget_destroy(dialog);
        return;
    }
    
    // Get the CIN of selected member
    gint cin;
    gchar *firstname, *lastname;
    
    gtk_tree_model_get(model, &iter,
                      0, &firstname,  // First Name
                      1, &lastname,   // Last Name
                      4, &cin,        // CIN
                      -1);
    
    // Ask for confirmation
    GtkWidget *confirm_dialog = gtk_message_dialog_new(GTK_WINDOW(window),
                                                      GTK_DIALOG_MODAL,
                                                      GTK_MESSAGE_QUESTION,
                                                      GTK_BUTTONS_YES_NO,
                                                      "Are you sure you want to restore %s %s (CIN: %d)?\n\nMember will be moved back to doc.txt.",
                                                      firstname, lastname, cin);
    
    gint response = gtk_dialog_run(GTK_DIALOG(confirm_dialog));
    gtk_widget_destroy(confirm_dialog);
    
    if (response == GTK_RESPONSE_YES) {
        // Restore the member
        int result = restaurer("restore.txt", "doc.txt", cin);
        
        GtkWidget *result_dialog;
        if (result == 1) {
            result_dialog = gtk_message_dialog_new(GTK_WINDOW(window),
                                                  GTK_DIALOG_MODAL,
                                                  GTK_MESSAGE_INFO,
                                                  GTK_BUTTONS_OK,
                                                  "Member %s %s restored successfully!\nMember moved back to doc.txt.",
                                                  firstname, lastname);
            
            // Remove the row from restore treeview
            GtkListStore *store = GTK_LIST_STORE(model);
            gtk_list_store_remove(store, &iter);
            
            // Refresh the restore treeview
            on_restoremember_show(object, NULL);
            
            // Also refresh the main treeview if visible
            GtkWidget *main_treeview = lookup_widget(window, "treeview1");
            if (main_treeview) {
                on_recherchemember_show(object, NULL);
            }
            
        } else {
            result_dialog = gtk_message_dialog_new(GTK_WINDOW(window),
                                                  GTK_DIALOG_MODAL,
                                                  GTK_MESSAGE_ERROR,
                                                  GTK_BUTTONS_OK,
                                                  "Error restoring member %s %s!\nMember not found in restore.txt.",
                                                  firstname, lastname);
        }
        
        gtk_dialog_run(GTK_DIALOG(result_dialog));
        gtk_widget_destroy(result_dialog);
    }
    
    // Free memory
    g_free(firstname);
    g_free(lastname);
}



void
on_recherchemember_show(GtkWidget *widget, gpointer user_data)
{
    GtkWidget *treeview;
    GtkListStore *store;
    GtkTreeIter iter;
    FILE *file;
    char line[256];
    member m;
    
    // Get the treeview widget
    treeview = lookup_widget(widget, "treeview1");
    
    if (!treeview) {
        g_print("Error: Could not find treeview widget\n");
        return;
    }
    
    // Setup treeview if not already done
    if (!gtk_tree_view_get_model(GTK_TREE_VIEW(treeview))) {
        setup_treeview_columns(treeview);
    }
    
    // Get the store from treeview
    store = GTK_LIST_STORE(gtk_tree_view_get_model(GTK_TREE_VIEW(treeview)));
    
    // Clear existing data
    gtk_list_store_clear(store);
    
    // Open the file
    file = fopen("doc.txt", "r");
    if (!file) {
        g_print("Error: Could not open doc.txt\n");
        return;
    }
    
    g_print("Loading data from doc.txt...\n");
    
    // Read and parse each line
    while (fgets(line, sizeof(line), file)) {
        line[strcspn(line, "\n")] = 0; // Remove newline
        
        // Parse with pipe separator
        if (sscanf(line, "%[^|]|%[^|]|%[^|]|%[^|]|%d|%[^|]|%f|%[^|]|%[^|]|%[^\n]",
                   m.firstname, m.lastname, m.phonenumber, m.gender, 
                   &m.cin, m.startdate, &m.amountpaid, m.username, 
                   m.password, m.monthsub) == 10) {
            
            // Add new row
            gtk_list_store_append(store, &iter);
            
            // Set ALL 10 columns (we still store username/password, just don't display them)
            gtk_list_store_set(store, &iter,
                              0, m.firstname,      // Column 0: First Name
                              1, m.lastname,       // Column 1: Last Name  
                              2, m.phonenumber,    // Column 2: Phone
                              3, m.gender,         // Column 3: Gender
                              4, m.cin,            // Column 4: CIN
                              5, m.startdate,      // Column 5: Start Date
                              6, m.amountpaid,     // Column 6: Amount Paid
                              7, m.username,       // Column 7: Username (stored but hidden)
                              8, m.password,       // Column 8: Password (stored but hidden)
                              9, m.monthsub,       // Column 9: Month Sub
                              -1);
            
            g_print("Loaded: %s %s\n", m.firstname, m.lastname);
        } else {
            g_print("Failed to parse line: %s\n", line);
        }
    }
    
    fclose(file);
    g_print("Data loaded successfully!\n");
}

void
on_cancelsearchlist_clicked            (GtkWidget       *object,
                                        gpointer         user_data)
{
GtkWidget *window = gtk_widget_get_toplevel(object);
    GtkWidget *treeview;
    GtkWidget *entry_cin;
    GtkWidget *entry_phone;
    GtkWidget *radiobutton_male;
    GtkWidget *radiobutton_female;
    GtkWidget *radiobutton_all;
    
    if (!window || !GTK_IS_WINDOW(window)) {
        g_print("Error: Could not find window\n");
        return;
    }
    
    // Get the treeview
    treeview = lookup_widget(window, "treeview1");
    if (!treeview) {
        g_print("Error: Could not find treeview widget\n");
        return;
    }
    
    // Clear search fields
    entry_cin = lookup_widget(window, "entry_cin_search");
    entry_phone = lookup_widget(window, "entry_phone_search");
    radiobutton_male = lookup_widget(window, "radiobutton_male");
    radiobutton_female = lookup_widget(window, "radiobutton_female");
    radiobutton_all = lookup_widget(window, "radiobutton_all");
    
    if (entry_cin) {
        gtk_entry_set_text(GTK_ENTRY(entry_cin), "");
    }
    
    if (entry_phone) {
        gtk_entry_set_text(GTK_ENTRY(entry_phone), "");
    }
    
    // Set "All" as default radio button
    if (radiobutton_all) {
        gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(radiobutton_all), TRUE);
    }
    
    // Setup treeview if needed
    if (!gtk_tree_view_get_model(GTK_TREE_VIEW(treeview))) {
        setup_treeview_columns(treeview);
    }
    
    // Get the store
    GtkListStore *store = GTK_LIST_STORE(gtk_tree_view_get_model(GTK_TREE_VIEW(treeview)));
    
    // Clear existing data
    gtk_list_store_clear(store);
    
    // Load ALL data from file
    FILE *file = fopen("doc.txt", "r");
    if (!file) {
        g_print("Error: Could not open doc.txt\n");
        return;
    }
    
    char line[256];
    member m;
    GtkTreeIter iter;
    
    while (fgets(line, sizeof(line), file)) {
        line[strcspn(line, "\n")] = 0; // Remove newline
        
        if (sscanf(line, "%[^|]|%[^|]|%[^|]|%[^|]|%d|%[^|]|%f|%[^|]|%[^|]|%[^\n]",
                   m.firstname, m.lastname, m.phonenumber, m.gender, 
                   &m.cin, m.startdate, &m.amountpaid, m.username, 
                   m.password, m.monthsub) == 10) {
            
            gtk_list_store_append(store, &iter);
            gtk_list_store_set(store, &iter,
                              0, m.firstname,
                              1, m.lastname,
                              2, m.phonenumber,
                              3, m.gender,
                              4, m.cin,
                              5, m.startdate,
                              6, m.amountpaid,
                              7, m.username,
                              8, m.password,
                              9, m.monthsub,
                              -1);
        }
    }
    
    fclose(file);
    g_print("Search canceled - showing all members\n"); 
}


void
on_modifylsit_clicked                  (GtkWidget       *object,
                                        gpointer         user_data)
{
    GtkWidget *window = gtk_widget_get_toplevel(object);
    GtkWidget *treeview;
    GtkWidget *textview;
    GtkTreeSelection *selection;
    GtkTreeModel *model;
    GtkTreeIter iter;
    
    if (!window || !GTK_IS_WINDOW(window)) {
        g_print("Error: Could not find window\n");
        return;
    }
    
    // Get the treeview
    treeview = lookup_widget(window, "treeview1");
    if (!treeview) {
        g_print("Error: Could not find treeview widget\n");
        return;
    }
    
    // Get the textview
    textview = lookup_widget(window, "textview2");
    if (!textview) {
        GtkWidget *dialog = gtk_message_dialog_new(GTK_WINDOW(window),
                                                  GTK_DIALOG_MODAL,
                                                  GTK_MESSAGE_ERROR,
                                                  GTK_BUTTONS_OK,
                                                  "Error: Could not find textview!");
        gtk_dialog_run(GTK_DIALOG(dialog));
        gtk_widget_destroy(dialog);
        return;
    }
    
    // Get the selection from treeview
    selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(treeview));
    if (!gtk_tree_selection_get_selected(selection, &model, &iter)) {
        GtkWidget *dialog = gtk_message_dialog_new(GTK_WINDOW(window),
                                                  GTK_DIALOG_MODAL,
                                                  GTK_MESSAGE_ERROR,
                                                  GTK_BUTTONS_OK,
                                                  "Please select a member to modify!");
        gtk_dialog_run(GTK_DIALOG(dialog));
        gtk_widget_destroy(dialog);
        return;
    }
    
    // Get current member data (to keep unchanged fields)
    gchar *firstname, *lastname, *phone, *gender, *startdate, *username, *password, *monthsub;
    gint cin;
    gfloat amountpaid;
    
    gtk_tree_model_get(model, &iter,
                      0, &firstname,     // First Name
                      1, &lastname,      // Last Name
                      2, &phone,         // Phone Number (might change)
                      3, &gender,        // Gender
                      4, &cin,           // CIN (unique ID - won't change)
                      5, &startdate,     // Start Date (might change)
                      6, &amountpaid,    // Amount Paid (might change)
                      7, &username,      // Username
                      8, &password,      // Password
                      9, &monthsub,      // Month Subscription (might change)
                      -1);
    
    // Get the text from textview
    GtkTextBuffer *buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(textview));
    GtkTextIter start, end;
    gtk_text_buffer_get_start_iter(buffer, &start);
    gtk_text_buffer_get_end_iter(buffer, &end);
    gchar *new_text = gtk_text_buffer_get_text(buffer, &start, &end, FALSE);
    
    // Parse the new text to extract updated values
    // Format expected: "Phone Number: X\nStart Date: Y\nAmount Paid: Z\nMonth Subscription: W"
    char new_phone[50] = "";
    char new_startdate[50] = "";
    float new_amountpaid = 0;
    char new_monthsub[50] = "";
    
    // Simple parsing - adjust based on your exact format
    char *line = strtok(new_text, "\n");
    while (line != NULL) {
        if (strstr(line, "Phone Number:") != NULL) {
            sscanf(line, "Phone Number: %49[^\n]", new_phone);
        } else if (strstr(line, "Start Date:") != NULL) {
            sscanf(line, "Start Date: %49[^\n]", new_startdate);
        } else if (strstr(line, "Amount Paid:") != NULL) {
            sscanf(line, "Amount Paid: %f", &new_amountpaid);
        } else if (strstr(line, "Month Subscription:") != NULL) {
            sscanf(line, "Month Subscription: %49[^\n]", new_monthsub);
        }
        line = strtok(NULL, "\n");
    }
    
    g_free(new_text);
    
    // Create updated member structure
    member updated_member;
    
    // Copy existing data
    strncpy(updated_member.firstname, firstname, sizeof(updated_member.firstname)-1);
    strncpy(updated_member.lastname, lastname, sizeof(updated_member.lastname)-1);
    strncpy(updated_member.gender, gender, sizeof(updated_member.gender)-1);
    strncpy(updated_member.username, username, sizeof(updated_member.username)-1);
    strncpy(updated_member.password, password, sizeof(updated_member.password)-1);
    updated_member.cin = cin;
    
    // Use new values if provided, otherwise keep old ones
    if (strlen(new_phone) > 0) {
        strncpy(updated_member.phonenumber, new_phone, sizeof(updated_member.phonenumber)-1);
    } else {
        strncpy(updated_member.phonenumber, phone, sizeof(updated_member.phonenumber)-1);
    }
    
    if (strlen(new_startdate) > 0) {
        strncpy(updated_member.startdate, new_startdate, sizeof(updated_member.startdate)-1);
    } else {
        strncpy(updated_member.startdate, startdate, sizeof(updated_member.startdate)-1);
    }
    
    if (new_amountpaid > 0) {
        updated_member.amountpaid = new_amountpaid;
    } else {
        updated_member.amountpaid = amountpaid;
    }
    
    if (strlen(new_monthsub) > 0) {
        strncpy(updated_member.monthsub, new_monthsub, sizeof(updated_member.monthsub)-1);
    } else {
        strncpy(updated_member.monthsub, monthsub, sizeof(updated_member.monthsub)-1);
    }
    
    // Call modifier function from member.c
    int result = modifier("doc.txt", cin, updated_member);
    
    // Free memory
    g_free(firstname);
    g_free(lastname);
    g_free(phone);
    g_free(gender);
    g_free(startdate);
    g_free(username);
    g_free(password);
    g_free(monthsub);
    
    // Show result
    GtkWidget *dialog;
    if (result == 1) {
        dialog = gtk_message_dialog_new(GTK_WINDOW(window),
                                       GTK_DIALOG_MODAL,
                                       GTK_MESSAGE_INFO,
                                       GTK_BUTTONS_OK,
                                       "Member information updated successfully!");
        
        // Refresh the treeview to show updated data
        on_recherchemember_show(object, NULL);
        
    } else {
        dialog = gtk_message_dialog_new(GTK_WINDOW(window),
                                       GTK_DIALOG_MODAL,
                                       GTK_MESSAGE_ERROR,
                                       GTK_BUTTONS_OK,
                                       "Error updating member information!");
    }
    
    gtk_dialog_run(GTK_DIALOG(dialog));
    gtk_widget_destroy(dialog);
}


void
on_treeview1_cursor_changed            (GtkTreeView     *treeview,
                                        gpointer         user_data)
{
    GtkWidget *window;
    GtkWidget *textview;
    GtkTreeSelection *selection;
    GtkTreeModel *model;
    GtkTreeIter iter;
    
    window = gtk_widget_get_toplevel(GTK_WIDGET(treeview));
    if (!window || !GTK_IS_WINDOW(window)) {
        return;
    }
    
    textview = lookup_widget(GTK_WIDGET(treeview), "textview2");
    if (!textview) {
        g_print("Error: Could not find textview_details widget\n");
        return;
    }
    
    GtkTextBuffer *buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(textview));
    selection = gtk_tree_view_get_selection(treeview);
    
    if (gtk_tree_selection_get_selected(selection, &model, &iter)) {
        // Get ONLY the 4 requested fields
        gchar *phone, *startdate, *monthsub;
        gfloat amountpaid;
        
        gtk_tree_model_get(model, &iter,
                          2, &phone,         // Column 2: Phone Number
                          5, &startdate,     // Column 5: Start Date
                          6, &amountpaid,    // Column 6: Amount Paid
                          9, &monthsub,      // Column 9: Month Subscription
                          -1);
        
        // Create formatted text
        gchar *details = g_strdup_printf(
            "Phone Number: %s\n"
            "Start Date: %s\n"
            "Amount Paid: %.2f\n"
            "Month Subscription: %s\n",
            phone,
            startdate,
            amountpaid,
            monthsub);
        
        gtk_text_buffer_set_text(buffer, details, -1);
        
        // Free memory
        g_free(phone);
        g_free(startdate);
        g_free(monthsub);
        g_free(details);
    } else {
        gtk_text_buffer_set_text(buffer, "Select a member to view details", -1);
    }
}


void
on_deletemember_show                   (GtkWidget       *widget,
                                        gpointer         user_data)
{
    GtkWidget *treeview;
    GtkListStore *store;
    GtkTreeIter iter;
    FILE *file;
    char line[256];
    member m;
    
    // Get the treeview widget - CHANGED: treeview3 instead of treeview1
    treeview = lookup_widget(widget, "treeview3");
    
    if (!treeview) {
        g_print("Error: Could not find treeview3 widget\n");
        return;
    }
    
    // Setup treeview if not already done
    if (!gtk_tree_view_get_model(GTK_TREE_VIEW(treeview))) {
        // You can reuse the same setup function or create a separate one
        setup_treeview_columns(treeview); // This should work for both
    }
    
    // Get the store from treeview
    store = GTK_LIST_STORE(gtk_tree_view_get_model(GTK_TREE_VIEW(treeview)));
    
    // Clear existing data
    gtk_list_store_clear(store);
    
    // Open the file
    file = fopen("doc.txt", "r");
    if (!file) {
        g_print("Error: Could not open doc.txt\n");
        return;
    }
    
    g_print("Loading data from doc.txt into treeview3...\n");
    
    // Read and parse each line
    while (fgets(line, sizeof(line), file)) {
        line[strcspn(line, "\n")] = 0; // Remove newline
        
        // Parse with pipe separator
        if (sscanf(line, "%[^|]|%[^|]|%[^|]|%[^|]|%d|%[^|]|%f|%[^|]|%[^|]|%[^\n]",
                   m.firstname, m.lastname, m.phonenumber, m.gender, 
                   &m.cin, m.startdate, &m.amountpaid, m.username, 
                   m.password, m.monthsub) == 10) {
            
            // Add new row
            gtk_list_store_append(store, &iter);
            
            // Set ALL 10 columns (we still store username/password, just don't display them)
            gtk_list_store_set(store, &iter,
                              0, m.firstname,      // Column 0: First Name
                              1, m.lastname,       // Column 1: Last Name  
                              2, m.phonenumber,    // Column 2: Phone
                              3, m.gender,         // Column 3: Gender
                              4, m.cin,            // Column 4: CIN
                              5, m.startdate,      // Column 5: Start Date
                              6, m.amountpaid,     // Column 6: Amount Paid
                              7, m.username,       // Column 7: Username (stored but hidden)
                              8, m.password,       // Column 8: Password (stored but hidden)
                              9, m.monthsub,       // Column 9: Month Sub
                              -1);
            
            g_print("Loaded into treeview3: %s %s\n", m.firstname, m.lastname);
        } else {
            g_print("Failed to parse line: %s\n", line);
        }
    }
    
    fclose(file);
    g_print("Data loaded successfully into treeview3!\n");
}


void
on_cancelsearchdel_clicked             (GtkWidget       *object,
                                        gpointer         user_data)
{
GtkWidget *window = gtk_widget_get_toplevel(object);
    GtkWidget *treeview;
    GtkWidget *entry_cin;
    GtkWidget *entry_phone;
    GtkWidget *radiobutton_male;
    GtkWidget *radiobutton_female;
    GtkWidget *radiobutton_all;
    
    if (!window || !GTK_IS_WINDOW(window)) {
        g_print("Error: Could not find window\n");
        return;
    }
    
    // Get the treeview
    treeview = lookup_widget(window, "treeview3");
    if (!treeview) {
        g_print("Error: Could not find treeview widget\n");
        return;
    }
    
    // Clear search fields
    entry_cin = lookup_widget(window, "entry15");
    entry_phone = lookup_widget(window, "entry16");
    radiobutton_male = lookup_widget(window, "radiobutton7");
    radiobutton_female = lookup_widget(window, "radiobutton8");
    radiobutton_all = lookup_widget(window, "radiobutton15");
    
    if (entry_cin) {
        gtk_entry_set_text(GTK_ENTRY(entry_cin), "");
    }
    
    if (entry_phone) {
        gtk_entry_set_text(GTK_ENTRY(entry_phone), "");
    }
    
    // Set "All" as default radio button
    if (radiobutton_all) {
        gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(radiobutton_all), TRUE);
    }
    
    // Setup treeview if needed
    if (!gtk_tree_view_get_model(GTK_TREE_VIEW(treeview))) {
        setup_treeview_columns(treeview);
    }
    
    // Get the store
    GtkListStore *store = GTK_LIST_STORE(gtk_tree_view_get_model(GTK_TREE_VIEW(treeview)));
    
    // Clear existing data
    gtk_list_store_clear(store);
    
    // Load ALL data from file
    FILE *file = fopen("doc.txt", "r");
    if (!file) {
        g_print("Error: Could not open doc.txt\n");
        return;
    }
    
    char line[256];
    member m;
    GtkTreeIter iter;
    
    while (fgets(line, sizeof(line), file)) {
        line[strcspn(line, "\n")] = 0; // Remove newline
        
        if (sscanf(line, "%[^|]|%[^|]|%[^|]|%[^|]|%d|%[^|]|%f|%[^|]|%[^|]|%[^\n]",
                   m.firstname, m.lastname, m.phonenumber, m.gender, 
                   &m.cin, m.startdate, &m.amountpaid, m.username, 
                   m.password, m.monthsub) == 10) {
            
            gtk_list_store_append(store, &iter);
            gtk_list_store_set(store, &iter,
                              0, m.firstname,
                              1, m.lastname,
                              2, m.phonenumber,
                              3, m.gender,
                              4, m.cin,
                              5, m.startdate,
                              6, m.amountpaid,
                              7, m.username,
                              8, m.password,
                              9, m.monthsub,
                              -1);
        }
    }
    
    fclose(file);
    g_print("Search canceled - showing all members\n"); 
}


void
on_restoremember_show                  (GtkWidget       *widget,
                                        gpointer         user_data)
{
 GtkWidget *treeview;
    GtkListStore *store;
    GtkTreeIter iter;
    FILE *file;
    char line[256];
    member m;
    
    // Get the treeview widget - CHANGED: treeview4 instead of treeview1
    treeview = lookup_widget(widget, "treeview4");
    
    if (!treeview) {
        g_print("Error: Could not find treeview3 widget\n");
        return;
    }
    
    // Setup treeview if not already done
    if (!gtk_tree_view_get_model(GTK_TREE_VIEW(treeview))) {
        // You can reuse the same setup function or create a separate one
        setup_treeview_columns(treeview); // This should work for both
    }
    
    // Get the store from treeview
    store = GTK_LIST_STORE(gtk_tree_view_get_model(GTK_TREE_VIEW(treeview)));
    
    // Clear existing data
    gtk_list_store_clear(store);
    
    // Open the file
    file = fopen("restore.txt", "r");
    if (!file) {
        g_print("Error: Could not open doc.txt\n");
        return;
    }
    
    g_print("Loading data from restore.txt into treeview3...\n");
    
    // Read and parse each line
    while (fgets(line, sizeof(line), file)) {
        line[strcspn(line, "\n")] = 0; // Remove newline
        
        // Parse with pipe separator
        if (sscanf(line, "%[^|]|%[^|]|%[^|]|%[^|]|%d|%[^|]|%f|%[^|]|%[^|]|%[^\n]",
                   m.firstname, m.lastname, m.phonenumber, m.gender, 
                   &m.cin, m.startdate, &m.amountpaid, m.username, 
                   m.password, m.monthsub) == 10) {
            
            // Add new row
            gtk_list_store_append(store, &iter);
            
            // Set ALL 10 columns (we still store username/password, just don't display them)
            gtk_list_store_set(store, &iter,
                              0, m.firstname,      // Column 0: First Name
                              1, m.lastname,       // Column 1: Last Name  
                              2, m.phonenumber,    // Column 2: Phone
                              3, m.gender,         // Column 3: Gender
                              4, m.cin,            // Column 4: CIN
                              5, m.startdate,      // Column 5: Start Date
                              6, m.amountpaid,     // Column 6: Amount Paid
                              7, m.username,       // Column 7: Username (stored but hidden)
                              8, m.password,       // Column 8: Password (stored but hidden)
                              9, m.monthsub,       // Column 9: Month Sub
                              -1);
            
            g_print("Loaded into treeview4: %s %s\n", m.firstname, m.lastname);
        } else {
            g_print("Failed to parse line: %s\n", line);
        }
    }
    
    fclose(file);
    g_print("Data loaded successfully into treeview3!\n");
}


void
on_cancelsearchrestore_clicked         (GtkWidget       *object,
                                        gpointer         user_data)
{
GtkWidget *window = gtk_widget_get_toplevel(object);
    GtkWidget *treeview;
    GtkWidget *entry_cin;
    GtkWidget *entry_phone;
    GtkWidget *radiobutton_male;
    GtkWidget *radiobutton_female;
    GtkWidget *radiobutton_all;
    
    if (!window || !GTK_IS_WINDOW(window)) {
        g_print("Error: Could not find window\n");
        return;
    }
    
    // Get the treeview
    treeview = lookup_widget(window, "treeview4");
    if (!treeview) {
        g_print("Error: Could not find treeview widget\n");
        return;
    }
    
    // Clear search fields
    entry_cin = lookup_widget(window, "entry17");
    entry_phone = lookup_widget(window, "entry18");
    radiobutton_male = lookup_widget(window, "radiobutton9");
    radiobutton_female = lookup_widget(window, "radiobutton10");
    radiobutton_all = lookup_widget(window, "radiobutton16");
    
    if (entry_cin) {
        gtk_entry_set_text(GTK_ENTRY(entry_cin), "");
    }
    
    if (entry_phone) {
        gtk_entry_set_text(GTK_ENTRY(entry_phone), "");
    }
    
    // Set "All" as default radio button
    if (radiobutton_all) {
        gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(radiobutton_all), TRUE);
    }
    
    // Setup treeview if needed
    if (!gtk_tree_view_get_model(GTK_TREE_VIEW(treeview))) {
        setup_treeview_columns(treeview);
    }
    
    // Get the store
    GtkListStore *store = GTK_LIST_STORE(gtk_tree_view_get_model(GTK_TREE_VIEW(treeview)));
    
    // Clear existing data
    gtk_list_store_clear(store);
    
    // Load ALL data from file
    FILE *file = fopen("restore.txt", "r");
    if (!file) {
        g_print("Error: Could not open doc.txt\n");
        return;
    }
    
    char line[256];
    member m;
    GtkTreeIter iter;
    
    while (fgets(line, sizeof(line), file)) {
        line[strcspn(line, "\n")] = 0; // Remove newline
        
        if (sscanf(line, "%[^|]|%[^|]|%[^|]|%[^|]|%d|%[^|]|%f|%[^|]|%[^|]|%[^\n]",
                   m.firstname, m.lastname, m.phonenumber, m.gender, 
                   &m.cin, m.startdate, &m.amountpaid, m.username, 
                   m.password, m.monthsub) == 10) {
            
            gtk_list_store_append(store, &iter);
            gtk_list_store_set(store, &iter,
                              0, m.firstname,
                              1, m.lastname,
                              2, m.phonenumber,
                              3, m.gender,
                              4, m.cin,
                              5, m.startdate,
                              6, m.amountpaid,
                              7, m.username,
                              8, m.password,
                              9, m.monthsub,
                              -1);
        }
    }
    
    fclose(file);
    g_print("Search canceled - showing all members\n"); 
}




void
on_Login_show                          (GtkWidget       *widget,
                                        gpointer         user_data)
{
    GtkWidget *entry_password;
    GtkWidget *checkbutton_show;
    
    // Get the password entry
    entry_password = lookup_widget(widget, "entry2");
    if (entry_password) {
        // Set password to hidden by default
        gtk_entry_set_visibility(GTK_ENTRY(entry_password), FALSE);
    }
    
    // Get the checkbutton and set it to unchecked
    checkbutton_show = lookup_widget(widget, "checkbutton1");
    if (checkbutton_show) {
        gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(checkbutton_show), FALSE);
    }
}


void
on_gender_toggled                      (GtkToggleButton *togglebutton,
                                        gpointer         user_data)
{
    GtkWidget *window = gtk_widget_get_toplevel(GTK_WIDGET(togglebutton));
    GtkWidget *image = lookup_widget(window, "image85");
    
    if (!image) return;
    
    if (gtk_toggle_button_get_active(togglebutton)) {
        // Instead of using widget name, check which radio button it is by position
        // OR check button label
        
        GtkWidget *radiobutton1 = lookup_widget(window, "radiobutton1");
        GtkWidget *radiobutton2 = lookup_widget(window, "radiobutton2");
        
        if (!radiobutton1 || !radiobutton2) {
            g_print("ERROR: Radio buttons not found by name!\n");
            return;
        }
        
        // Check if this is radiobutton1
        if (togglebutton == GTK_TOGGLE_BUTTON(radiobutton1)) {
            g_print("Male button clicked\n");
            
            // Use ABSOLUTE path - CHANGE THIS!
            const char *male_path = "/home/ali/Projects/PROJET/pourvalider/PROJET/src/male.png";
            
            if (g_file_test(male_path, G_FILE_TEST_EXISTS)) {
                gtk_image_set_from_file(GTK_IMAGE(image), male_path);
                g_print("Male image loaded\n");
            } else {
                // Fallback color
                GdkPixbuf *pixbuf = gdk_pixbuf_new(GDK_COLORSPACE_RGB, FALSE, 8, 200, 200);
                gdk_pixbuf_fill(pixbuf, 0x0000FF00); // Blue
                gtk_image_set_from_pixbuf(GTK_IMAGE(image), pixbuf);
                g_object_unref(pixbuf);
                g_print("Male image not found, using blue\n");
            }
            
        } else if (togglebutton == GTK_TOGGLE_BUTTON(radiobutton2)) {
            g_print("Female button clicked\n");
            
            // Use ABSOLUTE path - CHANGE THIS!
            const char *female_path = "/home/ali/Projects/PROJET/pourvalider/PROJET/src/female.png";
            
            if (g_file_test(female_path, G_FILE_TEST_EXISTS)) {
                gtk_image_set_from_file(GTK_IMAGE(image), female_path);
                g_print("Female image loaded\n");
            } else {
                // Fallback color
                GdkPixbuf *pixbuf = gdk_pixbuf_new(GDK_COLORSPACE_RGB, FALSE, 8, 200, 200);
                gdk_pixbuf_fill(pixbuf, 0xFF00FF00); // Pink
                gtk_image_set_from_pixbuf(GTK_IMAGE(image), pixbuf);
                g_object_unref(pixbuf);
                g_print("Female image not found, using pink\n");
            }
        }
    }
}


void
on_cancelsearReq_clicked               (GtkWidget       *widget,
                                        gpointer         user_data)
{
    GtkWidget *window = gtk_widget_get_toplevel(widget);
    GtkWidget *treeview;
    GtkWidget *combobox_speciality;
    GtkWidget *combobox_level;
    GtkWidget *radiobutton_male;
    GtkWidget *radiobutton_female;
    GtkWidget *radiobutton_all;
    
    if (!window || !GTK_IS_WINDOW(window)) {
        g_print("Error: Could not find window\n");
        return;
    }
    
    // Get the treeview
    treeview = lookup_widget(window, "treeview5"); // Coach treeview
    if (!treeview) {
        g_print("Error: Could not find treeview5 widget\n");
        return;
    }
    
    // Clear search fields
    combobox_speciality = lookup_widget(window, "combobox1");
    combobox_level = lookup_widget(window, "combobox2");
    radiobutton_male = lookup_widget(window, "radiobutton12");
    radiobutton_female = lookup_widget(window, "radiobutton13");
    radiobutton_all = lookup_widget(window, "radiobutton18");
    
    // Reset comboboxes to "Select" option
    if (combobox_speciality) {
        gtk_combo_box_set_active(GTK_COMBO_BOX(combobox_speciality), 0); // First item
    }
    
    if (combobox_level) {
        gtk_combo_box_set_active(GTK_COMBO_BOX(combobox_level), 0);
    }
    
    // Set "All" as default radio button for gender
    if (radiobutton_all) {
        gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(radiobutton_all), TRUE);
    }
    
    // Setup treeview if needed
    if (!gtk_tree_view_get_model(GTK_TREE_VIEW(treeview))) {
        // Create model with 6 columns (firstname, lastname, gender, speciality, level, availability)
        GtkListStore *store = gtk_list_store_new(6,
                              G_TYPE_STRING,  // 0: firstname
                              G_TYPE_STRING,  // 1: lastname
                              G_TYPE_STRING,  // 2: gender
                              G_TYPE_STRING,  // 3: speciality
                              G_TYPE_STRING,  // 4: level
                              G_TYPE_STRING); // 5: availability
        
        gtk_tree_view_set_model(GTK_TREE_VIEW(treeview), GTK_TREE_MODEL(store));
        
        // Create columns
        GtkCellRenderer *renderer;
        GtkTreeViewColumn *column;
        
        // Column 0: First Name
        renderer = gtk_cell_renderer_text_new();
        column = gtk_tree_view_column_new_with_attributes("First Name", renderer, "text", 0, NULL);
        gtk_tree_view_append_column(GTK_TREE_VIEW(treeview), column);
        
        // Column 1: Last Name
        renderer = gtk_cell_renderer_text_new();
        column = gtk_tree_view_column_new_with_attributes("Last Name", renderer, "text", 1, NULL);
        gtk_tree_view_append_column(GTK_TREE_VIEW(treeview), column);
        
        // Column 2: Gender
        renderer = gtk_cell_renderer_text_new();
        column = gtk_tree_view_column_new_with_attributes("Gender", renderer, "text", 2, NULL);
        gtk_tree_view_append_column(GTK_TREE_VIEW(treeview), column);
        
        // Column 3: Speciality
        renderer = gtk_cell_renderer_text_new();
        column = gtk_tree_view_column_new_with_attributes("Speciality", renderer, "text", 3, NULL);
        gtk_tree_view_append_column(GTK_TREE_VIEW(treeview), column);
        
        // Column 4: Level
        renderer = gtk_cell_renderer_text_new();
        column = gtk_tree_view_column_new_with_attributes("Level", renderer, "text", 4, NULL);
        gtk_tree_view_append_column(GTK_TREE_VIEW(treeview), column);
        
        // Column 5: Availability
        renderer = gtk_cell_renderer_text_new();
        column = gtk_tree_view_column_new_with_attributes("Availability", renderer, "text", 5, NULL);
        gtk_tree_view_append_column(GTK_TREE_VIEW(treeview), column);
    }
    
    // Get the store
    GtkListStore *store = GTK_LIST_STORE(gtk_tree_view_get_model(GTK_TREE_VIEW(treeview)));
    
    // Clear existing data
    gtk_list_store_clear(store);
    
    // Load ALL data from coach.txt
    FILE *file = fopen("coach.txt", "r");
    if (!file) {
        g_print("Error: Could not open coach.txt\n");
        return;
    }
    
    char line[256];
    GtkTreeIter iter;
    
    while (fgets(line, sizeof(line), file)) {
        line[strcspn(line, "\n")] = 0; // Remove newline
        
        // Parse: firstname|lastname|gender|speciality|level|availability
        char firstname[50], lastname[50], gender[20], speciality[50], level[50], availability[50];
        
        if (sscanf(line, "%[^|]|%[^|]|%[^|]|%[^|]|%[^|]|%[^\n]",
                   firstname, lastname, gender, speciality, level, availability) == 6) {
            
            gtk_list_store_append(store, &iter);
            gtk_list_store_set(store, &iter,
                              0, firstname,
                              1, lastname,
                              2, gender,
                              3, speciality,
                              4, level,
                              5, availability,
                              -1);
        }
    }
    
    fclose(file);
    g_print("Coach search canceled - showing all coaches\n");
}


void
on_reqcoach_show(GtkWidget *widget, gpointer user_data)
{
    GtkWidget *combobox;
    GtkWidget *treeview;
    
    // ==================== PART 1: Load Combobox ====================
    combobox = lookup_widget(widget, "combobox1");
    
    if (combobox) {
        // Clear existing items by creating new model
        GtkListStore *store = gtk_list_store_new(1, G_TYPE_STRING);
        gtk_combo_box_set_model(GTK_COMBO_BOX(combobox), GTK_TREE_MODEL(store));
        g_object_unref(store);
        
        // Add "Select" option
        gtk_combo_box_append_text(GTK_COMBO_BOX(combobox), "-- Select Specialty --");
        
        // Load from Speciality.txt
        FILE *f = fopen("Speciality.txt", "r");
        if (f) {
            char line[100];
            while (fgets(line, sizeof(line), f)) {
                line[strcspn(line, "\n")] = 0;
                if (strlen(line) > 0) {
                    gtk_combo_box_append_text(GTK_COMBO_BOX(combobox), line);
                }
            }
            fclose(f);
        }
        
        // Set first item active
        gtk_combo_box_set_active(GTK_COMBO_BOX(combobox), 0);
    }
    
    // ==================== PART 2: Load TreeView ====================
    treeview = lookup_widget(widget, "treeview5");
    
    if (treeview) {
        // Setup treeview columns if needed
        if (!gtk_tree_view_get_model(GTK_TREE_VIEW(treeview))) {
            // NOW WITH 6 COLUMNS: firstname, lastname, gender, speciality, level, availability
            GtkListStore *store = gtk_list_store_new(6,
                                  G_TYPE_STRING,  // 0: firstname
                                  G_TYPE_STRING,  // 1: lastname
                                  G_TYPE_STRING,  // 2: gender
                                  G_TYPE_STRING,  // 3: speciality
                                  G_TYPE_STRING,  // 4: level
                                  G_TYPE_STRING); // 5: availability
            
            gtk_tree_view_set_model(GTK_TREE_VIEW(treeview), GTK_TREE_MODEL(store));
            
            // Create columns
            GtkCellRenderer *renderer;
            GtkTreeViewColumn *column;
            
            // Column 0: First Name
            renderer = gtk_cell_renderer_text_new();
            column = gtk_tree_view_column_new_with_attributes("First Name", renderer, "text", 0, NULL);
            gtk_tree_view_append_column(GTK_TREE_VIEW(treeview), column);
            
            // Column 1: Last Name
            renderer = gtk_cell_renderer_text_new();
            column = gtk_tree_view_column_new_with_attributes("Last Name", renderer, "text", 1, NULL);
            gtk_tree_view_append_column(GTK_TREE_VIEW(treeview), column);
            
            // Column 2: Gender (NEW)
            renderer = gtk_cell_renderer_text_new();
            column = gtk_tree_view_column_new_with_attributes("Gender", renderer, "text", 2, NULL);
            gtk_tree_view_append_column(GTK_TREE_VIEW(treeview), column);
            
            // Column 3: Speciality
            renderer = gtk_cell_renderer_text_new();
            column = gtk_tree_view_column_new_with_attributes("Speciality", renderer, "text", 3, NULL);
            gtk_tree_view_append_column(GTK_TREE_VIEW(treeview), column);
            
            // Column 4: Level
            renderer = gtk_cell_renderer_text_new();
            column = gtk_tree_view_column_new_with_attributes("Level", renderer, "text", 4, NULL);
            gtk_tree_view_append_column(GTK_TREE_VIEW(treeview), column);
            
            // Column 5: Availability
            renderer = gtk_cell_renderer_text_new();
            column = gtk_tree_view_column_new_with_attributes("Availability", renderer, "text", 5, NULL);
            gtk_tree_view_append_column(GTK_TREE_VIEW(treeview), column);
        }
        
        // Load data
        GtkListStore *store = GTK_LIST_STORE(gtk_tree_view_get_model(GTK_TREE_VIEW(treeview)));
        gtk_list_store_clear(store);
        
        FILE *f = fopen("coach.txt", "r");
        if (f) {
            char line[256];
            GtkTreeIter iter;
            
            while (fgets(line, sizeof(line), f)) {
                line[strcspn(line, "\n")] = 0;
                // NEW FORMAT: firstname|lastname|gender|speciality|level|availability
                char first[50], last[50], gender[20], spec[50], level[50], avail[50];
                
                if (sscanf(line, "%[^|]|%[^|]|%[^|]|%[^|]|%[^|]|%[^\n]",
                           first, last, gender, spec, level, avail) == 6) {
                    gtk_list_store_append(store, &iter);
                    gtk_list_store_set(store, &iter,
                                       0, first,
                                       1, last,
                                       2, gender,
                                       3, spec,
                                       4, level,
                                       5, avail,
                                       -1);
                }
            }
            fclose(f);
        }
    }
}


void
on_Statsview_show                      (GtkWidget       *widget,
                                        gpointer         user_data)
{
    // ========== 1. GET TEXTVIEW WIDGETS ==========
    GtkWidget *textview_quick = lookup_widget(widget, "textview7");
    GtkWidget *textview_gender = lookup_widget(widget, "textview6");
    GtkWidget *textview_sub = lookup_widget(widget, "textview9");
    GtkWidget *label_timestamp = lookup_widget(widget, "label166");
    
    if (!textview_quick) {
        g_print("Error: Could not find textview7 widget\n");
        return;
    }
    
    // ========== 2. ADD TIMESTAMP ==========
    time_t rawtime;
    struct tm *timeinfo;
    char timestamp[100];
    
    time(&rawtime);
    timeinfo = localtime(&rawtime);
    strftime(timestamp, sizeof(timestamp), "Generated: %Y-%m-%d %H:%M:%S", timeinfo);
    
    if (label_timestamp) {
        gtk_label_set_text(GTK_LABEL(label_timestamp), timestamp);
    }
    
    // ========== 3. SIMPLE COUNTERS ==========
    int total_members = 0;
    int male = 0, female = 0;
    float total_money = 0.0;
    int sub1 = 0, sub3 = 0, sub12 = 0;
    
    FILE *file = fopen("doc.txt", "r");
    if (file) {
        char line[256];
        member m;
        
        while (fgets(line, sizeof(line), file)) {
            line[strcspn(line, "\n")] = 0;
            
            if (sscanf(line, "%[^|]|%[^|]|%[^|]|%[^|]|%d|%[^|]|%f|%[^|]|%[^|]|%[^\n]",
                       m.firstname, m.lastname, m.phonenumber, m.gender, 
                       &m.cin, m.startdate, &m.amountpaid, m.username, 
                       m.password, m.monthsub) == 10) {
                
                total_members++;
                total_money += m.amountpaid;
                
                // Gender count
                if (strcasecmp(m.gender, "Male") == 0) male++;
                else if (strcasecmp(m.gender, "Female") == 0) female++;
                
                // Subscription type
                int months = atoi(m.monthsub);
                if (months == 1) sub1++;
                else if (months == 3) sub3++;
                else if (months == 12) sub12++;
            }
        }
        fclose(file);
    }
    
    // ========== 4. CALCULATE SIMPLE VALUES ==========
    float male_percent = (total_members > 0) ? (male * 100.0 / total_members) : 0;
    float female_percent = (total_members > 0) ? (female * 100.0 / total_members) : 0;
    float avg_payment = (total_members > 0) ? (total_money / total_members) : 0;
    int active_today = (total_members > 0) ? (int)(total_members * 0.7) : 0;
    
    // ========== 5. UPDATE EACH TEXTVIEW ==========
    
    // 5.1 Quick Overview
    if (textview_quick) {
        GtkTextBuffer *buf = gtk_text_view_get_buffer(GTK_TEXT_VIEW(textview_quick));
        
        char text[256];
        snprintf(text, sizeof(text),
            "+------- OVERVIEW -------+\n"
            "| Members:     %4d      |\n"
            "| Active:      %4d      |\n"
            "| Revenue: TND%8.2f |\n"
            "| Avg Sub: TND%8.2f |\n"
            "+------------------------+",
            total_members, active_today, total_money, avg_payment);
        
        gtk_text_buffer_set_text(buf, text, -1);
    }
    
    // 5.2 Gender Distribution
    if (textview_gender) {
        GtkTextBuffer *buf = gtk_text_view_get_buffer(GTK_TEXT_VIEW(textview_gender));
        
        // Use ASCII characters for bars
        char male_bar[16] = "";
        char female_bar[16] = "";
        
        int male_blocks = (int)(male_percent / 6.67);  // 15 max
        int female_blocks = (int)(female_percent / 6.67);
        
        if (male_blocks > 15) male_blocks = 15;
        if (female_blocks > 15) female_blocks = 15;
        
        // Use '#' for bars (ASCII safe)
        for (int i = 0; i < male_blocks; i++) male_bar[i] = '#';
        male_bar[male_blocks] = '\0';
        
        for (int i = 0; i < female_blocks; i++) female_bar[i] = '#';
        female_bar[female_blocks] = '\0';
        
        char text[256];
        snprintf(text, sizeof(text),
            "+------ GENDER -------+\n"
            "| Male:   %5.1f%%      |\n"
            "| %-15s (%2d)  |\n"
            "| Female: %5.1f%%      |\n"
            "| %-15s (%2d)  |\n"
            "+--------------------+",
            male_percent, male_bar, male,
            female_percent, female_bar, female);
        
        gtk_text_buffer_set_text(buf, text, -1);
    }
    
    // 5.3 Subscription Analysis
    if (textview_sub) {
        GtkTextBuffer *buf = gtk_text_view_get_buffer(GTK_TEXT_VIEW(textview_sub));
        
        // Calculate percentages
        float sub1_percent = (total_members > 0) ? (sub1 * 100.0 / total_members) : 0;
        float sub3_percent = (total_members > 0) ? (sub3 * 100.0 / total_members) : 0;
        float sub12_percent = (total_members > 0) ? (sub12 * 100.0 / total_members) : 0;
        
        char text[256];
        snprintf(text, sizeof(text),
            "+-- SUBSCRIPTIONS ---+\n"
            "| 1 Month:  %3d (%5.1f%%) |\n"
            "| 3 Months: %3d (%5.1f%%) |\n"
            "| 12 Months:%3d (%5.1f%%) |\n"
            "|                    |\n"
            "| Total:    %3d      |\n"
            "+--------------------+",
            sub1, sub1_percent,
            sub3, sub3_percent,
            sub12, sub12_percent,
            total_members);
        
        gtk_text_buffer_set_text(buf, text, -1);
    }
    
    g_print("Stats loaded: %d members, TND%.2f revenue - %s\n", 
            total_members, total_money, timestamp);
}

void
on_return6_clicked                     (GtkWidget       *object,
                                        gpointer         user_data)
{
    GtkWidget *stats_window;
    GtkWidget *member_manage;
    
    
    stats_window = lookup_widget(object, "Statsview");
    if (stats_window != NULL) {
        gtk_widget_destroy(stats_window);
    }
    
    
    member_manage = create_MemberManage();
    gtk_widget_show_all(member_manage);
}
/////////////////////////////////////////////
/* --- 1. WriteMemoryCallback for libcurl --- */
static size_t WriteMemoryCallback(void *contents, size_t size, size_t nmemb, void *userp) {
    size_t realsize = size * nmemb;
    strncat((char*)userp, (char*)contents, realsize);
    return realsize;
}

/* --- 2. fetch_nutrition_data - makes API call --- */
static char* fetch_nutrition_data(const char* food_name) {
    CURL *curl;
    CURLcode res;
    static char response_buffer[4096] = {0};
    response_buffer[0] = '\0';
    
    curl = curl_easy_init();
    if(curl) {
        char url[512];
        const char *app_id = "39cb83c9";
        const char *app_key = "4b33a7dbe6cd4b56b49f636873f7506d";
        
        // URL encode the food name
        char *encoded_food = curl_easy_escape(curl, food_name, 0);
        if(!encoded_food) {
            snprintf(response_buffer, sizeof(response_buffer), "Error encoding food name");
            curl_easy_cleanup(curl);
            return response_buffer;
        }
        
        snprintf(url, sizeof(url), 
                 "https://api.edamam.com/api/nutrition-data?app_id=%s&app_key=%s&ingr=%s",
                 app_id, app_key, encoded_food);
        g_print("API URL: %s\n", url);
        
        curl_easy_setopt(curl, CURLOPT_URL, url);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteMemoryCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)response_buffer);
        curl_easy_setopt(curl, CURLOPT_USERAGENT, "libcurl-agent/1.0");
        curl_easy_setopt(curl, CURLOPT_TIMEOUT, 10L);
        
        res = curl_easy_perform(curl);
        
        if(res != CURLE_OK) {
            g_print("CURL Error: %s\n", curl_easy_strerror(res));
            snprintf(response_buffer, sizeof(response_buffer), 
                     "Network Error: %s", curl_easy_strerror(res));
        } else {
            long http_code = 0;
            curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &http_code);
            g_print("HTTP Response Code: %ld\n", http_code);
        }
        
        curl_free(encoded_food);
        curl_easy_cleanup(curl);
    } else {
        snprintf(response_buffer, sizeof(response_buffer), "Failed to initialize CURL.");
    }
    
    return response_buffer;
}

/* --- 3. display_parsed_nutrition - parses JSON response --- */
static void display_parsed_nutrition(GtkTextBuffer *buffer, const char *json_response, const char *food_name) {
    char display_text[2048];
    
    // Check if we got an error or HTML instead of JSON
    if (strstr(json_response, "<html>") != NULL || strstr(json_response, "<!doctype") != NULL) {
        snprintf(display_text, sizeof(display_text), 
                "API Error: Got HTML instead of JSON.\n"
                "Your API keys might be invalid or expired.\n"
                "Please check your Edamam dashboard.");
        gtk_text_buffer_set_text(buffer, display_text, -1);
        return;
    }
    
    // Simple JSON parsing (look for calories, protein, carbs, fat)
    char *calories_start = strstr(json_response, "\"ENERC_KCAL\":");
    char *protein_start = strstr(json_response, "\"PROCNT\":");
    char *carbs_start = strstr(json_response, "\"CHOCDF\":");
    char *fat_start = strstr(json_response, "\"FAT\":");
    
    if (calories_start && protein_start) {
        // Extract values
        float calories = 0, protein = 0, carbs = 0, fat = 0;
        
        sscanf(calories_start + 13, "{\"label\":\"Energy\",\"quantity\":%f", &calories);
        sscanf(protein_start + 9, "{\"label\":\"Protein\",\"quantity\":%f", &protein);
        
        if (carbs_start) {
            sscanf(carbs_start + 9, "{\"label\":\"Carbs\",\"quantity\":%f", &carbs);
        }
        
        if (fat_start) {
            sscanf(fat_start + 6, "{\"label\":\"Fat\",\"quantity\":%f", &fat);
        }
        
        // Create formatted display
        if (calories > 0) {
            snprintf(display_text, sizeof(display_text),
                    "🍎 Nutrition for: %s\n"
                    "══════════════════════════════════\n"
                    "🔥 Calories: %.1f kcal\n"
                    "💪 Protein: %.1fg\n"
                    "🌾 Carbohydrates: %.1fg\n"
                    "🧈 Fat: %.1fg\n"
                    "══════════════════════════════════\n",
                    food_name, calories, protein, carbs, fat);
        } else {
            snprintf(display_text, sizeof(display_text),
                    "⚠️ No nutrition data found for '%s'\n\n"
                    "❌ You must include a QUANTITY!\n\n"
                    "✅ Try these instead:\n"
                    "• 1 apple\n"
                    "• 100g chicken breast\n"
                    "• 1 large egg\n"
                    "• 1 cup milk\n"
                    "• 1 slice bread",
                    food_name);
        }
    } else {
        // Couldn't parse, show raw data (first 500 chars)
        char raw_data[600];
        strncpy(raw_data, json_response, 500);
        raw_data[500] = '\0';
        
        snprintf(display_text, sizeof(display_text),
                "Raw API Response (first 500 chars):\n%s", raw_data);
    }
    
    gtk_text_buffer_set_text(buffer, display_text, -1);
}


void
on_foodbutton_clicked                  (GtkWidget       *object,
                                        gpointer         user_data)
{
    GtkWidget *member_dashboard;
    GtkWidget *my_food;
    
    
    member_dashboard = lookup_widget(object, "MemberDashboard");
    if (member_dashboard != NULL) {
        gtk_widget_destroy(member_dashboard);
    }
    
    
    my_food = create_apiFoodStats();
    gtk_widget_show_all(my_food);
}


void
on_searchfood_clicked                  (GtkWidget       *object,
                                        gpointer         user_data)
{
    GtkWidget *window = gtk_widget_get_toplevel(object);
    GtkWidget *textview;
    
    if (!window) return;
    
    textview = lookup_widget(window, "textview10");
    if (!textview) return;
    
    GtkTextBuffer *buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(textview));
    
    GtkWidget *entry = lookup_widget(window, "entry22");
    if (!entry) {
        gtk_text_buffer_set_text(buffer, "Error: Could not find input field.", -1);
        return;
    }
    
    const gchar *food_name = gtk_entry_get_text(GTK_ENTRY(entry));
    if (!food_name || strlen(food_name) == 0) {
        gtk_text_buffer_set_text(buffer, "Please enter a food name.\n\nExamples:\n• 1 apple\n• 100g chicken\n• 1 cup milk", -1);
        return;
    }
    
    gtk_text_buffer_set_text(buffer, "Searching...", -1);
    char *api_result = fetch_nutrition_data(food_name);
    
    /* Parse and display the JSON nicely */
    display_parsed_nutrition(buffer, api_result, food_name);
}


void
on_returnFood_clicked                  (GtkWidget       *object,
                                        gpointer         user_data)
{
    GtkWidget *member_dashboard;
    GtkWidget *my_food;
    
    
    my_food = lookup_widget(object, "apiFoodStats");
    if (my_food != NULL) {
        gtk_widget_destroy(my_food);
    }
    
    
    member_dashboard = create_MemberDashboard();
    gtk_widget_show_all(member_dashboard);
}
void
on_button58_clicked                    (GtkWidget       *object,
					gpointer         user_data)
{
    GtkWidget *member_dashboard;
    GtkWidget *my_coach;
    
    
    my_coach = lookup_widget(object, "Mycoach");
    if (my_coach != NULL) {
        gtk_widget_destroy(my_coach);
    }
    
    
    member_dashboard = create_MemberDashboard();
    gtk_widget_show_all(member_dashboard);
}
//Wissem

#define CENTRE_FILE "/home/wissem/Desktop/Abschlussprojekt (another copy)/Integrated project/centre.txt"

/* ===================== Fenêtres ===================== */
GtkWidget *ww1 = NULL;
GtkWidget *ww2 = NULL;
GtkWidget *ww3 = NULL;
GtkWidget *ww5 = NULL;
GtkWidget *ww6 = NULL;

/* ===================== MAIN ===================== */

/* ===================== STATS ===================== */

void afficher_stats(GtkWidget *textview)
{
    GtkTextBuffer *buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(textview));
    gtk_text_buffer_set_text(buffer, "", -1);

    FILE *f = fopen(CENTRE_FILE, "r");
    if (!f) return;

    int total_centres = 0;
    int total_employes = 0;
    int max_emp = -1, min_emp = 1e6;
    char max_name[35] = "", min_name[35] = "";

    center c;
    while (fscanf(f, "%s %s %s %s %d %d",
        c.cid, c.cname, c.cadress, c.cemail, &c.cphone_int, &c.cnemp) == 6)
    {
        total_centres++;
        total_employes += c.cnemp;
        if (c.cnemp > max_emp)
        {
            max_emp = c.cnemp;
            strcpy(max_name, c.cname);
        }
        if (c.cnemp < min_emp)
        {
            min_emp = c.cnemp;
            strcpy(min_name, c.cname);
        }
    }
    fclose(f);

    char stats[512];
    double avg_emp = total_centres ? (double)total_employes / total_centres : 0;
    snprintf(stats, sizeof(stats),
        "Nombre total de centres : %d\n"
        "Nombre total d'employés : %d\n"
        "Centre avec le plus d'employés : %s (%d)\n"
        "Centre avec le moins d'employés : %s (%d)\n"
        "Moyenne d'employés par centre : %.2f",
        total_centres, total_employes, max_name, max_emp, min_name, min_emp, avg_emp);

    gtk_text_buffer_set_text(buffer, stats, -1);
}

void on_MAIN_STATS_clicked(GtkButton *button, gpointer user_data)
{
    GtkWidget *current = gtk_widget_get_toplevel(GTK_WIDGET(button));
    GtkWidget *w = create_ww3();
    gtk_widget_show(w);

    GtkWidget *treeview1WM = lookup_widget(w, "treeview1WM");
    if (treeview1WM)
        afficher_centers(treeview1WM);

    GtkWidget *textview_stats = lookup_widget(w, "textview_stats");
    if (textview_stats)
        afficher_stats(textview_stats);

    if (current)
        gtk_widget_destroy(current);
}

void on_MAIN_ADD_clicked(GtkButton *button, gpointer user_data)
{
    GtkWidget *current = gtk_widget_get_toplevel(GTK_WIDGET(button));
    GtkWidget *w = create_ww2();
    gtk_widget_show(w);
    if (current)
        gtk_widget_destroy(current);
}

void on_MAIN_LISTS_clicked(GtkButton *button, gpointer user_data)
{
    GtkWidget *current = gtk_widget_get_toplevel(GTK_WIDGET(button));
    GtkWidget *w = create_ww3();
    gtk_widget_show(w);

    GtkWidget *treeview1WM = lookup_widget(w, "treeview1WM");
    if (treeview1WM)
        afficher_centers(treeview1WM);

    GtkWidget *textview_stats = lookup_widget(w, "textview_stats");
    if (textview_stats)
        afficher_stats(textview_stats);

    if (current)
        gtk_widget_destroy(current);
}

void on_MAIN_RETURN_clicked(GtkButton *button, gpointer user_data)
{
	GtkWidget *current = gtk_widget_get_toplevel(GTK_WIDGET(button));
    GtkWidget *w = create_AdminDashboard();
    gtk_widget_show(w);
if (current)
        gtk_widget_destroy(current);
}

/* ===================== AJOUT CENTRE ===================== */

void on_ADD_C_clicked(GtkButton *button, gpointer user_data)
{
    GtkWidget *window = gtk_widget_get_toplevel(GTK_WIDGET(button));

    GtkWidget *ID_C   = lookup_widget(window, "ID_C");
    GtkWidget *N_C    = lookup_widget(window, "N_C");
    GtkWidget *ADER_C = lookup_widget(window, "ADER_C");
    GtkWidget *G_C    = lookup_widget(window, "G_C");
    GtkWidget *PH_C   = lookup_widget(window, "PH_C");
    GtkWidget *NE_C   = lookup_widget(window, "cnemp");

    if (!ID_C || !N_C || !ADER_C || !G_C || !PH_C || !NE_C)
        return;

    center C;
    strncpy(C.cid, gtk_entry_get_text(GTK_ENTRY(ID_C)), sizeof(C.cid)-1); C.cid[sizeof(C.cid)-1] = '\0';
    strncpy(C.cname, gtk_entry_get_text(GTK_ENTRY(N_C)), sizeof(C.cname)-1); C.cname[sizeof(C.cname)-1] = '\0';
    strncpy(C.cadress, gtk_entry_get_text(GTK_ENTRY(ADER_C)), sizeof(C.cadress)-1); C.cadress[sizeof(C.cadress)-1] = '\0';
    strncpy(C.cemail, gtk_entry_get_text(GTK_ENTRY(G_C)), sizeof(C.cemail)-1); C.cemail[sizeof(C.cemail)-1] = '\0';

    // Vérification du téléphone
    const char *phone_str = gtk_entry_get_text(GTK_ENTRY(PH_C));
    char *endptr;
    long phone_long = strtol(phone_str, &endptr, 10);
    if (*endptr != '\0')
    {
        GtkWidget *d = gtk_message_dialog_new(GTK_WINDOW(window),
            GTK_DIALOG_MODAL, GTK_MESSAGE_ERROR, GTK_BUTTONS_OK,
            "Numéro de téléphone invalide !");
        gtk_dialog_run(GTK_DIALOG(d));
        gtk_widget_destroy(d);
        return;
    }
    C.cphone_int = (int)phone_long;

    C.cnemp = gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(NE_C));

    if (strlen(C.cid) == 0 || strlen(C.cname) == 0 || strlen(C.cadress) == 0 ||
        strlen(C.cemail) == 0 || phone_long == 0)
    {
        GtkWidget *d = gtk_message_dialog_new(GTK_WINDOW(window),
            GTK_DIALOG_MODAL, GTK_MESSAGE_ERROR, GTK_BUTTONS_OK,
            "Il y a un champ vide !");
        gtk_dialog_run(GTK_DIALOG(d));
        gtk_widget_destroy(d);
        return;
    }

    if (addcenter(C))
{
    GtkWidget *d = gtk_message_dialog_new(GTK_WINDOW(window),
        GTK_DIALOG_MODAL, GTK_MESSAGE_INFO, GTK_BUTTONS_OK,
        "Centre ajouté avec succès !");
    gtk_dialog_run(GTK_DIALOG(d));
    gtk_widget_destroy(d);

    // Go to list window
    GtkWidget *w = create_ww3();
    gtk_widget_show(w);

    GtkWidget *treeview1WM = lookup_widget(w, "treeview1WM");
    if (treeview1WM)
        afficher_centers(treeview1WM);

    GtkWidget *textview_stats = lookup_widget(w, "textview_stats");
    if (textview_stats)
        afficher_stats(textview_stats);

    gtk_widget_destroy(window); // destroy ww2
}

}

/* ===================== RESET ===================== */

void on_RESET_C_clicked(GtkButton *button, gpointer user_data)
{
    GtkWidget *window = gtk_widget_get_toplevel(GTK_WIDGET(button));

    gtk_entry_set_text(GTK_ENTRY(lookup_widget(window, "ID_C")), "");
    gtk_entry_set_text(GTK_ENTRY(lookup_widget(window, "N_C")), "");
    gtk_entry_set_text(GTK_ENTRY(lookup_widget(window, "ADER_C")), "");
    gtk_entry_set_text(GTK_ENTRY(lookup_widget(window, "G_C")), "");
    gtk_entry_set_text(GTK_ENTRY(lookup_widget(window, "PH_C")), "");
    gtk_spin_button_set_value(GTK_SPIN_BUTTON(lookup_widget(window, "cnemp")), 0);
}

/* ===================== RETURN BUTTONS ===================== */

void on_RETURN_C_clicked(GtkButton *button, gpointer user_data)
{
    GtkWidget *w = create_ww1();
    gtk_widget_show(w);

    GtkWidget *current = gtk_widget_get_toplevel(GTK_WIDGET(button));
    if (current)
        gtk_widget_destroy(current);
}

void on_RETURN_L_clicked(GtkButton *button, gpointer user_data)
{
    GtkWidget *w = create_ww1();
    gtk_widget_show(w);

    GtkWidget *current = gtk_widget_get_toplevel(GTK_WIDGET(button));
    if (current)
        gtk_widget_destroy(current);
}

/* ===================== SUPPRIMER ===================== */

void on_DELETE_L_clicked(GtkButton *button, gpointer user_data)
{
    GtkWidget *treeview = lookup_widget(GTK_WIDGET(button), "treeview2WM");
    GtkTreeSelection *selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(treeview));
    GtkTreeModel *model;
    GtkTreeIter iter;

    if (gtk_tree_selection_get_selected(selection, &model, &iter))
    {
        gchar *cid;
        gtk_tree_model_get(model, &iter, 0, &cid, -1);
        deletecenter(CENTRE_FILE, cid);
        gtk_list_store_remove(GTK_LIST_STORE(model), &iter);
        g_free(cid);

        GtkWidget *textview2WM = lookup_widget(gtk_widget_get_toplevel(GTK_WIDGET(button)), "textview2WM");
        if (textview2WM)
            afficher_stats(textview2WM);
    }
}

/* ===================== EDITER ===================== */

void on_EDITE_L_clicked(GtkButton *button, gpointer user_data)
{

    GtkWidget *window = gtk_widget_get_toplevel(GTK_WIDGET(button));
    GtkTreeView *treeview = GTK_TREE_VIEW(lookup_widget(window, "treeview1WM"));

    GtkTreeSelection *selection = gtk_tree_view_get_selection(treeview);
    GtkTreeModel *model;
    GtkTreeIter iter;

    if (gtk_tree_selection_get_selected(selection, &model, &iter))
    {
        gchar *id, *name, *adr, *email;
        int phone_int, nemp;

        gtk_tree_model_get(model, &iter,
            0, &id,
            1, &name,
            2, &adr,
            3, &email,
            4, &phone_int,
            5, &nemp,
            -1);

        ww5 = create_ww5();
        gtk_widget_show(ww5);

        gtk_entry_set_text(GTK_ENTRY(lookup_widget(ww5, "entry2WM")), id);
        gtk_entry_set_text(GTK_ENTRY(lookup_widget(ww5, "entry1WM")), name);
        gtk_entry_set_text(GTK_ENTRY(lookup_widget(ww5, "entry3WM")), adr);
        gtk_entry_set_text(GTK_ENTRY(lookup_widget(ww5, "entry4WM")), email);

        char phone_str[20];
        snprintf(phone_str, sizeof(phone_str), "%d", phone_int);
        gtk_entry_set_text(GTK_ENTRY(lookup_widget(ww5, "entry5WM")), phone_str);

        gtk_spin_button_set_value(GTK_SPIN_BUTTON(lookup_widget(ww5, "cnemp4")), nemp);

        g_free(id); g_free(name); g_free(adr); g_free(email);
    }
}

/* ===================== VALIDER MODIFICATION ===================== */

void on_button3WM_clicked(GtkButton *button, gpointer user_data)
{
    GtkWidget *window = gtk_widget_get_toplevel(GTK_WIDGET(button));
    center C;

    strncpy(C.cid, gtk_entry_get_text(GTK_ENTRY(lookup_widget(window, "entry2WM"))), sizeof(C.cid)-1);
    C.cid[sizeof(C.cid)-1] = '\0';
    strncpy(C.cname, gtk_entry_get_text(GTK_ENTRY(lookup_widget(window, "entry1WM"))), sizeof(C.cname)-1);
    C.cname[sizeof(C.cname)-1] = '\0';
    strncpy(C.cadress, gtk_entry_get_text(GTK_ENTRY(lookup_widget(window, "entry3WM"))), sizeof(C.cadress)-1);
    C.cadress[sizeof(C.cadress)-1] = '\0';
    strncpy(C.cemail, gtk_entry_get_text(GTK_ENTRY(lookup_widget(window, "entry4WM"))), sizeof(C.cemail)-1);
    C.cemail[sizeof(C.cemail)-1] = '\0';

    // Vérification du téléphone
    const char *phone_str = gtk_entry_get_text(GTK_ENTRY(lookup_widget(window, "entry5WM")));
    char *endptr;
    long phone_long = strtol(phone_str, &endptr, 10);
    if (*endptr != '\0')
    {
        GtkWidget *d = gtk_message_dialog_new(GTK_WINDOW(window),
            GTK_DIALOG_MODAL, GTK_MESSAGE_ERROR, GTK_BUTTONS_OK,
            "Numéro de téléphone invalide !");
        gtk_dialog_run(GTK_DIALOG(d));
        gtk_widget_destroy(d);
        return;
    }
    C.cphone_int = (int)phone_long;

    C.cnemp = gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(lookup_widget(window, "cnemp4")));

    deletecenter(CENTRE_FILE, C.cid);
    addcenter(C);

    GtkWidget *dialog = gtk_message_dialog_new(GTK_WINDOW(window),
        GTK_DIALOG_MODAL, GTK_MESSAGE_INFO, GTK_BUTTONS_OK,
        "Modification effectuée avec succès !");
    gtk_dialog_run(GTK_DIALOG(dialog));
    gtk_widget_destroy(dialog);

    // Retour au window1
    GtkWidget *w1 = create_ww1();
    gtk_widget_show(w1);

    // Fermer le treeview window
    GtkWidget *w3 = lookup_widget(window, "ww3");
    if (w3)
        gtk_widget_destroy(w3);

    // Fermer window5
    gtk_widget_destroy(window);
}

/* ===================== AFFICHAGE ===================== */
void afficher_centers(GtkWidget *treeview)
{
    GList *columns = gtk_tree_view_get_columns(GTK_TREE_VIEW(treeview));
    for (GList *iter = columns; iter != NULL; iter = iter->next)
        gtk_tree_view_remove_column(GTK_TREE_VIEW(treeview), GTK_TREE_VIEW_COLUMN(iter->data));
    g_list_free(columns);

    GtkListStore *store = gtk_list_store_new(
        6, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING,
        G_TYPE_STRING, G_TYPE_INT, G_TYPE_INT); // note: téléphone en int

    GtkTreeIter iter;
    FILE *f = fopen(CENTRE_FILE, "r");

    if (f)
    {
        center c;
        while (fscanf(f, "%s %s %s %s %d %d",
            c.cid, c.cname, c.cadress, c.cemail, &c.cphone_int, &c.cnemp) == 6)
        {
            gtk_list_store_append(store, &iter);
            gtk_list_store_set(store, &iter,
                0, c.cid,
                1, c.cname,
                2, c.cadress,
                3, c.cemail,
                4, c.cphone_int,
                5, c.cnemp,
                -1);
        }
        fclose(f);
    }

    gtk_tree_view_set_model(GTK_TREE_VIEW(treeview), GTK_TREE_MODEL(store));
    g_object_unref(store);

    const char *titles[] = {"ID", "Nom", "Adresse", "Email", "Téléphone", "Employés"};
    for (int i = 0; i < 6; i++)
    {
        GtkCellRenderer *renderer = gtk_cell_renderer_text_new();
        GtkTreeViewColumn *column = gtk_tree_view_column_new_with_attributes(
            titles[i], renderer, "text", i, NULL);
        gtk_tree_view_append_column(GTK_TREE_VIEW(treeview), column);
    }
}


/* ===================== TREEVIEW CALLBACKS VIDES ===================== */

void on_treeview1WM_cursor_changed(GtkTreeView *treeview, gpointer user_data) { }
void on_treeview1WM_row_activated(GtkTreeView *treeview, GtkTreePath *path,
                                GtkTreeViewColumn *column, gpointer user_data) { }







void
on_delcWM_clicked                      (GtkButton       *button,
                                        gpointer         user_data)
{
    GtkWidget *current = gtk_widget_get_toplevel(GTK_WIDGET(button));
    GtkWidget *w = create_ww6();
    gtk_widget_show(w);

    GtkWidget *treeview2WM = lookup_widget(w, "treeview2WM");
    if (treeview2WM)
        afficher_centers(treeview2WM);

    GtkWidget *textview2WM = lookup_widget(w, "textview2WM");
    if (textview2WM)
        afficher_stats(textview2WM);

    if (current)
        gtk_widget_destroy(current);
}


void
on_treeview2WM_cursor_changed          (GtkTreeView     *treeview,
                                        gpointer         user_data)
{

}


void
on_treeview2WM_row_activated           (GtkTreeView     *treeview,
                                        GtkTreePath     *path,
                                        GtkTreeViewColumn *column,
                                        gpointer         user_data)
{

}


void
on_RETURN_L2_clicked                   (GtkButton       *button,
                                        gpointer         user_data)
{
    GtkWidget *w = create_ww1();
    gtk_widget_show(w);

    GtkWidget *current = gtk_widget_get_toplevel(GTK_WIDGET(button));
    if (current)
        gtk_widget_destroy(current);
}
//lien entre dashboard et centres
void
on_button7_clicked                     (GtkButton       *button,
                                        gpointer         user_data)
{
	GtkWidget *current = gtk_widget_get_toplevel(GTK_WIDGET(button));
    GtkWidget *w = create_ww1();
    gtk_widget_show(w);
if (current)
        gtk_widget_destroy(current);
}
//Eya
void setup_course_treeview_columns(GtkWidget *treeview)
{
    // Create ListStore with 10 columns (matching course structure)
    GtkListStore *store = gtk_list_store_new(10,
                              G_TYPE_INT,     // 0: id
                              G_TYPE_STRING,  // 1: typecourse
                              G_TYPE_STRING,  // 2: level
                              G_TYPE_STRING,  // 3: gender
                              G_TYPE_STRING,  // 4: date
                              G_TYPE_INT,     // 5: hour
                              G_TYPE_INT,     // 6: duration
                              G_TYPE_STRING,  // 7: status
                              G_TYPE_STRING,  // 8: rescoach
                              G_TYPE_INT);    // 9: room
    
    gtk_tree_view_set_model(GTK_TREE_VIEW(treeview), GTK_TREE_MODEL(store));
    
    // Create 10 visible columns
    GtkCellRenderer *renderer;
    GtkTreeViewColumn *column;
    
    // Column 0: ID
    renderer = gtk_cell_renderer_text_new();
    column = gtk_tree_view_column_new_with_attributes("ID", renderer, "text", 0, NULL);
    gtk_tree_view_append_column(GTK_TREE_VIEW(treeview), column);
    
    // Column 1: Type Course
    renderer = gtk_cell_renderer_text_new();
    column = gtk_tree_view_column_new_with_attributes("Type", renderer, "text", 1, NULL);
    gtk_tree_view_append_column(GTK_TREE_VIEW(treeview), column);
    
    // Column 2: Level
    renderer = gtk_cell_renderer_text_new();
    column = gtk_tree_view_column_new_with_attributes("Level", renderer, "text", 2, NULL);
    gtk_tree_view_append_column(GTK_TREE_VIEW(treeview), column);
    
    // Column 3: Gender
    renderer = gtk_cell_renderer_text_new();
    column = gtk_tree_view_column_new_with_attributes("Gender", renderer, "text", 3, NULL);
    gtk_tree_view_append_column(GTK_TREE_VIEW(treeview), column);
    
    // Column 4: Date
    renderer = gtk_cell_renderer_text_new();
    column = gtk_tree_view_column_new_with_attributes("Date", renderer, "text", 4, NULL);
    gtk_tree_view_append_column(GTK_TREE_VIEW(treeview), column);
    
    // Column 5: Hour
    renderer = gtk_cell_renderer_text_new();
    column = gtk_tree_view_column_new_with_attributes("Hour", renderer, "text", 5, NULL);
    gtk_tree_view_append_column(GTK_TREE_VIEW(treeview), column);
    
    // Column 6: Duration
    renderer = gtk_cell_renderer_text_new();
    column = gtk_tree_view_column_new_with_attributes("Duration", renderer, "text", 6, NULL);
    gtk_tree_view_append_column(GTK_TREE_VIEW(treeview), column);
    
    // Column 7: Status
    renderer = gtk_cell_renderer_text_new();
    column = gtk_tree_view_column_new_with_attributes("Status", renderer, "text", 7, NULL);
    gtk_tree_view_append_column(GTK_TREE_VIEW(treeview), column);
    
    // Column 8: Responsible Coach
    renderer = gtk_cell_renderer_text_new();
    column = gtk_tree_view_column_new_with_attributes("Coach", renderer, "text", 8, NULL);
    gtk_tree_view_append_column(GTK_TREE_VIEW(treeview), column);
    
    // Column 9: Room
    renderer = gtk_cell_renderer_text_new();
    column = gtk_tree_view_column_new_with_attributes("Room", renderer, "text", 9, NULL);
    gtk_tree_view_append_column(GTK_TREE_VIEW(treeview), column);
    
    // Optional: Make columns resizable and sortable
    gtk_tree_view_set_rules_hint(GTK_TREE_VIEW(treeview), TRUE);
    gtk_tree_view_set_headers_clickable(GTK_TREE_VIEW(treeview), TRUE);
}

void
on_statsbutton_clicked                 (GtkWidget       *object,
                                        gpointer         user_data)
{
   GtkWidget *stats_course;
    GtkWidget *course_manage;
    
    
    course_manage = lookup_widget(object, "CourseManag");
    if (course_manage != NULL) {
        gtk_widget_destroy(course_manage);
    }
    
   
    stats_course = create_ewstats();
    gtk_widget_show_all(stats_course);
}


void
on_restorebutton_clicked               (GtkWidget       *object,
                                        gpointer         user_data)
{
    GtkWidget *restore_course;
    GtkWidget *course_manage;
    
    
    course_manage = lookup_widget(object, "CourseManag");
    if (course_manage != NULL) {
        gtk_widget_destroy(course_manage);
    }
    
   
    restore_course = create_ewrestorecours();
    gtk_widget_show_all(restore_course);
}


void
on_deletebutton_clicked                (GtkWidget       *object,
                                        gpointer         user_data)
{
    GtkWidget *delete_course;
    GtkWidget *course_manage;
    
    
    course_manage = lookup_widget(object, "CourseManag");
    if (course_manage != NULL) {
        gtk_widget_destroy(course_manage);
    }
    
   
    delete_course = create_ewdeletecous();
    gtk_widget_show_all(delete_course);
}


void
on_listbutton_clicked                  (GtkWidget       *object,
                                        gpointer         user_data)
{
    GtkWidget *search_course;
    GtkWidget *course_manage;
    
    
    course_manage = lookup_widget(object, "CourseManag");
    if (course_manage != NULL) {
        gtk_widget_destroy(course_manage);
    }
    
   
    search_course = create_ewrecherchecours();
    gtk_widget_show_all(search_course);
}


void
on_addbutton_clicked                   (GtkWidget       *object,
                                        gpointer         user_data)
{
    GtkWidget *add_course;
    GtkWidget *course_manage;
    
    
    course_manage = lookup_widget(object, "CourseManag");
    if (course_manage != NULL) {
        gtk_widget_destroy(course_manage);
    }
    
   
    add_course = create_ew_addcourse();
    gtk_widget_show_all(add_course);
}



void
on_return1EW_clicked                     (GtkWidget       *object,
                                        gpointer         user_data)
{
    GtkWidget *add_course;
    GtkWidget *course_manage;
    
    
    add_course = lookup_widget(object, "ew_addcourse");
    if (add_course != NULL) {
        gtk_widget_destroy(add_course);
    }
    
    
    course_manage = create_CourseManag();
    gtk_widget_show_all(course_manage);
}


void
on_return2EW_clicked                     (GtkWidget       *object,
                                        gpointer         user_data)
{
    GtkWidget *search_course;
    GtkWidget *course_manage;
    
    
    search_course = lookup_widget(object, "ewrecherchecours");
    if (search_course != NULL) {
        gtk_widget_destroy(search_course);
    }
    
    
    course_manage = create_CourseManag();
    gtk_widget_show_all(course_manage);
}


void
on_return3_clicked                     (GtkWidget       *object,
                                        gpointer         user_data)
{
    GtkWidget *delete_course;
    GtkWidget *course_manage;
    
    
    delete_course = lookup_widget(object, "ewdeletecous");
    if (delete_course != NULL) {
        gtk_widget_destroy(delete_course);
    }
    
    
    course_manage = create_CourseManag();
    gtk_widget_show_all(course_manage);
}


void
on_return4EW_clicked                     (GtkWidget       *object,
                                        gpointer         user_data)
{
    GtkWidget *restore_course;
    GtkWidget *course_manage;
    
    
    restore_course = lookup_widget(object, "ewrestorecours");
    if (restore_course != NULL) {
        gtk_widget_destroy(restore_course);
    }
    
    
    course_manage = create_CourseManag();
    gtk_widget_show_all(course_manage);
}


void
on_return5EW_clicked                     (GtkWidget       *object,
                                        gpointer         user_data)
{
    GtkWidget *stats_course;
    GtkWidget *course_manage;
    
    
    stats_course = lookup_widget(object, "ewstats");
    if (stats_course != NULL) {
        gtk_widget_destroy(stats_course);
    }
    
    
    course_manage = create_CourseManag();
    gtk_widget_show_all(course_manage);
}


void
on_saveadd_clicked                     (GtkWidget       *object,
                                        gpointer         user_data)
{
 GtkWidget *window = gtk_widget_get_toplevel(object);
    
    if (window == NULL || !GTK_IS_WINDOW(window)) {
        g_print("Error: Could not find window!\n");
        return;
    }
    
    // ========== GET ALL WIDGETS ==========
    
    // Entry widgets
    GtkWidget *entry_id = lookup_widget(window, "ewentry1id");
    GtkWidget *entry_rescoach = lookup_widget(window, "ewentry1coach");
    GtkWidget *entry_room = lookup_widget(window, "ewentry2room");
    
    // ComboBox widgets
    GtkWidget *combobox_typecourse = lookup_widget(window, "ewcombobox1type");
    GtkWidget *combobox_gender = lookup_widget(window, "ewcombobox1gender");
    
    // Calendar widget
    GtkWidget *calendar = lookup_widget(window, "ewcalendar1");
    
    // SpinButton widgets
    GtkWidget *spinbutton_hour = lookup_widget(window, "ewspinbutton1hour");
    GtkWidget *spinbutton_duration = lookup_widget(window, "ewspinbutton2dur");
    
    // CheckButton widgets for level (3 options)
    GtkWidget *checkbutton_beginner = lookup_widget(window, "ewcheck1level");
    GtkWidget *checkbutton_intermediate = lookup_widget(window, "ewcheck2level");
    GtkWidget *checkbutton_advanced = lookup_widget(window, "ewcheck3level");
    
    // RadioButton widgets for status (3 options)
    GtkWidget *radiobutton_available = lookup_widget(window, "ewradiobutton1");
    GtkWidget *radiobutton_full = lookup_widget(window, "ewradiobutton2");
    GtkWidget *radiobutton_cancelled = lookup_widget(window, "ewradiobutton3");
    
    // Check if widgets were found
    if (!entry_id || !entry_rescoach || !entry_room ||
        !combobox_typecourse || !combobox_gender ||
        !calendar || !spinbutton_hour || !spinbutton_duration) {
        GtkWidget *dialog = gtk_message_dialog_new(GTK_WINDOW(window),
                                                  GTK_DIALOG_MODAL,
                                                  GTK_MESSAGE_ERROR,
                                                  GTK_BUTTONS_OK,
                                                  "Error: Could not find all form fields!");
        gtk_dialog_run(GTK_DIALOG(dialog));
        gtk_widget_destroy(dialog);
        return;
    }
    
    // ========== GET VALUES FROM WIDGETS ==========
    
    // Get ID
    const gchar *id_str = gtk_entry_get_text(GTK_ENTRY(entry_id));
    if (strlen(id_str) == 0) {
        GtkWidget *dialog = gtk_message_dialog_new(GTK_WINDOW(window),
                                                  GTK_DIALOG_MODAL,
                                                  GTK_MESSAGE_ERROR,
                                                  GTK_BUTTONS_OK,
                                                  "Please enter Course ID!");
        gtk_dialog_run(GTK_DIALOG(dialog));
        gtk_widget_destroy(dialog);
        return;
    }
    
    // Validate ID is a number
    for (int i = 0; id_str[i] != '\0'; i++) {
        if (!isdigit(id_str[i])) {
            GtkWidget *dialog = gtk_message_dialog_new(GTK_WINDOW(window),
                                                      GTK_DIALOG_MODAL,
                                                      GTK_MESSAGE_ERROR,
                                                      GTK_BUTTONS_OK,
                                                      "Course ID must be a number!");
            gtk_dialog_run(GTK_DIALOG(dialog));
            gtk_widget_destroy(dialog);
            return;
        }
    }
    
    int course_id = atoi(id_str);
    
if (course_id_exists("course.txt", course_id)) {
    GtkWidget *dialog = gtk_message_dialog_new(GTK_WINDOW(window),
                                              GTK_DIALOG_MODAL,
                                              GTK_MESSAGE_ERROR,
                                              GTK_BUTTONS_OK,
                                              "Error: Course ID %d already exists!\n"
                                              "Please use a unique ID number.",
                                              course_id);
    gtk_dialog_run(GTK_DIALOG(dialog));
    gtk_widget_destroy(dialog);
    return;
}

    
    // Get course type from ComboBox
    const gchar *typecourse = NULL;
    gint active_type = gtk_combo_box_get_active(GTK_COMBO_BOX(combobox_typecourse));
    if (active_type >= 0) {
        typecourse = gtk_combo_box_get_active_text(GTK_COMBO_BOX(combobox_typecourse));
    }
    if (!typecourse || strlen(typecourse) == 0) {
        GtkWidget *dialog = gtk_message_dialog_new(GTK_WINDOW(window),
                                                  GTK_DIALOG_MODAL,
                                                  GTK_MESSAGE_ERROR,
                                                  GTK_BUTTONS_OK,
                                                  "Please select Course Type!");
        gtk_dialog_run(GTK_DIALOG(dialog));
        gtk_widget_destroy(dialog);
        return;
    }
    
    // Get level from CheckButtons (can select multiple)
    char level[50] = "";
    if (checkbutton_beginner && gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(checkbutton_beginner))) {
        if (strlen(level) > 0) strcat(level, ",");
        strcat(level, "Beginner");
    }
    if (checkbutton_intermediate && gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(checkbutton_intermediate))) {
        if (strlen(level) > 0) strcat(level, ",");
        strcat(level, "Intermediate");
    }
    if (checkbutton_advanced && gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(checkbutton_advanced))) {
        if (strlen(level) > 0) strcat(level, ",");
        strcat(level, "Advanced");
    }
    
    if (strlen(level) == 0) {
        GtkWidget *dialog = gtk_message_dialog_new(GTK_WINDOW(window),
                                                  GTK_DIALOG_MODAL,
                                                  GTK_MESSAGE_ERROR,
                                                  GTK_BUTTONS_OK,
                                                  "Please select at least one Level!");
        gtk_dialog_run(GTK_DIALOG(dialog));
        gtk_widget_destroy(dialog);
        if (typecourse) g_free((gchar*)typecourse);
        return;
    }
    
    // Get gender from ComboBox
    const gchar *gender = NULL;
    gint active_gender = gtk_combo_box_get_active(GTK_COMBO_BOX(combobox_gender));
    if (active_gender >= 0) {
        gender = gtk_combo_box_get_active_text(GTK_COMBO_BOX(combobox_gender));
    }
    if (!gender || strlen(gender) == 0) {
        GtkWidget *dialog = gtk_message_dialog_new(GTK_WINDOW(window),
                                                  GTK_DIALOG_MODAL,
                                                  GTK_MESSAGE_ERROR,
                                                  GTK_BUTTONS_OK,
                                                  "Please select Gender!");
        gtk_dialog_run(GTK_DIALOG(dialog));
        gtk_widget_destroy(dialog);
        if (typecourse) g_free((gchar*)typecourse);
        return;
    }
    
    // Get date from Calendar
    guint year, month, day;
    gtk_calendar_get_date(GTK_CALENDAR(calendar), &year, &month, &day);
    month += 1; // GTK calendar months are 0-11
    
    char date_str[20];
    snprintf(date_str, sizeof(date_str), "%04d-%02d-%02d", year, month, day);
    
    // Get hour from SpinButton
    gint hour = (gint)gtk_spin_button_get_value(GTK_SPIN_BUTTON(spinbutton_hour));
    
    // Get duration from SpinButton
    gint duration = (gint)gtk_spin_button_get_value(GTK_SPIN_BUTTON(spinbutton_duration));
    
    // Get status from RadioButtons
    char status[20] = "";
    if (radiobutton_available && gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(radiobutton_available))) {
        strcpy(status, "Available");
    } else if (radiobutton_full && gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(radiobutton_full))) {
        strcpy(status, "Full");
    } else if (radiobutton_cancelled && gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(radiobutton_cancelled))) {
        strcpy(status, "Cancelled");
    } else {
        GtkWidget *dialog = gtk_message_dialog_new(GTK_WINDOW(window),
                                                  GTK_DIALOG_MODAL,
                                                  GTK_MESSAGE_ERROR,
                                                  GTK_BUTTONS_OK,
                                                  "Please select Status!");
        gtk_dialog_run(GTK_DIALOG(dialog));
        gtk_widget_destroy(dialog);
        if (typecourse) g_free((gchar*)typecourse);
        if (gender) g_free((gchar*)gender);
        return;
    }
    
    // Get responsible coach
    const gchar *rescoach = gtk_entry_get_text(GTK_ENTRY(entry_rescoach));
    if (strlen(rescoach) == 0) {
        GtkWidget *dialog = gtk_message_dialog_new(GTK_WINDOW(window),
                                                  GTK_DIALOG_MODAL,
                                                  GTK_MESSAGE_ERROR,
                                                  GTK_BUTTONS_OK,
                                                  "Please enter Responsible Coach!");
        gtk_dialog_run(GTK_DIALOG(dialog));
        gtk_widget_destroy(dialog);
        if (typecourse) g_free((gchar*)typecourse);
        if (gender) g_free((gchar*)gender);
        return;
    }
    
    // Get room number
    const gchar *room_str = gtk_entry_get_text(GTK_ENTRY(entry_room));
    if (strlen(room_str) == 0) {
        GtkWidget *dialog = gtk_message_dialog_new(GTK_WINDOW(window),
                                                  GTK_DIALOG_MODAL,
                                                  GTK_MESSAGE_ERROR,
                                                  GTK_BUTTONS_OK,
                                                  "Please enter Room Number!");
        gtk_dialog_run(GTK_DIALOG(dialog));
        gtk_widget_destroy(dialog);
        if (typecourse) g_free((gchar*)typecourse);
        if (gender) g_free((gchar*)gender);
        return;
    }
    
    // Validate room is a number
    for (int i = 0; room_str[i] != '\0'; i++) {
        if (!isdigit(room_str[i])) {
            GtkWidget *dialog = gtk_message_dialog_new(GTK_WINDOW(window),
                                                      GTK_DIALOG_MODAL,
                                                      GTK_MESSAGE_ERROR,
                                                      GTK_BUTTONS_OK,
                                                      "Room Number must be a number!");
            gtk_dialog_run(GTK_DIALOG(dialog));
            gtk_widget_destroy(dialog);
            if (typecourse) g_free((gchar*)typecourse);
            if (gender) g_free((gchar*)gender);
            return;
        }
    }
    
    int room = atoi(room_str);
    
    // ========== CREATE COURSE STRUCTURE ==========
    
    course new_course;
    
    // Fill the structure
    new_course.id = course_id;
    
    strncpy(new_course.typecourse, typecourse, sizeof(new_course.typecourse)-1);
    new_course.typecourse[sizeof(new_course.typecourse)-1] = '\0';
    
    strncpy(new_course.level, level, sizeof(new_course.level)-1);
    new_course.level[sizeof(new_course.level)-1] = '\0';
    
    strncpy(new_course.gender, gender, sizeof(new_course.gender)-1);
    new_course.gender[sizeof(new_course.gender)-1] = '\0';
    
    strncpy(new_course.date, date_str, sizeof(new_course.date)-1);
    new_course.date[sizeof(new_course.date)-1] = '\0';
    
    new_course.hour = hour;
    new_course.duration = duration;
    
    strncpy(new_course.status, status, sizeof(new_course.status)-1);
    new_course.status[sizeof(new_course.status)-1] = '\0';
    
    strncpy(new_course.rescoach, rescoach, sizeof(new_course.rescoach)-1);
    new_course.rescoach[sizeof(new_course.rescoach)-1] = '\0';
    
    new_course.room = room;
    
    // Free memory from combo box texts
    if (typecourse) g_free((gchar*)typecourse);
    if (gender) g_free((gchar*)gender);
    
    // ========== SAVE TO FILE ==========
    
    int result = ajouterEW("course.txt", new_course);
    
    // ========== SHOW RESULT ==========
    
    GtkWidget *dialog;
    if (result == 1) {
        dialog = gtk_message_dialog_new(GTK_WINDOW(window),
                                       GTK_DIALOG_MODAL,
                                       GTK_MESSAGE_INFO,
                                       GTK_BUTTONS_OK,
                                       "Course added successfully!\n\n"
                                       "ID: %d\n"
                                       "Type: %s\n"
                                       "Level: %s\n"
                                       "Date: %s\n"
                                       "Time: %d:00\n"
                                       "Duration: %d hours",
                                       new_course.id, new_course.typecourse, 
                                       new_course.level, new_course.date,
                                       new_course.hour, new_course.duration);
        
        // Clear form fields
        gtk_entry_set_text(GTK_ENTRY(entry_id), "");
        gtk_entry_set_text(GTK_ENTRY(entry_rescoach), "");
        gtk_entry_set_text(GTK_ENTRY(entry_room), "");
        
        // Reset ComboBoxes
        gtk_combo_box_set_active(GTK_COMBO_BOX(combobox_typecourse), -1);
        gtk_combo_box_set_active(GTK_COMBO_BOX(combobox_gender), -1);
        
        // Reset Calendar to today
        time_t t = time(NULL);
        struct tm *today = localtime(&t);
        gtk_calendar_select_month(GTK_CALENDAR(calendar), today->tm_mon, today->tm_year + 1900);
        gtk_calendar_select_day(GTK_CALENDAR(calendar), today->tm_mday);
        
        // Reset SpinButtons
        gtk_spin_button_set_value(GTK_SPIN_BUTTON(spinbutton_hour), 9.0);
        gtk_spin_button_set_value(GTK_SPIN_BUTTON(spinbutton_duration), 1.0);
        
        // Reset CheckButtons
        if (checkbutton_beginner) gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(checkbutton_beginner), FALSE);
        if (checkbutton_intermediate) gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(checkbutton_intermediate), FALSE);
        if (checkbutton_advanced) gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(checkbutton_advanced), FALSE);
        
        // Reset RadioButtons
        if (radiobutton_available) gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(radiobutton_available), TRUE);
        if (radiobutton_full) gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(radiobutton_full), FALSE);
        if (radiobutton_cancelled) gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(radiobutton_cancelled), FALSE);
        
    } else {
        dialog = gtk_message_dialog_new(GTK_WINDOW(window),
                                       GTK_DIALOG_MODAL,
                                       GTK_MESSAGE_ERROR,
                                       GTK_BUTTONS_OK,
                                       "Error saving course!");
    }
    
    gtk_dialog_run(GTK_DIALOG(dialog));
    gtk_widget_destroy(dialog);
}


void
on_ew_addcourse_show                   (GtkWidget       *widget,
                                        gpointer         user_data)
{
    FILE *file;
    char line[100];
    GtkWidget *combobox;
    
    // Get the combo box widget
    combobox = lookup_widget(widget, "ewcombobox1type");
    if (!combobox) {
        g_print("Error: Could not find combo box widget\n");
        return;
    }
    
    // Clear existing items in GTK 2.x way
    GtkTreeModel *model = gtk_combo_box_get_model(GTK_COMBO_BOX(combobox));
    if (model) {
        GtkListStore *store = GTK_LIST_STORE(model);
        gtk_list_store_clear(store);
    }
    
    // Open the type.txt file
    file = fopen("type.txt", "r");
    if (file == NULL) {
        g_print("Warning: type.txt file not found. Creating default types.\n");
        
        // Add default types
        gtk_combo_box_append_text(GTK_COMBO_BOX(combobox), "Yoga");
        gtk_combo_box_append_text(GTK_COMBO_BOX(combobox), "Cardio");
        gtk_combo_box_append_text(GTK_COMBO_BOX(combobox), "Strength Training");
        gtk_combo_box_append_text(GTK_COMBO_BOX(combobox), "Zumba");
        gtk_combo_box_append_text(GTK_COMBO_BOX(combobox), "Pilates");
        gtk_combo_box_append_text(GTK_COMBO_BOX(combobox), "Spinning");
        
        gtk_combo_box_set_active(GTK_COMBO_BOX(combobox), 0);
        return;
    }
    
    // Read each line from the file
    while (fgets(line, sizeof(line), file)) {
        // Remove newline character
        line[strcspn(line, "\n")] = 0;
        
        // Skip empty lines
        if (strlen(line) == 0) continue;
        
        // Add to combo box
        gtk_combo_box_append_text(GTK_COMBO_BOX(combobox), line);
    }
    
    fclose(file);
    
    // Set the first item as active
    gtk_combo_box_set_active(GTK_COMBO_BOX(combobox), 0);
}


void
on_ewrecherchecours_show               (GtkWidget       *widget,
                                        gpointer         user_data)
{
    GtkWidget *treeview;
    GtkListStore *store;
    GtkTreeIter iter;
    FILE *file;
    char line[256];
    course c;
    
    // Get the treeview widget
    treeview = lookup_widget(widget, "ewtreeview1list");  // Adjust widget name if different
    
    if (!treeview) {
        g_print("Error: Could not find treeview widget\n");
        return;
    }
    
    // Setup treeview if not already done
    if (!gtk_tree_view_get_model(GTK_TREE_VIEW(treeview))) {
        setup_course_treeview_columns(treeview);
    }
    
    // Get the store from treeview
    store = GTK_LIST_STORE(gtk_tree_view_get_model(GTK_TREE_VIEW(treeview)));
    
    // Clear existing data
    gtk_list_store_clear(store);
    
    // Open the file
    file = fopen("course.txt", "r");
    if (!file) {
        g_print("Error: Could not open course.txt\n");
        // Show error dialog
        GtkWidget *dialog = gtk_message_dialog_new(GTK_WINDOW(widget),
                                                  GTK_DIALOG_MODAL,
                                                  GTK_MESSAGE_ERROR,
                                                  GTK_BUTTONS_OK,
                                                  "Error: Could not open course database!");
        gtk_dialog_run(GTK_DIALOG(dialog));
        gtk_widget_destroy(dialog);
        return;
    }
    
    g_print("Loading courses from course.txt...\n");
    
    // Read and parse each line
    int count = 0;
    while (fgets(line, sizeof(line), file)) {
        line[strcspn(line, "\n")] = 0; // Remove newline
        
        // Parse with pipe separator (as per your course.c format)
        if (sscanf(line, "%d|%[^|]|%[^|]|%[^|]|%[^|]|%d|%d|%[^|]|%[^|]|%d",
                   &c.id, c.typecourse, c.level, c.gender, 
                   c.date, &c.hour, &c.duration, c.status, 
                   c.rescoach, &c.room) == 10) {
            
            // Add new row
            gtk_list_store_append(store, &iter);
            
            // Set ALL 10 columns
            gtk_list_store_set(store, &iter,
                              0, c.id,           // Column 0: ID
                              1, c.typecourse,   // Column 1: Type Course  
                              2, c.level,        // Column 2: Level
                              3, c.gender,       // Column 3: Gender
                              4, c.date,         // Column 4: Date
                              5, c.hour,         // Column 5: Hour
                              6, c.duration,     // Column 6: Duration
                              7, c.status,       // Column 7: Status
                              8, c.rescoach,     // Column 8: Responsible Coach
                              9, c.room,         // Column 9: Room
                              -1);
            
            count++;
            g_print("Loaded course ID: %d - %s\n", c.id, c.typecourse);
        } else {
            g_print("Failed to parse line: %s\n", line);
        }
    }
    
    fclose(file);
    
    
    g_print("Data loaded successfully! Total: %d courses\n", count);
}


void
on_searchlistEW_clicked                  (GtkWidget       *object,
                                        gpointer         user_data)
{
    GtkWidget *window = gtk_widget_get_toplevel(object);
    GtkWidget *treeview;
    GtkWidget *entry_id;
    GtkWidget *radiobutton_mixed;    // Mixed gender
    GtkWidget *radiobutton_male;     // Male only
    GtkWidget *radiobutton_female;   // Female only
    GtkWidget *radiobutton_all;      // All (no filter)
    
    if (!window || !GTK_IS_WINDOW(window)) {
        g_print("Error: Could not find window\n");
        return;
    }
    
    // Get the treeview
    treeview = lookup_widget(window, "ewtreeview1list");  // Adjust widget name if different
    if (!treeview) {
        g_print("Error: Could not find treeview widget\n");
        return;
    }
    
    // Get search widgets
    entry_id = lookup_widget(window, "ewentry5id");  // Adjust widget name
    
    // Get radio buttons for gender
    radiobutton_mixed = lookup_widget(window, "ewradiobutton1mixed");  // Adjust widget names
    radiobutton_male = lookup_widget(window, "ewradiobutton2male");
    radiobutton_female = lookup_widget(window, "ewradiobutton3fem");
    radiobutton_all = lookup_widget(window, "radiobutton1");
    
    // Check if widgets exist
    if (!entry_id) {
        GtkWidget *dialog = gtk_message_dialog_new(GTK_WINDOW(window),
                                                  GTK_DIALOG_MODAL,
                                                  GTK_MESSAGE_ERROR,
                                                  GTK_BUTTONS_OK,
                                                  "Error: Could not find ID field!");
        gtk_dialog_run(GTK_DIALOG(dialog));
        gtk_widget_destroy(dialog);
        return;
    }
    
    // Get search ID
    const gchar *id_str = gtk_entry_get_text(GTK_ENTRY(entry_id));
    
    // Determine gender filter based on radio buttons
    char gender_filter[20] = "";
    
    if (radiobutton_mixed && gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(radiobutton_mixed))) {
        strcpy(gender_filter, "Mixed");
    } 
    else if (radiobutton_male && gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(radiobutton_male))) {
        strcpy(gender_filter, "Male only");
    } 
    else if (radiobutton_female && gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(radiobutton_female))) {
        strcpy(gender_filter, "Female only");
    }
    else if (radiobutton_all && gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(radiobutton_all))) {
        // "All" means no gender filter (empty string)
        gender_filter[0] = '\0';
    }
    else {
        // Default to "All" if nothing selected
        gender_filter[0] = '\0';
    }
    
    // Convert ID string to int (0 if empty)
    int search_id = 0;
    if (strlen(id_str) > 0) {
        // Validate ID is a number
        int valid = 1;
        for (int i = 0; id_str[i] != '\0'; i++) {
            if (!isdigit(id_str[i])) {
                valid = 0;
                break;
            }
        }
        if (valid) {
            search_id = atoi(id_str);
        } else {
            GtkWidget *dialog = gtk_message_dialog_new(GTK_WINDOW(window),
                                                      GTK_DIALOG_MODAL,
                                                      GTK_MESSAGE_ERROR,
                                                      GTK_BUTTONS_OK,
                                                      "Error: Course ID must be a number!");
            gtk_dialog_run(GTK_DIALOG(dialog));
            gtk_widget_destroy(dialog);
            return;
        }
    }
    
    // Setup treeview if needed
    if (!gtk_tree_view_get_model(GTK_TREE_VIEW(treeview))) {
        setup_course_treeview_columns(treeview);
    }
    
    // Get the store
    GtkListStore *store = GTK_LIST_STORE(gtk_tree_view_get_model(GTK_TREE_VIEW(treeview)));
    
    // Clear existing data
    gtk_list_store_clear(store);
    
    // Perform search
    course results[100];
    
    // Prepare gender string for search
    char *gender_to_search = NULL;
    if (strlen(gender_filter) > 0) {
        gender_to_search = gender_filter;
    }
    
    // Call your search function from course.c
    int result_count = chercher_multi_criteriaEW("course.txt", search_id, 
                                              gender_to_search, results, 100);
    
    if (result_count == 0) {
        // Show no results message
        GtkWidget *dialog = gtk_message_dialog_new(GTK_WINDOW(window),
                                                  GTK_DIALOG_MODAL,
                                                  GTK_MESSAGE_INFO,
                                                  GTK_BUTTONS_OK,
                                                  "No courses found matching your criteria.\n\n"
                                                  "Search criteria:\n"
                                                  "• ID: %s\n"
                                                  "• Gender: %s",
                                                  (strlen(id_str) > 0) ? id_str : "Any",
                                                  (strlen(gender_filter) > 0) ? gender_filter : "Any");
        gtk_dialog_run(GTK_DIALOG(dialog));
        gtk_widget_destroy(dialog);
        return;
    }
    
    // Display results
    GtkTreeIter iter;
    for (int i = 0; i < result_count; i++) {
        gtk_list_store_append(store, &iter);
        gtk_list_store_set(store, &iter,
                          0, results[i].id,
                          1, results[i].typecourse,
                          2, results[i].level,
                          3, results[i].gender,
                          4, results[i].date,
                          5, results[i].hour,
                          6, results[i].duration,
                          7, results[i].status,
                          8, results[i].rescoach,
                          9, results[i].room,
                          -1);
    }
    

    
    g_print("Search complete: Found %d courses (ID: %s, Gender: %s)\n", 
            result_count, 
            (strlen(id_str) > 0) ? id_str : "Any",
            (strlen(gender_filter) > 0) ? gender_filter : "Any");
}


void
on_canclsearchlist_clicked             (GtkButton       *button,
                                        gpointer         user_data)
{
    GtkWidget *window = gtk_widget_get_toplevel(GTK_WIDGET(button));
    GtkWidget *treeview;
    GtkWidget *entry_id;
    
    if (!window || !GTK_IS_WINDOW(window)) {
        return;
    }
    
    // Get the treeview
    treeview = lookup_widget(window, "ewtreeview1list");
    if (!treeview) {
        return;
    }
    
    // Clear ID entry field
    entry_id = lookup_widget(window, "ewentry5id");
    if (entry_id) {
        gtk_entry_set_text(GTK_ENTRY(entry_id), "");
    }
    
    // Reset radio buttons to "All"
    GtkWidget *radiobutton_all = lookup_widget(window, "radiobutton1");
    if (radiobutton_all) {
        gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(radiobutton_all), TRUE);
    }
    
    // Call the show function to reload all courses
    on_ewrecherchecours_show(window, NULL);
}


void
on_ewtreeview1list_cursor_changed      (GtkTreeView     *treeview,
                                        gpointer         user_data)
{
    GtkWidget *window;
    GtkWidget *textview;  // Or use multiple entry widgets if you prefer
    GtkTreeSelection *selection;
    GtkTreeModel *model;
    GtkTreeIter iter;
    
    window = gtk_widget_get_toplevel(GTK_WIDGET(treeview));
    if (!window || !GTK_IS_WINDOW(window)) {
        return;
    }
    
    // If you have a textview to show details
    textview = lookup_widget(GTK_WIDGET(treeview), "ewtextview1info");  // Adjust name
    if (textview) {
        GtkTextBuffer *buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(textview));
        selection = gtk_tree_view_get_selection(treeview);
        
        if (gtk_tree_selection_get_selected(selection, &model, &iter)) {
            // Get course details from selected row
            gint id, hour, duration, room;
            gchar *typecourse, *level, *gender, *date, *status, *rescoach;
            
            gtk_tree_model_get(model, &iter,
                              0, &id,             // Column 0: ID
                              1, &typecourse,     // Column 1: Type Course
                              2, &level,          // Column 2: Level
                              3, &gender,         // Column 3: Gender
                              4, &date,           // Column 4: Date
                              5, &hour,           // Column 5: Hour
                              6, &duration,       // Column 6: Duration
                              7, &status,         // Column 7: Status
                              8, &rescoach,       // Column 8: Responsible Coach
                              9, &room,           // Column 9: Room
                              -1);
            
            // Create formatted text
            gchar *details = g_strdup_printf(
                "Course ID: %d\n"
                "Type: %s\n"
                "Level: %s\n"
                "Gender: %s\n"
                "Date: %s\n"
                "Time: %d:00\n"
                "Duration: %d hours\n"
                "Status: %s\n"
                "Coach: %s\n"
                "Room: %d",
                id, typecourse, level, gender, date,
                hour, duration, status, rescoach, room);
            
            gtk_text_buffer_set_text(buffer, details, -1);
            
            // Store selected course ID in global variable or pass to modify button
            // You could use a global variable or user_data
            
            // Free memory
            g_free(typecourse);
            g_free(level);
            g_free(gender);
            g_free(date);
            g_free(status);
            g_free(rescoach);
            g_free(details);
        } else {
            gtk_text_buffer_set_text(buffer, "Select a course to view details", -1);
        }
    }
    
    // Alternative: Store selected course ID for modify button
    // You can use a global variable or widget data
    static int selected_course_id = 0;
    selection = gtk_tree_view_get_selection(treeview);
    
    if (gtk_tree_selection_get_selected(selection, &model, &iter)) {
        gtk_tree_model_get(model, &iter, 0, &selected_course_id, -1);
        g_print("Selected course ID: %d\n", selected_course_id);
        
        // Store in window data for access by modify button
        g_object_set_data(G_OBJECT(window), "selected_course_id", GINT_TO_POINTER(selected_course_id));
    }
}


void
on_modifylist_clicked                  (GtkWidget       *object,
                                        gpointer         user_data)
{
    GtkWidget *window = gtk_widget_get_toplevel(object);
    GtkWidget *treeview;
    GtkWidget *textview;
    GtkTreeSelection *selection;
    GtkTreeModel *model;
    GtkTreeIter iter;
    
    if (!window || !GTK_IS_WINDOW(window)) {
        g_print("Error: Could not find window\n");
        return;
    }
    
    // Get the treeview
    treeview = lookup_widget(window, "ewtreeview1list");  // Adjust name
    if (!treeview) {
        g_print("Error: Could not find treeview widget\n");
        return;
    }
    
    // Get selection
    selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(treeview));
    if (!gtk_tree_selection_get_selected(selection, &model, &iter)) {
        GtkWidget *dialog = gtk_message_dialog_new(GTK_WINDOW(window),
                                                  GTK_DIALOG_MODAL,
                                                  GTK_MESSAGE_ERROR,
                                                  GTK_BUTTONS_OK,
                                                  "Please select a course to modify!");
        gtk_dialog_run(GTK_DIALOG(dialog));
        gtk_widget_destroy(dialog);
        return;
    }
    
    // Get current course data
    gint id, hour, duration, room;
    gchar *typecourse, *level, *gender, *date, *status, *rescoach;
    
    gtk_tree_model_get(model, &iter,
                      0, &id,             // Column 0: ID
                      1, &typecourse,     // Column 1: Type Course
                      2, &level,          // Column 2: Level
                      3, &gender,         // Column 3: Gender
                      4, &date,           // Column 4: Date
                      5, &hour,           // Column 5: Hour
                      6, &duration,       // Column 6: Duration
                      7, &status,         // Column 7: Status
                      8, &rescoach,       // Column 8: Responsible Coach
                      9, &room,           // Column 9: Room
                      -1);
    
    // Create a modification dialog/window
    // You have two options:
    
    // OPTION 1: Open a new modify window with pre-filled fields
    // OPTION 2: Use existing fields in current window (like textview)
    
    // For now, let's assume you have a textview with modified data
    textview = lookup_widget(window, "ewtextview1info");  // Adjust name
    if (!textview) {
        // Free memory first
        g_free(typecourse);
        g_free(level);
        g_free(gender);
        g_free(date);
        g_free(status);
        g_free(rescoach);
        
        GtkWidget *dialog = gtk_message_dialog_new(GTK_WINDOW(window),
                                                  GTK_DIALOG_MODAL,
                                                  GTK_MESSAGE_ERROR,
                                                  GTK_BUTTONS_OK,
                                                  "Error: Could not find textview for modifications!");
        gtk_dialog_run(GTK_DIALOG(dialog));
        gtk_widget_destroy(dialog);
        return;
    }
    
    // Get the modified text from textview
    GtkTextBuffer *buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(textview));
    GtkTextIter start, end;
    gtk_text_buffer_get_start_iter(buffer, &start);
    gtk_text_buffer_get_end_iter(buffer, &end);
    gchar *modified_text = gtk_text_buffer_get_text(buffer, &start, &end, FALSE);
    
    // Parse the modified text to extract updated values
    // Format: "Course ID: X\nType: Y\nLevel: Z\nGender: W\nDate: V\nTime: U\nDuration: T\nStatus: S\nCoach: R\nRoom: Q"
    course updated_course;
    updated_course.id = id;  // ID shouldn't change
    
    // Simple parsing - adjust based on your exact format
    char *line = strtok(modified_text, "\n");
    while (line != NULL) {
        if (strstr(line, "Type:") != NULL) {
            sscanf(line, "Type: %49[^\n]", updated_course.typecourse);
        } else if (strstr(line, "Level:") != NULL) {
            sscanf(line, "Level: %49[^\n]", updated_course.level);
        } else if (strstr(line, "Gender:") != NULL) {
            sscanf(line, "Gender: %19[^\n]", updated_course.gender);
        } else if (strstr(line, "Date:") != NULL) {
            sscanf(line, "Date: %19[^\n]", updated_course.date);
        } else if (strstr(line, "Time:") != NULL) {
            int time_val;
            sscanf(line, "Time: %d", &time_val);
            updated_course.hour = time_val;
        } else if (strstr(line, "Duration:") != NULL) {
            int dur_val;
            sscanf(line, "Duration: %d", &dur_val);
            updated_course.duration = dur_val;
        } else if (strstr(line, "Status:") != NULL) {
            sscanf(line, "Status: %19[^\n]", updated_course.status);
        } else if (strstr(line, "Coach:") != NULL) {
            sscanf(line, "Coach: %49[^\n]", updated_course.rescoach);
        } else if (strstr(line, "Room:") != NULL) {
            int room_val;
            sscanf(line, "Room: %d", &room_val);
            updated_course.room = room_val;
        }
        line = strtok(NULL, "\n");
    }
    
    g_free(modified_text);
    
    // Call modifier function from course.c
    int result = modifierEW("course.txt", id, updated_course);
    
    // Free memory
    g_free(typecourse);
    g_free(level);
    g_free(gender);
    g_free(date);
    g_free(status);
    g_free(rescoach);
    
    // Show result
    GtkWidget *dialog;
    if (result == 1) {
        dialog = gtk_message_dialog_new(GTK_WINDOW(window),
                                       GTK_DIALOG_MODAL,
                                       GTK_MESSAGE_INFO,
                                       GTK_BUTTONS_OK,
                                       "Course information updated successfully!\n\n"
                                       "ID: %d\n"
                                       "Type: %s\n"
                                       "Coach: %s",
                                       updated_course.id, 
                                       updated_course.typecourse,
                                       updated_course.rescoach);
        
        // Refresh the treeview to show updated data
        // Assuming you have a show function for this window
        on_ewrecherchecours_show(window, NULL);
        
    } else {
        dialog = gtk_message_dialog_new(GTK_WINDOW(window),
                                       GTK_DIALOG_MODAL,
                                       GTK_MESSAGE_ERROR,
                                       GTK_BUTTONS_OK,
                                       "Error updating course information!");
    }
    
    gtk_dialog_run(GTK_DIALOG(dialog));
    gtk_widget_destroy(dialog);
}


void
on_ewdeletecous_show                   (GtkWidget       *widget,
                                        gpointer         user_data)
{
    GtkWidget *treeview;
    GtkListStore *store;
    GtkTreeIter iter;
    FILE *file;
    char line[256];
    course c;
    
    // Get the treeview widget
    treeview = lookup_widget(widget, "ewtreeview2");  // Adjust widget name if different
    
    if (!treeview) {
        g_print("Error: Could not find treeview widget\n");
        return;
    }
    
    // Setup treeview if not already done
    if (!gtk_tree_view_get_model(GTK_TREE_VIEW(treeview))) {
        setup_course_treeview_columns(treeview);
    }
    
    // Get the store from treeview
    store = GTK_LIST_STORE(gtk_tree_view_get_model(GTK_TREE_VIEW(treeview)));
    
    // Clear existing data
    gtk_list_store_clear(store);
    
    // Open the file
    file = fopen("course.txt", "r");
    if (!file) {
        g_print("Error: Could not open course.txt\n");
        // Show error dialog
        GtkWidget *dialog = gtk_message_dialog_new(GTK_WINDOW(widget),
                                                  GTK_DIALOG_MODAL,
                                                  GTK_MESSAGE_ERROR,
                                                  GTK_BUTTONS_OK,
                                                  "Error: Could not open course database!");
        gtk_dialog_run(GTK_DIALOG(dialog));
        gtk_widget_destroy(dialog);
        return;
    }
    
    g_print("Loading courses from course.txt...\n");
    
    // Read and parse each line
    int count = 0;
    while (fgets(line, sizeof(line), file)) {
        line[strcspn(line, "\n")] = 0; // Remove newline
        
        // Parse with pipe separator (as per your course.c format)
        if (sscanf(line, "%d|%[^|]|%[^|]|%[^|]|%[^|]|%d|%d|%[^|]|%[^|]|%d",
                   &c.id, c.typecourse, c.level, c.gender, 
                   c.date, &c.hour, &c.duration, c.status, 
                   c.rescoach, &c.room) == 10) {
            
            // Add new row
            gtk_list_store_append(store, &iter);
            
            // Set ALL 10 columns
            gtk_list_store_set(store, &iter,
                              0, c.id,           // Column 0: ID
                              1, c.typecourse,   // Column 1: Type Course  
                              2, c.level,        // Column 2: Level
                              3, c.gender,       // Column 3: Gender
                              4, c.date,         // Column 4: Date
                              5, c.hour,         // Column 5: Hour
                              6, c.duration,     // Column 6: Duration
                              7, c.status,       // Column 7: Status
                              8, c.rescoach,     // Column 8: Responsible Coach
                              9, c.room,         // Column 9: Room
                              -1);
            
            count++;
            g_print("Loaded course ID: %d - %s\n", c.id, c.typecourse);
        } else {
            g_print("Failed to parse line: %s\n", line);
        }
    }
    
    fclose(file);
    
    
    g_print("Data loaded successfully! Total: %d courses\n", count);
}


void
on_searchdeleteEW_clicked                (GtkWidget       *object,
                                        gpointer         user_data)
{
   GtkWidget *window = gtk_widget_get_toplevel(object);
    GtkWidget *treeview;
    GtkWidget *entry_id;
    GtkWidget *radiobutton_mixed;    // Mixed gender
    GtkWidget *radiobutton_male;     // Male only
    GtkWidget *radiobutton_female;   // Female only
    GtkWidget *radiobutton_all;      // All (no filter)
    
    if (!window || !GTK_IS_WINDOW(window)) {
        g_print("Error: Could not find window\n");
        return;
    }
    
    // Get the treeview
    treeview = lookup_widget(window, "ewtreeview2");  // Adjust widget name if different
    if (!treeview) {
        g_print("Error: Could not find treeview widget\n");
        return;
    }
    
    // Get search widgets
    entry_id = lookup_widget(window, "ewentry5id");  // Adjust widget name
    
    // Get radio buttons for gender
    radiobutton_mixed = lookup_widget(window, "ewradiobutton4mix");  // Adjust widget names
    radiobutton_male = lookup_widget(window, "ewradiobutton5mal");
    radiobutton_female = lookup_widget(window, "ewradiobutton6fem");
    radiobutton_all = lookup_widget(window, "radiobutton2");
    
    // Check if widgets exist
    if (!entry_id) {
        GtkWidget *dialog = gtk_message_dialog_new(GTK_WINDOW(window),
                                                  GTK_DIALOG_MODAL,
                                                  GTK_MESSAGE_ERROR,
                                                  GTK_BUTTONS_OK,
                                                  "Error: Could not find ID field!");
        gtk_dialog_run(GTK_DIALOG(dialog));
        gtk_widget_destroy(dialog);
        return;
    }
    
    // Get search ID
    const gchar *id_str = gtk_entry_get_text(GTK_ENTRY(entry_id));
    
    // Determine gender filter based on radio buttons
    char gender_filter[20] = "";
    
    if (radiobutton_mixed && gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(radiobutton_mixed))) {
        strcpy(gender_filter, "Mixed");
    } 
    else if (radiobutton_male && gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(radiobutton_male))) {
        strcpy(gender_filter, "Male only");
    } 
    else if (radiobutton_female && gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(radiobutton_female))) {
        strcpy(gender_filter, "Female only");
    }
    else if (radiobutton_all && gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(radiobutton_all))) {
        // "All" means no gender filter (empty string)
        gender_filter[0] = '\0';
    }
    else {
        // Default to "All" if nothing selected
        gender_filter[0] = '\0';
    }
    
    // Convert ID string to int (0 if empty)
    int search_id = 0;
    if (strlen(id_str) > 0) {
        // Validate ID is a number
        int valid = 1;
        for (int i = 0; id_str[i] != '\0'; i++) {
            if (!isdigit(id_str[i])) {
                valid = 0;
                break;
            }
        }
        if (valid) {
            search_id = atoi(id_str);
        } else {
            GtkWidget *dialog = gtk_message_dialog_new(GTK_WINDOW(window),
                                                      GTK_DIALOG_MODAL,
                                                      GTK_MESSAGE_ERROR,
                                                      GTK_BUTTONS_OK,
                                                      "Error: Course ID must be a number!");
            gtk_dialog_run(GTK_DIALOG(dialog));
            gtk_widget_destroy(dialog);
            return;
        }
    }
    
    // Setup treeview if needed
    if (!gtk_tree_view_get_model(GTK_TREE_VIEW(treeview))) {
        setup_course_treeview_columns(treeview);
    }
    
    // Get the store
    GtkListStore *store = GTK_LIST_STORE(gtk_tree_view_get_model(GTK_TREE_VIEW(treeview)));
    
    // Clear existing data
    gtk_list_store_clear(store);
    
    // Perform search
    course results[100];
    
    // Prepare gender string for search
    char *gender_to_search = NULL;
    if (strlen(gender_filter) > 0) {
        gender_to_search = gender_filter;
    }
    
    // Call your search function from course.c
    int result_count = chercher_multi_criteriaEW("course.txt", search_id, 
                                              gender_to_search, results, 100);
    
    if (result_count == 0) {
        // Show no results message
        GtkWidget *dialog = gtk_message_dialog_new(GTK_WINDOW(window),
                                                  GTK_DIALOG_MODAL,
                                                  GTK_MESSAGE_INFO,
                                                  GTK_BUTTONS_OK,
                                                  "No courses found matching your criteria.\n\n"
                                                  "Search criteria:\n"
                                                  "• ID: %s\n"
                                                  "• Gender: %s",
                                                  (strlen(id_str) > 0) ? id_str : "Any",
                                                  (strlen(gender_filter) > 0) ? gender_filter : "Any");
        gtk_dialog_run(GTK_DIALOG(dialog));
        gtk_widget_destroy(dialog);
        return;
    }
    
    // Display results
    GtkTreeIter iter;
    for (int i = 0; i < result_count; i++) {
        gtk_list_store_append(store, &iter);
        gtk_list_store_set(store, &iter,
                          0, results[i].id,
                          1, results[i].typecourse,
                          2, results[i].level,
                          3, results[i].gender,
                          4, results[i].date,
                          5, results[i].hour,
                          6, results[i].duration,
                          7, results[i].status,
                          8, results[i].rescoach,
                          9, results[i].room,
                          -1);
    }
    

    
    g_print("Search complete: Found %d courses (ID: %s, Gender: %s)\n", 
            result_count, 
            (strlen(id_str) > 0) ? id_str : "Any",
            (strlen(gender_filter) > 0) ? gender_filter : "Any");
}


void
on_cancelsearchdelEW_clicked             (GtkButton       *button,
                                        gpointer         user_data)
{
    GtkWidget *window = gtk_widget_get_toplevel(GTK_WIDGET(button));
    GtkWidget *treeview;
    GtkWidget *entry_id;
    
    if (!window || !GTK_IS_WINDOW(window)) {
        return;
    }
    
    // Get the treeview
    treeview = lookup_widget(window, "ewtreeview2");
    if (!treeview) {
        return;
    }
    
    // Clear ID entry field
    entry_id = lookup_widget(window, "ewentry6id");
    if (entry_id) {
        gtk_entry_set_text(GTK_ENTRY(entry_id), "");
    }
    
    // Reset radio buttons to "All"
    GtkWidget *radiobutton_all = lookup_widget(window, "radiobutton2");
    if (radiobutton_all) {
        gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(radiobutton_all), TRUE);
    }
    
    // Call the show function to reload all courses
    on_ewrecherchecours_show(window, NULL);
}


void
on_deleteC_clicked                     (GtkWidget       *object,
                                        gpointer         user_data)
{
    GtkWidget *window = gtk_widget_get_toplevel(object);
    GtkWidget *treeview;
    GtkTreeSelection *selection;
    GtkTreeModel *model;
    GtkTreeIter iter;
    
    if (!window || !GTK_IS_WINDOW(window)) {
        g_print("Error: Could not find window\n");
        return;
    }
    
    // Get the treeview
    treeview = lookup_widget(window, "ewtreeview2");
    if (!treeview) {
        g_print("Error: Could not find ewtreeview2 widget\n");
        GtkWidget *dialog = gtk_message_dialog_new(GTK_WINDOW(window),
                                                  GTK_DIALOG_MODAL,
                                                  GTK_MESSAGE_ERROR,
                                                  GTK_BUTTONS_OK,
                                                  "Error: Could not find course list!");
        gtk_dialog_run(GTK_DIALOG(dialog));
        gtk_widget_destroy(dialog);
        return;
    }
    
    // Get the selection
    selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(treeview));
    if (!gtk_tree_selection_get_selected(selection, &model, &iter)) {
        GtkWidget *dialog = gtk_message_dialog_new(GTK_WINDOW(window),
                                                  GTK_DIALOG_MODAL,
                                                  GTK_MESSAGE_WARNING,
                                                  GTK_BUTTONS_OK,
                                                  "Please select a course to delete!");
        gtk_dialog_run(GTK_DIALOG(dialog));
        gtk_widget_destroy(dialog);
        return;
    }
    
    // Get the course ID and details
    gint course_id;
    gchar *typecourse = NULL;
    gchar *coach = NULL;
    
    gtk_tree_model_get(model, &iter,
                      0, &course_id,     // Column 0: ID
                      1, &typecourse,    // Column 1: Type Course
                      8, &coach,         // Column 8: Coach
                      -1);
    
    // Ask for confirmation
    GtkWidget *confirm_dialog = gtk_message_dialog_new(GTK_WINDOW(window),
                                                      GTK_DIALOG_MODAL,
                                                      GTK_MESSAGE_QUESTION,
                                                      GTK_BUTTONS_YES_NO,
                                                      "Delete Course?\n\n"
                                                      "ID: %d\n"
                                                      "Type: %s\n"
                                                      "Coach: %s\n\n"
                                                      "Course will be moved to restore.txt for recovery.",
                                                      course_id, 
                                                      typecourse ? typecourse : "Unknown",
                                                      coach ? coach : "Unknown");
    
    gint response = gtk_dialog_run(GTK_DIALOG(confirm_dialog));
    gtk_widget_destroy(confirm_dialog);
    
    if (response == GTK_RESPONSE_YES) {
        // Delete the course
        int result = supprimerEW("course.txt", course_id);
        
        if (result == 1) {
            // Success - remove from treeview
            GtkListStore *store = GTK_LIST_STORE(model);
            gtk_list_store_remove(store, &iter);
            
            GtkWidget *success_dialog = gtk_message_dialog_new(GTK_WINDOW(window),
                                                              GTK_DIALOG_MODAL,
                                                              GTK_MESSAGE_INFO,
                                                              GTK_BUTTONS_OK,
                                                              "✓ Course deleted successfully!\n\n"
                                                              "ID: %d\n"
                                                              "Type: %s\n"
                                                              "Coach: %s\n\n"
                                                              "Course has been moved to restore.txt.",
                                                              course_id,
                                                              typecourse ? typecourse : "Unknown",
                                                              coach ? coach : "Unknown");
            gtk_dialog_run(GTK_DIALOG(success_dialog));
            gtk_widget_destroy(success_dialog);
            
            g_print("Course ID %d deleted and moved to restore.txt\n", course_id);
        } else {
            GtkWidget *error_dialog = gtk_message_dialog_new(GTK_WINDOW(window),
                                                            GTK_DIALOG_MODAL,
                                                            GTK_MESSAGE_ERROR,
                                                            GTK_BUTTONS_OK,
                                                            "✗ Error deleting course!\n\n"
                                                            "ID: %d\n"
                                                            "Type: %s\n\n"
                                                            "Course may not exist or file error occurred.",
                                                            course_id,
                                                            typecourse ? typecourse : "Unknown");
            gtk_dialog_run(GTK_DIALOG(error_dialog));
            gtk_widget_destroy(error_dialog);
            
            g_print("Error deleting course ID %d\n", course_id);
        }
    } else {
        g_print("Delete cancelled for course ID %d\n", course_id);
    }
    
    // Free memory
    if (typecourse) g_free(typecourse);
    if (coach) g_free(coach);
}


void
on_ewrestorecours_show                 (GtkWidget       *widget,
                                        gpointer         user_data)
{
    GtkWidget *treeview;
    GtkListStore *store;
    GtkTreeIter iter;
    FILE *file;
    char line[256];
    course c;
    
    // Get the treeview widget - adjust name based on your Glade file
    treeview = lookup_widget(widget, "ewtreeview3");  // Or whatever your treeview name is
    
    if (!treeview) {
        g_print("Error: Could not find treeview widget\n");
        return;
    }
    
    // Setup treeview if not already done
    if (!gtk_tree_view_get_model(GTK_TREE_VIEW(treeview))) {
        setup_course_treeview_columns(treeview);
    }
    
    // Get the store from treeview
    store = GTK_LIST_STORE(gtk_tree_view_get_model(GTK_TREE_VIEW(treeview)));
    
    // Clear existing data
    gtk_list_store_clear(store);
    
    // Open the restore.txt file
    file = fopen("restoreEW.txt", "r");
    if (!file) {
        g_print("Warning: Could not open restore.txt (file may be empty)\n");
        
        // Show info dialog
        GtkWidget *dialog = gtk_message_dialog_new(GTK_WINDOW(widget),
                                                  GTK_DIALOG_MODAL,
                                                  GTK_MESSAGE_INFO,
                                                  GTK_BUTTONS_OK,
                                                  "No deleted courses found in restore.txt.");
        gtk_dialog_run(GTK_DIALOG(dialog));
        gtk_widget_destroy(dialog);
        return;
    }
    
    g_print("Loading data from restore.txt...\n");
    
    int count = 0;
    // Read and parse each line
    while (fgets(line, sizeof(line), file)) {
        line[strcspn(line, "\n")] = 0; // Remove newline
        
        // Parse with pipe separator (same format as course.txt)
        if (sscanf(line, "%d|%[^|]|%[^|]|%[^|]|%[^|]|%d|%d|%[^|]|%[^|]|%d",
                   &c.id, c.typecourse, c.level, c.gender, 
                   c.date, &c.hour, &c.duration, c.status, 
                   c.rescoach, &c.room) == 10) {
            
            // Add new row
            gtk_list_store_append(store, &iter);
            
            // Set ALL 10 columns
            gtk_list_store_set(store, &iter,
                              0, c.id,           // Column 0: ID
                              1, c.typecourse,   // Column 1: Type Course
                              2, c.level,        // Column 2: Level
                              3, c.gender,       // Column 3: Gender
                              4, c.date,         // Column 4: Date
                              5, c.hour,         // Column 5: Hour
                              6, c.duration,     // Column 6: Duration
                              7, c.status,       // Column 7: Status
                              8, c.rescoach,     // Column 8: Responsible Coach
                              9, c.room,         // Column 9: Room
                              -1);
            
            count++;
            g_print("Loaded from restore.txt: ID %d - %s\n", c.id, c.typecourse);
        } else {
            g_print("Failed to parse line from restore.txt: %s\n", line);
        }
    }
    
    fclose(file);
    
    if (count == 0) {
        g_print("restore.txt is empty\n");
        
        GtkWidget *dialog = gtk_message_dialog_new(GTK_WINDOW(widget),
                                                  GTK_DIALOG_MODAL,
                                                  GTK_MESSAGE_INFO,
                                                  GTK_BUTTONS_OK,
                                                  "No deleted courses available for restoration.");
        gtk_dialog_run(GTK_DIALOG(dialog));
        gtk_widget_destroy(dialog);
    } else {
        g_print("Successfully loaded %d courses from restore.txt\n", count);
        
        GtkWidget *dialog = gtk_message_dialog_new(GTK_WINDOW(widget),
                                                  GTK_DIALOG_MODAL,
                                                  GTK_MESSAGE_INFO,
                                                  GTK_BUTTONS_OK,
                                                  "Loaded %d deleted courses from restore.txt.\n"
                                                  "Select a course to restore.",
                                                  count);
        gtk_dialog_run(GTK_DIALOG(dialog));
        gtk_widget_destroy(dialog);
    }
}


void
on_searchrestoreEW_clicked               (GtkWidget       *object,
                                        gpointer         user_data)
{
   GtkWidget *window = gtk_widget_get_toplevel(object);
    GtkWidget *treeview;
    GtkWidget *entry_id;
    GtkWidget *radiobutton_mixed;    // Mixed gender
    GtkWidget *radiobutton_male;     // Male only
    GtkWidget *radiobutton_female;   // Female only
    GtkWidget *radiobutton_all;      // All (no filter)
    
    if (!window || !GTK_IS_WINDOW(window)) {
        g_print("Error: Could not find window\n");
        return;
    }
    
    // Get the treeview
    treeview = lookup_widget(window, "ewtreeview3");  // Adjust widget name if different
    if (!treeview) {
        g_print("Error: Could not find treeview widget\n");
        return;
    }
    
    // Get search widgets
    entry_id = lookup_widget(window, "ewentry5id");  // Adjust widget name
    
    // Get radio buttons for gender
    radiobutton_mixed = lookup_widget(window, "ewradiobutton7mix");  // Adjust widget names
    radiobutton_male = lookup_widget(window, "ewradiobutton8ma");
    radiobutton_female = lookup_widget(window, "ewradiobutton9fe");
    radiobutton_all = lookup_widget(window, "radiobutton3");
    
    // Check if widgets exist
    if (!entry_id) {
        GtkWidget *dialog = gtk_message_dialog_new(GTK_WINDOW(window),
                                                  GTK_DIALOG_MODAL,
                                                  GTK_MESSAGE_ERROR,
                                                  GTK_BUTTONS_OK,
                                                  "Error: Could not find ID field!");
        gtk_dialog_run(GTK_DIALOG(dialog));
        gtk_widget_destroy(dialog);
        return;
    }
    
    // Get search ID
    const gchar *id_str = gtk_entry_get_text(GTK_ENTRY(entry_id));
    
    // Determine gender filter based on radio buttons
    char gender_filter[20] = "";
    
    if (radiobutton_mixed && gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(radiobutton_mixed))) {
        strcpy(gender_filter, "Mixed");
    } 
    else if (radiobutton_male && gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(radiobutton_male))) {
        strcpy(gender_filter, "Male only");
    } 
    else if (radiobutton_female && gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(radiobutton_female))) {
        strcpy(gender_filter, "Female only");
    }
    else if (radiobutton_all && gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(radiobutton_all))) {
        // "All" means no gender filter (empty string)
        gender_filter[0] = '\0';
    }
    else {
        // Default to "All" if nothing selected
        gender_filter[0] = '\0';
    }
    
    // Convert ID string to int (0 if empty)
    int search_id = 0;
    if (strlen(id_str) > 0) {
        // Validate ID is a number
        int valid = 1;
        for (int i = 0; id_str[i] != '\0'; i++) {
            if (!isdigit(id_str[i])) {
                valid = 0;
                break;
            }
        }
        if (valid) {
            search_id = atoi(id_str);
        } else {
            GtkWidget *dialog = gtk_message_dialog_new(GTK_WINDOW(window),
                                                      GTK_DIALOG_MODAL,
                                                      GTK_MESSAGE_ERROR,
                                                      GTK_BUTTONS_OK,
                                                      "Error: Course ID must be a number!");
            gtk_dialog_run(GTK_DIALOG(dialog));
            gtk_widget_destroy(dialog);
            return;
        }
    }
    
    // Setup treeview if needed
    if (!gtk_tree_view_get_model(GTK_TREE_VIEW(treeview))) {
        setup_course_treeview_columns(treeview);
    }
    
    // Get the store
    GtkListStore *store = GTK_LIST_STORE(gtk_tree_view_get_model(GTK_TREE_VIEW(treeview)));
    
    // Clear existing data
    gtk_list_store_clear(store);
    
    // Perform search
    course results[100];
    
    // Prepare gender string for search
    char *gender_to_search = NULL;
    if (strlen(gender_filter) > 0) {
        gender_to_search = gender_filter;
    }
    
    // Call your search function from course.c
    int result_count = chercher_multi_criteriaEW("course.txt", search_id, 
                                              gender_to_search, results, 100);
    
    if (result_count == 0) {
        // Show no results message
        GtkWidget *dialog = gtk_message_dialog_new(GTK_WINDOW(window),
                                                  GTK_DIALOG_MODAL,
                                                  GTK_MESSAGE_INFO,
                                                  GTK_BUTTONS_OK,
                                                  "No courses found matching your criteria.\n\n"
                                                  "Search criteria:\n"
                                                  "• ID: %s\n"
                                                  "• Gender: %s",
                                                  (strlen(id_str) > 0) ? id_str : "Any",
                                                  (strlen(gender_filter) > 0) ? gender_filter : "Any");
        gtk_dialog_run(GTK_DIALOG(dialog));
        gtk_widget_destroy(dialog);
        return;
    }
    
    // Display results
    GtkTreeIter iter;
    for (int i = 0; i < result_count; i++) {
        gtk_list_store_append(store, &iter);
        gtk_list_store_set(store, &iter,
                          0, results[i].id,
                          1, results[i].typecourse,
                          2, results[i].level,
                          3, results[i].gender,
                          4, results[i].date,
                          5, results[i].hour,
                          6, results[i].duration,
                          7, results[i].status,
                          8, results[i].rescoach,
                          9, results[i].room,
                          -1);
    }
    

    
    g_print("Search complete: Found %d courses (ID: %s, Gender: %s)\n", 
            result_count, 
            (strlen(id_str) > 0) ? id_str : "Any",
            (strlen(gender_filter) > 0) ? gender_filter : "Any");
}


void
on_cancelsearchrest_clicked            (GtkButton       *button,
                                        gpointer         user_data)
{
    GtkWidget *window = gtk_widget_get_toplevel(GTK_WIDGET(button));
    GtkWidget *treeview;
    GtkWidget *entry_id;
    
    if (!window || !GTK_IS_WINDOW(window)) {
        return;
    }
    
    // Get the treeview
    treeview = lookup_widget(window, "ewtreeview3");
    if (!treeview) {
        return;
    }
    
    // Clear ID entry field
    entry_id = lookup_widget(window, "ewentry7id");
    if (entry_id) {
        gtk_entry_set_text(GTK_ENTRY(entry_id), "");
    }
    
    // Reset radio buttons to "All"
    GtkWidget *radiobutton_all = lookup_widget(window, "radiobutton3");
    if (radiobutton_all) {
        gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(radiobutton_all), TRUE);
    }
    
    // Call the show function to reload all courses
    on_ewrecherchecours_show(window, NULL);
}


void
on_restoreC_clicked                    (GtkWidget       *object,
                                        gpointer         user_data)
{
    GtkWidget *window = gtk_widget_get_toplevel(object);
    GtkWidget *treeview;
    GtkTreeSelection *selection;
    GtkTreeModel *model;
    GtkTreeIter iter;
    
    if (!window || !GTK_IS_WINDOW(window)) {
        g_print("Error: Could not find window\n");
        return;
    }
    
    // Get the treeview for restore window
    treeview = lookup_widget(window, "ewtreeview3"); // Adjust based on your actual treeview name
    if (!treeview) {
        g_print("Error: Could not find treeview widget for restore\n");
        
        GtkWidget *dialog = gtk_message_dialog_new(GTK_WINDOW(window),
                                                  GTK_DIALOG_MODAL,
                                                  GTK_MESSAGE_ERROR,
                                                  GTK_BUTTONS_OK,
                                                  "Error: Could not find course list!");
        gtk_dialog_run(GTK_DIALOG(dialog));
        gtk_widget_destroy(dialog);
        return;
    }
    
    // Get the selection
    selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(treeview));
    if (!gtk_tree_selection_get_selected(selection, &model, &iter)) {
        GtkWidget *dialog = gtk_message_dialog_new(GTK_WINDOW(window),
                                                  GTK_DIALOG_MODAL,
                                                  GTK_MESSAGE_WARNING,
                                                  GTK_BUTTONS_OK,
                                                  "Please select a course to restore!");
        gtk_dialog_run(GTK_DIALOG(dialog));
        gtk_widget_destroy(dialog);
        return;
    }
    
    // Get the course ID and details
    gint course_id;
    gchar *typecourse, *coach;
    
    gtk_tree_model_get(model, &iter,
                      0, &course_id,     // Column 0: ID
                      1, &typecourse,    // Column 1: Type Course
                      8, &coach,         // Column 8: Coach
                      -1);
    
    // Ask for confirmation
    GtkWidget *confirm_dialog = gtk_message_dialog_new(GTK_WINDOW(window),
                                                      GTK_DIALOG_MODAL,
                                                      GTK_MESSAGE_QUESTION,
                                                      GTK_BUTTONS_YES_NO,
                                                      "Restore Course?\n\n"
                                                      "ID: %d\n"
                                                      "Type: %s\n"
                                                      "Coach: %s\n\n"
                                                      "Course will be moved back to course.txt.",
                                                      course_id, typecourse, coach);
    
    gint response = gtk_dialog_run(GTK_DIALOG(confirm_dialog));
    gtk_widget_destroy(confirm_dialog);
    
    if (response == GTK_RESPONSE_YES) {
        // Restore the course using your restaurer function from course.c
        int result = restaurerEW("restoreEW.txt", "course.txt", course_id);
        
        GtkWidget *result_dialog;
        if (result == 1) {
            result_dialog = gtk_message_dialog_new(GTK_WINDOW(window),
                                                  GTK_DIALOG_MODAL,
                                                  GTK_MESSAGE_INFO,
                                                  GTK_BUTTONS_OK,
                                                  "✓ Course restored successfully!\n\n"
                                                  "ID: %d\n"
                                                  "Type: %s\n"
                                                  "Coach: %s\n\n"
                                                  "Course has been moved back to course.txt.",
                                                  course_id, typecourse, coach);
            
            // Remove the row from restore treeview
            GtkListStore *store = GTK_LIST_STORE(model);
            gtk_list_store_remove(store, &iter);
            
            // Refresh the restore treeview to show remaining courses
            on_ewrestorecours_show(object, NULL);
            
            // Optional: Refresh main course list if it's visible
            GtkWidget *main_treeview = lookup_widget(window, "ewtreeview1list");
            if (main_treeview) {
                // If there's a function to refresh main course list, call it
                // For example: on_ewrecherchecours_show(object, NULL);
            }
            
        } else {
            result_dialog = gtk_message_dialog_new(GTK_WINDOW(window),
                                                  GTK_DIALOG_MODAL,
                                                  GTK_MESSAGE_ERROR,
                                                  GTK_BUTTONS_OK,
                                                  "✗ Error restoring course!\n\n"
                                                  "ID: %d\n"
                                                  "Type: %s\n\n"
                                                  "Course may not exist in restore.txt.",
                                                  course_id, typecourse);
        }
        
        gtk_dialog_run(GTK_DIALOG(result_dialog));
        gtk_widget_destroy(result_dialog);
        
        g_print("Course ID %d restore result: %d\n", course_id, result);
    } else {
        g_print("Restore cancelled for course ID %d\n", course_id);
    }
    
    // Free memory
    g_free(typecourse);
    g_free(coach);
}


void
on_ewstats_show                        (GtkWidget       *widget,
                                        gpointer         user_data)
{
    FILE *file;
    char line[256];
    course c;
    int total_courses = 0;
    int complete_courses = 0;  // Changed from filled_courses to complete_courses
    
    // For complete courses type display
    char complete_types[500] = "";  // Buffer to store complete course types
    int complete_type_count = 0;
    
    GtkWidget *entry_total;
    GtkWidget *entry_complete;  // Changed from entry_filled to entry_complete
    
    // Get the entry widgets
    entry_total = lookup_widget(widget, "ewentry7tot");
    entry_complete = lookup_widget(widget, "ewentry8most");  // Your widget might be named "filled" but shows complete
    
    // Check if widgets exist
    if (!entry_total || !entry_complete) {
        g_print("Error: Could not find stat entry widgets\n");
        return;
    }
    
    // Open the course.txt file
    file = fopen("course.txt", "r");
    if (!file) {
        g_print("Error: Could not open course.txt\n");
        
        // Set entries to 0
        gtk_entry_set_text(GTK_ENTRY(entry_total), "0");
        gtk_entry_set_text(GTK_ENTRY(entry_complete), "0 - No complete courses");
        
        GtkWidget *dialog = gtk_message_dialog_new(GTK_WINDOW(widget),
                                                  GTK_DIALOG_MODAL,
                                                  GTK_MESSAGE_ERROR,
                                                  GTK_BUTTONS_OK,
                                                  "Error: Could not open course database!");
        gtk_dialog_run(GTK_DIALOG(dialog));
        gtk_widget_destroy(dialog);
        return;
    }
    
    g_print("Calculating statistics from course.txt...\n");
    
    // Read and parse each line
    while (fgets(line, sizeof(line), file)) {
        line[strcspn(line, "\n")] = 0; // Remove newline
        
        // Parse with pipe separator
        if (sscanf(line, "%d|%[^|]|%[^|]|%[^|]|%[^|]|%d|%d|%[^|]|%[^|]|%d",
                   &c.id, c.typecourse, c.level, c.gender, 
                   c.date, &c.hour, &c.duration, c.status, 
                   c.rescoach, &c.room) == 10) {
            
            total_courses++;
            
            // Check if course is complete (case-insensitive)
            // Changed from "full" to "complete"
            char status_lower[20];
            strncpy(status_lower, c.status, sizeof(status_lower)-1);
            status_lower[sizeof(status_lower)-1] = '\0';
            
            // Convert to lowercase for comparison
            for (int i = 0; status_lower[i]; i++) {
                status_lower[i] = tolower(status_lower[i]);
            }
            
            // Check for "complete" status
            if (strcmp(status_lower, "complete") == 0) {
                complete_courses++;
                complete_type_count++;
                
                // Add course type to the list (limit to avoid overflow)
                if (strlen(complete_types) < sizeof(complete_types) - 50) {
                    if (complete_type_count == 1) {
                        // First complete course
                        snprintf(complete_types + strlen(complete_types), 
                                sizeof(complete_types) - strlen(complete_types),
                                "%s (ID: %d)", c.typecourse, c.id);
                    } else {
                        // Subsequent complete courses
                        snprintf(complete_types + strlen(complete_types), 
                                sizeof(complete_types) - strlen(complete_types),
                                ", %s (ID: %d)", c.typecourse, c.id);
                    }
                }
            }
        }
    }
    
    fclose(file);
    
    // Display total courses
    char total_str[50];
    snprintf(total_str, sizeof(total_str), "%d", total_courses);
    gtk_entry_set_text(GTK_ENTRY(entry_total), total_str);
    
    // Display complete courses with their types
    char complete_str[600];
    if (complete_courses == 0) {
        snprintf(complete_str, sizeof(complete_str), "0 - No complete courses");
    } else {
        snprintf(complete_str, sizeof(complete_str), "%d - %s", complete_courses, complete_types);
        
        // If the list is too long, truncate it
        if (strlen(complete_str) > 100) {
            // Show first few courses and "..." for the rest
            complete_str[97] = '.';
            complete_str[98] = '.';
            complete_str[99] = '.';
            complete_str[100] = '\0';
        }
    }
    
    gtk_entry_set_text(GTK_ENTRY(entry_complete), complete_str);
    
    g_print("Statistics calculated: Total=%d, Complete=%d\n", total_courses, complete_courses);
}







/*
void
on_searchdeleteEW_clicked              (GtkButton       *button,
                                        gpointer         user_data)
{

}


void
on_searchrestoreEW_clicked             (GtkButton       *button,
                                        gpointer         user_data)
{

}
*/

void
on_button14_clicked                    (GtkWidget       *button,
                                        gpointer         user_data)
{
	GtkWidget *current = gtk_widget_get_toplevel(GTK_WIDGET(button));
    GtkWidget *w = create_CourseManag();
    gtk_widget_show(w);
if (current)
        gtk_widget_destroy(current);
}


void
on_ewbutton6home_clicked               (GtkWidget       *button,
                                        gpointer         user_data)
{
	GtkWidget *current = gtk_widget_get_toplevel(GTK_WIDGET(button));
    GtkWidget *w = create_AdminDashboard();
    gtk_widget_show(w);
if (current)
        gtk_widget_destroy(current);
}




void
on_coach_add                           (GtkButton       *button,
                                        gpointer         user_data)
{
    GtkWidget *coach_manag;
    GtkWidget *addCoachAdmin;

    coach_manag = lookup_widget(button, "CoachManagAdmin");
    if (coach_manag != NULL) {
        gtk_widget_destroy(coach_manag);
    }

    addCoachAdmin = create_addCoachAdmin();
    gtk_widget_show_all(addCoachAdmin);
}


void
on_coach_delete                        (GtkButton       *button,
                                        gpointer         user_data)
{
    GtkWidget *coach_manag;
    GtkWidget *deleteCoach;

    coach_manag = lookup_widget(button, "CoachManagAdmin");
    if (coach_manag != NULL) {
        gtk_widget_destroy(coach_manag);
    }

    deleteCoach = create_deleteCoach();
    gtk_widget_show_all(deleteCoach);
}


void
on_coach_restore                       (GtkButton       *button,
                                        gpointer         user_data)
{
    GtkWidget *coach_manag;
    GtkWidget *restoreCoach;

    coach_manag = lookup_widget(button, "CoachManagAdmin");
    if (coach_manag != NULL) {
        gtk_widget_destroy(coach_manag);
    }

    restoreCoach = create_restoreCoach();
    gtk_widget_show_all(restoreCoach);
}


void
on_coach_displayed                     (GtkButton       *button,
                                        gpointer         user_data)
{
    GtkWidget *coach_manag;
    GtkWidget *rechercheCoach;

    coach_manag = lookup_widget(button, "CoachManagAdmin");
    if (coach_manag != NULL) {
        gtk_widget_destroy(coach_manag);
    }

    rechercheCoach = create_rechercheCoach();
    gtk_widget_show_all(rechercheCoach);
}


void
on_coach_add_crud                      (GtkButton       *button,
                                        gpointer         user_data)
{

}


void
on_rechercheCoach_show                 (GtkWidget       *widget,
                                        gpointer         user_data)
{

}


void
on_treeview_coach_display              (GtkTreeView     *treeview,
                                        gpointer         user_data)
{

}


void
on_coach_search                        (GtkButton       *button,
                                        gpointer         user_data)
{

}


void
on_coach_modifyed                      (GtkButton       *button,
                                        gpointer         user_data)
{

}


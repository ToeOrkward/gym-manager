#include <gtk/gtk.h>


void
on_login_clicked                       (GtkWidget       *object,
                                        gpointer         user_data);

void
on_members_clicked                     (GtkWidget       *object,
                                        gpointer         user_data);

void
on_button15_clicked                    (GtkWidget       *object,
                                        gpointer         user_data);

void
on_add_clicked                         (GtkWidget       *object,
                                        gpointer         user_data);

void
on_list_clicked                        (GtkWidget       *object,
                                        gpointer         user_data);

void
on_modify_clicked                      (GtkWidget       *object,
                                        gpointer         user_data);

void
on_delete_clicked                      (GtkWidget       *object,
                                        gpointer         user_data);

void
on_restore_clicked                     (GtkWidget       *object,
                                        gpointer         user_data);

void
on_stats_clicked                       (GtkWidget       *object,
                                        gpointer         user_data);

void
on_home2admindash_clicked              (GtkWidget       *object,
                                        gpointer         user_data);

void
on_admin_coaches_clicked               (GtkWidget       *object,
                                        gpointer         user_data);

void
on_admin_events_clicked                (GtkWidget       *object,
                                        gpointer         user_data);

void
on_admin_equipments_clicked            (GtkWidget       *object,
                                        gpointer         user_data);

void
on_req_clicked                         (GtkWidget       *object,
                                        gpointer         user_data);

void
on_return1_clicked                     (GtkWidget       *object,
                                        gpointer         user_data);

void
on_saveaddition_clicked                (GtkWidget       *object,
                                        gpointer         user_data);

void
on_searchlist_clicked                  (GtkWidget       *object,
                                        gpointer         user_data);

void
on_return2_clicked                     (GtkWidget       *object,
                                        gpointer         user_data);

void
on_searchmodify_clicked                (GtkWidget       *object,
                                        gpointer         user_data);


void
on_return3_clicked                     (GtkWidget       *object,
                                        gpointer         user_data);

void
on_searchdelete_clicked                (GtkWidget       *object,
                                        gpointer         user_data);

void
on_return4_clicked                     (GtkWidget       *object,
                                        gpointer         user_data);

void
on_searchrestore_clicked               (GtkWidget       *object,
                                        gpointer         user_data);

void
on_return5_clicked                     (GtkWidget       *object,
                                        gpointer         user_data);

void
on_returnC_clicked                     (GtkWidget       *object,
                                        gpointer         user_data);

void
on_sendreq_clicked                     (GtkWidget       *object,
                                        gpointer         user_data);

void
on_searchcoach_clicked                 (GtkWidget       *object,
                                        gpointer         user_data);

void
on_modifiermember_clicked              (GtkWidget       *object,
                                        gpointer         user_data);

void
on_deletmember_clicked                 (GtkWidget       *object,
                                        gpointer         user_data);

void
on_restormember_clicked                (GtkWidget       *object,
                                        gpointer         user_data);

void
on_recherchemember_show                (GtkWidget       *widget,
                                        gpointer         user_data);

void
on_cancelsearchlist_clicked            (GtkWidget       *widget,
                                        gpointer         user_data);

void
on_modifylsit_clicked                  (GtkWidget       *widget,
                                        gpointer         user_data);

void
on_treeview1_cursor_changed            (GtkTreeView     *treeview,
                                        gpointer         user_data);

void
on_deletemember_show                   (GtkWidget       *widget,
                                        gpointer         user_data);

void
on_cancelsearchdel_clicked             (GtkWidget       *widget,
                                        gpointer         user_data);



void
on_restoremember_show                  (GtkWidget       *widget,
                                        gpointer         user_data);

void
on_cancelsearchrestore_clicked         (GtkWidget       *widget,
                                        gpointer         user_data);

void
on_checkbutton1_toggled                (GtkToggleButton *togglebutton,
                                        gpointer         user_data);

void
on_Login_show                          (GtkWidget       *widget,
                                        gpointer         user_data);


void
on_gender_toggled                      (GtkToggleButton *togglebutton,
                                        gpointer         user_data);

void
on_cancelsearReq_clicked               (GtkWidget       *widget,
                                        gpointer         user_data);

void
on_reqcoach_show                       (GtkWidget       *widget,
                                        gpointer         user_data);
void
on_Statsview_show                       (GtkWidget       *widget,
                                        gpointer         user_data);


void
on_return6_clicked                     (GtkWidget       *widget,
                                        gpointer         user_data);

void
on_foodbutton_clicked                  (GtkWidget       *object,
                                        gpointer         user_data);

void
on_searchfood_clicked                  (GtkWidget       *object,
                                        gpointer         user_data);

void
on_returnFood_clicked                  (GtkWidget       *object,
                                        gpointer         user_data);

void
on_button58_clicked                    (GtkWidget       *object,
                                        gpointer         user_data);
//Wissem

void
on_ADD_C_clicked                       (GtkButton       *button,
                                        gpointer         user_data);

void
on_RESET_C_clicked                     (GtkButton       *button,
                                        gpointer         user_data);



void
on_EDITE_L_clicked                     (GtkButton       *button,
                                        gpointer         user_data);

void
on_DELETE_L_clicked                    (GtkButton       *button,
                                        gpointer         user_data);

void
on_RETURN_L_clicked                    (GtkButton       *button,
                                        gpointer         user_data);

void
on_MAIN_ADD_clicked                    (GtkButton       *button,
                                        gpointer         user_data);

void
on_MAIN_STATS_clicked                  (GtkButton       *button,
                                        gpointer         user_data);

void
on_MAIN_LISTS_clicked                  (GtkButton       *button,
                                        gpointer         user_data);

void
on_MAIN_RETURN_clicked                 (GtkButton       *button,
                                        gpointer         user_data);

void
on_RETURN_C_clicked                    (GtkButton       *button,
                                        gpointer         user_data);
void afficher_centers(GtkWidget *treeview);
void on_VALIDE_EDIT_clicked(GtkButton *button, gpointer user_data);


void
on_button3WM_clicked                     (GtkButton       *button,
                                        gpointer         user_data);

void
on_treeview1WM_cursor_changed          (GtkTreeView     *treeview,
                                        gpointer         user_data);

void
on_treeview1WM_row_activated           (GtkTreeView     *treeview,
                                        GtkTreePath     *path,
                                        GtkTreeViewColumn *column,
                                        gpointer         user_data);



void
on_delcWM_clicked                      (GtkButton       *button,
                                        gpointer         user_data);

void
on_treeview2WM_cursor_changed          (GtkTreeView     *treeview,
                                        gpointer         user_data);

void
on_treeview2WM_row_activated           (GtkTreeView     *treeview,
                                        GtkTreePath     *path,
                                        GtkTreeViewColumn *column,
                                        gpointer         user_data);

void
on_RETURN_L2_clicked                   (GtkButton       *button,
                                        gpointer         user_data);

void
on_button7_clicked                     (GtkButton       *button,
                                        gpointer         user_data);

//Eya
void
on_statsbutton_clicked                 (GtkWidget       *object,
                                        gpointer         user_data);

void
on_restorebutton_clicked               (GtkWidget       *object,
                                        gpointer         user_data);

void
on_deletebutton_clicked                (GtkWidget       *object,
                                        gpointer         user_data);

void
on_listbutton_clicked                  (GtkWidget       *object,
                                        gpointer         user_data);

void
on_addbutton_clicked                   (GtkWidget       *object,
                                        gpointer         user_data);



void
on_return1EW_clicked                     (GtkWidget       *object,
                                        gpointer         user_data);

void
on_return2EW_clicked                     (GtkWidget       *object,
                                        gpointer         user_data);

void
on_return3EW_clicked                     (GtkWidget       *object,
                                        gpointer         user_data);

void
on_return4EW_clicked                     (GtkWidget       *object,
                                        gpointer         user_data);

void
on_return5EW_clicked                     (GtkWidget       *object,
                                        gpointer         user_data);

void
on_saveadd_clicked                     (GtkWidget       *object,
                                        gpointer         user_data);

void
on_ew_addcourse_show                   (GtkWidget       *widget,
                                        gpointer         user_data);

void
on_ewrecherchecours_show               (GtkWidget       *widget,
                                        gpointer         user_data);

void
on_searchlistEW_clicked                  (GtkWidget       *widget,
                                        gpointer         user_data);

void
on_canclsearchlist_clicked             (GtkButton       *button,
                                        gpointer         user_data);

void
on_ewtreeview1list_cursor_changed      (GtkTreeView     *treeview,
                                        gpointer         user_data);

void
on_modifylist_clicked                  (GtkWidget       *object,
                                        gpointer         user_data);

void
on_ewdeletecous_show                   (GtkWidget       *widget,
                                        gpointer         user_data);

void
on_searchdeleteEW_clicked                (GtkWidget       *object,
                                        gpointer         user_data);

void
on_cancelsearchdelEW_clicked             (GtkButton       *button,
                                        gpointer         user_data);

void
on_deleteC_clicked                     (GtkWidget       *object,
                                        gpointer         user_data);

void
on_ewrestorecours_show                 (GtkWidget       *widget,
                                        gpointer         user_data);

void
on_searchrestoreEW_clicked               (GtkWidget       *object,
                                        gpointer         user_data);

void
on_cancelsearchrest_clicked            (GtkButton       *button,
                                        gpointer         user_data);

void
on_restoreC_clicked                    (GtkWidget       *object,
                                        gpointer         user_data);

void
on_ewstats_show                        (GtkWidget       *widget,
                                        gpointer         user_data);

void
on_cancelsearchdelEW_clicked           (GtkButton       *button,
                                        gpointer         user_data);


void
on_button14_clicked                    (GtkWidget       *button,
                                        gpointer         user_data);

void
on_ewbutton6home_clicked               (GtkWidget       *button,
                                        gpointer         user_data);



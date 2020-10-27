#include <gtk/gtk.h>
#include "pc_terminal.h"
/*------------------------------------------------------------
 * GUI
 *------------------------------------------------------------
 */

static void
to_safe (GtkWidget *widget,
             gpointer   data)
{
  GtkWidget *label = data;
  mode = SAFE;
  send = true;
  gchar *str = g_strdup_printf ("switch to safe mode");
  gtk_label_set_text (GTK_LABEL (label), str);
  initialize();
  update_gui();
}

static void
to_panic (GtkWidget *widget,
             gpointer   data)
{

  GtkWidget *label = data;
  mode = PANIC;
  send = true;
  gchar *str = g_strdup_printf ("switch to panic mode");
  gtk_label_set_text (GTK_LABEL (label), str);
  update_gui();
}

static void
to_manual (GtkWidget *widget,
             gpointer   data)
{
  GtkWidget *label = data;
  if(mode!=SAFE)
  {
     gchar *str = g_strdup_printf ("please go back to safe mode first");
     gtk_label_set_text (GTK_LABEL (label), str);
     return;
  }
  else if(!check_joystick())
  {
     gchar *str = g_strdup_printf ("Please set joystick to origin");
     gtk_label_set_text (GTK_LABEL (label), str);
     return;
  }
  mode = MANUAL;
  send = true;
  gchar *str = g_strdup_printf ("switch to manual mode");
  gtk_label_set_text (GTK_LABEL (label), str);
  update_gui();
}

static void
to_calibration (GtkWidget *widget,
             gpointer   data)
{
  GtkWidget *label = data;
  if(mode!=SAFE)
  {
     gchar *str = g_strdup_printf ("please go back to safe mode first");
     gtk_label_set_text (GTK_LABEL (label), str);
     return;
  }
  else if(!check_joystick())
  {
     gchar *str = g_strdup_printf ("Please set joystick to origin");
     gtk_label_set_text (GTK_LABEL (label), str);
     return;
  }
  mode = CALIBRATION;
  send = true;
  gchar *str = g_strdup_printf ("switch to calibration mode");
  gtk_label_set_text (GTK_LABEL (label), str);
  update_gui();
}

static void
to_yaw (GtkWidget *widget,
             gpointer   data)
{
  GtkWidget *label = data;
  if(mode!=SAFE)
  {
     gchar *str = g_strdup_printf ("please go back to safe mode first");
     gtk_label_set_text (GTK_LABEL (label), str);
     return;
  }
  else if(!check_joystick())
  {
     gchar *str = g_strdup_printf ("Please set joystick to origin");
     gtk_label_set_text (GTK_LABEL (label), str);
     return;
  }

  mode = YAW;
  send = true;
  gchar *str = g_strdup_printf ("switch to yaw control mode");
  gtk_label_set_text (GTK_LABEL (label), str);
  update_gui();
}

static void
to_full (GtkWidget *widget,
             gpointer   data)
{
  GtkWidget *label = data;
  if(mode!=SAFE)
  {
     gchar *str = g_strdup_printf ("please go back to safe mode first");
     gtk_label_set_text (GTK_LABEL (label), str);
     return;
  }
  else if(!check_joystick())
  {
     gchar *str = g_strdup_printf ("Please set joystick to origin");
     gtk_label_set_text (GTK_LABEL (label), str);
     return;
  }
  mode = FULL;
  send = true;
  gchar *str = g_strdup_printf ("switch to full control mode");
  gtk_label_set_text (GTK_LABEL (label), str);
  update_gui();
}

static void
to_raw (GtkWidget *widget,
             gpointer   data)
{
  GtkWidget *label = data;
  if(raw == 0) raw = 1;
  else raw = 0;
  send = true;
  gchar *str;
  if(raw == 0) str = g_strdup_printf ("disable raw mode");
  else if(raw == 1) str = g_strdup_printf ("enable raw mode");
  gtk_label_set_text (GTK_LABEL (label), str);
  update_gui();
}

static void
to_height (GtkWidget *widget,
             gpointer   data)
{
  GtkWidget *label = data;
  if(height == 0) height = 1;
  else height = 0;
  send = true;
  gchar *str;
  if(height == 0) str = g_strdup_printf ("disable height control");
  else if(height == 1) str = g_strdup_printf ("enable height control");
  gtk_label_set_text (GTK_LABEL (label), str);
  update_gui();
}

static void
to_exit (GtkWidget *widget,
             gpointer   data)
{
  GtkWidget *label = data;
  mode = EXIT;
  send = true;
  gchar *str = g_strdup_printf ("exit");
  gtk_label_set_text (GTK_LABEL (label), str);
  update_gui();
}



/* This is the callback function. 
 * It is a handler function which reacts to the signal. 
 * In this case, it will notify the user the value of their scale as a label.
 */

static void
throttle_moved (GtkRange *range,
              gpointer  user_data)
{
   GtkWidget *label = user_data;

   /* Get the value of the range, and convert it into a string which will be
    * used as a new label for the horizontal scale.
    * %.0f - stands for a double that will have 0 decimal places.
    */

   if(mode == SAFE || mode == CALIBRATION) 
   {
	//printf("can't increase throttle in safe mode!\n");
        gchar *str = g_strdup_printf ("can't change values in safe/calibration mode!");
        gtk_label_set_text (GTK_LABEL (info), str);
	gtk_range_set_value(range,0);
   }
   else
   {
   	k_throttle = gtk_range_get_value (range);
	height = 0;
	send = true;
   }

   /* Note: Using g_strdup_printf returns a string that must be freed. 
    * (In which is done below)
    */
   gchar *str = g_strdup_printf ("throttle: %u", k_throttle);
   gtk_label_set_text (GTK_LABEL (label), str);

   g_free(str);
}

static void
roll_moved (GtkRange *range,
              gpointer  user_data)
{
   GtkWidget *label = user_data;

   /* Get the value of the range, and convert it into a string which will be
    * used as a new label for the horizontal scale.
    * %.0f - stands for a double that will have 0 decimal places.
    */
   if(mode == SAFE || mode == CALIBRATION) 
   {
	//printf("can't increase throttle in safe mode!\n");
        gchar *str = g_strdup_printf ("can't change values in safe/calibration mode!");
        gtk_label_set_text (GTK_LABEL (info), str);
	gtk_range_set_value(range,0);
   }
   else
  {
   	k_roll = gtk_range_get_value (range);
	send = true;
  }
   /* Note: Using g_strdup_printf returns a string that must be freed. 
    * (In which is done below)
    */
   gchar *str = g_strdup_printf ("roll: %d", k_roll);
   gtk_label_set_text (GTK_LABEL (label), str);

   g_free(str);
}

static void
pitch_moved (GtkRange *range,
              gpointer  user_data)
{
   GtkWidget *label = user_data;

   /* Get the value of the range, and convert it into a string which will be
    * used as a new label for the horizontal scale.
    * %.0f - stands for a double that will have 0 decimal places.
    */
   if(mode == SAFE || mode == CALIBRATION) 
   {
	//printf("can't increase throttle in safe mode!\n");
        gchar *str = g_strdup_printf ("can't change values in safe/calibration mode!");
        gtk_label_set_text (GTK_LABEL (info), str);
	gtk_range_set_value(range,0);
   }
   else
  {
   	k_pitch = gtk_range_get_value (range);
	send = true;
  }
   /* Note: Using g_strdup_printf returns a string that must be freed. 
    * (In which is done below)
    */
   gchar *str = g_strdup_printf ("pitch: %d", k_pitch);
   gtk_label_set_text (GTK_LABEL (label), str);

   g_free(str);
}

static void
yaw_moved (GtkRange *range,
              gpointer  user_data)
{
   GtkWidget *label = user_data;

   /* Get the value of the range, and convert it into a string which will be
    * used as a new label for the horizontal scale.
    * %.0f - stands for a double that will have 0 decimal places.
    */
   if(mode == SAFE || mode == CALIBRATION) 
   {
	//printf("can't increase throttle in safe mode!\n");
        gchar *str = g_strdup_printf ("can't change values in safe/calibration mode!");
        gtk_label_set_text (GTK_LABEL (info), str);
	gtk_range_set_value(range,0);
   }
   else
  {
   	k_yaw = gtk_range_get_value (range);
	send = true;
  }
   /* Note: Using g_strdup_printf returns a string that must be freed. 
    * (In which is done below)
    */
   gchar *str = g_strdup_printf ("yaw: %d", k_yaw);
   gtk_label_set_text (GTK_LABEL (label), str);

   g_free(str);
}

static void
P_moved (GtkRange *range,
              gpointer  user_data)
{
   GtkWidget *label = user_data;

   /* Get the value of the range, and convert it into a string which will be
    * used as a new label for the horizontal scale.
    * %.0f - stands for a double that will have 0 decimal places.
    */


   P = gtk_range_get_value (range);
   send = true;
   /* Note: Using g_strdup_printf returns a string that must be freed. 
    * (In which is done below)
    */
   gchar *str = g_strdup_printf ("P: %d", P);
   gtk_label_set_text (GTK_LABEL (label), str);

   g_free(str);
}

static void
P1_moved (GtkRange *range,
              gpointer  user_data)
{
   GtkWidget *label = user_data;

   /* Get the value of the range, and convert it into a string which will be
    * used as a new label for the horizontal scale.
    * %.0f - stands for a double that will have 0 decimal places.
    */
   P1 = gtk_range_get_value (range);
   send = true;
   /* Note: Using g_strdup_printf returns a string that must be freed. 
    * (In which is done below)
    */
   gchar *str = g_strdup_printf ("P1: %d", P1);
   gtk_label_set_text (GTK_LABEL (label), str);

   g_free(str);
}

static void
P2_moved (GtkRange *range,
              gpointer  user_data)
{
   GtkWidget *label = user_data;

   /* Get the value of the range, and convert it into a string which will be
    * used as a new label for the horizontal scale.
    * %.0f - stands for a double that will have 0 decimal places.
    */


   P2 = gtk_range_get_value (range);
   send = true;

   /* Note: Using g_strdup_printf returns a string that must be freed. 
    * (In which is done below)
    */
   gchar *str = g_strdup_printf ("P2: %d", P2);
   gtk_label_set_text (GTK_LABEL (label), str);

   g_free(str);
}

static void
Q_moved (GtkRange *range,
              gpointer  user_data)
{
   GtkWidget *label = user_data;

   /* Get the value of the range, and convert it into a string which will be
    * used as a new label for the horizontal scale.
    * %.0f - stands for a double that will have 0 decimal places.
    */
   Q = gtk_range_get_value (range);
   send = true;
   /* Note: Using g_strdup_printf returns a string that must be freed. 
    * (In which is done below)
    */
   gchar *str = g_strdup_printf ("Q: %d", Q);
   gtk_label_set_text (GTK_LABEL (label), str);

   g_free(str);
}

static void
activate (GtkApplication *app,
          gpointer        user_data)
{


  /* The Adjustment object represents a value 
   * which has an associated lower and upper bound.
   */


  /* Create a window with a title and a default size */
  window = gtk_application_window_new (app);
  gtk_window_set_title (GTK_WINDOW (window), "Drone Control Panel");
  gtk_window_set_default_size (GTK_WINDOW (window), 500, 300);
  gtk_container_set_border_width (GTK_CONTAINER (window), 5);

  //button_box = gtk_button_box_new (GTK_ORIENTATION_HORIZONTAL);
  //gtk_container_add (GTK_CONTAINER (window), button_box);

  info = gtk_label_new (g_strdup_printf ("program started"));
  cur_mode = gtk_label_new (g_strdup_printf ("current mode: SAFE"));

  b_safe = gtk_button_new_with_label ("SAFE");
  g_signal_connect (b_safe, "clicked", G_CALLBACK (to_safe), info);
  b_panic = gtk_button_new_with_label ("PANIC");
  g_signal_connect (b_panic, "clicked", G_CALLBACK (to_panic), info);
  b_manual = gtk_button_new_with_label ("MANUAL");
  g_signal_connect (b_manual, "clicked", G_CALLBACK (to_manual), info);
  b_calibration = gtk_button_new_with_label ("CALIBTRATION");
  g_signal_connect (b_calibration, "clicked", G_CALLBACK (to_calibration), info);
  b_yaw = gtk_button_new_with_label ("YAW");
  g_signal_connect (b_yaw, "clicked", G_CALLBACK (to_yaw), info);
  b_full = gtk_button_new_with_label ("FULL");
  g_signal_connect (b_full, "clicked", G_CALLBACK (to_full), info);
  b_raw = gtk_button_new_with_label ("RAW");
  g_signal_connect (b_raw, "clicked", G_CALLBACK (to_raw), info);
  b_height = gtk_button_new_with_label ("HEIGHT");
  g_signal_connect (b_height, "clicked", G_CALLBACK (to_height), info);
  b_exit = gtk_button_new_with_label ("EXIT");
  g_signal_connect (b_exit, "clicked", G_CALLBACK (to_exit), info);

  //g_signal_connect_swapped (b_safe, "clicked", G_CALLBACK (gtk_widget_destroy), window);
  //gtk_container_add (GTK_CONTAINER (button_box), b_safe);


  /* Two labels to be shown in the window */
  throttle_label = gtk_label_new (g_strdup_printf ("throttle: %u", k_throttle));
  roll_label = gtk_label_new (g_strdup_printf ("roll: %u", k_roll));
  pitch_label = gtk_label_new (g_strdup_printf ("pitch: %u", k_pitch));
  yaw_label = gtk_label_new (g_strdup_printf ("yaw: %u", k_yaw));
  P_label = gtk_label_new (g_strdup_printf ("P: %u", P));
  P1_label = gtk_label_new (g_strdup_printf ("P1: %u", P1));
  P2_label = gtk_label_new (g_strdup_printf ("P2: %u", P2));
  Q_label = gtk_label_new (g_strdup_printf ("Q: %u", Q));
  //vlabel = gtk_label_new ("Move the scale handle...");

   
  /* gtk_adjustment_new takes six parameters, three of which 
   * may be difficult to understand:
   * step increment- move the handle with the arrow keys on your keyboard to see.
   * page increment - move the handle by clicking away from it 
   * on the scale to see.
   * page size - not used here.
   */
  throttle_adjustment = gtk_adjustment_new (0, 0, 65535, 5, 10, 0);
  roll_adjustment = gtk_adjustment_new (0, -32768, 32767, 5, 10, 0);
  pitch_adjustment = gtk_adjustment_new (0, -32768, 32767, 5, 10, 0);
  yaw_adjustment = gtk_adjustment_new (0, -32768, 32767, 5, 10, 0);
  P_adjustment = gtk_adjustment_new (0, 0, 255, 1, 5, 0);
  P1_adjustment = gtk_adjustment_new (0, 0, 255, 1, 5, 0);
  P2_adjustment = gtk_adjustment_new (0, 0, 255, 1, 5, 0);
  Q_adjustment = gtk_adjustment_new (0, 0, 65535, 5, 10, 0);
  /* Create the Horizontal scale, making sure the 
   * digits used have no decimals.
   */
  throttle_scale = gtk_scale_new (GTK_ORIENTATION_HORIZONTAL, throttle_adjustment);
  roll_scale = gtk_scale_new (GTK_ORIENTATION_HORIZONTAL, roll_adjustment);
  pitch_scale = gtk_scale_new (GTK_ORIENTATION_HORIZONTAL, pitch_adjustment);
  yaw_scale = gtk_scale_new (GTK_ORIENTATION_HORIZONTAL, yaw_adjustment);
  P_scale = gtk_scale_new (GTK_ORIENTATION_HORIZONTAL, P_adjustment);
  P1_scale = gtk_scale_new (GTK_ORIENTATION_HORIZONTAL, P1_adjustment);
  P2_scale = gtk_scale_new (GTK_ORIENTATION_HORIZONTAL, P2_adjustment);
  Q_scale = gtk_scale_new (GTK_ORIENTATION_HORIZONTAL, Q_adjustment);

  gtk_scale_set_digits (GTK_SCALE (throttle_scale), 0); 
  gtk_scale_set_digits (GTK_SCALE (roll_scale), 0);
  gtk_scale_set_digits (GTK_SCALE (pitch_scale), 0);
  gtk_scale_set_digits (GTK_SCALE (yaw_scale), 0);
  gtk_scale_set_digits (GTK_SCALE (P_scale), 0);
  gtk_scale_set_digits (GTK_SCALE (P1_scale), 0);
  gtk_scale_set_digits (GTK_SCALE (P2_scale), 0);
  gtk_scale_set_digits (GTK_SCALE (Q_scale), 0);
  /* Allow it to expand horizontally (if there's space), and 
   * set the vertical alignment
   */
  gtk_widget_set_hexpand (throttle_scale, TRUE);
  gtk_widget_set_valign (throttle_scale, GTK_ALIGN_START);
  gtk_widget_set_hexpand (roll_scale, TRUE);
  gtk_widget_set_valign (roll_scale, GTK_ALIGN_START);
  gtk_widget_set_hexpand (pitch_scale, TRUE);
  gtk_widget_set_valign (pitch_scale, GTK_ALIGN_START);
  gtk_widget_set_hexpand (yaw_scale, TRUE);
  gtk_widget_set_valign (yaw_scale, GTK_ALIGN_START);
  gtk_widget_set_hexpand (P_scale, TRUE);
  gtk_widget_set_valign (P_scale, GTK_ALIGN_START);
  gtk_widget_set_hexpand (P1_scale, TRUE);
  gtk_widget_set_valign (P1_scale, GTK_ALIGN_START);
  gtk_widget_set_hexpand (P2_scale, TRUE);
  gtk_widget_set_valign (P2_scale, GTK_ALIGN_START);
  gtk_widget_set_hexpand (Q_scale, TRUE);
  gtk_widget_set_valign (Q_scale, GTK_ALIGN_START);
  /* Connecting the "value-changed" signal for the horizontal scale 
   * to the appropriate callback function. 
   * take note that GtkRange is part of GtkScale's Object Hierarchy.
   */
  g_signal_connect (throttle_scale, 
                    "value-changed", 
                    G_CALLBACK (throttle_moved), 
                    throttle_label);
  g_signal_connect (roll_scale, 
                    "value-changed", 
                    G_CALLBACK (roll_moved), 
                    roll_label);
  g_signal_connect (pitch_scale, 
                    "value-changed", 
                    G_CALLBACK (pitch_moved), 
                    pitch_label);
  g_signal_connect (yaw_scale, 
                    "value-changed", 
                    G_CALLBACK (yaw_moved), 
                    yaw_label);
  g_signal_connect (P_scale, 
                    "value-changed", 
                    G_CALLBACK (P_moved), 
                    P_label);
  g_signal_connect (P1_scale, 
                    "value-changed", 
                    G_CALLBACK (P1_moved), 
                    P1_label);
  g_signal_connect (P2_scale, 
                    "value-changed", 
                    G_CALLBACK (P2_moved), 
                    P2_label);
  g_signal_connect (Q_scale, 
                    "value-changed", 
                    G_CALLBACK (Q_moved), 
                    Q_label);

  /* Create a grid and arrange everything accordingly */
  grid = gtk_grid_new ();
  gtk_grid_set_column_spacing (GTK_GRID (grid), 10);
  //gtk_grid_set_column_homogeneous (GTK_GRID (grid), TRUE);
  gtk_grid_attach (GTK_GRID (grid), throttle_scale, 1, 0, 1, 1);
  gtk_grid_attach (GTK_GRID (grid), throttle_label, 2, 0, 1, 1);
  gtk_grid_attach (GTK_GRID (grid), roll_scale, 1, 1, 1, 1);
  gtk_grid_attach (GTK_GRID (grid), roll_label, 2, 1, 1, 1);
  gtk_grid_attach (GTK_GRID (grid), pitch_scale, 1, 2, 1, 1);
  gtk_grid_attach (GTK_GRID (grid), pitch_label, 2, 2, 1, 1);
  gtk_grid_attach (GTK_GRID (grid), yaw_scale, 1, 3, 1, 1);
  gtk_grid_attach (GTK_GRID (grid), yaw_label, 2, 3, 1, 1);
  gtk_grid_attach (GTK_GRID (grid), P_scale, 1, 4, 1, 1);
  gtk_grid_attach (GTK_GRID (grid), P_label, 2, 4, 1, 1);
  gtk_grid_attach (GTK_GRID (grid), P1_scale, 1, 5, 1, 1);
  gtk_grid_attach (GTK_GRID (grid), P1_label, 2, 5, 1, 1);
  gtk_grid_attach (GTK_GRID (grid), P2_scale, 1, 6, 1, 1);
  gtk_grid_attach (GTK_GRID (grid), P2_label, 2, 6, 1, 1);
  gtk_grid_attach (GTK_GRID (grid), Q_scale, 1, 7, 1, 1);
  gtk_grid_attach (GTK_GRID (grid), Q_label, 2, 7, 1, 1);

  gtk_grid_attach (GTK_GRID (grid), b_safe, 0, 0, 1, 1);
  gtk_grid_attach (GTK_GRID (grid), b_panic, 0, 1, 1, 1);
  gtk_grid_attach (GTK_GRID (grid), b_manual, 0, 2, 1, 1);
  gtk_grid_attach (GTK_GRID (grid), b_calibration, 0, 3, 1, 1);
  gtk_grid_attach (GTK_GRID (grid), b_yaw, 0, 4, 1, 1);
  gtk_grid_attach (GTK_GRID (grid), b_full, 0, 5, 1, 1);
  gtk_grid_attach (GTK_GRID (grid), b_raw, 0, 6, 1, 1);
  gtk_grid_attach (GTK_GRID (grid), b_height, 0, 7, 1, 1);
  gtk_grid_attach (GTK_GRID (grid), b_exit, 0, 8, 1, 1);

  gtk_grid_attach (GTK_GRID (grid), info, 1, 8, 2, 1);
  gtk_grid_attach (GTK_GRID (grid), cur_mode, 1, 9, 2, 1);
  gtk_container_add (GTK_CONTAINER (window), grid);

  gtk_widget_show_all (window);
}

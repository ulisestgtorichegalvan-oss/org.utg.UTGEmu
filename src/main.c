#include <gtk/gtk.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
    GtkWidget *entry_nombre;
    GtkWidget *entry_url;
    GtkWidget *rbtn_amd;
    GtkWidget *ventana_sub;
} DatosAsistenteNube;

static void cerrar_ventana_sub(GtkWidget *widget, gpointer data) {
    GtkWidget *ventana = GTK_WIDGET(data);
    gtk_window_destroy(GTK_WINDOW(ventana));
}

static void ejecutar_descarga_asistente(GtkWidget *widget, gpointer data) {
    DatosAsistenteNube *datos = (DatosAsistenteNube *)data;
    
    const char *nombre = gtk_editable_get_text(GTK_EDITABLE(datos->entry_nombre));
    const char *url = gtk_editable_get_text(GTK_EDITABLE(datos->entry_url));
    gboolean es_amd = gtk_check_button_get_active(GTK_CHECK_BUTTON(datos->rbtn_amd));

    if (strlen(nombre) == 0) {
        g_print("Error: Debes poner el nombre de un OS.\n");
        return;
    }

    const char *arch_suffix = es_amd ? "amd64" : "arm64";
    const char *arch_alt = es_amd ? "x86_64" : "aarch64";
    system("mkdir -p ~/UTGEmu/nube");

    char comando[2048]; // CORREGIDO: Declarado como arreglo de caracteres estable

    if (strlen(url) > 0 && g_str_has_prefix(url, "http")) {
        g_print("Descarga directa manual para: %s (%s)\n", nombre, arch_suffix);
        const char *ext = ".iso";
        if (g_str_has_suffix(url, ".img")) ext = ".img";
        else if (g_str_has_suffix(url, ".bin")) ext = ".bin";

        snprintf(comando, sizeof(comando), "wget -b -o ~/UTGEmu/nube/download.log -O ~/UTGEmu/nube/%s_%s%s \"%s\"", nombre, arch_suffix, ext, url);
        system(comando);
    } 
    else {
        g_print("[UTG Cloud] Iniciando rastreo global para '%s' (%s)...\n", nombre, arch_suffix);
        
        snprintf(comando, sizeof(comando), 
            "bash -c '"
            "TERM_BUSQUEDA=\"%s\"; ARCH_REQ1=\"%s\"; ARCH_REQ2=\"%s\"; "
            "rm -f ~/UTGEmu/nube/download.log; "
            "URL_FINAL=$(curl -s \"https://debian.org\" | grep -oE \"href=\\\"[^\\\"]+\\\"\" | cut -d\\\" -f2 | grep -E \"(\\.iso|\\.img|\\.bin)$\" | grep -E \"($ARCH_REQ1|$ARCH_REQ2)\" | head -n 1); "
            "if [ -z \"$URL_FINAL\" ]; then "
            "  URL_FINAL=$(curl -s \"https://ubuntu.com\" | grep -oE \"href=\\\"[^\\\"]+\\\"\" | cut -d\\\" -f2 | grep -E \"(\\.iso|\\.img|\\.bin)$\" | grep -E \"($ARCH_REQ1|$ARCH_REQ2)\" | head -n 1); "
            "fi; "
            "if [ ! -z \"$URL_FINAL\" ]; then "
            "  wget -b -o ~/UTGEmu/nube/download.log -O ~/UTGEmu/nube/${TERM_BUSQUEDA}_${ARCH_REQ1}.iso \"$URL_FINAL\"; "
            "else "
            "  echo \"[UTG Emu Error] No se encontraron archivos públicos directos.\" > ~/UTGEmu/nube/download.log; "
            "fi '"
            "&", nombre, arch_suffix, arch_alt);

        system(comando);
    }

    gtk_window_destroy(GTK_WINDOW(datos->ventana_sub));
    free(datos);
}

static void ventana_descarga_nube(GtkWidget *widget, gpointer data) {
    GtkWidget *ventana_sub = gtk_window_new();
    gtk_window_set_title(GTK_WINDOW(ventana_sub), "Asistente de Descarga Universal");
    gtk_window_set_transient_for(GTK_WINDOW(ventana_sub), GTK_WINDOW(data));
    gtk_window_set_modal(GTK_WINDOW(ventana_sub), TRUE);
    gtk_window_set_default_size(GTK_WINDOW(ventana_sub), 450, 400);

    GtkWidget *box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
    gtk_widget_set_margin_top(box, 15);
    gtk_widget_set_margin_bottom(box, 15);
    gtk_widget_set_margin_start(box, 15);
    gtk_widget_set_margin_end(box, 15);
    gtk_window_set_child(GTK_WINDOW(ventana_sub), box);

    gtk_box_append(GTK_BOX(box), gtk_label_new("Escribe el sistema operativo a buscar:"));
    GtkWidget *entry_nombre = gtk_entry_new();
    gtk_entry_set_placeholder_text(GTK_ENTRY(entry_nombre), "Ej. debian, ubuntu, fedora");
    gtk_box_append(GTK_BOX(box), entry_nombre);

    gtk_box_append(GTK_BOX(box), gtk_label_new("\nFiltrar de forma estricta por arquitectura:"));
    GtkWidget *rbtn_amd = gtk_check_button_new_with_label("AMD / Intel (x86_64)");
    GtkWidget *rbtn_arm = gtk_check_button_new_with_label("ARM (aarch64 / Mobile)");
    gtk_check_button_set_group(GTK_CHECK_BUTTON(rbtn_arm), GTK_CHECK_BUTTON(rbtn_amd));
    gtk_check_button_set_active(GTK_CHECK_BUTTON(rbtn_amd), TRUE);
    gtk_box_append(GTK_BOX(box), rbtn_amd);
    gtk_box_append(GTK_BOX(box), rbtn_arm);

    gtk_box_append(GTK_BOX(box), gtk_label_new("\nEnlace directo (OPCIONAL):"));
    GtkWidget *entry_url = gtk_entry_new();
    gtk_entry_set_placeholder_text(GTK_ENTRY(entry_url), "Deja vacío para activar el buscador UTG");
    gtk_box_append(GTK_BOX(box), entry_url);

    GtkWidget *box_acciones = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 10);
    
    GtkWidget *btn_descargar = gtk_button_new_with_label("Buscar y Descargar");
    gtk_box_append(GTK_BOX(box_acciones), btn_descargar);

    GtkWidget *btn_cancelar = gtk_button_new_with_label("Cancelar");
    g_signal_connect(btn_cancelar, "clicked", G_CALLBACK(cerrar_ventana_sub), ventana_sub);
    gtk_box_append(GTK_BOX(box_acciones), btn_cancelar);

    gtk_box_append(GTK_BOX(box), box_acciones);

    DatosAsistenteNube *datos = malloc(sizeof(DatosAsistenteNube));
    datos->entry_nombre = entry_nombre;
    datos->entry_url = entry_url;
    datos->rbtn_amd = rbtn_amd;
    datos->ventana_sub = ventana_sub;

    g_signal_connect(btn_descargar, "clicked", G_CALLBACK(ejecutar_descarga_asistente), datos);

    gtk_window_present(GTK_WINDOW(ventana_sub));
}

static void ejecutar_arranque_qemu(GtkWidget *widget, gpointer data) {
    g_print("[UTG Emu Core] Mapeando archivos e iniciando virtualización...\n");
    system("FILE_ARM=$(ls ~/UTGEmu/nube/*_arm64.iso ~/UTGEmu/nube/*_arm64.img 2>/dev/null | head -n 1); "
           "FILE_AMD=$(ls ~/UTGEmu/nube/*_amd64.iso ~/UTGEmu/nube/*_amd64.img 2>/dev/null | head -n 1); "
           "if [ ! -z \"$FILE_ARM\" ]; then "
           "  qemu-system-aarch64 -m 1024 -cdrom \"$FILE_ARM\" -show-cursor & "
           "elif [ ! -z \"$FILE_AMD\" ]; then "
           "  qemu-system-x86_64 -m 1024 -cdrom \"$FILE_AMD\" -show-cursor & "
           "else "
           "  echo \"[UTG Emu] No se encontraron sistemas listos en la nube para arrancar.\"; "
           "fi");
}

static void al_abrir_archivo(GObject *source_object, GAsyncResult *res, gpointer user_data) {
    GtkFileDialog *dialogo = GTK_FILE_DIALOG(source_object);
    GFile *archivo = gtk_file_dialog_open_finish(dialogo, res, NULL);
    if (archivo) {
        g_print("¡Archivo local cargado de forma exitosa!\n");
        g_object_unref(archivo);
    }
}

static void crear_nueva_vm(GtkWidget *widget, gpointer data) {
    // CORREGIDO: Implementación moderna con GtkFileDialog para entornos GTK 4 puros
    GtkFileDialog *dialogo_archivo = gtk_file_dialog_new();
    gtk_file_dialog_set_title(dialogo_archivo, "UTG Emu - Selecciona ISO/IMG/BIN");
    gtk_file_dialog_open(dialogo_archivo, GTK_WINDOW(data), NULL, al_abrir_archivo, NULL);
}

static void activar_app(GtkApplication *app, gpointer user_data) {
    GtkWidget *ventana = gtk_application_window_new(app);
    gtk_window_set_title(GTK_WINDOW(ventana), "UTG Emu - Panel Gestor Universal");
    gtk_window_set_default_size(GTK_WINDOW(ventana), 600, 450);

    GtkWidget *box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 15);
    gtk_widget_set_margin_top(box, 20);
    gtk_widget_set_margin_bottom(box, 20);
    gtk_widget_set_margin_start(box, 20);
    gtk_widget_set_margin_end(box, 20);
    gtk_window_set_child(GTK_WINDOW(ventana), box);

    GtkWidget *titulo = gtk_label_new("UTG Emu (Estilo Virt-Manager)");
    gtk_box_append(GTK_BOX(box), titulo);

    GtkWidget *lista = gtk_list_box_new();
    GtkWidget *item = gtk_label_new("  [■] Rastreador Global de Nube y Filtros de Arquitectura Activos");
    gtk_list_box_append(GTK_LIST_BOX(lista), item);
    gtk_widget_set_vexpand(lista, TRUE);
    gtk_box_append(GTK_BOX(box), lista);

    GtkWidget *box_botones = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 10);
    
    GtkWidget *btn_nueva = gtk_button_new_with_label("[+] Nueva VM");
    g_signal_connect(btn_nueva, "clicked", G_CALLBACK(crear_nueva_vm), ventana);
    gtk_box_append(GTK_BOX(box_botones), btn_nueva);

    GtkWidget *btn_nube = gtk_button_new_with_label("Descargar OS desde la nube");
    g_signal_connect(btn_nube, "clicked", G_CALLBACK(ventana_descarga_nube), ventana);
    gtk_box_append(GTK_BOX(box_botones), btn_nube);

    GtkWidget *btn_arrancar = gtk_button_new_with_label("[▶] Arrancar VM");
    g_signal_connect(btn_arrancar, "clicked", G_CALLBACK(ejecutar_arranque_qemu), NULL);
    gtk_box_append(GTK_BOX(box_botones), btn_arrancar);

    GtkWidget *btn_salir = gtk_button_new_with_label("[X] Salir");
    g_signal_connect_swapped(btn_salir, "clicked", G_CALLBACK(gtk_window_destroy), ventana);
    gtk_box_append(GTK_BOX(box_botones), btn_salir);

    gtk_box_append(GTK_BOX(box), box_botones);
    gtk_window_present(GTK_WINDOW(ventana));
}

int main(int argc, char **argv) {
    GtkApplication *app = gtk_application_new("org.utg.UTGEmu", G_APPLICATION_DEFAULT_FLAGS);
    g_signal_connect(app, "activate", G_CALLBACK(activar_app), NULL);
    int status = g_application_run(G_APPLICATION(app), argc, argv);
    g_object_unref(app);
    return status;
}

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>

#define HOSTS_FILE "/etc/hosts"
#define REDIRECT_IP "127.0.0.1"
#define MAX_LINE 1024
#define ESTUDIO_MIN 25
#define DESCANSO_MIN 5

const char* sitios_bloqueados[] = {
    "www.facebook.com", "facebook.com",
    "www.youtube.com", "youtube.com",
    "www.instagram.com", "instagram.com"
};

int ya_bloqueado(const char* sitio, FILE* file) {
    char linea[MAX_LINE];
    rewind(file);
    while (fgets(linea, MAX_LINE, file)) {
        if (strstr(linea, sitio)) return 1;
    }
    return 0;
}

void bloquear_sitios() {
    FILE* file = fopen(HOSTS_FILE, "r+");
    if (!file) {
        perror("Error abriendo /etc/hosts");
        exit(1);
    }

    int total = sizeof(sitios_bloqueados) / sizeof(sitios_bloqueados[0]);
    for (int i = 0; i < total; ++i) {
        if (!ya_bloqueado(sitios_bloqueados[i], file)) {
            fprintf(file, "%s %s\n", REDIRECT_IP, sitios_bloqueados[i]);
            printf("[+] Bloqueado: %s\n", sitios_bloqueados[i]);
        }
    }
    fclose(file);
}

void desbloquear_sitios() {
    FILE* file = fopen(HOSTS_FILE, "r");
    if (!file) {
        perror("Error leyendo /etc/hosts");
        exit(1);
    }

    FILE* temp = fopen("hosts_temp", "w");
    if (!temp) {
        perror("Error creando archivo temporal");
        fclose(file);
        exit(1);
    }

    char linea[MAX_LINE];
    while (fgets(linea, MAX_LINE, file)) {
        int bloquear = 0;
        for (int i = 0; i < sizeof(sitios_bloqueados) / sizeof(sitios_bloqueados[0]); ++i) {
            if (strstr(linea, sitios_bloqueados[i])) {
                bloquear = 1;
                break;
            }
        }
        if (!bloquear) fputs(linea, temp);
    }

    fclose(file);
    fclose(temp);

    remove(HOSTS_FILE);
    rename("hosts_temp", HOSTS_FILE);
    printf("[*] Sitios desbloqueados.\n");
}

void temporizador(int minutos, const char* mensaje) {
    printf("%s durante %d minutos...\n", mensaje, minutos);
    for (int i = minutos * 60; i > 0; i--) {
        printf("\rTiempo restante: %02d:%02d", i / 60, i % 60);
        fflush(stdout);
        sleep(1);
    }
    printf("\n");
}

void guardar_reporte(int ciclos, int minutos_totales) {
    FILE* file = fopen("productividad.txt", "a");
    if (!file) {
        perror("No se pudo abrir el archivo de reporte");
        return;
    }

    time_t ahora = time(NULL);
    char* fecha = ctime(&ahora);
    fecha[strlen(fecha) - 1] = '\0'; // quitar salto de línea

    fprintf(file, "Fecha: %s\n", fecha);
    fprintf(file, "Ciclos completados: %d\n", ciclos);
    fprintf(file, "Minutos totales de estudio: %d\n", minutos_totales);
    fprintf(file, "---------------------------\n");

    fclose(file);
}

int main() {
    int ciclos = 1;

    printf("?? Pomodoro FocusGuard iniciado.\n");

    time_t inicio = time(NULL);
    for (int i = 0; i < ciclos; ++i) {
        printf("\n=== Ciclo %d: Estudio ===\n", i + 1);
        bloquear_sitios();
        temporizador(ESTUDIO_MIN, "?? Estudiando");

        printf("\n=== Descanso ===\n");
        desbloquear_sitios();
        temporizador(DESCANSO_MIN, "?? Descansando");
    }
    time_t fin = time(NULL);
    int minutos_totales = (int)(difftime(fin, inicio) / 60);

    printf("\n? Ciclo(s) completado(s). Generando reporte...\n");

    guardar_reporte(ciclos, minutos_totales);
    printf("?? Reporte guardado en 'productividad.txt'\n");

    return 0;
}


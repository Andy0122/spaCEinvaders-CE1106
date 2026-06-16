/**
 * @file main.c
 * @brief Punto de entrada principal para el cliente C de spaCEinvaders.
 * Gestiona el ciclo de vida de la aplicación, la entrada del usuario y el renderizado.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "raylib.h"

#include "../include/constantes.h"
#include "../include/red.h"
#include "../include/gui.h"
#include "../include/structs.h"
#include "../include/control_uart.h"

/**
 * @brief Marca los proyectiles enemigos que salen del límite inferior de la pantalla.
 * @param lista Puntero a la lista de proyectiles a evaluar.
 */
static void marcar_balas_enemigas_fuera_de_pantalla(ListaBala* lista) {
    NodoBala* actual = lista->cabeza;
    while (actual != NULL) {
        if (actual->y > ALTO_PANTALLA) {
            actual->y = -999; 
        }
        actual = actual->siguiente;
    }
}

int main() {
    int opcion_rol = 1;
    int partida_id = 1;
    char cadena_handshake[64];

    // Interfaz de consola inicial
    printf("======================================\n");
    printf("     CLIENTE SPA-CE-INVADERS (C)      \n");
    printf("======================================\n");
    printf("1. Entrar como Jugador 1 (Partida 1)\n");
    printf("2. Entrar como Jugador 2 (Partida 2)\n");
    printf("3. Entrar como Espectador\n");
    printf("Seleccione una opcion: ");
    scanf("%d", &opcion_rol);

    if (opcion_rol == 3) {
        printf("Que partida desea observar? (1 o 2): ");
        scanf("%d", &partida_id);
        sprintf(cadena_handshake, "ESPECTADOR|%d\n", partida_id);
    } else if (opcion_rol == 1) {
        sprintf(cadena_handshake, "JUGADOR|1\n");
        partida_id = 1;
    } else {
        sprintf(cadena_handshake, "JUGADOR|2\n");
        partida_id = 2;
    }

    // Inicialización del modelo de datos
    Jugador arreglo_jugadores[2] = {
        {1, ANCHO_PANTALLA / 2, VIDAS_INICIALES, 0},
        {2, ANCHO_PANTALLA / 2, VIDAS_INICIALES, 0}
    };

    Bunker arreglo_bunkers[CANTIDAD_BUNKERS];
    int espacio_bunkers = ANCHO_PANTALLA / CANTIDAD_BUNKERS;
    for (int i = 0; i < CANTIDAD_BUNKERS; i++) {
        arreglo_bunkers[i].id               = i;
        arreglo_bunkers[i].x                = (espacio_bunkers * i) + (espacio_bunkers / 4);
        arreglo_bunkers[i].y                = ALTO_PANTALLA - 150;
        arreglo_bunkers[i].porcentaje_salud = 100;
    }

    Extraterrestre arreglo_aliens[MAX_ALIENS];
    for (int i = 0; i < MAX_ALIENS; i++) {
        arreglo_aliens[i].id     = -1;
        arreglo_aliens[i].estado = 0;
        arreglo_aliens[i].x      = 0;
        arreglo_aliens[i].y      = 0;
        arreglo_aliens[i].tipo   = 10;
    }

    Ovni mi_ovni = {0, 0, 0, 0, 0, 0};

    ListaBala lista_balas;
    lista_inicializar(&lista_balas);

    ListaBala lista_balas_enemigas;
    lista_inicializar(&lista_balas_enemigas);

    // Conexión TCP y vinculación de memoria
    vincular_punteros_red(arreglo_jugadores, arreglo_aliens, arreglo_bunkers,
                          &mi_ovni, &lista_balas, &lista_balas_enemigas);

    if (!inicializar_conexion(cadena_handshake)) {
        printf("[FATAL] No se pudo establecer la conexion de red.\n");
        lista_destruir(&lista_balas);
        lista_destruir(&lista_balas_enemigas);
        return EXIT_FAILURE;
    }

    // Configuración del motor gráfico y de hardware
    inicializar_gui();
    int mi_indice_jugador  = partida_id - 1;
    int control_uart_activo = 0;

    if (opcion_rol == 1) {
        if (!uart_inicializar(PUERTO_CONTROL)) {
            printf("[ADVERTENCIA] Controlador ESP8266 no detectado. Utilizando entrada estandar.\n");
        } else {
            control_uart_activo = 1;
        }
    }

    #define FRAMES_POR_TICK_SERVIDOR 12
    int contador_frames_bala_enemiga = 0;

    // Ciclo principal de actualización y renderizado
    while (!WindowShouldClose()) {

        // Procesamiento de entradas
        if (opcion_rol != 3) {
            // Entradas del Jugador 1 (Vía UART - ESP8266)
            if (opcion_rol == 1 && control_uart_activo) {
                char cmd;
                while (uart_leer_comando(&cmd)) {
                    if (cmd == 'L') {
                        arreglo_jugadores[mi_indice_jugador].posicion_x -= 5;
                        if (arreglo_jugadores[mi_indice_jugador].posicion_x < 0)
                            arreglo_jugadores[mi_indice_jugador].posicion_x = 0;
                        enviar_comando_servidor("JUGADOR|MOVER_IZQ\n");
                    } else if (cmd == 'R') {
                        arreglo_jugadores[mi_indice_jugador].posicion_x += 5;
                        if (arreglo_jugadores[mi_indice_jugador].posicion_x > (ANCHO_PANTALLA - ANCHO_CANON))
                            arreglo_jugadores[mi_indice_jugador].posicion_x = ANCHO_PANTALLA - ANCHO_CANON;
                        enviar_comando_servidor("JUGADOR|MOVER_DER\n");
                    } else if (cmd == 'S') {
                        printf("[INFO] Disparo recibido por UART\n");
                        enviar_comando_servidor("JUGADOR|DISPARAR\n");
                    }
                }
            }

            // Entradas del Jugador 2 (Vía Teclado estándar)
            if (opcion_rol == 2 || (opcion_rol == 1 && !control_uart_activo)) {
                if (IsKeyDown(KEY_LEFT)) {
                    arreglo_jugadores[mi_indice_jugador].posicion_x -= 5;
                    if (arreglo_jugadores[mi_indice_jugador].posicion_x < 0)
                        arreglo_jugadores[mi_indice_jugador].posicion_x = 0;
                    enviar_comando_servidor("JUGADOR|MOVER_IZQ\n");
                }
                if (IsKeyDown(KEY_RIGHT)) {
                    arreglo_jugadores[mi_indice_jugador].posicion_x += 5;
                    if (arreglo_jugadores[mi_indice_jugador].posicion_x > (ANCHO_PANTALLA - ANCHO_CANON))
                        arreglo_jugadores[mi_indice_jugador].posicion_x = ANCHO_PANTALLA - ANCHO_CANON;
                    
                    char cmd_der[64];
                    snprintf(cmd_der, sizeof(cmd_der), "JUGADOR|MOVER_DER|%d\n", ANCHO_PANTALLA - ANCHO_CANON);
                    enviar_comando_servidor(cmd_der);
                }
                if (IsKeyPressed(KEY_SPACE)) {
                    printf("[INFO] Disparo por teclado\n");
                    enviar_comando_servidor("JUGADOR|DISPARAR\n");
                }
            }
        }

        // Actualización de físicas locales
        lista_mover_balas(&lista_balas); 

        contador_frames_bala_enemiga++;
        if (contador_frames_bala_enemiga >= FRAMES_POR_TICK_SERVIDOR) {
            lista_mover_balas(&lista_balas_enemigas); 
            contador_frames_bala_enemiga = 0;
        }

        // Depuración de estructuras de memoria dinámicas
        lista_eliminar_fuera_de_pantalla(&lista_balas);
        marcar_balas_enemigas_fuera_de_pantalla(&lista_balas_enemigas);
        lista_eliminar_fuera_de_pantalla(&lista_balas_enemigas);

        // Renderizado
        BeginDrawing();
        ClearBackground(BLACK);

        dibujar_hud(arreglo_jugadores[mi_indice_jugador].puntuacion, arreglo_jugadores[mi_indice_jugador].vidas);
        dibujar_bunkers(arreglo_bunkers, CANTIDAD_BUNKERS);
        dibujar_matriz_aliens(arreglo_aliens, MAX_ALIENS);
        dibujar_ovni(&mi_ovni);

        if (arreglo_jugadores[mi_indice_jugador].vidas > 0) {
            dibujar_jugador(&arreglo_jugadores[mi_indice_jugador]);
        }

        dibujar_balas(&lista_balas);
        dibujar_balas_enemigas(&lista_balas_enemigas);

        EndDrawing();
    }

    // Liberación de recursos
    cerrar_gui();
    lista_destruir(&lista_balas);
    lista_destruir(&lista_balas_enemigas);

    if (opcion_rol == 1 && control_uart_activo) {
        uart_cerrar();
    }

    cerrar_conexion();
    printf("[INFO] Sistema cerrado correctamente.\n");
    return EXIT_SUCCESS;
}
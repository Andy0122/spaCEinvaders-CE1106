/**
 * @file main.c
 * @brief Punto de entrada principal para el cliente C de spaCEinvaders.
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

int main() {

    /* -------------------------------------------------------
     * 1. Menú Inicial y Selección de Rol
     * ------------------------------------------------------- */
    int opcion_rol = 1;
    int partida_id = 1;
    char cadena_handshake[64];

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
        partida_id = 2; // Asegura que el Jugador 2 vea la partida 2
    }

    /* -------------------------------------------------------
     * 2. Estructuras de datos locales
     * ------------------------------------------------------- */

    /* Jugadores */
    Jugador arreglo_jugadores[2] = {
        {1, ANCHO_PANTALLA / 2, VIDAS_INICIALES, 0},
        {2, ANCHO_PANTALLA / 2, VIDAS_INICIALES, 0}
    };

    /* Bunkers */
    Bunker arreglo_bunkers[CANTIDAD_BUNKERS];
    int espacio_bunkers = ANCHO_PANTALLA / CANTIDAD_BUNKERS;
    for (int i = 0; i < CANTIDAD_BUNKERS; i++) {
        arreglo_bunkers[i].id = i;
        arreglo_bunkers[i].x = (espacio_bunkers * i) + (espacio_bunkers / 4);
        arreglo_bunkers[i].y = ALTO_PANTALLA - 150;
        arreglo_bunkers[i].porcentaje_salud = 100;
    }

    Extraterrestre arreglo_aliens[MAX_ALIENS];
    for (int i = 0; i < MAX_ALIENS; i++) {
        arreglo_aliens[i].id = -1; // Slot vacío
        arreglo_aliens[i].estado = 0;
        arreglo_aliens[i].x = 0;
        arreglo_aliens[i].y = 0;
        arreglo_aliens[i].tipo = 10;
    }

    // Pre-poblar la cuadrícula visible inicial
    int indice = 0;
    for (int fila = 0; fila < FILAS_ALIENS; fila++) {
        for (int col = 0; col < COLUMNAS_ALIENS; col++) {
            arreglo_aliens[indice].id = indice;
            arreglo_aliens[indice].x = 50 + (col * (ANCHO_ALIEN + 15));
            arreglo_aliens[indice].y = 80 + (fila * (ALTO_ALIEN + 15));
            arreglo_aliens[indice].estado = 1;
            arreglo_aliens[indice].tipo = (fila == 0) ? 40 : (fila <= 2) ? 20 : 10;
            indice++;
        }
    }

    /* OVNI */
    Ovni mi_ovni = {0, 0, 0, 0, 0, 0};

    /*
     * Lista enlazada de balas activas del jugador.
     * Se inicializa vacía y se llena dinámicamente con cada disparo.
     */
    ListaBala lista_balas;
    lista_inicializar(&lista_balas);

    /* -------------------------------------------------------
     * 3. Vincular punteros de red y conectar
     * ------------------------------------------------------- */
    vincular_punteros_red(arreglo_jugadores, arreglo_aliens, arreglo_bunkers, &mi_ovni, &lista_balas);

    if (!inicializar_conexion(cadena_handshake)) {
        printf("[FATAL] Abortando ejecucion por fallo de red.\n");
        lista_destruir(&lista_balas);
        return EXIT_FAILURE;
    }

    /* -------------------------------------------------------
     * 4. Inicialización de GUI y UART
     * ------------------------------------------------------- */
    inicializar_gui();
    int mi_indice_jugador = partida_id - 1;
    int control_uart_activo = 0;

    if (opcion_rol == 1) {
        if (!uart_inicializar(PUERTO_CONTROL)) {
            printf("[ADVERTENCIA] ESP8266 no detectada. El Jugador 1 NO podra moverse hasta que se conecte el hardware.\n");
        } else {
            control_uart_activo = 1;
        }
    }

    /* -------------------------------------------------------
     * 5. Game Loop
     * ------------------------------------------------------- */
    while (!WindowShouldClose()) {

        /* ---SECCIÓN DE ENTRADA (Bloqueada para Espectadores) --- */
        if (opcion_rol != 3) {

            // Jugador 1: control físico por ESP8266
            if (opcion_rol == 1 && control_uart_activo) {
                char cmd;
                while (uart_leer_comando(&cmd)) {
                    if (cmd == 'L') {
                        arreglo_jugadores[mi_indice_jugador].posicion_x -= 5;
                        if (arreglo_jugadores[mi_indice_jugador].posicion_x < 0) arreglo_jugadores[mi_indice_jugador].posicion_x = 0;
                        enviar_comando_servidor("JUGADOR|MOVER_IZQ\n");
                    }
                    else if (cmd == 'R') {
                        arreglo_jugadores[mi_indice_jugador].posicion_x += 5;
                        if (arreglo_jugadores[mi_indice_jugador].posicion_x > (ANCHO_PANTALLA - ANCHO_CANON))
                            arreglo_jugadores[mi_indice_jugador].posicion_x = ANCHO_PANTALLA - ANCHO_CANON;
                        enviar_comando_servidor("JUGADOR|MOVER_DER\n");
                    }
                    else if (cmd == 'S') {
                        printf("[INFO] Disparo recibido por UART\n");
                        enviar_comando_servidor("JUGADOR|DISPARAR\n");
                    }
                }
            }

            // Jugador 2: teclado
            else if (opcion_rol == 2) {
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
                    enviar_comando_servidor("JUGADOR|MOVER_DER\n");
                }
                if (IsKeyPressed(KEY_SPACE)) {
                    printf("[INFO] Disparo por teclado\n");
                    enviar_comando_servidor("JUGADOR|DISPARAR\n");
                }
            }
        }

        /* --- ACTUALIZACIÓN DE FÍSICA LOCAL --- */
        lista_mover_balas(&lista_balas);
        lista_eliminar_fuera_de_pantalla(&lista_balas);

        /* --- RENDERIZADO --- */
        BeginDrawing();
        ClearBackground(BLACK);

        dibujar_hud(arreglo_jugadores[mi_indice_jugador].puntuacion,
                    arreglo_jugadores[mi_indice_jugador].vidas);
        dibujar_bunkers(arreglo_bunkers, CANTIDAD_BUNKERS);
        dibujar_matriz_aliens(arreglo_aliens, MAX_ALIENS);
        dibujar_ovni(&mi_ovni);
        dibujar_balas(&lista_balas);

        if (arreglo_jugadores[mi_indice_jugador].vidas > 0) {
            dibujar_jugador(&arreglo_jugadores[mi_indice_jugador]);
        }

        EndDrawing();
    }

    /* -------------------------------------------------------
     * 6. Limpieza y cierre
     * ------------------------------------------------------- */
    cerrar_gui();
    lista_destruir(&lista_balas);

    if (opcion_rol == 1 && control_uart_activo) {
        uart_cerrar();
    }

    // Cierre de conexión con Winsock y limpieza de sockets
    cerrar_conexion();
    printf("[INFO] Ejecucion finalizada correctamente.\n");
    return EXIT_SUCCESS;
}
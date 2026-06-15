/**
 * @file main.c
 * @brief Punto de entrada principal para el cliente C de spaCEinvaders.
 * Gestiona el menú inicial (A8), la conexión (A5) y el ciclo de renderizado (A3).
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
    // 1. Menú Inicial y Selección de Rol
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
    } else {
        sprintf(cadena_handshake, "JUGADOR|2\n");
        partida_id = 2; // Asegura que el Jugador 2 vea la partida 2
    }

    // 2. Creación de arreglos en memoria local
    Jugador arreglo_jugadores[2] = {
        {1, ANCHO_PANTALLA / 2, VIDAS_INICIALES, 0},
        {2, ANCHO_PANTALLA / 2, VIDAS_INICIALES, 0}
    };

    Bunker arreglo_bunkers[CANTIDAD_BUNKERS];
    int espacio_bunkers = ANCHO_PANTALLA / CANTIDAD_BUNKERS;
    for (int i = 0; i < CANTIDAD_BUNKERS; i++) {
        arreglo_bunkers[i].id = i;
        arreglo_bunkers[i].x = (espacio_bunkers * i) + (espacio_bunkers / 4);
        arreglo_bunkers[i].y = ALTO_PANTALLA - 150;
        arreglo_bunkers[i].porcentaje_salud = 100; 
    }

    int total_aliens = FILAS_ALIENS * COLUMNAS_ALIENS;
    Extraterrestre arreglo_aliens[FILAS_ALIENS * COLUMNAS_ALIENS];
    int indice = 0;
    
    for (int fila = 0; fila < FILAS_ALIENS; fila++) {
        for (int col = 0; col < COLUMNAS_ALIENS; col++) {
            arreglo_aliens[indice].id = indice;
            arreglo_aliens[indice].x = 50 + (col * (ANCHO_ALIEN + 15));
            arreglo_aliens[indice].y = 80 + (fila * (ALTO_ALIEN + 15));
            arreglo_aliens[indice].estado = 1; 
            
            if (fila == 0) arreglo_aliens[indice].tipo = 40; 
            else if (fila == 1 || fila == 2) arreglo_aliens[indice].tipo = 20; 
            else arreglo_aliens[indice].tipo = 10; 
            
            indice++;
        }
    }

    // Vinculamos la memoria para que la red actualice la pantalla en segundo plano
    vincular_punteros_red(arreglo_jugadores, arreglo_aliens, arreglo_bunkers);

    // 3. Inicialización de Red
    if (!inicializar_conexion(cadena_handshake)) {
        printf("[FATAL] Abortando ejecucion por fallo de red.\n");
        return EXIT_FAILURE;
    }

    // 4. Inicialización de la Interfaz Gráfica
    inicializar_gui();
    int mi_indice_jugador = partida_id - 1; 

    if (opcion_rol == 1) {
        if (!uart_inicializar(PUERTO_CONTROL)) {
            printf("[ADVERTENCIA] ESP8266 no detectada. El Jugador 1 NO podra moverse hasta que se conecte el hardware.\n");
        }
    }

    // 5. Ciclo de Vida Principal (Game Loop Gráfico)
    while (!WindowShouldClose()) {
        
        // --- SECCIÓN DE ENTRADA (Bloqueada para Espectadores) ---
        if (opcion_rol != 3) {
            
            if (opcion_rol == 1) {
                // Jugador 1: control físico por ESP8266 
                char cmd;
                while (uart_leer_comando(&cmd)) {
                    if (cmd == 'L') {
                        arreglo_jugadores[mi_indice_jugador].posicion_x -= 5;
                        if (arreglo_jugadores[mi_indice_jugador].posicion_x < 0) arreglo_jugadores[mi_indice_jugador].posicion_x = 0;
                        enviar_comando_servidor("JUGADOR|MOVER_IZQ\n");
                    }
                    else if (cmd == 'R') {
                        arreglo_jugadores[mi_indice_jugador].posicion_x += 5;
                        if (arreglo_jugadores[mi_indice_jugador].posicion_x > (ANCHO_PANTALLA - ANCHO_CANON)) arreglo_jugadores[mi_indice_jugador].posicion_x = ANCHO_PANTALLA - ANCHO_CANON;
                        enviar_comando_servidor("JUGADOR|MOVER_DER\n");
                    }
                    else if (cmd == 'S') {
                        printf("[INFO] Disparo recibido por UART\n");
                        // Aquí luego se conecta el disparo con la lógica del juego
                        enviar_comando_servidor("JUGADOR|DISPARAR\n");
                    }
                }
            }

            else if (opcion_rol == 2) {
                // Jugador 2: teclado
                if (IsKeyDown(KEY_LEFT)) {
                    arreglo_jugadores[mi_indice_jugador].posicion_x -= 5;
                    if (arreglo_jugadores[mi_indice_jugador].posicion_x < 0) {
                        arreglo_jugadores[mi_indice_jugador].posicion_x = 0;
                    }
                    enviar_comando_servidor("JUGADOR|MOVER_IZQ\n");
                }

                if (IsKeyDown(KEY_RIGHT)) {
                    arreglo_jugadores[mi_indice_jugador].posicion_x += 5;
                    if (arreglo_jugadores[mi_indice_jugador].posicion_x > (ANCHO_PANTALLA - ANCHO_CANON)) {
                        arreglo_jugadores[mi_indice_jugador].posicion_x = ANCHO_PANTALLA - ANCHO_CANON;
                    }
                    enviar_comando_servidor("JUGADOR|MOVER_DER\n");
                }

                if (IsKeyPressed(KEY_SPACE)) {
                    printf("[INFO] Disparo por teclado\n");
                    // Aquí luego se conecta el disparo con la lógica del juego
                    enviar_comando_servidor("JUGADOR|DISPARAR\n");
                }
            }
        }

        // --- SECCIÓN DE RENDERIZADO ---
        BeginDrawing();
        ClearBackground(BLACK);
        
        dibujar_hud(arreglo_jugadores[mi_indice_jugador].puntuacion, arreglo_jugadores[mi_indice_jugador].vidas);
        dibujar_bunkers(arreglo_bunkers, CANTIDAD_BUNKERS);
        dibujar_matriz_aliens(arreglo_aliens, total_aliens);
        
        if (arreglo_jugadores[mi_indice_jugador].vidas > 0) {
            dibujar_jugador(&arreglo_jugadores[mi_indice_jugador]);
        }
        
        EndDrawing();
    }

    // 6. Finalización limpia
    cerrar_gui();

    if (opcion_rol == 1) {
        uart_cerrar();
    }
    
    cerrar_conexion(); 
    
    printf("[INFO] Ejecucion finalizada correctamente.\n");

    return EXIT_SUCCESS;
}
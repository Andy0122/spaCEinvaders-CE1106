/**
 * @file main.c
 * @brief Punto de entrada principal para el cliente C de spaCEinvaders.
 *
 * Correcciones aplicadas:
 *  1. Colisiones bala-alien: en cada frame se recorre la lista de balas y el
 *     arreglo de aliens; si hay impacto se envía JUGADOR|ELIMINAR_ALIEN|id
 *     al servidor y se elimina la bala localmente.
 *  2. Pre-población eliminada: el arreglo de aliens ya NO se pre-puebla con
 *     IDs fijos (0..N). Todos los slots arrancan vacíos (id=-1) y se llenan
 *     únicamente con los mensajes ALIEN| que llegan del servidor, evitando
 *     el desajuste de IDs que impedía que parte de los aliens se movieran.
 *  3. Jugador 1 sin UART: si no hay ESP8266 conectada, el Jugador 1 también
 *     puede usar el teclado (flechas + espacio) para no quedar bloqueado.
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

/* -------------------------------------------------------
 * Detecta colisiones entre todas las balas activas y todos
 * los aliens vivos. Por cada impacto:
 *   - Marca la bala como fuera de pantalla (y = -999) para
 *     que lista_eliminar_fuera_de_pantalla() la retire.
 *   - Marca el alien como destruido localmente (estado = 0).
 *   - Envía JUGADOR|ELIMINAR_ALIEN|id al servidor.
 * ------------------------------------------------------- */
/*
static void verificar_colisiones_balas_aliens(ListaBala* lista,
                                               Extraterrestre aliens[],
                                               int total_aliens) {
    NodoBala* bala = lista->cabeza;
    while (bala != NULL) {
        // Bala ya marcada como eliminada este frame: saltar 
        if (bala->y < 0) {
            bala = bala->siguiente;
            continue;
        }

        for (int i = 0; i < total_aliens; i++) {
            if (aliens[i].id == -1 || aliens[i].estado == 0) continue;

           // Hitbox: bala centrada en (bala->x, bala->y), alien en rect (ax,ay,ANCHO_ALIEN,ALTO_ALIEN)
            int bx = bala->x;
            int by = bala->y;
            int ax = aliens[i].x;
            int ay = aliens[i].y;

            // Margen de 4px para que colisiones se sientan justas 
            if (bx >= ax - 4 && bx <= ax + ANCHO_ALIEN + 4 &&
                by >= ay - 4 && by <= ay + ALTO_ALIEN + 4) {

                // Destruir alien localmente 
                aliens[i].estado = 0;

                // Notificar al servidor 
                char cmd[64];
                snprintf(cmd, sizeof(cmd), "JUGADOR|ELIMINAR_ALIEN|%d\n", aliens[i].id);
                enviar_comando_servidor(cmd);

                // Marcar bala para eliminación 
                bala->y = -999;

                printf("[COLISION] Bala impacto alien id=%d\n", aliens[i].id);
                break; // una bala solo mata un alien 
            }
        }
        bala = bala->siguiente;
    }
}
*/

/* -------------------------------------------------------
 * Detecta colisiones entre todas las balas activas y el OVNI,
 * si está activo. Por cada impacto:
 *   - Marca la bala como fuera de pantalla (y = -999).
 *   - Apaga el OVNI localmente (activo = 0).
 *   - Envía JUGADOR|ELIMINAR_OVNI|id al servidor.
 * ------------------------------------------------------- */
/*
static void verificar_colisiones_balas_ovni(ListaBala* lista, Ovni* ovni) {
    if (ovni == NULL || ovni->activo != 1) return;

    NodoBala* bala = lista->cabeza;
    while (bala != NULL) {
        if (bala->y < 0) {
            bala = bala->siguiente;
            continue;
        }

        int bx = bala->x;
        int by = bala->y;
        int ox = ovni->x;
        int oy = ovni->y;

        // OVNI mide ANCHO_ALIEN*2 x ALTO_ALIEN (ver gui.c) 
        if (bx >= ox - 4 && bx <= ox + (ANCHO_ALIEN * 2) + 4 &&
            by >= oy - 4 && by <= oy + ALTO_ALIEN + 4) {

            ovni->activo = 0;

            char cmd[64];
            snprintf(cmd, sizeof(cmd), "JUGADOR|ELIMINAR_OVNI|%d\n", ovni->id);
            enviar_comando_servidor(cmd);

            bala->y = -999;

            printf("[COLISION] Bala impacto OVNI id=%d\n", ovni->id);
            break; // solo un OVNI activo a la vez
        }
        bala = bala->siguiente;
    }
}
*/

/* -------------------------------------------------------
 * Elimina balas enemigas que ya salieron por la parte INFERIOR
 * de la pantalla (y > ALTO_PANTALLA). Las balas del jugador suben y
 * lista_eliminar_fuera_de_pantalla() ya cubre su caso (y < 0); las balas
 * enemigas bajan, así que necesitan este chequeo complementario.
 * Reutiliza el campo y < 0 como marca de "eliminar este frame" para que
 * lista_eliminar_fuera_de_pantalla() haga el free() real después.
 * ------------------------------------------------------- */
static void marcar_balas_enemigas_fuera_de_pantalla(ListaBala* lista) {
    NodoBala* actual = lista->cabeza;
    while (actual != NULL) {
        if (actual->y > ALTO_PANTALLA) {
            actual->y = -999; /* lista_eliminar_fuera_de_pantalla() la limpiará */
        }
        actual = actual->siguiente;
    }
}

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
        partida_id = 2;
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
        arreglo_bunkers[i].id               = i;
        arreglo_bunkers[i].x                = (espacio_bunkers * i) + (espacio_bunkers / 4);
        arreglo_bunkers[i].y                = ALTO_PANTALLA - 150;
        arreglo_bunkers[i].porcentaje_salud = 100;
    }

    /*
     * FIX 2: Todos los slots vacíos (id = -1).
     * Las posiciones y tipos llegan exclusivamente del servidor
     * mediante mensajes ALIEN|id|x|y|estado, así los IDs siempre
     * coinciden y todos los aliens se mueven correctamente.
     */
    Extraterrestre arreglo_aliens[MAX_ALIENS];
    for (int i = 0; i < MAX_ALIENS; i++) {
        arreglo_aliens[i].id     = -1;
        arreglo_aliens[i].estado = 0;
        arreglo_aliens[i].x      = 0;
        arreglo_aliens[i].y      = 0;
        arreglo_aliens[i].tipo   = 10;
    }

    /* OVNI */
    Ovni mi_ovni = {0, 0, 0, 0, 0, 0};

    /* Lista enlazada de balas activas del jugador (suben) */
    ListaBala lista_balas;
    lista_inicializar(&lista_balas);

    /*
     * Lista enlazada de balas de los ALIENS (bajan). Reutiliza la misma
     * estructura ListaBala/NodoBala con velocidad negativa (ver red.c).
     * Antes el servidor manejaba estas balas internamente sin avisar al
     * cliente, así que el daño a bunkers/jugador parecía no tener causa
     * visible: ahora se dibujan en rojo descendiendo en pantalla.
     */
    ListaBala lista_balas_enemigas;
    lista_inicializar(&lista_balas_enemigas);

    /* -------------------------------------------------------
     * 3. Vincular punteros de red y conectar
     * ------------------------------------------------------- */
    vincular_punteros_red(arreglo_jugadores, arreglo_aliens, arreglo_bunkers,
                          &mi_ovni, &lista_balas, &lista_balas_enemigas);

    if (!inicializar_conexion(cadena_handshake)) {
        printf("[FATAL] Abortando ejecucion por fallo de red.\n");
        lista_destruir(&lista_balas);
        lista_destruir(&lista_balas_enemigas);
        return EXIT_FAILURE;
    }

    /* -------------------------------------------------------
     * 4. Inicialización de GUI y UART
     * ------------------------------------------------------- */
    inicializar_gui();
    int mi_indice_jugador  = partida_id - 1;
    int control_uart_activo = 0;

    if (opcion_rol == 1) {
        if (!uart_inicializar(PUERTO_CONTROL)) {
            /* FIX 3: sin UART el Jugador 1 usa teclado igualmente */
            printf("[ADVERTENCIA] ESP8266 no detectada. Usando teclado como fallback.\n");
        } else {
            control_uart_activo = 1;
        }
    }

    /*
     * Contador de frames para sincronizar el movimiento de las balas
     * ENEMIGAS con el tick real del servidor (TICK_MS=200ms en Juego.java).
     * A FPS_OBJETIVO=60, 200ms equivalen a exactamente 12 frames. El
     * servidor decide colisiones cada 200ms moviendo la bala 8px de una
     * sola vez; si el cliente la mueve 8px CADA FRAME (60 veces/seg en vez
     * de 5 veces/seg), la bala avanza 12x más rápido visualmente que la
     * posición que el servidor usa para resolver impactos: en pantalla
     * parece "atravesar" bunkers/jugador, cuando en realidad la colisión
     * real del servidor ocurre con la bala en una posición muy distinta
     * (mucho más arriba) a la que el ojo ve en ese instante.
     */
    #define FRAMES_POR_TICK_SERVIDOR 12
    int contador_frames_bala_enemiga = 0;

    /* -------------------------------------------------------
     * 5. Game Loop
     * ------------------------------------------------------- */
    while (!WindowShouldClose()) {

        /* --- SECCIÓN DE ENTRADA (bloqueada para Espectadores) --- */
        if (opcion_rol != 3) {

            /* Jugador 1 con ESP8266 */
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

            /*
             * Teclado: Jugador 2 siempre, y Jugador 1 cuando no hay UART.
             * FIX 3: opcion_rol == 1 sin UART también entra aquí.
             */
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

        /* --- ACTUALIZACIÓN DE FÍSICA LOCAL --- */
        lista_mover_balas(&lista_balas); /* balas del jugador: el cliente resuelve sus colisiones, se mueven cada frame */

        /* Balas ENEMIGAS: avanzan solo cada FRAMES_POR_TICK_SERVIDOR frames,
         * para que su velocidad visual coincida con la velocidad real que
         * el servidor usa al resolver colisiones contra bunkers/jugador. */
        contador_frames_bala_enemiga++;
        if (contador_frames_bala_enemiga >= FRAMES_POR_TICK_SERVIDOR) {
            lista_mover_balas(&lista_balas_enemigas); /* velocidad negativa: baja en pantalla */
            contador_frames_bala_enemiga = 0;
        }

        /* FIX 1: detectar colisiones ANTES de eliminar balas fuera de pantalla 
        if (opcion_rol != 3) {
            verificar_colisiones_balas_aliens(&lista_balas, arreglo_aliens, MAX_ALIENS);
            verificar_colisiones_balas_ovni(&lista_balas, &mi_ovni);
        }*/

        lista_eliminar_fuera_de_pantalla(&lista_balas);
        marcar_balas_enemigas_fuera_de_pantalla(&lista_balas_enemigas);
        lista_eliminar_fuera_de_pantalla(&lista_balas_enemigas);

        /* --- RENDERIZADO --- */
        BeginDrawing();
        ClearBackground(BLACK);

        dibujar_hud(arreglo_jugadores[mi_indice_jugador].puntuacion,
                    arreglo_jugadores[mi_indice_jugador].vidas);
        dibujar_bunkers(arreglo_bunkers, CANTIDAD_BUNKERS);
        dibujar_matriz_aliens(arreglo_aliens, MAX_ALIENS);
        dibujar_ovni(&mi_ovni);

        if (arreglo_jugadores[mi_indice_jugador].vidas > 0) {
            dibujar_jugador(&arreglo_jugadores[mi_indice_jugador]);
        }

        /* Las balas se dibujan AL FINAL, encima de todo lo demás, para que
         * el impacto contra bunkers o el jugador sea siempre visible en el
         * frame exacto en que ocurre (antes el cañón/bunker se pintaban
         * encima de la bala, tapando el momento del choque). */
        dibujar_balas(&lista_balas);
        dibujar_balas_enemigas(&lista_balas_enemigas);

        EndDrawing();
    }

    /* -------------------------------------------------------
     * 6. Limpieza y cierre
     * ------------------------------------------------------- */
    cerrar_gui();
    lista_destruir(&lista_balas);
    lista_destruir(&lista_balas_enemigas);

    if (opcion_rol == 1 && control_uart_activo) {
        uart_cerrar();
    }

    cerrar_conexion();
    printf("[INFO] Ejecucion finalizada correctamente.\n");
    return EXIT_SUCCESS;
}
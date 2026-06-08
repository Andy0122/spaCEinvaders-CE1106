/**
 * @file main.c
 * @brief Punto de entrada principal para el cliente C de spaCEinvaders.
 * Orquesta la inicialización de módulos, datos de prueba y el ciclo principal de juego.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "raylib.h"

#include "../include/constantes.h"
#include "../include/red.h"
#include "../include/gui.h"     
#include "../include/structs.h" 

int main() {
    printf("======================================\n");
    printf("     CLIENTE SPA-CE-INVADERS (C)      \n");
    printf("======================================\n");

    // 1. Inicialización de la comunicación (Comentado temporalmente para pruebas locales)
    /*
    if (!inicializar_conexion()) {
        printf("[FATAL] Abortando ejecucion por fallo de red.\n");
        return EXIT_FAILURE;
    }
    */

    // 2. Inicialización de la Interfaz Gráfica
    inicializar_gui();

    // --- CREACIÓN DE DATOS DUMMY (MAQUETA VISUAL) ---
    // A reemplazar en la Actividad A5 con los datos que lleguen del servidor Java
    Jugador jugador_local = {1, ANCHO_PANTALLA / 2, VIDAS_INICIALES, 0};

    Bunker bunkers[CANTIDAD_BUNKERS];
    int espacio_bunkers = ANCHO_PANTALLA / CANTIDAD_BUNKERS;
    for (int i = 0; i < CANTIDAD_BUNKERS; i++) {
        bunkers[i].id = i;
        bunkers[i].x = (espacio_bunkers * i) + (espacio_bunkers / 4);
        bunkers[i].y = ALTO_PANTALLA - 150;
        bunkers[i].porcentaje_salud = 100; 
    }

    int total_aliens = FILAS_ALIENS * COLUMNAS_ALIENS;
    Extraterrestre aliens[FILAS_ALIENS * COLUMNAS_ALIENS];
    int indice = 0;
    
    for (int fila = 0; fila < FILAS_ALIENS; fila++) {
        for (int col = 0; col < COLUMNAS_ALIENS; col++) {
            aliens[indice].id = indice;
            aliens[indice].x = 50 + (col * (ANCHO_ALIEN + 15));
            aliens[indice].y = 80 + (fila * (ALTO_ALIEN + 15));
            aliens[indice].estado = 1; 
            
            if (fila == 0) aliens[indice].tipo = 40; 
            else if (fila == 1 || fila == 2) aliens[indice].tipo = 20; 
            else aliens[indice].tipo = 10; 
            
            indice++;
        }
    }
    // ------------------------------------------------

    // 3. Ciclo de Vida Principal (Game Loop Gráfico)
    while (!WindowShouldClose()) {
        
        // --- SECCIÓN DE ENTRADA (Manejo temporal por teclado) ---
        // Nota: Esta sección será reemplazada por la lectura UART de la ESP32 (Actividad A7)
        if (IsKeyDown(KEY_LEFT)) {
            jugador_local.posicion_x -= 5;
            if (jugador_local.posicion_x < 0) {
                jugador_local.posicion_x = 0;
            }
        }
        if (IsKeyDown(KEY_RIGHT)) {
            jugador_local.posicion_x += 5;
            if (jugador_local.posicion_x > (ANCHO_PANTALLA - ANCHO_CANON)) {
                jugador_local.posicion_x = ANCHO_PANTALLA - ANCHO_CANON;
            }
        }
        if (IsKeyPressed(KEY_SPACE)) {
            // Lógica de disparo temporal
        }

        // --- SECCIÓN DE RENDERIZADO ---
        BeginDrawing();
        ClearBackground(BLACK);
        
        dibujar_hud(jugador_local.puntuacion, jugador_local.vidas);
        dibujar_jugador(&jugador_local);
        dibujar_bunkers(bunkers, CANTIDAD_BUNKERS);
        dibujar_matriz_aliens(aliens, total_aliens);
        
        EndDrawing();
    }

    // 4. Finalización limpia y liberación de memoria/recursos
    cerrar_gui();
    // cerrar_conexion(); // Descomentar cuando se active la red
    printf("[INFO] Ejecucion finalizada correctamente.\n");

    return EXIT_SUCCESS;
}
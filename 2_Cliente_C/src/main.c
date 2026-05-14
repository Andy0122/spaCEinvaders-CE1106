/**
 * @file main.c
 * @brief Punto de entrada principal para el cliente C de spaCEinvaders.
 * Orquesta la inicialización de módulos (Red, GUI, Estructuras) y el ciclo principal de juego.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../include/constantes.h"
#include "../include/red.h"
// #include "../include/gui.h" // Se habilitará cuando desarrolles la GUI
// #include "../include/structs.h"

int main() {
    printf("======================================\n");
    printf("     CLIENTE SPA-CE-INVADERS (C)      \n");
    printf("======================================\n");

    // 1. Inicialización de la comunicación
    if (!inicializar_conexion()) {
        printf("[FATAL] Abortando ejecucion por fallo de red.\n");
        return EXIT_FAILURE;
    }

    // 2. Ciclo de Vida Principal (Game Loop de terminal temporal)
    char buffer_entrada[TAMANO_BUFFER];
    
    // Este ciclo simulará temporalmente los inputs del usuario
    while (1) {
        // Obtenemos input de consola (hasta que integres la GUI y los botones del Pico)
        if (fgets(buffer_entrada, TAMANO_BUFFER, stdin) != NULL) {
            
            // Si el usuario escribe "salir", rompemos el ciclo
            if (strncmp(buffer_entrada, "salir", 5) == 0) {
                break;
            }

            // Despachamos el input hacia el servidor Java
            enviar_comando_servidor(buffer_entrada);
        }
    }

    // 3. Finalización limpia y liberación de memoria/recursos
    cerrar_conexion();
    printf("[INFO] Ejecucion finalizada correctamente.\n");

    return EXIT_SUCCESS;
}
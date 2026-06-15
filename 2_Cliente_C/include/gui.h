#ifndef GUI_H
#define GUI_H

/**
 * @file gui.h
 * @brief Interfaz para el manejo de la interfaz gráfica del cliente.
 * Encapsula las funciones de renderizado utilizando la biblioteca Raylib.
 */

#include "structs.h"
#include "constantes.h"

void inicializar_gui();
void dibujar_jugador(Jugador *j);
void dibujar_hud(int puntuacion, int vidas);
void dibujar_bunkers(Bunker bunkers[], int cantidad);
void dibujar_matriz_aliens(Extraterrestre aliens[], int total_aliens);
void dibujar_ovni(Ovni* o);
void dibujar_balas(ListaBala* lista); 
void cerrar_gui();

#endif // GUI_H
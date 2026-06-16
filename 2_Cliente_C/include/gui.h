#ifndef GUI_H
#define GUI_H

/**
 * @file gui.h
 * @brief Interfaz del motor de renderizado gráfico.
 */

#include "structs.h"
#include "constantes.h"

/**
 * @brief Inicializa el contexto de ventana y carga los recursos en VRAM.
 */
void inicializar_gui();

/**
 * @brief Renderiza la entidad principal del jugador.
 * @param j Referencia a la entidad a dibujar.
 */
void dibujar_jugador(Jugador *j);

/**
 * @brief Renderiza la interfaz de información en pantalla (Heads-Up Display).
 * @param puntuacion Valor numérico de los puntos actuales.
 * @param vidas Valor numérico de las vidas restantes.
 */
void dibujar_hud(int puntuacion, int vidas);

/**
 * @brief Renderiza el estado actual de las defensas terrestres.
 * @param bunkers Arreglo de entidades defensivas.
 * @param cantidad Tamaño del arreglo.
 */
void dibujar_bunkers(Bunker bunkers[], int cantidad);

/**
 * @brief Renderiza la cuadrícula de entidades hostiles.
 * @param aliens Arreglo de entidades enemigas.
 * @param total_aliens Tamaño del arreglo.
 */
void dibujar_matriz_aliens(Extraterrestre aliens[], int total_aliens);

/**
 * @brief Renderiza la entidad de bonificación si se encuentra activa.
 * @param o Referencia a la entidad OVNI.
 */
void dibujar_ovni(Ovni* o);

/**
 * @brief Renderiza los proyectiles activos aliados.
 * @param lista Estructura dinámica de proyectiles.
 */
void dibujar_balas(ListaBala* lista);

/**
 * @brief Renderiza los proyectiles activos hostiles.
 * @param lista Estructura dinámica de proyectiles.
 */
void dibujar_balas_enemigas(ListaBala* lista);

/**
 * @brief Descarga los recursos de la VRAM y destruye el contexto de ventana.
 */
void cerrar_gui();

#endif // GUI_H
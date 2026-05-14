#ifndef STRUCTS_H
#define STRUCTS_H

/**
 * @file structs.h
 * @brief Definición de las estructuras de datos (Modelo) del cliente.
 * Representa la información que será sincronizada desde el servidor Java.
 */

/**
 * @struct Jugador
 * @brief Modela el cañón controlado por el usuario.
 */
typedef struct {
    int id_jugador;    /**< Identificador único en red (Socket ID) */
    int posicion_x;    /**< Posición en el eje X de la pantalla */
    int vidas;         /**< Contador de vidas (Inicia en 3 según PDF) */
    int puntuacion;    /**< Puntaje acumulado */
} Jugador;

/**
 * @struct Extraterrestre
 * @brief Modela a los enemigos (Calamar, Cangrejo, Pulpo).
 */
typedef struct {
    int id;            /**< Identificador único del alienígena */
    int x;             /**< Posición actual en el eje X */
    int y;             /**< Posición actual en el eje Y */
    int puntos;        /**< Puntos otorgados: 10, 20 o 40 */
} Extraterrestre;

#endif // STRUCTS_H
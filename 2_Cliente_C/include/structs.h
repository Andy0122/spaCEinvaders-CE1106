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
    int tipo;          /**< 10 (Calamar), 20 (Cangrejo) o 40 (Pulpo) */
    int estado;        /**< 1 = Vivo, 0 = Muerto */
} Extraterrestre;

/**
 * @struct Bunker
 * @brief Modela los escudos de protección terrestre.
 * Representa las barreras físicas que se degradan con los disparos.
 */
typedef struct {
    int id;                 /**< Identificador único del escudo */
    int x;                  /**< Posición actual en el eje X de la pantalla */
    int y;                  /**< Posición actual en el eje Y de la pantalla */
    int porcentaje_salud;   /**< Salud restante del escudo: 100, 70, 40 o 0 */
} Bunker;

/**
 * @brief Estructura que representa la entidad del OVNI.
 * Almacena su posición, recompensa y su estado de visibilidad en pantalla.
 */
typedef struct {
    int id;
    int x;
    int y;
    int velocidad;
    int puntosExtra;
    int activo; // 1 = visible en pantalla, 0 = oculto/destruido
} Ovni;

#endif // STRUCTS_H
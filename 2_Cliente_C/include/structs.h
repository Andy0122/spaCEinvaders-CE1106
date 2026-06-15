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

/* ==========================================
 * Nodo de la Lista Enlazada
 * ==========================================
 * Cada nodo representa una bala activa en pantalla.
 * Campos:
 *   x         - posición horizontal actual de la bala
 *   y         - posición vertical actual (decrece cada frame: sube hacia arriba)
 *   velocidad - píxeles que avanza por frame (siempre positivo, se resta a y)
 *   siguiente - puntero al siguiente nodo, NULL si es el último
 */
typedef struct NodoBala {
    int x;
    int y;
    int velocidad;
    struct NodoBala* siguiente;
} NodoBala;

/**
 * @brief Lista enlazada de balas activas del jugador.
 * Campos:
 *   cabeza    - puntero al primer nodo, NULL si la lista está vacía
 *   cantidad  - número de balas activas actualmente en la lista
 */
typedef struct {
    NodoBala* cabeza;
    int cantidad;
} ListaBala;

/* ==========================================
 * Interfaz pública
 * ========================================== */

/**
 * @brief Inicializa la lista: cabeza en NULL y cantidad en 0.
 * Debe llamarse una vez al inicio del programa antes de cualquier otra operación.
 */
void lista_inicializar(ListaBala* lista);

/**
 * @brief Inserta una bala nueva al frente de la lista.
 * @param lista    Puntero a la lista de balas.
 * @param x        Posición horizontal inicial de la bala.
 * @param y        Posición vertical inicial de la bala (parte sobre el cañón).
 * @param velocidad Píxeles que avanza por frame.
 */
void lista_insertar_frente(ListaBala* lista, int x, int y, int velocidad);

/**
 * @brief Elimina todos los nodos cuya posición y sea menor a 0 (salieron de pantalla).
 * Recorre la lista y libera la memoria de cada nodo eliminado.
 * @param lista Puntero a la lista de balas.
 */
void lista_eliminar_fuera_de_pantalla(ListaBala* lista);

/**
 * @brief Mueve todas las balas hacia arriba restando su velocidad a y.
 * @param lista Puntero a la lista de balas.
 */
void lista_mover_balas(ListaBala* lista);

/**
 * @brief Libera toda la memoria de la lista (todos los nodos).
 * Debe llamarse al cerrar el programa.
 * @param lista Puntero a la lista de balas.
 */
void lista_destruir(ListaBala* lista);

#endif // STRUCTS_H
#ifndef STRUCTS_H
#define STRUCTS_H

/**
 * @file structs.h
 * @brief Definición de las estructuras de datos y modelos del cliente.
 * Representa las entidades sincronizadas desde el servidor.
 */

/**
 * @struct Jugador
 * @brief Entidad que representa el cañón controlado por el usuario.
 */
typedef struct {
    int id_jugador;    /**< Identificador único en red (Socket ID). */
    int posicion_x;    /**< Coordenada en el eje X de la pantalla. */
    int vidas;         /**< Contador de vidas restantes. */
    int puntuacion;    /**< Puntaje acumulado en la sesión. */
} Jugador;

/**
 * @struct Extraterrestre
 * @brief Entidad enemiga estándar (Calamar, Cangrejo o Pulpo).
 */
typedef struct {
    int id;            /**< Identificador único de la entidad. */
    int x;             /**< Coordenada actual en el eje X. */
    int y;             /**< Coordenada actual en el eje Y. */
    int tipo;          /**< Clasificación por puntos (10, 20 o 40). */
    int estado;        /**< Estado lógico: 1 (Activo), 0 (Destruido). */
} Extraterrestre;

/**
 * @struct Bunker
 * @brief Estructura de defensa terrestre.
 */
typedef struct {
    int id;                 /**< Identificador único de la barrera. */
    int x;                  /**< Coordenada en el eje X. */
    int y;                  /**< Coordenada en el eje Y. */
    int porcentaje_salud;   /**< Nivel de integridad estructural (0-100). */
} Bunker;

/**
 * @struct Ovni
 * @brief Entidad especial de bonificación.
 */
typedef struct {
    int id;            /**< Identificador único de la entidad. */
    int x;             /**< Coordenada actual en el eje X. */
    int y;             /**< Coordenada actual en el eje Y. */
    int velocidad;     /**< Magnitud de desplazamiento por ciclo. */
    int puntosExtra;   /**< Valor de recompensa al ser destruido. */
    int activo;        /**< Estado de visibilidad: 1 (Visible), 0 (Oculto). */
} Ovni;

/**
 * @struct NodoBala
 * @brief Elemento base para la lista enlazada dinámica de proyectiles.
 */
typedef struct NodoBala {
    int x;                      /**< Coordenada horizontal actual. */
    int y;                      /**< Coordenada vertical actual. */
    int velocidad;              /**< Magnitud y dirección de desplazamiento. */
    struct NodoBala* siguiente; /**< Referencia al siguiente elemento de la colección. */
} NodoBala;

/**
 * @struct ListaBala
 * @brief Estructura de control para la memoria dinámica de proyectiles.
 */
typedef struct {
    NodoBala* cabeza;   /**< Puntero de acceso al primer elemento. */
    int cantidad;       /**< Cantidad de elementos activos en memoria. */
} ListaBala;


/**
 * @brief Inicializa la estructura de la lista enlazada.
 * @param lista Referencia a la lista.
 */
void lista_inicializar(ListaBala* lista);

/**
 * @brief Agrega un nuevo elemento al inicio de la estructura.
 * @param lista Referencia a la lista.
 * @param x Coordenada X inicial.
 * @param y Coordenada Y inicial.
 * @param velocidad Desplazamiento por iteración.
 */
void lista_insertar_frente(ListaBala* lista, int x, int y, int velocidad);

/**
 * @brief Libera los recursos de los elementos fuera del área de renderizado.
 * @param lista Referencia a la lista.
 */
void lista_eliminar_fuera_de_pantalla(ListaBala* lista);

/**
 * @brief Actualiza las coordenadas espaciales de los elementos iterables.
 * @param lista Referencia a la lista.
 */
void lista_mover_balas(ListaBala* lista);

/**
 * @brief Libera el bloque de memoria completo asignado a la estructura.
 * @param lista Referencia a la lista.
 */
void lista_destruir(ListaBala* lista);

#endif // STRUCTS_H
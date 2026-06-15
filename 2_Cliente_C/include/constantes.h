#ifndef CONSTANTES_H
#define CONSTANTES_H

/**
 * @file constantes.h
 * @brief Definición de constantes globales para el cliente de spaCEinvaders.
 * Cumple con el requerimiento de aislar los valores "hardcodeados" del código fuente.
 */

/* ==========================================
 * Constantes de Configuración de Red
 * ========================================== */
#define PUERTO_SERVIDOR 8080
#define IP_SERVIDOR "127.0.0.1"
#define TAMANO_BUFFER 1024
#define PUERTO_CONTROL "COM7"

/* ==========================================
 * Constantes de Protocolo de Comunicación
 * ========================================== */
#define ROL_JUGADOR "JUGADOR\n"
#define ROL_ESPECTADOR "ESPECTADOR\n"

/* ==========================================
 * Constantes de Interfaz Gráfica (GUI)
 * ========================================== */
#define ANCHO_PANTALLA 800  /**< Ancho de la ventana principal en píxeles */
#define ALTO_PANTALLA 600   /**< Alto de la ventana principal en píxeles */
#define FPS_OBJETIVO 60     /**< Tasa de refresco objetivo para Raylib */

/* ==========================================
 * Constantes de Dimensiones de Entidades
 * ========================================== */
#define ANCHO_CANON 40      /**< Ancho del sprite del cañón del jugador */
#define ALTO_CANON 20       /**< Alto del sprite del cañón del jugador */
#define ANCHO_ALIEN 30      /**< Ancho del sprite de los extraterrestres */
#define ALTO_ALIEN 30       /**< Alto del sprite de los extraterrestres */

/* ==========================================
 * Constantes de Matriz y Juego
 * ========================================== */
#define FILAS_ALIENS 5
#define COLUMNAS_ALIENS 11
#define CANTIDAD_BUNKERS 4
#define VIDAS_INICIALES 3

#endif // CONSTANTES_H
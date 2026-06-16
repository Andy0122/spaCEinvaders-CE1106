#ifndef CONSTANTES_H
#define CONSTANTES_H

/**
 * @file constantes.h
 * @brief Parámetros y constantes globales de configuración del sistema.
 */

// Configuración de Red e Interfaces
#define PUERTO_SERVIDOR 8080
#define IP_SERVIDOR "127.0.0.1"
#define TAMANO_BUFFER 1024
#define PUERTO_CONTROL "COM7"

// Protocolo de Comunicación
#define ROL_JUGADOR "JUGADOR\n"
#define ROL_ESPECTADOR "ESPECTADOR\n"

// Configuración del Motor Gráfico
#define ANCHO_PANTALLA 800  /**< Dimensión horizontal de la ventana. */
#define ALTO_PANTALLA 600   /**< Dimensión vertical de la ventana. */
#define FPS_OBJETIVO 60     /**< Tasa de refresco objetivo. */

// Dimensiones Físicas (Hitboxes)
#define ANCHO_CANON 40      
#define ALTO_CANON 20       
#define ANCHO_ALIEN 30      
#define ALTO_ALIEN 30       

// Parámetros de Juego
#define FILAS_ALIENS 5
#define COLUMNAS_ALIENS 11
#define CANTIDAD_BUNKERS 4
#define VIDAS_INICIALES 3
#define MAX_ALIENS 128      

// Físicas y Cinemática
#define VELOCIDAD_BALA 8            /**< Desplazamiento por frame de proyectiles aliados. */
#define VELOCIDAD_BALA_ENEMIGA 14   /**< Desplazamiento por ciclo de servidor de proyectiles hostiles. */

#endif // CONSTANTES_H
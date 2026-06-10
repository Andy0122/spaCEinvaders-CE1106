#ifndef RED_H
#define RED_H

/**
 * @file red.h
 * @brief Interfaz para el manejo de la comunicación cliente-servidor mediante Sockets.
 */

#include "structs.h"

/**
 * @brief Vincula los arreglos de memoria gráfica con el hilo de red.
 * Permite que la función de parseo actualice la pantalla asíncronamente.
 */
void vincular_punteros_red(Jugador jugadores[], Extraterrestre aliens[], Bunker bunkers[]);

/**
 * @brief Inicializa Winsock2, conecta con Java y lanza el hilo de escucha.
 * @param handshake Cadena de texto inicial (Ej: "JUGADOR|1\n").
 * @return 1 si la conexión fue exitosa, 0 en caso de error.
 */
int inicializar_conexion(const char* handshake);

/**
 * @brief Envía una cadena de texto (comando) al servidor.
 * @param mensaje Puntero a la cadena de caracteres a enviar.
 */
void enviar_comando_servidor(const char* mensaje);

/**
 * @brief Cierra el socket de forma segura.
 */
void cerrar_conexion();

#endif // RED_H
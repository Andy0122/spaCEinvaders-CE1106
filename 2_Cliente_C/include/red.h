#ifndef RED_H
#define RED_H

/**
 * @file red.h
 * @brief Interfaz para el subsistema de red y sincronización por Sockets.
 */

#include "structs.h"

/**
 * @brief Enlaza los modelos de datos locales con el procesador de mensajes de red.
 * @param jugadores Referencia al bloque de memoria de jugadores.
 * @param aliens Referencia al bloque de memoria de enemigos.
 * @param bunkers Referencia al bloque de memoria de defensas.
 * @param ovni Referencia a la entidad especial.
 * @param balas Referencia a la estructura de proyectiles aliados.
 * @param balas_enemigas Referencia a la estructura de proyectiles hostiles.
 */
void vincular_punteros_red(Jugador jugadores[], Extraterrestre aliens[], Bunker bunkers[], Ovni* ovni, ListaBala* balas, ListaBala* balas_enemigas);

/**
 * @brief Inicializa el subsistema de red y establece la conexión TCP.
 * @param handshake Cadena de identificación de protocolo inicial.
 * @return 1 en caso de éxito, 0 en caso de fallo de conexión.
 */
int inicializar_conexion(const char* handshake);

/**
 * @brief Transmite una instrucción formateada hacia el servidor.
 * @param mensaje Cadena de caracteres del comando.
 */
void enviar_comando_servidor(const char* mensaje);

/**
 * @brief Finaliza la conexión activa y libera los recursos del socket.
 */
void cerrar_conexion();

#endif // RED_H
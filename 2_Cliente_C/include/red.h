#ifndef RED_H
#define RED_H

/**
 * @file red.h
 * @brief Interfaz para el manejo de la comunicación cliente-servidor mediante Sockets.
 */

/**
 * @brief Inicializa la librería Winsock, crea el socket y conecta con el servidor.
 * @return 1 si la conexión fue exitosa, 0 en caso de error crítico.
 */
int inicializar_conexion();

/**
 * @brief Envía una cadena de texto (comando) al servidor.
 * @param mensaje Puntero a la cadena de caracteres a enviar.
 */
void enviar_comando_servidor(const char* mensaje);

/**
 * @brief Cierra el socket de forma segura y libera los recursos de red.
 */
void cerrar_conexion();

#endif // RED_H
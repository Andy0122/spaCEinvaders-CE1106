/**
 * @file red.c
 * @brief Implementación de las rutinas de comunicación de red usando Winsock2.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <winsock2.h>
#include <windows.h>

#include "../include/constantes.h"
#include "../include/red.h"

// Variable global (estática a este archivo) para mantener la conexión activa
static SOCKET socket_cliente;

/**
 * @brief Hilo concurrente encargado de escuchar los mensajes asíncronos del Servidor.
 * Se ejecuta en paralelo para no bloquear la interfaz del usuario.
 * 
 * @param lpParam Parámetro del hilo (no utilizado en esta implementación).
 * @return Código de salida del hilo.
 */
static DWORD WINAPI escuchar_servidor(LPVOID lpParam) {
    char buffer[TAMANO_BUFFER];
    int bytes_recibidos;

    while (1) {
        memset(buffer, 0, TAMANO_BUFFER);
        bytes_recibidos = recv(socket_cliente, buffer, TAMANO_BUFFER - 1, 0);
        
        if (bytes_recibidos <= 0) {
            printf("\n[ALERTA CRITICA] Se ha perdido la conexion con el Servidor Java.\n");
            exit(EXIT_FAILURE);
        }

        // Handshake inicial: El servidor pide identificación
        if (strncmp(buffer, "IDENTIFICATE", 12) == 0) {
            enviar_comando_servidor(ROL_JUGADOR);
            printf("[PROTOCOLO] Identidad enviada exitosamente: %s", ROL_JUGADOR);
        } else {
            // TODO: Integrar aquí el parseo de mensajes (Ej: "Crear Extraterrestre (1,1, 1000)")
            printf("\n[SERVIDOR]: %s", buffer);
            printf("> Esperando comando... ");
        }
    }
    return 0;
}

int inicializar_conexion() {
    WSADATA wsa;
    struct sockaddr_in config_servidor;

    printf("[INFO] Inicializando subsistema de red (Winsock)...\n");
    if (WSAStartup(MAKEWORD(2,2), &wsa) != 0) {
        printf("[ERROR] Fallo en inicializacion de Winsock. Codigo: %d\n", WSAGetLastError());
        return 0;
    }

    // Creacion del Socket TCP/IP
    if ((socket_cliente = socket(AF_INET, SOCK_STREAM, 0)) == INVALID_SOCKET) {
        printf("[ERROR] Creacion de socket fallida. Codigo: %d\n", WSAGetLastError());
        return 0;
    }

    // Configuracion de la IP y Puerto del servidor objetivo
    config_servidor.sin_family = AF_INET;
    config_servidor.sin_addr.s_addr = inet_addr(IP_SERVIDOR);
    config_servidor.sin_port = htons(PUERTO_SERVIDOR);

    // Intento de conexion
    printf("[INFO] Intentando establecer conexion con %s:%d...\n", IP_SERVIDOR, PUERTO_SERVIDOR);
    if (connect(socket_cliente, (struct sockaddr *)&config_servidor, sizeof(config_servidor)) < 0) {
        printf("[ERROR] Servidor inalcanzable. Verifique que Java este en ejecucion.\n");
        return 0;
    }

    printf("[INFO] Conexion TCP establecida correctamente.\n");

    // Lanzamiento del hilo de escucha asincrona
    CreateThread(NULL, 0, escuchar_servidor, NULL, 0, NULL);

    return 1; // Exito
}

void enviar_comando_servidor(const char* mensaje) {
    if (send(socket_cliente, mensaje, strlen(mensaje), 0) < 0) {
        printf("[ERROR] Fallo al despachar el mensaje hacia el servidor.\n");
    }
}

void cerrar_conexion() {
    closesocket(socket_cliente);
    WSACleanup();
    printf("[INFO] Recursos de red liberados.\n");
}
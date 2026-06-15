/**
 * @file red.c
 * @brief Implementación de las rutinas de comunicación y PARSEO (Actividad A5).
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <winsock2.h>
#include <windows.h>

#include "../include/constantes.h"
#include "../include/red.h"
#include "../include/structs.h"

// Variable global (estática a este archivo) para mantener la conexión activa
static SOCKET socket_cliente;

// Punteros globales para modificar la memoria de la interfaz gráfica directamente
static Jugador* ptr_jugadores;
static Extraterrestre* ptr_aliens;
static Bunker* ptr_bunkers;
static Ovni* ptr_ovni;
static char mensaje_handshake[64];

void vincular_punteros_red(Jugador jugadores[], Extraterrestre aliens[], Bunker bunkers[], Ovni* ovni) {
    ptr_jugadores = jugadores;
    ptr_aliens = aliens;
    ptr_bunkers = bunkers;
    ptr_ovni = ovni;
}

/**
 * @brief Desarma los mensajes de Java y actualiza los structs locales.
 * @param mensaje Línea de texto recibida por el socket.
 * * TODO: El servidor Java debe enviar strings con este formato exacto:
 * - ALIEN|id|x|y|estado\n
 * - JUGADOR|id|x|vidas|puntos\n
 * - BUNKER|id|salud\n
 * - OVNI|id|x|y|velocidad|puntosExtra\n (o estado si son 4 parámetros)
 */
static void parsear_mensaje(const char* mensaje) {
    char tipo[16];
    
    if (sscanf(mensaje, "%15[^|]", tipo) != 1) return;

    if (strcmp(tipo, "ALIEN") == 0) {
        int id, x, y, estado;
        if (sscanf(mensaje, "ALIEN|%d|%d|%d|%d", &id, &x, &y, &estado) == 4) {
            int total_aliens = FILAS_ALIENS * COLUMNAS_ALIENS;
            for (int i = 0; i < total_aliens; i++) {
                if (ptr_aliens[i].id == id) {
                    ptr_aliens[i].x = x;
                    ptr_aliens[i].y = y;
                    ptr_aliens[i].estado = estado;
                    break;
                }
            }
        }
    } 
    else if (strcmp(tipo, "JUGADOR") == 0) {
        int id, x, vidas, puntos;
        // Java envia: JUGADOR|id|x|vidas|puntos
        if (sscanf(mensaje, "JUGADOR|%d|%d|%d|%d", &id, &x, &vidas, &puntos) == 4) {
            // Depuracion visual en consola
            printf("[RED] Actualizando en tiempo real -> JUGADOR ID: %d, X: %d\n", id, x);

            for (int i = 0; i < 2; i++) { 
                if (ptr_jugadores[i].id_jugador == id) {
                    ptr_jugadores[i].posicion_x = x;
                    ptr_jugadores[i].vidas = vidas;
                    ptr_jugadores[i].puntuacion = puntos;
                    break;
                }
            }
        }
    }
    else if (strcmp(tipo, "BUNKER") == 0) {
        int id, salud;
        if (sscanf(mensaje, "BUNKER|%d|%d", &id, &salud) == 2) {
            for (int i = 0; i < CANTIDAD_BUNKERS; i++) {
                if (ptr_bunkers[i].id == id) {
                    ptr_bunkers[i].porcentaje_salud = salud;
                    break;
                }
            }
        }
    }
    // LÓGICA DE PARSEO DEL OVNI (Integración con el motor de Java)
    else if (strcmp(tipo, "OVNI") == 0) {
        int id, x, y, p4, p5;
        // Lee hasta 5 parámetros (dependiendo si es evento de creación o movimiento)
        int leidos = sscanf(mensaje, "OVNI|%d|%d|%d|%d|%d", &id, &x, &y, &p4, &p5);
        
        if (leidos >= 4 && ptr_ovni != NULL) {
            ptr_ovni->id = id;
            ptr_ovni->x = x;
            ptr_ovni->y = y;
            
            // Si llegan 5 datos, asumimos que es el evento inicial de CREACIÓN
            if (leidos == 5) {
                ptr_ovni->velocidad = p4;
                ptr_ovni->puntosExtra = p5;
                ptr_ovni->activo = 1; // Lo encendemos para renderizarlo
            } 
            // Si llegan 4 datos, es un evento de MOVIMIENTO donde p4 es el estado (activo/inactivo)
            else if (leidos == 4) {
                ptr_ovni->activo = p4; 
            }
            
            // Apagar por seguridad si sale muy lejos de los márgenes de la pantalla
            if (ptr_ovni->x > ANCHO_PANTALLA + 100 || ptr_ovni->x < -100) {
                ptr_ovni->activo = 0;
            }
        }
    }
}

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

        char* token_linea = strtok(buffer, "\n");
        while (token_linea != NULL) {
            // Se responde a la petición de identidad del servidor
            if (strncmp(token_linea, "IDENTIFICATE", 12) == 0) {
                enviar_comando_servidor(mensaje_handshake);
                printf("[PROTOCOLO] Identidad enviada al Servidor: %s", mensaje_handshake);
            } else {
                parsear_mensaje(token_linea);
            }

            token_linea = strtok(NULL, "\n");
        }
    }
    return 0;
}

int inicializar_conexion(const char* handshake) {
    strncpy(mensaje_handshake, handshake, sizeof(mensaje_handshake));

    WSADATA wsa;
    struct sockaddr_in config_servidor;

    printf("[INFO] Inicializando subsistema de red (Winsock)...\n");
    if (WSAStartup(MAKEWORD(2,2), &wsa) != 0) {
        printf("[ERROR] Fallo en inicializacion de Winsock.\n");
        return 0;
    }

    // Creacion del Socket TCP/IP
    if ((socket_cliente = socket(AF_INET, SOCK_STREAM, 0)) == INVALID_SOCKET) {
        printf("[ERROR] Creacion de socket fallida.\n");
        return 0;
    }

    // Configuracion de la IP y Puerto del servidor objetivo
    config_servidor.sin_family = AF_INET;
    config_servidor.sin_addr.s_addr = inet_addr(IP_SERVIDOR); 
    config_servidor.sin_port = htons(PUERTO_SERVIDOR); 
    
    printf("[INFO] Intentando conexion...\n");
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
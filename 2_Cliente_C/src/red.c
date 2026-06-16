/**
 * @file red.c
 * @brief Implementación de las rutinas de comunicación y parseo de mensajes del servidor.
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
static ListaBala* ptr_balas;
static char mensaje_handshake[64];

void vincular_punteros_red(Jugador jugadores[], Extraterrestre aliens[], Bunker bunkers[], Ovni* ovni, ListaBala* balas) {
    ptr_jugadores = jugadores;
    ptr_aliens = aliens;
    ptr_bunkers = bunkers;
    ptr_ovni = ovni;
    ptr_balas = balas;
}

/**
 * @brief Busca el índice de un alien por su id en el arreglo local.
 *        Si el id no existe, devuelve el primer slot libre (id == -1).
 *        Si no hay slots libres, devuelve -1 para ignorar el mensaje.
 */
static int buscar_o_reservar_slot_alien(int id) {
    int slot_libre = -1;
    for (int i = 0; i < MAX_ALIENS; i++) {
        if (ptr_aliens[i].id == id) {
            return i; // Encontrado
        }
        if (ptr_aliens[i].id == -1 && slot_libre == -1) {
            slot_libre = i; // Primer hueco disponible
        }
    }
    // No encontrado: asignamos el primer slot libre
    if (slot_libre != -1) {
        ptr_aliens[slot_libre].id = id;
    }
    return slot_libre;
}

/**
 * @brief Parsea una línea de texto recibida del servidor y actualiza los structs locales.
 *
 * Formatos esperados:
 *   ALIEN|id|x|y|estado          (estado 1=vivo, 0=destruido)
 *   JUGADOR|id|x|vidas|puntos
 *   BUNKER|id|salud
 *   OVNI|id|x|y|velocidad|puntos
 *   VELOCIDAD|valor               (informativo, no modifica estado local)
 *   IMPACTO_JUGADOR|vidas         (feedback de impacto, se actualiza via JUGADOR tb)
 */
static void parsear_mensaje(const char* mensaje) {
    char tipo[16];
    if (sscanf(mensaje, "%15[^|]", tipo) != 1) return;

    /* ------ ALIEN ------ */
    if (strcmp(tipo, "ALIEN") == 0) {
        int id, x, y, estado;
        if (sscanf(mensaje, "ALIEN|%d|%d|%d|%d", &id, &x, &y, &estado) == 4) {
            int idx = buscar_o_reservar_slot_alien(id);
            if (idx != -1) {
                ptr_aliens[idx].x = x;
                ptr_aliens[idx].y = y;
                ptr_aliens[idx].estado = estado;
            }
        }
    }

    /* ------ JUGADOR ------ */
    else if (strcmp(tipo, "JUGADOR") == 0) {
        int id, x, vidas, puntos;
        if (sscanf(mensaje, "JUGADOR|%d|%d|%d|%d", &id, &x, &vidas, &puntos) == 4) {
            for (int i = 0; i < 2; i++) {
                if (ptr_jugadores[i].id_jugador == id) {
                    if (abs(ptr_jugadores[i].posicion_x - x) > 10) {
                        ptr_jugadores[i].posicion_x = x;
                    }
                    ptr_jugadores[i].vidas = vidas;
                    ptr_jugadores[i].puntuacion = puntos;
                    break;
                }
            }
        }
    }

    /* ------ BUNKER ------ */
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

    /* ------ OVNI ------ */
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
            } else {
                // Evento de movimiento: p4 es el estado activo/inactivo
                ptr_ovni->activo = p4;
            }

            // Apagar por seguridad si sale muy lejos de los márgenes de la pantalla
            if (ptr_ovni->x > ANCHO_PANTALLA + 100 || ptr_ovni->x < -100) {
                ptr_ovni->activo = 0;
            }
        }
    }

    /* ------ VELOCIDAD (informativo) ------ */
    else if (strcmp(tipo, "VELOCIDAD") == 0) {
        int vel;
        if (sscanf(mensaje, "VELOCIDAD|%d", &vel) == 1) {
            printf("[RED] Velocidad de aliens actualizada: %d\n", vel);
        }
    }

    /* ------ IMPACTO_JUGADOR (el servidor confirmará via JUGADOR|id|...) ------ */
    else if (strcmp(tipo, "IMPACTO_JUGADOR") == 0) {
        int vidas;
        if (sscanf(mensaje, "IMPACTO_JUGADOR|%d", &vidas) == 1) {
            printf("[RED] Impacto recibido! Vidas restantes: %d\n", vidas);
            // Las vidas se sincronizan definitivamente por el mensaje JUGADOR siguiente
        }
    }

    /* ------ DISPARO (Se dibuja la bala validada por el servidor) ------ */
    else if (strcmp(tipo, "DISPARO") == 0) {
        int id, x, y;
        if (sscanf(mensaje, "DISPARO|%d|%d|%d", &id, &x, &y) == 3) {
            if (ptr_balas != NULL) {
                lista_insertar_frente(ptr_balas, x, y, VELOCIDAD_BALA);
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
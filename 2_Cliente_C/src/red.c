/**
 * @file red.c
 * @brief Implementación del protocolo de comunicación y sincronización de red.
 * Procesa la recepción de la trama de bytes y actualiza el estado local de la aplicación.
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
static ListaBala* ptr_balas_enemigas; 
static char            mensaje_handshake[64];

void vincular_punteros_red(Jugador jugadores[], Extraterrestre aliens[], Bunker bunkers[], Ovni* ovni, ListaBala* balas, ListaBala* balas_enemigas) {
    ptr_jugadores      = jugadores;
    ptr_aliens         = aliens;
    ptr_bunkers        = bunkers;
    ptr_ovni           = ovni;
    ptr_balas          = balas;
    ptr_balas_enemigas = balas_enemigas;
}

/**
 * @brief Obtiene el índice de almacenamiento de una entidad alienígena.
 * @param id Identificador en red de la entidad.
 * @return Índice del bloque de memoria asignado o -1 si no hay espacio.
 */
static int buscar_o_reservar_slot_alien(int id) {
    int slot_libre = -1;
    for (int i = 0; i < MAX_ALIENS; i++) {
        if (ptr_aliens[i].id == id) return i; // Encontrado
        if (ptr_aliens[i].id == -1 && slot_libre == -1) slot_libre = i; // Primer hueco disponible
    }

    // No encontrado: asignamos el primer slot libre
    if (slot_libre != -1) ptr_aliens[slot_libre].id = id;
    return slot_libre;
}

/**
 * @brief Identifica y elimina un proyectil tras el registro de un impacto en el servidor.
 * Compensa el retraso de red buscando la coincidencia más próxima por radio.
 * @param lista Estructura dinámica de proyectiles a analizar.
 * @param x_impacto Coordenada horizontal del servidor.
 * @param y_impacto Coordenada vertical del servidor.
 */
static void eliminar_bala_enemiga_cercana(ListaBala* lista, int x_impacto, int y_impacto) {
    if (lista == NULL) return;

    NodoBala* mas_cercana = NULL;
    int menor_distancia = 999999;
    NodoBala* actual = lista->cabeza;

    while (actual != NULL) {
        if (actual->y >= 0) { 
            int dx = actual->x - x_impacto;
            int dy = actual->y - y_impacto;
            int distancia = dx * dx + dy * dy; 
            
            if (distancia < menor_distancia) {
                menor_distancia = distancia;
                mas_cercana = actual;
            }
        }
        actual = actual->siguiente;
    }

    // Radio de búsqueda
    if (mas_cercana != NULL && menor_distancia <= 1600) {
        mas_cercana->y = -999;
    }
}

/**
 * @brief Decodifica una instrucción recibida y aplica los cambios en el modelo de datos.
 * @param mensaje Cadena de caracteres que contiene la instrucción del protocolo.
 *  Formatos esperados:
 *   ALIEN|id|x|y|estado          (estado 1=vivo, 0=destruido)
 *   JUGADOR|id|x|vidas|puntos
 *   BUNKER|id|salud
 *   OVNI|id|x|y|velocidad|puntos (creación, 5 campos tras tipo)
 *   OVNI|id|x|y|estado           (movimiento, 4 campos tras tipo)
 *   VELOCIDAD|valor               (informativo)
 *   IMPACTO_JUGADOR|vidas
 *   DISPARO|x|y
 *   GAME_OVER
 */
static void parsear_mensaje(const char* mensaje) {
    char tipo[32];
    if (sscanf(mensaje, "%31[^|]", tipo) != 1) return;

    if (strcmp(tipo, "ALIEN") == 0) {
        int id, tipo_alien, x, y, estado;
        if (sscanf(mensaje, "ALIEN|%d|%d|%d|%d|%d", &id, &tipo_alien, &x, &y, &estado) == 5) {
            int idx = buscar_o_reservar_slot_alien(id);
            if (idx != -1) {
                ptr_aliens[idx].tipo   = tipo_alien;
                ptr_aliens[idx].x      = x;
                ptr_aliens[idx].y      = y;
                ptr_aliens[idx].estado = estado;
            }
        }
    }
    else if (strcmp(tipo, "JUGADOR") == 0) {
        int id, x, vidas, puntos;
        if (sscanf(mensaje, "JUGADOR|%d|%d|%d|%d", &id, &x, &vidas, &puntos) == 4) {
            for (int i = 0; i < 2; i++) {
                if (ptr_jugadores[i].id_jugador == id) {
                    ptr_jugadores[i].posicion_x   = x;
                    ptr_jugadores[i].vidas        = vidas;
                    ptr_jugadores[i].puntuacion   = puntos;
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
    else if (strcmp(tipo, "OVNI") == 0) {
        int id, x, y, p4, p5;
        int leidos = sscanf(mensaje, "OVNI|%d|%d|%d|%d|%d", &id, &x, &y, &p4, &p5);
        if (leidos >= 4 && ptr_ovni != NULL) {
            ptr_ovni->id = id;
            ptr_ovni->x  = x;
            ptr_ovni->y  = y;

            if (leidos == 5) {
                // Evento de CREACIÓN: velocidad|puntos
                ptr_ovni->velocidad   = p4;
                ptr_ovni->puntosExtra = p5;
                ptr_ovni->activo      = 1;
            } else {
                // Evento de MOVIMIENTO: p4 es estado (1=activo, 0=destruido)
                ptr_ovni->activo = p4;
            }

            // Apagar si sale demasiado lejos de los márgenes
            if (ptr_ovni->x > ANCHO_PANTALLA + 100 || ptr_ovni->x < -100) {
                ptr_ovni->activo = 0;
            }
        }
    }

    else if (strcmp(tipo, "VELOCIDAD") == 0) {
        int vel;
        if (sscanf(mensaje, "VELOCIDAD|%d", &vel) == 1) {
            printf("[RED] Velocidad de aliens actualizada: %d\n", vel);
        }
    }

    else if (strcmp(tipo, "IMPACTO_JUGADOR") == 0) {
        int vidas;
        if (sscanf(mensaje, "IMPACTO_JUGADOR|%d", &vidas) == 1) {
            printf("[RED] Impacto recibido! Vidas restantes: %d\n", vidas);
            // Las vidas se sincronizan definitivamente por el mensaje JUGADOR siguiente
        }
    }
    else if (strcmp(tipo, "DISPARO") == 0) {
        int x, y;
        if (sscanf(mensaje, "DISPARO|%d|%d", &x, &y) == 2) {
            if (ptr_balas != NULL) {
                lista_insertar_frente(ptr_balas, x, y, VELOCIDAD_BALA);
            }
        }
    }
    else if (strcmp(tipo, "DISPARO_ENEMIGO") == 0) {
        int x, y;
        if (sscanf(mensaje, "DISPARO_ENEMIGO|%d|%d", &x, &y) == 2) {
            if (ptr_balas_enemigas != NULL) {
                lista_insertar_frente(ptr_balas_enemigas, x, y, -VELOCIDAD_BALA_ENEMIGA);
            }
        }
    }
    else if (strcmp(tipo, "BALA_ENEMIGA_DESTRUIDA") == 0) {
        int x, y;
        if (sscanf(mensaje, "BALA_ENEMIGA_DESTRUIDA|%d|%d", &x, &y) == 2) {
            eliminar_bala_enemiga_cercana(ptr_balas_enemigas, x, y);
        }
    }
    else if (strcmp(tipo, "BALA_JUGADOR_DESTRUIDA") == 0) {
        int x, y;
        if (sscanf(mensaje, "BALA_JUGADOR_DESTRUIDA|%d|%d", &x, &y) == 2) {
            // Reutilizamos la misma función de distancia, pero pasándole la lista de balas amarillas
            eliminar_bala_enemiga_cercana(ptr_balas, x, y);
        }
    }

    else if (strcmp(tipo, "GAME_OVER") == 0) {
        printf("[RED] GAME OVER recibido del servidor.\n");
    }
    else if (strcmp(tipo, "LIMPIAR_ALIENS") == 0) {
        for (int i = 0; i < MAX_ALIENS; i++) {
            ptr_aliens[i].id     = -1;
            ptr_aliens[i].estado = 0;
            ptr_aliens[i].x      = 0;
            ptr_aliens[i].y      = 0;
            ptr_aliens[i].tipo   = 10;
        }
        printf("[RED] Aliens limpiados, esperando nueva ola.\n");
    }
}

/**
 * @brief Subrutina de lectura continua asíncrona mediante Sockets.
 */
static DWORD WINAPI escuchar_servidor(LPVOID lpParam) {
    char buffer[TAMANO_BUFFER];
    // Acumulador: puede llegar a tener hasta 2 buffers completos sin salto de línea
    char acumulador[TAMANO_BUFFER * 2];
    int  bytes_acumulados = 0;
    int  bytes_recibidos;

    memset(acumulador, 0, sizeof(acumulador));

    while (1) {
        memset(buffer, 0, TAMANO_BUFFER);
        bytes_recibidos = recv(socket_cliente, buffer, TAMANO_BUFFER - 1, 0);

        if (bytes_recibidos <= 0) {
            printf("\n[ALERTA CRITICA] Se ha perdido la comunicacion con el servidor.\n");
            exit(EXIT_FAILURE);
        }

        // Añadir lo recibido al acumulador (con guardia de desbordamiento)
        if (bytes_acumulados + bytes_recibidos < (int)sizeof(acumulador) - 1) {
            memcpy(acumulador + bytes_acumulados, buffer, bytes_recibidos);
            bytes_acumulados += bytes_recibidos;
            acumulador[bytes_acumulados] = '\0';
        } else {
            // Acumulador lleno sin salto de línea: descartar y reiniciar
            bytes_acumulados = 0;
            memset(acumulador, 0, sizeof(acumulador));
            continue;
        }

        // Procesar todas las líneas COMPLETAS del acumulador
        char* inicio = acumulador;
        char* salto;
        while ((salto = strchr(inicio, '\n')) != NULL) {
            *salto = '\0';

            // Sanitización del retorno de carro (CR)
            int len = (int)strlen(inicio);
            if (len > 0 && inicio[len - 1] == '\r') {
                inicio[len - 1] = '\0';
                len--;
            }

            if (len > 0) {
                if (strncmp(inicio, "IDENTIFICATE", 12) == 0) {
                    enviar_comando_servidor(mensaje_handshake);
                    printf("[PROTOCOLO] Identidad enviada al Servidor: %s", mensaje_handshake);
                } else {
                    parsear_mensaje(inicio);
                }
            }
            inicio = salto + 1;
        }

        // Mover el fragmento incompleto restante al inicio del acumulador
        bytes_acumulados = (int)strlen(inicio);
        memmove(acumulador, inicio, bytes_acumulados);
        acumulador[bytes_acumulados] = '\0';
    }
    return 0;
}

int inicializar_conexion(const char* handshake) {
    strncpy(mensaje_handshake, handshake, sizeof(mensaje_handshake));
    mensaje_handshake[sizeof(mensaje_handshake) - 1] = '\0';

    WSADATA wsa;
    struct sockaddr_in config_servidor;

    printf("[INFO] Inicializando subsistema de red (Winsock)...\n");
    if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) {
        printf("[ERROR] Fallo en inicializacion de Winsock.\n");
        return 0;
    }

    // Creacion del Socket TCP/IP
    if ((socket_cliente = socket(AF_INET, SOCK_STREAM, 0)) == INVALID_SOCKET) {
        printf("[ERROR] Creacion de socket fallida.\n");
        return 0;
    }

    // Configuracion de la IP y Puerto del servidor objetivo
    config_servidor.sin_family      = AF_INET;
    config_servidor.sin_addr.s_addr = inet_addr(IP_SERVIDOR);
    config_servidor.sin_port        = htons(PUERTO_SERVIDOR);

     printf("[INFO] Intentando conexion...\n");
    if (connect(socket_cliente, (struct sockaddr*)&config_servidor, sizeof(config_servidor)) < 0) {
        printf("[ERROR] Servidor inalcanzable. Verifique que Java este en ejecucion.\n");
        return 0;
    }

    printf("[INFO] Conexion TCP establecida correctamente.\n");

    // Lanzamiento del hilo de escucha asincrona
    CreateThread(NULL, 0, escuchar_servidor, NULL, 0, NULL);
    return 1; // Exito
}

void enviar_comando_servidor(const char* mensaje) {
    if (send(socket_cliente, mensaje, (int)strlen(mensaje), 0) < 0) {
        printf("[ERROR] Fallo al despachar el mensaje hacia el servidor.\n");
    }
}

void cerrar_conexion() {
    closesocket(socket_cliente);
    WSACleanup();
    printf("[INFO] Recursos de red liberados.\n");
}
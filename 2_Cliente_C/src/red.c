/**
 * @file red.c
 * @brief Implementación de las rutinas de comunicación y parseo de mensajes del servidor.
 *
 * Correcciones aplicadas:
 *  1. Bug \r\n: se elimina el \r residual de cada línea antes de parsear.
 *  2. Bug DISPARO: el servidor emite DISPARO|x|y (2 campos), no 3.
 *  3. Bug sincronización jugador: se aplica siempre la posición del servidor.
 *  4. Bug TCP framing: acumulador de bytes para ensamblar líneas partidas entre recv().
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
static Jugador*        ptr_jugadores;
static Extraterrestre* ptr_aliens;
static Bunker*         ptr_bunkers;
static Ovni*           ptr_ovni;
static ListaBala*      ptr_balas;
static ListaBala*      ptr_balas_enemigas; /**< Balas de los aliens, bajan en pantalla */
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
 * @brief Marca para eliminación la bala enemiga MÁS CERCANA a (x,y).
 *
 * El servidor avisa la posición exacta donde resolvió la colisión, pero
 * por latencia de red la posición local de esa misma bala en el cliente
 * puede diferir ligeramente. Se busca dentro de un radio razonable (40px)
 * en vez de exigir coincidencia exacta, y se marca con y=-999 para que
 * lista_eliminar_fuera_de_pantalla() la limpie en el siguiente paso del
 * frame actual (mismo mecanismo ya usado para colisiones bala-alien).
 */
static void eliminar_bala_enemiga_cercana(ListaBala* lista, int x_impacto, int y_impacto) {
    if (lista == NULL) return;

    NodoBala* mas_cercana = NULL;
    int       menor_distancia = 999999;

    NodoBala* actual = lista->cabeza;
    while (actual != NULL) {
        if (actual->y >= 0) { /* ignorar las ya marcadas este frame */
            int dx = actual->x - x_impacto;
            int dy = actual->y - y_impacto;
            int distancia = dx * dx + dy * dy; /* distancia al cuadrado, evita sqrt */
            if (distancia < menor_distancia) {
                menor_distancia = distancia;
                mas_cercana = actual;
            }
        }
        actual = actual->siguiente;
    }

    /* Radio de búsqueda: 40px al cuadrado = 1600 */
    if (mas_cercana != NULL && menor_distancia <= 1600) {
        mas_cercana->y = -999;
    }
}

/**
 * @brief Parsea una línea de texto recibida del servidor y actualiza los structs locales.
 *
 * Formatos esperados:
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

    /* ------ ALIEN ------ */
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

    /* ------ JUGADOR ------ */
    else if (strcmp(tipo, "JUGADOR") == 0) {
        int id, x, vidas, puntos;
        if (sscanf(mensaje, "JUGADOR|%d|%d|%d|%d", &id, &x, &vidas, &puntos) == 4) {
            for (int i = 0; i < 2; i++) {
                if (ptr_jugadores[i].id_jugador == id) {
                    // FIX 3: aplicar siempre la posición autoritativa del servidor
                    ptr_jugadores[i].posicion_x  = x;
                    ptr_jugadores[i].vidas        = vidas;
                    ptr_jugadores[i].puntuacion   = puntos;
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

    /* ------ VELOCIDAD (informativo) ------ */
    else if (strcmp(tipo, "VELOCIDAD") == 0) {
        int vel;
        if (sscanf(mensaje, "VELOCIDAD|%d", &vel) == 1) {
            printf("[RED] Velocidad de aliens actualizada: %d\n", vel);
        }
    }

    /* ------ IMPACTO_JUGADOR ------ */
    else if (strcmp(tipo, "IMPACTO_JUGADOR") == 0) {
        int vidas;
        if (sscanf(mensaje, "IMPACTO_JUGADOR|%d", &vidas) == 1) {
            printf("[RED] Impacto recibido! Vidas restantes: %d\n", vidas);
            // Las vidas se sincronizan definitivamente por el mensaje JUGADOR siguiente
        }
    }

    /* ------ DISPARO (bala del jugador, sube) ------ */
    // FIX 1: el servidor emite DISPARO|x|y (solo 2 campos), no DISPARO|id|x|y
    else if (strcmp(tipo, "DISPARO") == 0) {
        int x, y;
        if (sscanf(mensaje, "DISPARO|%d|%d", &x, &y) == 2) {
            if (ptr_balas != NULL) {
                lista_insertar_frente(ptr_balas, x, y, VELOCIDAD_BALA);
            }
        }
    }

    /* ------ DISPARO_ENEMIGO (bala de un alien, baja) ------ */
    // Reutiliza la misma estructura ListaBala/NodoBala (requisito de uso de
    // structs), pero con velocidad NEGATIVA: lista_mover_balas() hace
    // "y -= velocidad", así que velocidad=-8 produce "y -= (-8) = y + 8",
    // es decir la bala BAJA en pantalla en vez de subir.
    else if (strcmp(tipo, "DISPARO_ENEMIGO") == 0) {
        int x, y;
        if (sscanf(mensaje, "DISPARO_ENEMIGO|%d|%d", &x, &y) == 2) {
            if (ptr_balas_enemigas != NULL) {
                lista_insertar_frente(ptr_balas_enemigas, x, y, -VELOCIDAD_BALA_ENEMIGA);
            }
        }
    }

    /* ------ BALA_ENEMIGA_DESTRUIDA ------ */
    // El servidor avisa que una bala enemiga impactó un bunker o al
    // jugador. Sin esto el cliente seguía dibujando y moviendo su copia
    // local de esa bala hasta que salía de pantalla por su cuenta, varios
    // frames después del impacto real ya resuelto en el servidor.
    else if (strcmp(tipo, "BALA_ENEMIGA_DESTRUIDA") == 0) {
        int x, y;
        if (sscanf(mensaje, "BALA_ENEMIGA_DESTRUIDA|%d|%d", &x, &y) == 2) {
            eliminar_bala_enemiga_cercana(ptr_balas_enemigas, x, y);
        }
    }

    /* ------ GAME_OVER ------ */
    else if (strcmp(tipo, "GAME_OVER") == 0) {
        printf("[RED] GAME OVER recibido del servidor.\n");
        // La GUI puede verificar las vidas para mostrar la pantalla de fin
    }

    /* ------ LIMPIAR_ALIENS ------ */
    // El servidor manda esto justo antes de transmitir una nueva ola.
    // Sin esto, los aliens de la ola anterior (con IDs ya no usados, ya que
    // el contador de ID en el servidor es global y no se reinicia por ola)
    // quedaban "fantasma" congelados en pantalla para siempre.
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
 * @brief Hilo de escucha asíncrona del servidor.
 *
 * FIX 2: usa un acumulador para ensamblar líneas completas, ya que TCP puede
 *         partir un mensaje entre dos recv() consecutivos.
 * FIX \r\n: elimina el \r antes de pasarle la línea a parsear_mensaje().
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
            printf("\n[ALERTA CRITICA] Se ha perdido la conexion con el Servidor Java.\n");
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

            // FIX \r\n: eliminar el \r residual de líneas Windows
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
    config_servidor.sin_family           = AF_INET;
    config_servidor.sin_addr.s_addr      = inet_addr(IP_SERVIDOR);
    config_servidor.sin_port             = htons(PUERTO_SERVIDOR);

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
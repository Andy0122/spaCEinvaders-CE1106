/**
 * @file estructuras_datos.c
 * @brief Implementación de la lista enlazada simple para balas activas.
 *
 * La lista enlazada permite agregar y eliminar balas en O(1) al frente,
 * y recorrerla en O(n) para actualizarlas cada frame del game loop.
 *
 */

#include <stdlib.h>
#include "structs.h"

/* --------------------------------------------------
 * lista_inicializar
 * -------------------------------------------------- */
void lista_inicializar(ListaBala* lista) {
    lista->cabeza   = NULL;
    lista->cantidad = 0;
}

/* --------------------------------------------------
 * lista_insertar_frente
 * Crea un nodo nuevo y lo enlaza al inicio de la lista.
 * Insertar al frente es O(1): no hay que recorrer nada.
 * -------------------------------------------------- */
void lista_insertar_frente(ListaBala* lista, int x, int y, int velocidad) {
    NodoBala* nuevo = (NodoBala*)malloc(sizeof(NodoBala));
    if (nuevo == NULL) return; /* fallo de memoria: se ignora el disparo */

    nuevo->x         = x;
    nuevo->y         = y;
    nuevo->velocidad = velocidad;
    nuevo->siguiente = lista->cabeza;

    lista->cabeza = nuevo;
    lista->cantidad++;
}

/* --------------------------------------------------
 * lista_mover_balas
 * Recorre la lista y resta la velocidad de cada nodo a su y,
 * haciendo que la bala suba en pantalla un frame.
 * -------------------------------------------------- */
void lista_mover_balas(ListaBala* lista) {
    NodoBala* actual = lista->cabeza;
    while (actual != NULL) {
        actual->y -= actual->velocidad;
        actual = actual->siguiente;
    }
}

/* --------------------------------------------------
 * lista_eliminar_fuera_de_pantalla
 * Recorre la lista con dos punteros (previo y actual) para
 * poder desenlazar y liberar nodos cuya y < 0 sin perder
 * el hilo de la lista.
 * -------------------------------------------------- */
void lista_eliminar_fuera_de_pantalla(ListaBala* lista) {
    NodoBala* previo = NULL;
    NodoBala* actual = lista->cabeza;

    while (actual != NULL) {
        NodoBala* siguiente = actual->siguiente;

        if (actual->y < 0) {
            /* Desenlazar el nodo */
            if (previo == NULL) {
                /* Era la cabeza */
                lista->cabeza = siguiente;
            } else {
                previo->siguiente = siguiente;
            }
            free(actual);
            lista->cantidad--;
            /* previo no cambia: el nodo anterior sigue siendo el mismo */
        } else {
            previo = actual;
        }

        actual = siguiente;
    }
}

/* --------------------------------------------------
 * lista_destruir
 * Libera todos los nodos restantes de la lista.
 * Se llama una sola vez al cerrar el programa.
 * -------------------------------------------------- */
void lista_destruir(ListaBala* lista) {
    NodoBala* actual = lista->cabeza;
    while (actual != NULL) {
        NodoBala* siguiente = actual->siguiente;
        free(actual);
        actual = siguiente;
    }
    lista->cabeza   = NULL;
    lista->cantidad = 0;
}
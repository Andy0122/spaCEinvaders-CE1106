/**
 * @file estructuras_datos.c
 * @brief Operaciones lógicas de la lista enlazada simple utilizada para memoria dinámica.
 */

#include <stdlib.h>
#include "../include/structs.h"

/**
 * @brief Establece los valores iniciales de la lista.
 * @param lista Puntero a la estructura a inicializar.
 */
void lista_inicializar(ListaBala* lista) {
    lista->cabeza   = NULL;
    lista->cantidad = 0;
}

/**
 * @brief Reserva un nuevo bloque de memoria y lo añade al inicio de la estructura enlazada.
 * @param lista Puntero a la lista.
 * @param x Coordenada X inicial.
 * @param y Coordenada Y inicial.
 * @param velocidad Escalador de avance por frame.
 */
void lista_insertar_frente(ListaBala* lista, int x, int y, int velocidad) {
    NodoBala* nuevo = (NodoBala*)malloc(sizeof(NodoBala));
    if (nuevo == NULL) return; 

    nuevo->x         = x;
    nuevo->y         = y;
    nuevo->velocidad = velocidad;
    nuevo->siguiente = lista->cabeza;

    lista->cabeza = nuevo;
    lista->cantidad++;
}

/**
 * @brief Itera la lista aplicando el escalador de velocidad a cada nodo.
 * @param lista Puntero a la lista de proyectiles a actualizar.
 */
void lista_mover_balas(ListaBala* lista) {
    NodoBala* actual = lista->cabeza;
    while (actual != NULL) {
        actual->y -= actual->velocidad;
        actual = actual->siguiente;
    }
}

/**
 * @brief Evalúa las coordenadas de cada nodo y libera aquellos marcados como inactivos o fuera del rango visible.
 * @param lista Puntero a la lista a sanear.
 */
void lista_eliminar_fuera_de_pantalla(ListaBala* lista) {
    NodoBala* previo = NULL;
    NodoBala* actual = lista->cabeza;

    while (actual != NULL) {
        NodoBala* siguiente = actual->siguiente;

        if (actual->y < 0) {
            if (previo == NULL) {
                lista->cabeza = siguiente;
            } else {
                previo->siguiente = siguiente;
            }
            free(actual);
            lista->cantidad--;
        } else {
            previo = actual;
        }

        actual = siguiente;
    }
}

/**
 * @brief Recorre la estructura completa liberando su bloque de memoria asignado.
 * @param lista Puntero a la estructura a destruir.
 */
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
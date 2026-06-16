package estructuras_datos;

import java.util.function.Consumer;

/**
 * @class ListaEnlazada
 * @brief Estructura de datos dinámica de enlace simple.
 * Implementa operaciones optimizadas de inserción en cabecera O(1) e iteración lineal O(n).
 * @param <T> Tipo de dato genérico a almacenar.
 */
public class ListaEnlazada<T> {

    private Nodo<T> cabeza;
    private int cantidad;

    public ListaEnlazada() {
        this.cabeza = null;
        this.cantidad = 0;
    }

    /**
     * @brief Inserta un nuevo elemento en la cabecera de la estructura.
     * @param dato Elemento a insertar.
     */
    public void insertarFrente(T dato) {
        Nodo<T> nuevo = new Nodo<>(dato);
        nuevo.setSiguiente(cabeza);
        cabeza = nuevo;
        cantidad++;
    }

    /**
     * @brief Localiza y remueve la primera coincidencia del elemento especificado.
     * @param dato Referencia del elemento a remover.
     * @return true si la operación fue exitosa, false si el elemento no fue encontrado.
     */
    public boolean eliminar(T dato) {
        Nodo<T> previo = null;
        Nodo<T> actual = cabeza;

        while (actual != null) {
            if (actual.getDato() == dato || actual.getDato().equals(dato)) {
                if (previo == null) {
                    cabeza = actual.getSiguiente();
                } else {
                    previo.setSiguiente(actual.getSiguiente());
                }
                cantidad--;
                return true;
            }
            previo = actual;
            actual = actual.getSiguiente();
        }
        return false;
    }

    /**
     * @brief Ejecuta una operación definida sobre cada elemento de la estructura.
     * Protege la integridad de la iteración en caso de modificaciones concurrentes de la cabecera.
     * @param accion Interfaz funcional a ejecutar por cada nodo.
     */
    public void forEach(Consumer<T> accion) {
        Nodo<T> actual = cabeza;
        while (actual != null) {
            Nodo<T> siguiente = actual.getSiguiente();
            accion.accept(actual.getDato());
            actual = siguiente;
        }
    }

    /**
     * @brief Transforma la estructura dinámica en un arreglo estático de referencias.
     * @return Arreglo primitivo que contiene los elementos de la lista.
     */
    @SuppressWarnings("unchecked")
    public T[] toArray() {
        Object[] arr = new Object[cantidad];
        Nodo<T> actual = cabeza;
        int i = 0;
        while (actual != null) {
            arr[i++] = actual.getDato();
            actual = actual.getSiguiente();
        }
        return (T[]) arr;
    }

    public boolean estaVacia() {
        return cabeza == null;
    }

    public int getCantidad() {
        return cantidad;
    }

    public void limpiar() {
        cabeza = null;
        cantidad = 0;
    }
}
package estructuras_datos;

import java.util.function.Consumer;

/**
 * Lista enlazada simple genérica.
 * Se utiliza en el motor del juego para gestionar las balas enemigas activas:
 * inserción al frente en O(1) y recorrido en O(n) para actualizar posiciones.
 *
 * Patrón aplicado: Iterator implícito mediante el método forEach con Consumer<T>.
 */
public class ListaEnlazada<T> {

    private Nodo<T> cabeza;
    private int cantidad;

    public ListaEnlazada() {
        this.cabeza = null;
        this.cantidad = 0;
    }

    // ------------------------------------------------------------------
    // Inserción al frente — O(1)
    // ------------------------------------------------------------------
    public void insertarFrente(T dato) {
        Nodo<T> nuevo = new Nodo<>(dato);
        nuevo.setSiguiente(cabeza);
        cabeza = nuevo;
        cantidad++;
    }

    // ------------------------------------------------------------------
    // Eliminar por referencia exacta al dato — O(n)
    // ------------------------------------------------------------------
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

    // ------------------------------------------------------------------
    // Recorrido con Consumer<T> — permite lambdas limpias
    // ------------------------------------------------------------------
    public void forEach(Consumer<T> accion) {
        Nodo<T> actual = cabeza;
        while (actual != null) {
            // Guardar siguiente ANTES de ejecutar la acción, por si el Consumer
            // modifica la lista internamente
            Nodo<T> siguiente = actual.getSiguiente();
            accion.accept(actual.getDato());
            actual = siguiente;
        }
    }

    // ------------------------------------------------------------------
    // Obtener todos los datos como arreglo (útil para iteración con remove)
    // ------------------------------------------------------------------
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
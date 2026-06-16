package estructuras_datos;

/**
 * @class Nodo
 * @brief Unidad básica de almacenamiento para la estructura ListaEnlazada.
 * @param <T> Tipo de dato que almacena el nodo.
 */
public class Nodo<T> {
    private T dato;
    private Nodo<T> siguiente;

    /**
     * @brief Instancia un nuevo bloque de memoria con el dato especificado.
     * @param dato Elemento a almacenar.
     */
    public Nodo(T dato) {
        this.dato = dato;
        this.siguiente = null;
    }

    public T getDato() { return dato; }
    public void setDato(T dato) { this.dato = dato; }
    public Nodo<T> getSiguiente() { return siguiente; }
    public void setSiguiente(Nodo<T> siguiente) { this.siguiente = siguiente; }
}
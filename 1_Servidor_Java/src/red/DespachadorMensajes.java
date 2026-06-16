package red;

import java.io.PrintWriter;
import java.util.Collections;
import java.util.HashSet;
import java.util.Map;
import java.util.Set;
import java.util.concurrent.ConcurrentHashMap;

/**
 * @class DespachadorMensajes
 * @brief Gestiona el enrutamiento y la distribución de mensajes a los clientes conectados.
 * Actúa como el canal de comunicación centralizado para el patrón Observer.
 */
public class DespachadorMensajes {
    
    private static final Map<Integer, Set<PrintWriter>> salas = new ConcurrentHashMap<>();

    /**
     * @brief Registra un nuevo flujo de salida de cliente en una sala específica.
     * @param idPartida Identificador numérico de la sala.
     * @param cliente Flujo de salida asociado al socket del cliente.
     */
    public static void registrar(int idPartida, PrintWriter cliente) {
        salas.computeIfAbsent(idPartida, k -> Collections.synchronizedSet(new HashSet<>())).add(cliente);
        System.out.println("INFO [DespachadorMensajes]: Flujo de cliente añadido a la sala " + idPartida);
    }

    /**
     * @brief Remueve un flujo de cliente específico de una sala.
     * @param idPartida Identificador numérico de la sala.
     * @param cliente Flujo de salida a remover.
     */
    public static void remover(int idPartida, PrintWriter cliente) {
        Set<PrintWriter> sala = salas.get(idPartida);
        if (sala != null) {
            sala.remove(cliente);
        }
    }

    /**
     * @brief Transmite un mensaje de texto plano a todos los clientes registrados en una sala.
     * @param idPartida Identificador numérico de la sala.
     * @param mensaje Cadena de caracteres a transmitir.
     */
    public static void broadcast(int idPartida, String mensaje) {
        Set<PrintWriter> clientesSala = salas.get(idPartida);
        if (clientesSala != null) {
            synchronized (clientesSala) {
                for (PrintWriter cliente : clientesSala) {
                    cliente.println(mensaje); // Envía el texto plano para que C lo parsee
                }
            }
        }
    }
    
    /**
     * @brief Verifica si una sala carece de clientes conectados.
     * @param idPartida Identificador numérico de la sala.
     * @return true si la sala está vacía o no existe, false en caso contrario.
     */
    public static boolean estaSalaVacia(int idPartida) {
        Set<PrintWriter> sala = salas.get(idPartida);
        return sala == null || sala.isEmpty();
    }
}
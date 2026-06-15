package red;

import java.io.PrintWriter;
import java.util.Collections;
import java.util.HashSet;
import java.util.Map;
import java.util.Set;
import java.util.concurrent.ConcurrentHashMap;

public class DespachadorMensajes {
    // Mapa: ID_Partida -> Lista de PrintWriters (Clientes en esa sala)
    private static final Map<Integer, Set<PrintWriter>> salas = new ConcurrentHashMap<>();

    public static void registrar(int idPartida, PrintWriter cliente) {
        salas.computeIfAbsent(idPartida, k -> Collections.synchronizedSet(new HashSet<>())).add(cliente);
        System.out.println("[RED] Cliente añadido a la sala " + idPartida);
    }

    public static void remover(int idPartida, PrintWriter cliente) {
        Set<PrintWriter> sala = salas.get(idPartida);
        if (sala != null) {
            sala.remove(cliente);
        }
    }

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
}
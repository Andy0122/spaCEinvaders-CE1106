package red;

import java.io.PrintWriter;
import java.util.Collections;
import java.util.HashSet;
import java.util.Set;

public class DespachadorMensajes {
    private static final Set<PrintWriter> clientes = Collections.synchronizedSet(new HashSet<>());

    public static void registrar(PrintWriter cliente) {
        clientes.add(cliente);
    }

    public static void remover(PrintWriter cliente) {
        clientes.remove(cliente);
    }

    public static void broadcast(String mensaje) {
        synchronized (clientes) {
            for (PrintWriter cliente : clientes) {
                cliente.println(mensaje);
            }
        }
    }
}

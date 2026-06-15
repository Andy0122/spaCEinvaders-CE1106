import java.io.IOException;
import java.net.Socket;
import red.HiloCliente;
import red.ManejadorSockets;

/**
 * Punto de entrada del servidor spaCEinvaders.
 * Delega la gestión del ServerSocket a ManejadorSockets y lanza un HiloCliente
 * por cada conexión entrante.
 */
public class MainServidor {

    private static final int PUERTO = 8080;

    public static void main(String[] args) {
        System.out.println("[INFO] Iniciando Servidor spaCEinvaders en el puerto " + PUERTO + "...");

        ManejadorSockets manejador = new ManejadorSockets(PUERTO);
        try {
            manejador.iniciar();
            System.out.println("[INFO] Servidor en línea. Esperando conexiones de clientes...");

            while (manejador.estaActivo()) {
                Socket socketCliente = manejador.aceptarCliente();
                System.out.println("[CONEXIÓN] Cliente desde IP: " + socketCliente.getInetAddress().getHostAddress());
                Thread hilo = new Thread(new HiloCliente(socketCliente));
                hilo.start();
            }
        } catch (IOException e) {
            System.err.println("[ERROR CRÍTICO] Fallo en el servidor: " + e.getMessage());
            e.printStackTrace();
        } finally {
            manejador.cerrar();
        }
    }
}
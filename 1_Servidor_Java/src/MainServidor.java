import java.io.IOException;
import java.net.ServerSocket;
import java.net.Socket;

// IMPORTANTE: Importamos la clase HiloCliente desde el paquete de red
import red.HiloCliente; 

/**
 * Clase principal del Servidor de spaCEinvaders.
 * Implementa la arquitectura de servidor concurrente mediante ServerSocket.
 * Se encarga de escuchar peticiones de conexión en un puerto específico y 
 * delegar cada nueva conexión a un hilo independiente (Thread), permitiendo 
 * múltiples jugadores y espectadores simultáneos.
 */
public class MainServidor {
    
    /** Puerto de escucha para las conexiones TCP/IP de los clientes. */
    private static final int PUERTO = 5000;

    /**
     * Método de entrada principal del servidor.
     * 
     * @param args Argumentos de la línea de comandos (no utilizados).
     */
    public static void main(String[] args) {
        System.out.println("[INFO] Iniciando Servidor spaCEinvaders en el puerto " + PUERTO + "...");

        // Utilizamos try-with-resources para garantizar el cierre del ServerSocket
        try (ServerSocket serverSocket = new ServerSocket(PUERTO)) {
            System.out.println("[INFO] Servidor en línea. Esperando conexiones de clientes...");

            // Ciclo de escucha infinito para aceptar conexiones entrantes
            while (true) {
                // El hilo principal se bloquea aquí hasta que un cliente intente conectarse
                Socket socketCliente = serverSocket.accept(); 
                System.out.println("[CONEXIÓN] Nuevo cliente aceptado desde IP: " + socketCliente.getInetAddress().getHostAddress());

                // Se instancia y arranca un nuevo hilo dedicado exclusivamente a este cliente
                HiloCliente gestorCliente = new HiloCliente(socketCliente);
                Thread hiloDeEjecucion = new Thread(gestorCliente);
                hiloDeEjecucion.start();
            }

        } catch (IOException e) {
            System.err.println("[ERROR CRÍTICO] Fallo en la inicialización del servidor: " + e.getMessage());
            e.printStackTrace();
        }
    }
}
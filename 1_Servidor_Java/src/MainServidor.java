import java.io.IOException;
import java.net.Socket;
import red.HiloCliente;
import red.ManejadorSockets;

/**
 * @class MainServidor
 * @brief Punto de entrada principal para el componente servidor de la aplicación.
 * Orquesta la apertura del puerto de red y la asignación de hilos concurrentes 
 * para la atención de clientes.
 */
public class MainServidor {

    private static final int PUERTO = 8080;

    public static void main(String[] args) {
        System.out.println("INFO [MainServidor]: Inicializando el entorno de servidor en el puerto " + PUERTO + "...");

        ManejadorSockets manejador = new ManejadorSockets(PUERTO);
        try {
            manejador.iniciar();
            System.out.println("INFO [MainServidor]: Servicio activo. Aceptando peticiones entrantes...");

            while (manejador.estaActivo()) {
                Socket socketCliente = manejador.aceptarCliente();
                System.out.println("INFO [MainServidor]: Petición de conexión entrante resuelta desde IP -> " + socketCliente.getInetAddress().getHostAddress());
                
                Thread hilo = new Thread(new HiloCliente(socketCliente));
                hilo.start();
            }
        } catch (IOException e) {
            System.err.println("ERROR FATAL [MainServidor]: Falla general en la capa de red -> " + e.getMessage());
            e.printStackTrace();
        } finally {
            manejador.cerrar();
        }
    }
}
package red;

import java.io.IOException;
import java.net.ServerSocket;
import java.net.Socket;

/**
 * @class ManejadorSockets
 * @brief Abstracción del servidor TCP para la gestión de conexiones entrantes.
 */
public class ManejadorSockets {

    private final int puerto;
    private ServerSocket serverSocket;

    /**
     * @brief Instancia un nuevo manejador de sockets.
     * @param puerto Número de puerto a vincular.
     */
    public ManejadorSockets(int puerto) {
        this.puerto = puerto;
    }

    /**
     * @brief Inicializa el listener TCP.
     * @throws IOException Si el puerto ya se encuentra en uso.
     */
    public void iniciar() throws IOException {
        serverSocket = new ServerSocket(puerto);
        System.out.println("INFO [ManejadorSockets]: Escuchando peticiones en el puerto " + puerto);
    }

    /**
     * @brief Detiene la ejecución hasta recibir una nueva petición de conexión.
     * @return Socket que representa la conexión con el cliente.
     * @throws IOException Si ocurre un error de E/S durante la espera.
     */
    public Socket aceptarCliente() throws IOException {
        return serverSocket.accept();
    }

    /**
     * @brief Destruye el listener TCP y libera el puerto asignado.
     */
    public void cerrar() {
        try {
            if (serverSocket != null && !serverSocket.isClosed()) {
                serverSocket.close();
            }
        } catch (IOException e) {
            System.err.println("ERROR [ManejadorSockets]: Fallo crítico al cerrar el ServerSocket -> " + e.getMessage());
        }
    }

    public boolean estaActivo() {
        return serverSocket != null && !serverSocket.isClosed();
    }
}
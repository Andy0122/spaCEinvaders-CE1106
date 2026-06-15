package red;

import java.io.IOException;
import java.net.ServerSocket;
import java.net.Socket;

/**
 * ManejadorSockets encapsula la lógica de aceptación de conexiones del ServerSocket.
 * MainServidor lo utiliza para separar la responsabilidad de apertura de puerto
 * del ciclo principal de negocio.
 */
public class ManejadorSockets {

    private final int puerto;
    private ServerSocket serverSocket;

    public ManejadorSockets(int puerto) {
        this.puerto = puerto;
    }

    /** Abre el ServerSocket en el puerto configurado. */
    public void iniciar() throws IOException {
        serverSocket = new ServerSocket(puerto);
        System.out.println("[INFO] ManejadorSockets escuchando en puerto " + puerto);
    }

    /**
     * Bloquea hasta que un cliente se conecte y devuelve su socket.
     * @return Socket del cliente aceptado.
     */
    public Socket aceptarCliente() throws IOException {
        return serverSocket.accept();
    }

    /** Cierra el ServerSocket liberando el puerto. */
    public void cerrar() {
        try {
            if (serverSocket != null && !serverSocket.isClosed()) {
                serverSocket.close();
            }
        } catch (IOException e) {
            System.err.println("[ERROR] No se pudo cerrar el ServerSocket: " + e.getMessage());
        }
    }

    public boolean estaActivo() {
        return serverSocket != null && !serverSocket.isClosed();
    }
}
package red;

import java.io.BufferedReader;
import java.io.IOException;
import java.io.InputStreamReader;
import java.io.PrintWriter;
import java.net.Socket;

import modelo.Juego;
import red.DespachadorMensajes;

/**
 * Hilo de ejecución concurrente dedicado a gestionar el ciclo de vida y la 
 * comunicación bidireccional de un cliente individual (Jugador o Espectador).
 * Implementa la interfaz Runnable para evitar el bloqueo del hilo principal del servidor.
 */
public class HiloCliente implements Runnable {
    
    /** Socket de conexión punto a punto con el cliente en C. */
    private Socket socket;
    
    /** Flujo de salida de datos (del Servidor hacia el Cliente). */
    private PrintWriter flujoSalida;
    
    /** Flujo de entrada de datos (del Cliente hacia el Servidor). */
    private BufferedReader flujoEntrada;

    /**
     * Constructor de la clase HiloCliente.
     * 
     * @param socket Socket devuelto por el ServerSocket al aceptar la conexión.
     */
    public HiloCliente(Socket socket) {
        this.socket = socket;
    }

    /**
     * Método sobreescrito de la interfaz Runnable.
     * Contiene el ciclo de vida de la comunicación: Inicialización, Handshake 
     * (identidad del cliente), ciclo de escucha de comandos y cierre seguro.
     */
    @Override
    public void run() {
        try {
            // 1. Inicialización de los flujos de comunicación
            flujoEntrada = new BufferedReader(new InputStreamReader(socket.getInputStream()));
            // El parámetro 'true' habilita el auto-flush (envío inmediato de datos)
            flujoSalida = new PrintWriter(socket.getOutputStream(), true); 
            DespachadorMensajes.registrar(flujoSalida);

            // 2. Handshake (Protocolo de identificación inicial)
            // Se solicita al cliente que envíe su rol (JUGADOR, ADMIN o ESPECTADOR)
            flujoSalida.println("IDENTIFICATE");
            String rolCliente = flujoEntrada.readLine();
            
            System.out.println("[PROTOCOLO] Cliente " + socket.getInetAddress().getHostAddress() + " registrado como: " + rolCliente);

            // 3. Ciclo principal de escucha de mensajes del cliente
            String mensajeRecibido;
            while ((mensajeRecibido = flujoEntrada.readLine()) != null) {
                System.out.println("[RECV - " + rolCliente + "] " + mensajeRecibido);
                String respuesta = Juego.getInstancia().procesarComando(rolCliente, mensajeRecibido);
                flujoSalida.println(respuesta);
                DespachadorMensajes.broadcast("ESTADO|" + Juego.getInstancia().obtenerResumenEstado());
            }

        } catch (IOException e) {
            System.err.println("[DESCONEXIÓN ABRUPTA] El cliente cerró la conexión inesperadamente. Detalles: " + e.getMessage());
        } finally {
            DespachadorMensajes.remover(flujoSalida);
            // 4. Limpieza y cierre seguro de recursos (Vital para evitar fugas de memoria)
            cerrarConexion();
        }
    }

    /**
     * Método auxiliar para garantizar el cierre correcto del socket y sus flujos,
     * liberando los recursos del sistema operativo.
     */
    private void cerrarConexion() {
        try {
            if (flujoEntrada != null) flujoEntrada.close();
            if (flujoSalida != null) flujoSalida.close();
            if (socket != null && !socket.isClosed()) socket.close();
            System.out.println("[INFO] Recursos del cliente liberados correctamente.");
        } catch (IOException e) {
            System.err.println("[ERROR] No se pudo cerrar el socket adecuadamente: " + e.getMessage());
        }
    }
}
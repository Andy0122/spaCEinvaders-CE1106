package red;

import java.io.BufferedReader;
import java.io.IOException;
import java.io.InputStreamReader;
import java.io.PrintWriter;
import java.net.Socket;
import modelo.GestorPartidas;
import modelo.Juego;

/**
 * Hilo de ejecución concurrente dedicado a gestionar el ciclo de vida y la 
 * comunicación bidireccional de un cliente individual (Jugador o Espectador).
 */
public class HiloCliente implements Runnable {
    
    private Socket socket;
    private PrintWriter flujoSalida;
    private BufferedReader flujoEntrada;

    public HiloCliente(Socket socket) {
        this.socket = socket;
    }

    @Override
    public void run() {
        try {
            // Inicialización de los flujos de lectura/escritura del Socket
            this.flujoSalida = new PrintWriter(socket.getOutputStream(), true);
            this.flujoEntrada = new BufferedReader(new InputStreamReader(socket.getInputStream()));

            // Le enviamos la palabra clave que C está esperando para responder
            flujoSalida.println("IDENTIFICATE");
            
            // 1. Leer el Handshake de C
            String handshake = flujoEntrada.readLine();
            if (handshake == null) return;
            
            // Ej: "ESPECTADOR|2" o "JUGADOR|1"
            String[] datosHandshake = handshake.split("\\|");
            String rol = datosHandshake[0].trim();
            int idPartida = Integer.parseInt(datosHandshake[1].trim());

            // 2. Registrar en la sala correcta
            DespachadorMensajes.registrar(idPartida, flujoSalida);
            
            // 3. Obtener el Motor de Juego de esa sala
            Juego miPartida = GestorPartidas.obtenerOCrearPartida(idPartida);
            System.out.println("[CONEXION] Nuevo " + rol + " conectado a Sala " + idPartida);

            // 4. Ciclo de escucha de comandos (Las pulsaciones de teclas/ESP8266)
            String mensaje;
            while ((mensaje = flujoEntrada.readLine()) != null) {
                // El espectador NO puede mandar comandos al motor
                if (!rol.equals("ESPECTADOR")) {
                    miPartida.procesarComando(rol, mensaje);
                }
            }
        } catch (Exception e) {
            System.out.println("[DESCONEXION] Cliente perdió conexión.");
        } finally {
            cerrarConexion();
        }
    }

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
package modelo;

import java.util.concurrent.ConcurrentHashMap;

/**
 * @class GestorPartidas
 * @brief Administrador concurrente de sesiones de juego.
 * Gestiona el ciclo de vida y almacenamiento en memoria de múltiples instancias de partidas.
 */
public class GestorPartidas {
    private static final ConcurrentHashMap<Integer, Juego> partidas = new ConcurrentHashMap<>();

    /**
     * @brief Recupera una instancia de juego existente o inicializa una nueva si no existe.
     * @param idPartida Identificador numérico de la sala.
     * @return Referencia a la instancia de la partida.
     */
    public static Juego obtenerOCrearPartida(int idPartida) {
        return partidas.computeIfAbsent(idPartida, k -> {
            Juego nuevoJuego = new Juego(idPartida);
            nuevoJuego.iniciarJuego(); // Inicia la partida automáticamente al crearse
            System.out.println("INFO [GestorPartidas]: Instancia de juego inicializada para la sala " + idPartida);
            return nuevoJuego;
        });
    }

    /**
     * @brief Verifica la existencia de una partida en el registro activo.
     * @param idPartida Identificador numérico de la sala.
     * @return true si la partida está activa en memoria.
     */
    public static boolean existePartida(int idPartida) {
        return partidas.containsKey(idPartida);
    }

    /**
     * @brief Recupera una partida existente sin invocar su creación.
     * @param idPartida Identificador numérico de la sala.
     * @return Referencia a la instancia, o null si no se encuentra.
     */
    public static Juego obtenerPartidaExistente(int idPartida) {
        return partidas.get(idPartida);
    }
}
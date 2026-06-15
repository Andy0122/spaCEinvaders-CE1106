package modelo;

import java.util.concurrent.ConcurrentHashMap;

public class GestorPartidas {
    // Diccionario que guarda: ID_Partida -> Universo del Juego
    private static final ConcurrentHashMap<Integer, Juego> partidas = new ConcurrentHashMap<>();

    /**
     * Obtiene la instancia de un juego específico. Si no existe, la crea.
     */
    public static Juego obtenerOCrearPartida(int idPartida) {
        return partidas.computeIfAbsent(idPartida, k -> {
            Juego nuevoJuego = new Juego(idPartida);
            nuevoJuego.iniciarJuego(); // Inicia la partida automáticamente al crearse
            System.out.println("[GESTOR] Sala " + idPartida + " creada e inicializada.");
            return nuevoJuego;
        });
    }
}
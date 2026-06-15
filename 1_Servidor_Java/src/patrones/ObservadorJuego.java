package patrones;

import red.DespachadorMensajes;

/**
 * Patrón Observer: Centraliza todas las notificaciones broadcast del juego.
 * Ha sido adaptado para soportar múltiples partidas (Salas de Juego) simultáneas.
 *
 * Formato de mensajes de salida que espera el cliente en C (A5):
 * ALIEN|id|x|y|estado        (estado: 1=vivo, 0=destruido)
 * JUGADOR|id|x|vidas|puntos
 * BUNKER|id|salud
 */
public class ObservadorJuego {

    // ==========================================
    // NOTIFICACIONES DE EXTRATERRESTRES
    // ==========================================

    public static void notificarCreacionAlien(int idPartida, int idAlien, int tipo, int x, int y, int velocidad) {
        // Para el cliente en C, lo importante es la posición inicial y que está vivo (estado 1)
        String msj = "ALIEN|" + idAlien + "|" + x + "|" + y + "|1";
        DespachadorMensajes.broadcast(idPartida, msj);
    }

    public static void notificarMovimientoEnemigo(int idPartida, int idEnemigo, int x, int y, int estado) {
        String msj = "ALIEN|" + idEnemigo + "|" + x + "|" + y + "|" + estado;
        DespachadorMensajes.broadcast(idPartida, msj);
    }

    public static void notificarEnemigoDestruido(int idPartida, int idAlien, int puntosOtorgados, int puntuacionTotal) {
        // Al enviar estado = 0 o coordenadas en 0, el cliente C sabe que debe ocultarlo/borrarlo
        String msj = "ALIEN|" + idAlien + "|0|0|0";
        DespachadorMensajes.broadcast(idPartida, msj);
    }

    public static void notificarVelocidadEnemigos(int idPartida, int velocidad) {
        // Comando futuro para el motor
        String msj = "VELOCIDAD|" + velocidad;
        DespachadorMensajes.broadcast(idPartida, msj);
    }

    // ==========================================
    // NOTIFICACIONES DEL OVNI ALEATORIO
    // ==========================================

    public static void notificarCreacionOvni(int idPartida, int idOvni, int x, int y, int velocidad, int puntos) {
        String msj = "OVNI|" + idOvni + "|" + x + "|" + y + "|" + velocidad + "|" + puntos;
        DespachadorMensajes.broadcast(idPartida, msj);
    }

    // ==========================================
    // NOTIFICACIONES DEL JUGADOR
    // ==========================================

    public static void notificarJugadorMovido(int idPartida, int idJugador, int x, int vidas, int puntos) {
        String msj = "JUGADOR|" + idJugador + "|" + x + "|" + vidas + "|" + puntos;
        DespachadorMensajes.broadcast(idPartida, msj);
    }

    public static void notificarVidasJugador(int idPartida, int idJugador, int x, int vidas, int puntos) {
        String msj = "JUGADOR|" + idJugador + "|" + x + "|" + vidas + "|" + puntos;
        DespachadorMensajes.broadcast(idPartida, msj);
    }

    public static void notificarImpactoJugador(int idPartida, int vidasRestantes) {
        String msj = "IMPACTO_JUGADOR|" + vidasRestantes;
        DespachadorMensajes.broadcast(idPartida, msj);
    }

    public static void notificarDisparoJugador(int idPartida, int x, int y) {
        String msj = "DISPARO|" + x + "|" + y;
        DespachadorMensajes.broadcast(idPartida, msj);
    }

    // ==========================================
    // NOTIFICACIONES DE BUNKERS (ESCUDOS)
    // ==========================================

    public static void notificarBunkerActualizado(int idPartida, int idBunker, int salud) {
        String msj = "BUNKER|" + idBunker + "|" + salud;
        DespachadorMensajes.broadcast(idPartida, msj);
    }
}
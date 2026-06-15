package patrones;

import red.DespachadorMensajes;

/**
 * Centraliza todas las notificaciones broadcast del juego.
 *
 * Formato de mensajes de salida (especificado en §4):
 *   ALIEN|id|x|y|estado        (estado: 1=vivo, 0=destruido)
 *   JUGADOR|id|x|vidas|puntos
 *   BUNKER|id|salud
 *   OVNI|id|x|y|velocidad|puntosExtra
 *   DISPARO|JUGADOR|x|y
 *   VELOCIDAD_ENEMIGOS|valor
 */
public class ObservadorJuego {

    /** Informa la nueva posición del jugador. */
    public static void notificarJugadorMovido(int id, int x, int vidas, int puntos) {
        DespachadorMensajes.broadcast("JUGADOR|" + id + "|" + x + "|" + vidas + "|" + puntos);
    }

    /** Informa la posición actualizada de un enemigo (alien u ovni). */
    public static void notificarMovimientoEnemigo(int id, int x, int y, int estado) {
        DespachadorMensajes.broadcast("ALIEN|" + id + "|" + x + "|" + y + "|" + estado);
    }

    /** Informa que el jugador disparó. */
    public static void notificarDisparoJugador(int x, int y) {
        DespachadorMensajes.broadcast("DISPARO|JUGADOR|" + x + "|" + y);
    }

    /** Informa la creación de un alien (estado=1: vivo). */
    public static void notificarCreacionAlien(int id, int tipo, int x, int y, int velocidad) {
        DespachadorMensajes.broadcast("ALIEN|" + id + "|" + x + "|" + y + "|1");
    }

    /** Informa la creación de un ovni. */
    public static void notificarCreacionOvni(int id, int x, int y, int velocidad, int puntosExtra) {
        DespachadorMensajes.broadcast("OVNI|" + id + "|" + x + "|" + y + "|" + velocidad + "|" + puntosExtra);
    }

    /**
     * Informa que un alien fue destruido (estado=0).
     * También difunde la puntuación actualizada del jugador.
     */
    public static void notificarEnemigoDestruido(int id, int puntos, int puntuacionActual) {
        // Estado 0 = destruido, posición 0,0 indica que ya no existe en el tablero
        DespachadorMensajes.broadcast("ALIEN|" + id + "|0|0|0");
    }

    /** Informa que el jugador recibió un impacto (vidas restantes). */
    public static void notificarImpactoJugador(int vidas) {
        DespachadorMensajes.broadcast("IMPACTO_JUGADOR|" + vidas);
    }

    /** Informa las vidas y puntos actuales del jugador. */
    public static void notificarVidasJugador(int id, int x, int vidas, int puntos) {
        DespachadorMensajes.broadcast("JUGADOR|" + id + "|" + x + "|" + vidas + "|" + puntos);
    }

    /** Informa el estado de salud de un bunker. */
    public static void notificarBunkerActualizado(int id, int salud) {
        DespachadorMensajes.broadcast("BUNKER|" + id + "|" + salud);
    }

    /** Informa la velocidad actual de los enemigos. */
    public static void notificarVelocidadEnemigos(int velocidad) {
        DespachadorMensajes.broadcast("VELOCIDAD_ENEMIGOS|" + velocidad);
    }
}
package patrones;

import red.DespachadorMensajes;

/**
 * Patrón Observer: centraliza todas las notificaciones broadcast del juego.
 *
 * Mensajes que el cliente C espera:
 *   ALIEN|id|tipo|x|y|estado     (tipo: 10=calamar, 20=cangrejo, 40=pulpo)
 *   JUGADOR|id|x|vidas|puntos
 *   BUNKER|id|salud
 *   OVNI|id|x|y|velocidad|pts    (creación)
 *   OVNI|id|x|y|1                (movimiento activo)
 *   OVNI|id|x|y|0                (fuera de pantalla / destruido)
 *   VELOCIDAD|valor
 *   IMPACTO_JUGADOR|vidas
 *   DISPARO|x|y
 *   GAME_OVER
 *
 * CAMBIO: el mensaje ALIEN ahora incluye el tipo como segundo campo:
 *   ALIEN|id|tipo|x|y|estado
 * Esto permite al cliente C dibujar el sprite correcto (pulpo/cangrejo/calamar)
 * y calcular las colisiones con el hitbox exacto del sprite.
 */
public class ObservadorJuego {

    // ── Extraterrestres ───────────────────────────────────────────────────────

    /**
     * Convierte tipo interno (1/2/3) a puntos visuales (10/20/40)
     * que el cliente C usa para seleccionar el sprite.
     */
    private static int tipoPuntos(int tipo) {
        switch (tipo) {
            case 1: return 10;  // Calamar
            case 2: return 20;  // Cangrejo
            case 3: return 40;  // Pulpo
            default: return 10;
        }
    }

    public static void notificarCreacionAlien(int idPartida, int idAlien, int tipo, int x, int y, int velocidad) {
        // Incluir tipo en el mensaje para que el cliente sepa qué sprite usar
        String msj = "ALIEN|" + idAlien + "|" + tipoPuntos(tipo) + "|" + x + "|" + y + "|1";
        DespachadorMensajes.broadcast(idPartida, msj);
    }

    public static void notificarMovimientoEnemigo(int idPartida, int idEnemigo, int tipo, int x, int y, int estado) {
        String msj = "ALIEN|" + idEnemigo + "|" + tipoPuntos(tipo) + "|" + x + "|" + y + "|" + estado;
        DespachadorMensajes.broadcast(idPartida, msj);
    }

    public static void notificarEnemigoDestruido(int idPartida, int idAlien, int puntosOtorgados, int puntuacionTotal) {
        String msj = "ALIEN|" + idAlien + "|0|0|0|0";
        DespachadorMensajes.broadcast(idPartida, msj);
    }

    public static void notificarVelocidadEnemigos(int idPartida, int velocidad) {
        String msj = "VELOCIDAD|" + velocidad;
        DespachadorMensajes.broadcast(idPartida, msj);
    }

    // ── OVNI ──────────────────────────────────────────────────────────────────

    public static void notificarCreacionOvni(int idPartida, int idOvni, int x, int y, int velocidad, int puntos) {
        String msj = "OVNI|" + idOvni + "|" + x + "|" + y + "|" + velocidad + "|" + puntos;
        DespachadorMensajes.broadcast(idPartida, msj);
    }

    public static void notificarMovimientoOvni(int idPartida, int idOvni, int x, int y) {
        String msj = "OVNI|" + idOvni + "|" + x + "|" + y + "|1";
        DespachadorMensajes.broadcast(idPartida, msj);
    }

    public static void notificarOvniDestruido(int idPartida, int idOvni) {
        String msj = "OVNI|" + idOvni + "|0|0|0";
        DespachadorMensajes.broadcast(idPartida, msj);
    }

    // ── Jugador ───────────────────────────────────────────────────────────────

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

    /**
     * Notifica el disparo de un alien (bala descendente). Sin esto el
     * cliente nunca dibuja la bala que daña bunkers/jugador: el impacto
     * parecía "mágico" porque la causa (la bala bajando) era invisible.
     */
    public static void notificarDisparoEnemigo(int idPartida, int x, int y) {
        String msj = "DISPARO_ENEMIGO|" + x + "|" + y;
        DespachadorMensajes.broadcast(idPartida, msj);
    }

    /**
     * Notifica que una bala enemiga fue destruida (impactó un bunker o al
     * jugador) en la posición (x,y) donde ocurrió la colisión. Sin esto el
     * servidor eliminaba la bala internamente pero el cliente seguía
     * dibujándola y moviéndola hasta que salía de pantalla por su cuenta,
     * varios frames después del impacto real.
     */
    public static void notificarBalaEnemigaDestruida(int idPartida, int x, int y) {
        String msj = "BALA_ENEMIGA_DESTRUIDA|" + x + "|" + y;
        DespachadorMensajes.broadcast(idPartida, msj);
    }

    // ── Bunkers ───────────────────────────────────────────────────────────────

    public static void notificarBunkerActualizado(int idPartida, int idBunker, int salud) {
        String msj = "BUNKER|" + idBunker + "|" + salud;
        DespachadorMensajes.broadcast(idPartida, msj);
    }

    // ── Limpieza de aliens (nueva ola) ──────────────────────────────────────────

    /**
     * Indica al cliente que debe vaciar TODOS los slots de aliens antes de
     * recibir la nueva ola. Sin esto, los IDs nuevos de cada ola no
     * coinciden con los slots usados por la ola anterior (el contador de
     * ID de Extraterrestre es global y nunca se reinicia), dejando sprites
     * "fantasma" inmóviles en pantalla de oleadas previas.
     */
    public static void notificarLimpiarAliens(int idPartida) {
        DespachadorMensajes.broadcast(idPartida, "LIMPIAR_ALIENS");
    }

    // ── Game Over ─────────────────────────────────────────────────────────────

    public static void notificarGameOver(int idPartida) {
        DespachadorMensajes.broadcast(idPartida, "GAME_OVER");
    }

    public static void notificarBalaJugadorDestruida(int idPartida, int x, int y) {
        String msj = "BALA_JUGADOR_DESTRUIDA|" + x + "|" + y;
        DespachadorMensajes.broadcast(idPartida, msj);
    }

}
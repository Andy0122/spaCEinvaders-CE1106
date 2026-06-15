package modelo;

import java.util.ArrayList;
import java.util.Arrays;
import java.util.Collections;
import java.util.List;
import java.util.Map;
import java.util.concurrent.ConcurrentHashMap;
import patrones.FabricaEnemigos;
import patrones.ObservadorJuego;

/**
 * Motor que concentra TODA la lógica de una partida de Space Invaders.
 * Procesa comandos de ADMIN, JUGADOR y ESPECTADOR.
 * Formato de mensajes de salida (broadcast):
 *   ALIEN|id|x|y|estado      (estado 1=vivo, 0=destruido)
 *   JUGADOR|id|x|vidas|puntos
 *   BUNKER|id|salud
 *   OVNI|id|x|y|velocidad|puntosExtra
 */
public class Juego {
    
    private int idPartida;

    private Jugador jugador;
    private final Map<Integer, Enemigo> extraterrestres;
    private final Map<Integer, Ovni>    ovnis;
    private final List<Bunker>          bunkers;
    private int puntuacion;
    private int velocidadBaseExtraterrestres;
    private static final int INCREMENTO_VELOCIDAD = 1;

    public Juego(int idPartida) { 
        this.idPartida = idPartida;
        this.extraterrestres = new ConcurrentHashMap<>();
        this.ovnis           = new ConcurrentHashMap<>();
        this.bunkers         = Collections.synchronizedList(new ArrayList<>());
        this.velocidadBaseExtraterrestres = 2;
    }

    // -------------------------------------------------------------------------
    // Inicialización
    // -------------------------------------------------------------------------
    public synchronized void iniciarJuego() {
        this.puntuacion = 0;
        this.velocidadBaseExtraterrestres = 2;
        this.jugador = new Jugador(250, 450);
        this.extraterrestres.clear();
        this.ovnis.clear();
        this.bunkers.clear();
        
        // Cuatro bunkers equidistantes
        this.bunkers.add(new Bunker(0,  80, 380));
        this.bunkers.add(new Bunker(1, 180, 380));
        this.bunkers.add(new Bunker(2, 280, 380));
        this.bunkers.add(new Bunker(3, 380, 380));
        
        crearOlaInicial();
    }

    // -------------------------------------------------------------------------
    // Procesamiento de comandos
    // -------------------------------------------------------------------------
    public synchronized String procesarComando(String rol, String mensaje) {
        if (mensaje == null || mensaje.isEmpty()) return "ERROR|Comando vacío";

        String[] partes = tokenizarMensaje(mensaje);
        if (partes.length == 0) return "ERROR|Formato inválido";

        String tipo = partes[0].toUpperCase();
        try {
            switch (tipo) {
                case "ADMIN":      return procesarComandoAdmin(partes);
                case "JUGADOR":    return procesarComandoJugador(partes);
                case "ESPECTADOR": return "OK|PONG";
                default:           return "ERROR|Rol no reconocido";
            }
        } catch (NumberFormatException e) {
            return "ERROR|Parámetro numérico inválido";
        }
    }

    // -------------------------------------------------------------------------
    // Comandos ADMIN
    // -------------------------------------------------------------------------
    private String procesarComandoAdmin(String[] partes) {
        if (partes.length < 2) return "ERROR|Comando ADMIN inválido";

        String comando = partes[1].toUpperCase();
        switch (comando) {

            /*
             * CREAR (X, Y, Pts) — especificación §4.1.1.1
             * Crea un extraterrestre con puntos personalizados en la posición indicada.
             * Ejemplo recibido tras tokenizar: ["ADMIN","CREAR","1","1","1000"]
             */
            case "CREAR": {
                if (partes.length != 5) return "ERROR|ADMIN|CREAR|x|y|pts";
                int x   = Integer.parseInt(partes[2]);
                int y   = Integer.parseInt(partes[3]);
                int pts = Integer.parseInt(partes[4]);
                Enemigo e = FabricaEnemigos.crearExtraterrestrePorPuntos(x, y, pts);
                extraterrestres.put(e.getId(), e);
                actualizarVelocidadExtraterrestres();
                ObservadorJuego.notificarCreacionAlien(this.idPartida, e.getId(), e.getTipo(), x, y, e.getVelocidad());
                return "OK|ALIEN_CREADO|" + e.getId();
            }

            /*
             * CREAR_ALIEN tipo x y — crea un extraterrestre por tipo (1/2/3).
             */
            case "CREAR_ALIEN": {
                if (partes.length != 5) return "ERROR|ADMIN|CREAR_ALIEN|tipo|x|y";
                int tipoAlien = Integer.parseInt(partes[2]);
                int x         = Integer.parseInt(partes[3]);
                int y         = Integer.parseInt(partes[4]);
                Enemigo e = FabricaEnemigos.crearExtraterrestre(tipoAlien, x, y);
                extraterrestres.put(e.getId(), e);
                actualizarVelocidadExtraterrestres();
                ObservadorJuego.notificarCreacionAlien(this.idPartida, e.getId(), tipoAlien, x, y, e.getVelocidad());
                return "OK|ALIEN_CREADO|" + e.getId();
            }

            /*
             * OVNI dirección puntos — especificación §4.1.1.2
             * Ejemplo: OVNI I-D 1500  →  partes = ["ADMIN","OVNI","I-D","1500"]
             *          OVNI D-I 300   →  de derecha a izquierda
             */
            case "OVNI": {
                if (partes.length != 4) return "ERROR|ADMIN|OVNI|direccion|puntos";
                String dir   = partes[2].toUpperCase();
                int    pts   = Integer.parseInt(partes[3]);
                // I-D = izquierda→derecha (dirValor=+1), D-I = derecha→izquierda (dirValor=-1)
                int dirValor = dir.startsWith("I") ? 1 : -1;
                int xInicio  = dirValor > 0 ? 0 : 500;
                Ovni ovni = crearOvni(xInicio, 50, 5, pts, dirValor);
                ObservadorJuego.notificarCreacionOvni(this.idPartida, ovni.getId(), xInicio, 50, 5, pts);
                return "OK|OVNI_CREADO|" + ovni.getId();
            }

            /*
             * VELOCIDAD valor — especificación §4.1.1.3
             * Ejemplo: VELOCIDAD 100
             */
            case "VELOCIDAD": {
                if (partes.length != 3) return "ERROR|ADMIN|VELOCIDAD|valor";
                int vel = Integer.parseInt(partes[2]);
                setVelocidadBaseExtraterrestres(vel);
                ObservadorJuego.notificarVelocidadEnemigos(this.idPartida, vel);
                return "OK|VELOCIDAD_EXTRATERRESTRES_ACTUALIZADA|" + vel;
            }

            /*
             * BUNKERS porcentaje — especificación §4.1.1.4
             * Ejemplo: BUNKERS 70%  o  BUNKERS 70
             */
            case "BUNKERS": {
                if (partes.length != 3) return "ERROR|ADMIN|BUNKERS|porcentaje";
                String raw = partes[2].replace("%", "");
                int val = Integer.parseInt(raw);
                if (val < 0 || val > 100) return "ERROR|BUNKERS|porcentaje_invalido";
                for (Bunker b : bunkers) {
                    b.setVida(val);
                    ObservadorJuego.notificarBunkerActualizado(this.idPartida, b.getId(), val);
                }
                return "OK|BUNKERS_ACTUALIZADOS|" + val;
            }

            /*
             * DESTRUIR_ALIEN id — destruye un alien por id.
             */
            case "DESTRUIR_ALIEN": {
                if (partes.length != 3) return "ERROR|ADMIN|DESTRUIR_ALIEN|id";
                return destruirExtraterrestre(Integer.parseInt(partes[2]));
            }

            /*
             * SET_BUNKER_VIDA indice vida — ajusta la vida de un bunker individual.
             */
            case "SET_BUNKER_VIDA": {
                if (partes.length != 4) return "ERROR|ADMIN|SET_BUNKER_VIDA|indice|vida";
                int idx  = Integer.parseInt(partes[2]);
                int vida = Integer.parseInt(partes[3]);
                if (idx < 0 || idx >= bunkers.size()) return "ERROR|Índice de bunker inválido";
                bunkers.get(idx).setVida(vida);
                ObservadorJuego.notificarBunkerActualizado(this.idPartida, bunkers.get(idx).getId(), vida);
                return "OK|BUNKER_ACTUALIZADO|" + idx + "|" + vida;
            }

            /*
             * SET_JUGADOR_VIDAS vidas — ajusta las vidas del jugador.
             */
            case "SET_JUGADOR_VIDAS": {
                if (partes.length != 3) return "ERROR|ADMIN|SET_JUGADOR_VIDAS|vidas";
                int vidas = Integer.parseInt(partes[2]);
                jugador.agregarVida(vidas - jugador.getVidas());
                ObservadorJuego.notificarVidasJugador(this.idPartida, 0, jugador.getX(), jugador.getVidas(), puntuacion);
                return "OK|VIDAS_JUGADOR_ACTUALIZADAS|" + jugador.getVidas();
            }

            /*
             * REINICIAR — reinicia el juego completo.
             */
            case "REINICIAR": {
                iniciarJuego();
                return "OK|JUEGO_REINICIADO";
            }

            default:
                return "ERROR|Comando ADMIN desconocido: " + partes[1];
        }
    }

    // -------------------------------------------------------------------------
    // Comandos JUGADOR
    // -------------------------------------------------------------------------
    private String procesarComandoJugador(String[] partes) {
        if (partes.length < 2) return "ERROR|Comando JUGADOR inválido";

        String comando = partes[1].toUpperCase();
        switch (comando) {

            case "MOVER_IZQ": {
                jugador.moverIzquierda();
                ObservadorJuego.notificarJugadorMovido(this.idPartida, 0, jugador.getX(), jugador.getVidas(), puntuacion);
                return "OK|JUGADOR_MOVIDO|IZQ|" + jugador.getX();
            }

            case "MOVER_DER": {
                int limite = partes.length == 3 ? Integer.parseInt(partes[2]) : 500;
                jugador.moverDerecha(limite);
                ObservadorJuego.notificarJugadorMovido(this.idPartida, 0, jugador.getX(), jugador.getVidas(), puntuacion);
                return "OK|JUGADOR_MOVIDO|DER|" + jugador.getX();
            }

            case "DISPARAR": {
                ObservadorJuego.notificarDisparoJugador(this.idPartida, jugador.getX(), jugador.getY());
                return "OK|DISPARO|" + jugador.getX() + "|" + jugador.getY();
            }

            case "IMPACTO": {
                // El jugador recibió una bala enemiga → pierde una vida
                jugador.recibirImpacto();
                ObservadorJuego.notificarImpactoJugador(this.idPartida, jugador.getVidas());
                ObservadorJuego.notificarVidasJugador(this.idPartida, 0, jugador.getX(), jugador.getVidas(), puntuacion);
                return "OK|VIDAS|" + jugador.getVidas();
            }

            case "ELIMINAR_ALIEN": {
                if (partes.length != 3) return "ERROR|JUGADOR|ELIMINAR_ALIEN|id";
                return destruirExtraterrestre(Integer.parseInt(partes[2]));
            }

            default:
                return "ERROR|Comando JUGADOR desconocido: " + partes[1];
        }
    }

    // -------------------------------------------------------------------------
    // Lógica interna
    // -------------------------------------------------------------------------

    /** Crea la ola inicial: 2 filas × 6 columnas de extraterrestres tipo 1 y 2. */
    private void crearOlaInicial() {
        for (int fila = 0; fila < 2; fila++) {
            for (int col = 0; col < 6; col++) {
                int tipo = 1 + fila;
                int x    = 80 + col  * 70;
                int y    = 80 + fila * 60;
                Enemigo e = FabricaEnemigos.crearExtraterrestre(tipo, x, y);
                extraterrestres.put(e.getId(), e);
            }
        }
        actualizarVelocidadExtraterrestres();
    }

    private void setVelocidadBaseExtraterrestres(int velocidad) {
        this.velocidadBaseExtraterrestres = Math.max(1, velocidad);
        actualizarVelocidadExtraterrestres();
    }

    private void actualizarVelocidadExtraterrestres() {
        for (Enemigo e : extraterrestres.values()) {
            e.setVelocidad(velocidadBaseExtraterrestres);
        }
    }

    /**
     * Destruye un extraterrestre, suma sus puntos y verifica si la ola terminó.
     * Si la ola se completa: +1 vida al jugador, velocidad sube y comienza nueva ola.
     */
    private synchronized String destruirExtraterrestre(int idAlien) {
        Enemigo alien = extraterrestres.remove(idAlien);
        if (alien == null) return "ERROR|ALIEN_NO_ENCONTRADO|" + idAlien;

        puntuacion += alien.getPuntos();
        // Broadcast con estado=0 para indicar destrucción
        ObservadorJuego.notificarEnemigoDestruido(this.idPartida, idAlien, alien.getPuntos(), puntuacion);
        ObservadorJuego.notificarVidasJugador(this.idPartida, 0, jugador.getX(), jugador.getVidas(), puntuacion);

        verificarOlaCompleta();
        return "OK|ALIEN_DESTRUIDO|" + idAlien + "|PUNTOS|" + puntuacion;
    }

    /** Cuando no quedan aliens: +1 vida, mayor velocidad y nueva ola. */
    private void verificarOlaCompleta() {
        if (extraterrestres.isEmpty()) {
            jugador.agregarVida(1);
            velocidadBaseExtraterrestres += INCREMENTO_VELOCIDAD;
            crearOlaInicial();
            ObservadorJuego.notificarVidasJugador(this.idPartida, 0, jugador.getX(), jugador.getVidas(), puntuacion);
        }
    }

    // -------------------------------------------------------------------------
    // Métodos públicos de acceso y utilidad
    // -------------------------------------------------------------------------

    public synchronized Enemigo crearExtraterrestre(int tipo, int x, int y) {
        Enemigo e = FabricaEnemigos.crearExtraterrestre(tipo, x, y);
        e.setVelocidad(velocidadBaseExtraterrestres);
        extraterrestres.put(e.getId(), e);
        return e;
    }

    public synchronized Ovni crearOvni(int x, int y, int velocidad, int puntosExtra) {
        Ovni ovni = new Ovni(x, y, velocidad, puntosExtra);
        ovnis.put(ovni.getId(), ovni);
        return ovni;
    }

    public synchronized Ovni crearOvni(int x, int y, int velocidad, int puntosExtra, int direccion) {
        Ovni ovni = new Ovni(x, y, velocidad, puntosExtra, direccion);
        ovnis.put(ovni.getId(), ovni);
        return ovni;
    }

    public synchronized String moverExtraterrestre(int idAlien, int deltaX, int deltaY) {
        Enemigo alien = extraterrestres.get(idAlien);
        if (alien == null) return "ERROR|ALIEN_NO_ENCONTRADO|" + idAlien;
        alien.mover(deltaX, deltaY);
        ObservadorJuego.notificarMovimientoEnemigo(this.idPartida, idAlien, alien.getPosicionX(), alien.getPosicionY(), 1);
        return "OK|ALIEN_MOVIDO|" + idAlien + "|" + alien.getPosicionX() + "|" + alien.getPosicionY();
    }

    public synchronized String moverOvni(int idOvni) {
        Ovni ovni = ovnis.get(idOvni);
        if (ovni == null) return "ERROR|OVNI_NO_ENCONTRADO|" + idOvni;
        ovni.mover();
        ObservadorJuego.notificarMovimientoEnemigo(this.idPartida, idOvni, ovni.getX(), ovni.getY(), 1);
        return "OK|OVNI_MOVIDO|" + idOvni + "|" + ovni.getX() + "|" + ovni.getY();
    }

    public synchronized String obtenerResumenEstado() {
        StringBuilder sb = new StringBuilder();
        sb.append("VIDAS|").append(jugador.getVidas());
        sb.append("|PUNTOS|").append(puntuacion);
        sb.append("|ALIENS|").append(extraterrestres.size());
        sb.append("|OVNIS|").append(ovnis.size());
        sb.append("|BUNKERS|");
        for (int i = 0; i < bunkers.size(); i++) {
            if (i > 0) sb.append(",");
            sb.append(bunkers.get(i).getVida());
        }
        sb.append("|VELOCIDAD_ALIENS|").append(velocidadBaseExtraterrestres);
        return sb.toString();
    }

    public synchronized List<Enemigo> getExtraterrestres() { return new ArrayList<>(extraterrestres.values()); }
    public synchronized List<Ovni>    getOvnis()           { return new ArrayList<>(ovnis.values()); }
    public synchronized List<Bunker>  getBunkers()         { return new ArrayList<>(bunkers); }

    // -------------------------------------------------------------------------
    // Tokenizador de mensajes
    // -------------------------------------------------------------------------
    /**
     * Convierte una cadena de entrada en tokens, tratando espacios, comas y
     * paréntesis como separadores.  Ejemplo: "Crear (1,1, 1000)" → ["Crear","1","1","1000"]
     */
    private String[] tokenizarMensaje(String mensaje) {
        String normalizado = mensaje.trim()
            .replaceAll("[(),]", "|")
            .replaceAll("\\s+", "|");
        return Arrays.stream(normalizado.split("\\|"))
            .filter(t -> !t.isEmpty())
            .toArray(String[]::new);
    }
}
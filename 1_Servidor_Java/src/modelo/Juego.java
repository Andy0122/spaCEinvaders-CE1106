package modelo;

import estructuras_datos.ListaEnlazada;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.Collections;
import java.util.List;
import java.util.Map;
import java.util.Random;
import java.util.concurrent.ConcurrentHashMap;
import java.util.concurrent.Executors;
import java.util.concurrent.ScheduledExecutorService;
import java.util.concurrent.TimeUnit;
import patrones.FabricaEnemigos;
import patrones.ObservadorJuego;

/**
 * @class Juego
 * @brief Motor principal de lógica del sistema.
 * * Implementa el procesamiento en tiempo real de la partida, controlando las colisiones,
 * cinemática de entidades, rutinas de inteligencia artificial básica (disparos aleatorios)
 * y resolución de eventos de estado. Actúa como el núcleo del patrón Observer para 
 * notificar cambios de estado a la capa de red.
 * 
 * Patrones de diseño aplicados:
 *   1. Factory Method → FabricaEnemigos
 *   2. Observer       → ObservadorJuego + DespachadorMensajes
 *
 * Hilos internos (ScheduledExecutorService):
 *   - gameLoop  : mueve aliens y disparan cada TICK_MS ms
 *   - loopOvni  : aparece un OVNI aleatorio cada INTERVALO_OVNI_SEG segundos
 *
 * Formato de mensajes de salida (broadcast):
 *   ALIEN|id|x|y|estado         (estado: 1=vivo, 0=destruido)
 *   JUGADOR|id|x|vidas|puntos
 *   BUNKER|id|salud
 *   OVNI|id|x|y|velocidad|pts   (creación)
 *   OVNI|id|x|y|1               (movimiento activo)
 *   OVNI|id|x|y|0               (fuera de pantalla)
 *   VELOCIDAD|valor
 *   IMPACTO_JUGADOR|vidas
 *   DISPARO|x|y
 *   GAME_OVER
 */
public class Juego {
    // -------------------------------------------------------------------------
    // Constantes del motor
    // -------------------------------------------------------------------------
    /** ms entre ticks del game loop. Reducido de 500 → 200 para más velocidad. */
    private static final int    TICK_MS = 200;
    private static final int    INTERVALO_OVNI_SEG = 20;
    private static final int    LIMITE_X_MAX = 750;
    private static final int    LIMITE_Y_GAMEOVER = 520;
    private static final int    DELTA_Y_ALIEN = 20;
    private static final double PROB_DISPARO_ALIEN = 0.008; 
    private static final int    VELOCIDAD_BALA_ENEMIGA = 14; 
    private static final int INCREMENTO_VELOCIDAD = 2;  

    // -------------------------------------------------------------------------
    // Estado de la partida
    // -------------------------------------------------------------------------
    private final int idPartida;
    private Jugador   jugador;
    private boolean   gameOver;
    private int       puntuacion;
    private int       velocidadBaseExtraterrestres;

    /** Dirección horizontal actual de los aliens: +1 = derecha, -1 = izquierda */
    private int direccionAliens = 1;

    private final Map<Integer, Enemigo> extraterrestres;
    private final Map<Integer, Ovni>    ovnis;
    private final List<Bunker>          bunkers;

    /**
     * Lista enlazada propia de balas enemigas activas.
     * Cada bala es un int[]{x, y} con la coordenada actual.
     */
    private final ListaEnlazada<int[]> balasEnemigas;
    private final ListaEnlazada<int[]> balasJugador;

    private final Random rng = new Random();

    // -------------------------------------------------------------------------
    // Concurrencia
    // -------------------------------------------------------------------------
    private ScheduledExecutorService scheduler;

    /**
     * @brief Inicializa una nueva instancia del motor de juego.
     * @param idPartida Identificador asignado a la sala.
     */
    public Juego(int idPartida) {
        this.idPartida                    = idPartida;
        this.extraterrestres              = new ConcurrentHashMap<>();
        this.ovnis                        = new ConcurrentHashMap<>();
        this.bunkers                      = Collections.synchronizedList(new ArrayList<>());
        this.balasEnemigas                = new ListaEnlazada<>();
        this.balasJugador                 = new ListaEnlazada<>();
        this.velocidadBaseExtraterrestres = 4; 
    }

    /**
     * @brief Reinicia el estado del modelo y lanza los hilos de procesamiento.
     */
    public synchronized void iniciarJuego() {
        this.puntuacion                   = 0;
        this.gameOver                     = false;
        this.direccionAliens              = 1;
        this.velocidadBaseExtraterrestres = 4; 
        this.jugador                      = new Jugador(400, 540);

        this.extraterrestres.clear();
        this.ovnis.clear();
        this.bunkers.clear();
        this.balasEnemigas.limpiar();
        this.balasJugador.limpiar();

        this.bunkers.add(new Bunker(0,  50, 450));
        this.bunkers.add(new Bunker(1, 250, 450));
        this.bunkers.add(new Bunker(2, 450, 450));
        this.bunkers.add(new Bunker(3, 650, 450));

        crearOlaInicial();
        arrancarLoops();
    }

    /**
     * @brief Sincroniza explícitamente el estado completo del modelo hacia los suscriptores.
     */
    public synchronized void reenviarEstadoActual() {
        broadcastEstadoInicial();
    }

     /**
     * Envía a todos los clientes de la sala el estado completo de la partida
     * recién inicializada: aliens, bunkers y jugador.
     * Esto garantiza que la cuadrícula aparezca inmediatamente en pantalla
     * sin tener que esperar el primer tick del game loop.
     */
    private void broadcastEstadoInicial() {
        // Limpiar aliens viejos en el cliente ANTES de mandar la nueva ola.
        // Evita sprites fantasma cuando los IDs de la ola anterior no
        // coinciden con los nuevos (el contador de ID es global, no por ola).
        ObservadorJuego.notificarLimpiarAliens(idPartida);

        for (Enemigo e : extraterrestres.values()) {
            ObservadorJuego.notificarCreacionAlien(idPartida, e.getId(),
                    e.getTipo(), e.getPosicionX(), e.getPosicionY(), e.getVelocidad());
        }
        for (Bunker b : bunkers) {
            ObservadorJuego.notificarBunkerActualizado(idPartida, b.getId(), b.getVida());
        }
        ObservadorJuego.notificarVidasJugador(idPartida, idPartida,
                jugador.getX(), jugador.getVidas(), puntuacion);
    }

    /** Arranca (o reinicia) los hilos del game loop y del OVNI periódico. */
    private void arrancarLoops() {
        detenerLoops();
        scheduler = Executors.newScheduledThreadPool(2);

        // ── Loop principal: movimiento de aliens + disparos + colisiones ──
        scheduler.scheduleAtFixedRate(() -> {
            try { tickJuego(); }
            catch (Exception ex) {
                System.err.println("ERROR [Juego]: Fallo en rutina principal -> " + ex.getMessage());
            }
        }, TICK_MS, TICK_MS, TimeUnit.MILLISECONDS);

        // ── Loop OVNI: aparece un OVNI aleatorio cada N segundos ──
        scheduler.scheduleAtFixedRate(() -> {
            try { tickOvni(); }
            catch (Exception ex) {
                System.err.println("ERROR [Juego]: Fallo en rutina de entidad especial -> " + ex.getMessage());
            }
        }, INTERVALO_OVNI_SEG, INTERVALO_OVNI_SEG, TimeUnit.SECONDS);

        System.out.println("INFO [Juego]: Procesos asíncronos iniciados para sala " + idPartida);
    }

    private void detenerLoops() {
        if (scheduler != null && !scheduler.isShutdown()) {
            scheduler.shutdownNow();
        }
    }

    /**
     * @brief Ciclo maestro de simulación física y resolución de reglas.
     */
    private synchronized void tickJuego() {
        if (gameOver) return;

        moverAliensEnFormacion();
        moverOvnisActivos();  
        procesarDisparosEnemigos();
        moverBalasEnemigas();
        moverBalasJugador(); 
        verificarColisionesBalasJugador(); 
        verificarColisionesBalas();
        verificarGameOver();
    }

    // ─── Movimiento en formación ──────────────────────────────────────────────
    private void moverAliensEnFormacion() {
        if (extraterrestres.isEmpty()) return;

        int xMin = Integer.MAX_VALUE;
        int xMax = Integer.MIN_VALUE;
        for (Enemigo e : extraterrestres.values()) {
            if (e.getPosicionX() < xMin) xMin = e.getPosicionX();
            if (e.getPosicionX() > xMax) xMax = e.getPosicionX();
        }

        int delta   = velocidadBaseExtraterrestres * direccionAliens;
        boolean rebotar = (direccionAliens > 0 && xMax + delta > LIMITE_X_MAX)
                       || (direccionAliens < 0 && xMin + delta < 0);

        if (rebotar) {
            for (Enemigo e : extraterrestres.values()) {
                e.mover(0, DELTA_Y_ALIEN);
                ObservadorJuego.notificarMovimientoEnemigo(idPartida, e.getId(), e.getTipo(),
                        e.getPosicionX(), e.getPosicionY(), 1);
            }
            direccionAliens = -direccionAliens;
        } else {
            for (Enemigo e : extraterrestres.values()) {
                e.mover(delta, 0);
                ObservadorJuego.notificarMovimientoEnemigo(idPartida, e.getId(), e.getTipo(),
                        e.getPosicionX(), e.getPosicionY(), 1);
            }
        }
    }

    // ─── Disparos aleatorios de aliens ───────────────────────────────────────
    private void procesarDisparosEnemigos() {
        for (Enemigo e : extraterrestres.values()) {
            if (rng.nextDouble() < PROB_DISPARO_ALIEN) {
                balasEnemigas.insertarFrente(new int[]{ e.getPosicionX(), e.getPosicionY() });
                // Avisar al cliente para que dibuje y mueva esta bala visualmente
                ObservadorJuego.notificarDisparoEnemigo(idPartida, e.getPosicionX(), e.getPosicionY());
            }
        }
    }

    // ─── Movimiento de balas enemigas ─────────────────────────────────────────
    private void moverBalasEnemigas() {
        List<int[]> aEliminar = new ArrayList<>();
        balasEnemigas.forEach(bala -> {
            bala[1] += VELOCIDAD_BALA_ENEMIGA;
            if (bala[1] > LIMITE_Y_GAMEOVER + 20) aEliminar.add(bala);
        });
        for (int[] b : aEliminar) balasEnemigas.eliminar(b);
    }

    // ─── Colisiones de balas con jugador y bunkers ────────────────────────────
    private void verificarColisionesBalas() {
        List<int[]> aEliminar = new ArrayList<>();

        balasEnemigas.forEach(bala -> {
            // ¿Impacta bunker?
            for (Bunker b : bunkers) {
                if (!b.estaDestruido()
                        && Math.abs(bala[0] - b.getPosicionX()) < 30
                        && Math.abs(bala[1] - b.getPosicionY()) < 25) {
                    b.recibirImpacto();
                    ObservadorJuego.notificarBunkerActualizado(idPartida, b.getId(), b.getVida());
                    ObservadorJuego.notificarBalaEnemigaDestruida(idPartida, bala[0], bala[1]);
                    aEliminar.add(bala);
                    return;
                }
            }

            // ¿Impacta jugador?
            if (Math.abs(bala[0] - jugador.getX()) < 25
                    && Math.abs(bala[1] - jugador.getY()) < 25) {
                jugador.recibirImpacto();
                ObservadorJuego.notificarImpactoJugador(idPartida, jugador.getVidas());
                ObservadorJuego.notificarVidasJugador(idPartida, idPartida,
                        jugador.getX(), jugador.getVidas(), puntuacion);
                ObservadorJuego.notificarBalaEnemigaDestruida(idPartida, bala[0], bala[1]);
                aEliminar.add(bala);
            }
        });

        for (int[] b : aEliminar) balasEnemigas.eliminar(b);
    }

    // ─── Game Over ────────────────────────────────────────────────────────────
    private void verificarGameOver() {
        if (!jugador.estaVivo()) {
            declararGameOver("JUGADOR SIN VIDAS");
            return;
        }
        for (Enemigo e : extraterrestres.values()) {
            if (e.getPosicionY() >= LIMITE_Y_GAMEOVER) {
                declararGameOver("INVASION COMPLETADA");
                return;
            }
        }
    }

    private void declararGameOver(String motivo) {
        if (gameOver) return;
        gameOver = true;
        System.out.println("INFO [Juego]: Sala " + idPartida + " — FIN DEL JUEGO: " + motivo);
        ObservadorJuego.notificarGameOver(idPartida);
        detenerLoops();
    }

    // =========================================================================
    // Tick del OVNI periódico
    // =========================================================================
    private synchronized void tickOvni() {
        if (gameOver) return;

        // Si no hay OVNI activo, generar uno nuevo aleatorio
        if (ovnis.isEmpty()) {
            int dirValor         = rng.nextBoolean() ? 1 : -1;
            // Para que aparezca fuera de la pantalla y entre suavemente
            int xInicio          = dirValor > 0 ? -60 : LIMITE_X_MAX + 60; 
            int puntosAleatorios = 50 + rng.nextInt(451); // 50–500
            
            // Le ponemos velocidad de 5 o 6 para que atraviese el cielo
            Ovni nuevo = new Ovni(xInicio, 30, 30, puntosAleatorios, dirValor);
            ovnis.put(nuevo.getId(), nuevo);
            ObservadorJuego.notificarCreacionOvni(idPartida, nuevo.getId(),
                    xInicio, 30, 5, puntosAleatorios);
            System.out.println("INFO [Juego]: Sala " + idPartida + " — Entidad OVNI despachada (ID: " + nuevo.getId() + ")");
        }
    }

    private void moverOvnisActivos() {
        List<Integer> fuera = new ArrayList<>();
        for (Map.Entry<Integer, Ovni> entry : ovnis.entrySet()) {
            Ovni o = entry.getValue();
            o.mover();
            int x = o.getX();
            // Si sale completamente de los bordes, lo eliminamos
            if (x < -100 || x > LIMITE_X_MAX + 100) {
                fuera.add(entry.getKey());
                ObservadorJuego.notificarOvniDestruido(idPartida, o.getId());
            } else {
                ObservadorJuego.notificarMovimientoOvni(idPartida, o.getId(), x, o.getY());
            }
        }
        fuera.forEach(ovnis::remove);
    }

    /**
     * @brief Interfaz de enrutamiento y ejecución de instrucciones externas.
     * @param rol Clasificación del originador ("ADMIN", "JUGADOR", "ESPECTADOR").
     * @param mensaje Trama cruda de la instrucción.
     * @return Código de respuesta para el protocolo de red.
     */
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

            case "CREAR": {
                if (partes.length != 5) return "ERROR|ADMIN|CREAR|x|y|pts";
                int x   = Integer.parseInt(partes[2]);
                int y   = Integer.parseInt(partes[3]);
                int pts = Integer.parseInt(partes[4]);
                Enemigo e = FabricaEnemigos.crearExtraterrestrePorPuntos(x, y, pts);
                e.setVelocidad(velocidadBaseExtraterrestres);
                extraterrestres.put(e.getId(), e);
                ObservadorJuego.notificarCreacionAlien(idPartida, e.getId(), e.getTipo(), x, y, e.getVelocidad());
                return "OK|ALIEN_CREADO|" + e.getId();
            }

            case "CREAR_ALIEN": {
                if (partes.length != 5) return "ERROR|ADMIN|CREAR_ALIEN|tipo|x|y";
                int tipo = Integer.parseInt(partes[2]);
                int x    = Integer.parseInt(partes[3]);
                int y    = Integer.parseInt(partes[4]);
                Enemigo e = FabricaEnemigos.crearExtraterrestre(tipo, x, y);
                e.setVelocidad(velocidadBaseExtraterrestres);
                extraterrestres.put(e.getId(), e);
                ObservadorJuego.notificarCreacionAlien(idPartida, e.getId(), tipo, x, y, e.getVelocidad());
                return "OK|ALIEN_CREADO|" + e.getId();
            }

            case "OVNI": {
                if (partes.length != 4) return "ERROR|ADMIN|OVNI|direccion|puntos";
                String dir   = partes[2].toUpperCase();
                int    pts   = Integer.parseInt(partes[3]);
                int dirValor = dir.startsWith("I") ? 1 : -1;
                int xInicio  = dirValor > 0 ? 0 : LIMITE_X_MAX;
                Ovni ovni = new Ovni(xInicio, 30, 5, pts, dirValor);
                ovnis.put(ovni.getId(), ovni);
                ObservadorJuego.notificarCreacionOvni(idPartida, ovni.getId(), xInicio, 30, 5, pts);
                return "OK|OVNI_CREADO|" + ovni.getId();
            }

            case "VELOCIDAD": {
                if (partes.length != 3) return "ERROR|ADMIN|VELOCIDAD|valor";
                int vel = Integer.parseInt(partes[2]);
                setVelocidadBaseExtraterrestres(vel);
                ObservadorJuego.notificarVelocidadEnemigos(idPartida, vel);
                return "OK|VELOCIDAD_EXTRATERRESTRES_ACTUALIZADA|" + vel;
            }

            case "BUNKERS": {
                if (partes.length != 3) return "ERROR|ADMIN|BUNKERS|porcentaje";
                String raw = partes[2].replace("%", "");
                int val = Integer.parseInt(raw);
                if (val < 0 || val > 100) return "ERROR|BUNKERS|porcentaje_invalido";
                for (Bunker b : bunkers) {
                    b.setVida(val);
                    ObservadorJuego.notificarBunkerActualizado(idPartida, b.getId(), val);
                }
                return "OK|BUNKERS_ACTUALIZADOS|" + val;
            }

            case "DESTRUIR_ALIEN": {
                if (partes.length != 3) return "ERROR|ADMIN|DESTRUIR_ALIEN|id";
                return destruirExtraterrestre(Integer.parseInt(partes[2]));
            }

            case "SET_BUNKER_VIDA": {
                if (partes.length != 4) return "ERROR|ADMIN|SET_BUNKER_VIDA|indice|vida";
                int idx  = Integer.parseInt(partes[2]);
                int vida = Integer.parseInt(partes[3]);
                if (idx < 0 || idx >= bunkers.size()) return "ERROR|Índice de bunker inválido";
                bunkers.get(idx).setVida(vida);
                ObservadorJuego.notificarBunkerActualizado(idPartida, bunkers.get(idx).getId(), vida);
                return "OK|BUNKER_ACTUALIZADO|" + idx + "|" + vida;
            }

            case "SET_JUGADOR_VIDAS": {
                if (partes.length != 3) return "ERROR|ADMIN|SET_JUGADOR_VIDAS|vidas";
                int vidas = Integer.parseInt(partes[2]);
                jugador.agregarVida(vidas - jugador.getVidas());
                ObservadorJuego.notificarVidasJugador(idPartida, idPartida,
                        jugador.getX(), jugador.getVidas(), puntuacion);
                return "OK|VIDAS_JUGADOR_ACTUALIZADAS|" + jugador.getVidas();
            }

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
        if (gameOver) return "ERROR|GAME_OVER";

        String comando = partes[1].toUpperCase();
        switch (comando) {

            case "MOVER_IZQ": {
                jugador.moverIzquierda();
                ObservadorJuego.notificarJugadorMovido(idPartida, idPartida,
                        jugador.getX(), jugador.getVidas(), puntuacion);
                return "OK|JUGADOR_MOVIDO|IZQ|" + jugador.getX();
            }

            case "MOVER_DER": {
                int limite = partes.length == 3 ? Integer.parseInt(partes[2]) : LIMITE_X_MAX;
                jugador.moverDerecha(limite);
                ObservadorJuego.notificarJugadorMovido(idPartida, idPartida,
                        jugador.getX(), jugador.getVidas(), puntuacion);
                return "OK|JUGADOR_MOVIDO|DER|" + jugador.getX();
            }

            case "DISPARAR": {
                // Centramos la bala con respecto al cañón (aprox +20 en X)
                int bx = jugador.getX() + 20; 
                int by = jugador.getY() - 10;
                balasJugador.insertarFrente(new int[]{bx, by});
        
                ObservadorJuego.notificarDisparoJugador(idPartida, bx, by);
                return "OK|DISPARO|" + bx + "|" + by;
            }

            case "IMPACTO": {
                jugador.recibirImpacto();
                ObservadorJuego.notificarImpactoJugador(idPartida, jugador.getVidas());
                ObservadorJuego.notificarVidasJugador(idPartida, idPartida,
                        jugador.getX(), jugador.getVidas(), puntuacion);
                if (!jugador.estaVivo()) declararGameOver("JUGADOR SIN VIDAS (manual)");
                return "OK|VIDAS|" + jugador.getVidas();
            }

            case "ELIMINAR_ALIEN": {
                if (partes.length != 3) return "ERROR|JUGADOR|ELIMINAR_ALIEN|id";
                return destruirExtraterrestre(Integer.parseInt(partes[2]));
            }

            case "ELIMINAR_OVNI": {
                if (partes.length != 3) return "ERROR|JUGADOR|ELIMINAR_OVNI|id";
                return destruirOvni(Integer.parseInt(partes[2]));
            }

            default:
                return "ERROR|Comando JUGADOR desconocido: " + partes[1];
        }
    }

    // =========================================================================
    // Lógica interna
    // =========================================================================
    private void crearOlaInicial() {
        // 3 filas: tipo 1 (calamar, 10 pts), tipo 2 (cangrejo, 20 pts),
        // tipo 3 (pulpo, 40 pts) — antes solo había 2 filas (calamar/cangrejo).
        for (int fila = 0; fila < 3; fila++) {
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

    private synchronized String destruirExtraterrestre(int idAlien) {
        Enemigo alien = extraterrestres.remove(idAlien);
        if (alien == null) return "ERROR|ALIEN_NO_ENCONTRADO|" + idAlien;

        puntuacion += alien.getPuntos();
        ObservadorJuego.notificarEnemigoDestruido(idPartida, idAlien, alien.getPuntos(), puntuacion);
        ObservadorJuego.notificarVidasJugador(idPartida, idPartida,
                jugador.getX(), jugador.getVidas(), puntuacion);
        verificarOlaCompleta();
        return "OK|ALIEN_DESTRUIDO|" + idAlien + "|PUNTOS|" + puntuacion;
    }

    /**
     * Destruye un OVNI por impacto del jugador. A diferencia de los aliens,
     * los puntos otorgados son los puntosExtra ALEATORIOS asignados al
     * crear ese OVNI específico (50-500), no un valor fijo.
     */
    private synchronized String destruirOvni(int idOvni) {
        Ovni ovni = ovnis.remove(idOvni);
        if (ovni == null) return "ERROR|OVNI_NO_ENCONTRADO|" + idOvni;

        puntuacion += ovni.getPuntosExtra();
        ObservadorJuego.notificarOvniDestruido(idPartida, idOvni);
        ObservadorJuego.notificarVidasJugador(idPartida, idPartida,
                jugador.getX(), jugador.getVidas(), puntuacion);
        return "OK|OVNI_DESTRUIDO|" + idOvni + "|PUNTOS|" + puntuacion;
    }

    private void verificarOlaCompleta() {
        if (extraterrestres.isEmpty()) {
            jugador.agregarVida(1);
            velocidadBaseExtraterrestres += INCREMENTO_VELOCIDAD;
            direccionAliens = 1;
            balasEnemigas.limpiar();
            crearOlaInicial();
            broadcastEstadoInicial();
            ObservadorJuego.notificarVidasJugador(idPartida, idPartida,
                    jugador.getX(), jugador.getVidas(), puntuacion);
            System.out.println("INFO [Juego]: Sala " + idPartida + " — Nueva oleada generada. Velocidad actual=" + velocidadBaseExtraterrestres);
        }
    }

    // =========================================================================
    // Métodos públicos de acceso
    // =========================================================================
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
        ObservadorJuego.notificarMovimientoEnemigo(idPartida, idAlien, alien.getTipo(),
                alien.getPosicionX(), alien.getPosicionY(), 1);
        return "OK|ALIEN_MOVIDO|" + idAlien + "|" + alien.getPosicionX() + "|" + alien.getPosicionY();
    }

    public synchronized String moverOvni(int idOvni) {
        Ovni ovni = ovnis.get(idOvni);
        if (ovni == null) return "ERROR|OVNI_NO_ENCONTRADO|" + idOvni;
        ovni.mover();
        ObservadorJuego.notificarMovimientoOvni(idPartida, ovni.getId(), ovni.getX(), ovni.getY());
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
        sb.append("|GAME_OVER|").append(gameOver ? "1" : "0");
        return sb.toString();
    }

    public boolean isGameOver() { return gameOver; }

    public synchronized List<Enemigo> getExtraterrestres() { return new ArrayList<>(extraterrestres.values()); }
    public synchronized List<Ovni>    getOvnis()           { return new ArrayList<>(ovnis.values()); }
    public synchronized List<Bunker>  getBunkers()         { return new ArrayList<>(bunkers); }

    // =========================================================================
    // Tokenizador
    // =========================================================================
    private String[] tokenizarMensaje(String mensaje) {
        String normalizado = mensaje.trim()
            .replaceAll("[(),]", "|")
            .replaceAll("\\s+", "|");
        return Arrays.stream(normalizado.split("\\|"))
            .filter(t -> !t.isEmpty())
            .toArray(String[]::new);
    }

    // ─── Lógica de balas del JUGADOR (Centralizada en el Servidor) ──────
    private void moverBalasJugador() {
        List<int[]> aEliminar = new ArrayList<>();
        // Como el tick es cada 200ms, la bala debe avanzar bastante para igualar los 60fps del cliente
        balasJugador.forEach(bala -> {
            bala[1] -= 96; 
            if (bala[1] < -50) aEliminar.add(bala); // Salió de pantalla
        });
        for (int[] b : aEliminar) balasJugador.eliminar(b);
    }

    private void verificarColisionesBalasJugador() {
        List<int[]> aEliminar = new ArrayList<>();

        balasJugador.forEach(bala -> {
            boolean impactada = false;

            // 1. ¿Toca a algún Alien?
            for (Enemigo e : extraterrestres.values()) {
                // Hitbox con tolerancia y alargado hacia abajo (para capturar la bala en movimiento)
                if (bala[0] >= e.getPosicionX() - 15 && bala[0] <= e.getPosicionX() + 45 &&
                    bala[1] <= e.getPosicionY() + 40 && bala[1] >= e.getPosicionY() - 96) {
                    destruirExtraterrestre(e.getId());
                    impactada = true;
                    break;
                }
            }

            // 2. ¿Toca al OVNI?
            if (!impactada) {
                for (Ovni o : ovnis.values()) {
                    if (bala[0] >= o.getX() - 15 && bala[0] <= o.getX() + 75 &&
                        bala[1] <= o.getY() + 40 && bala[1] >= o.getY() - 96) {
                        destruirOvni(o.getId());
                        impactada = true;
                        break;
                    }
                }
            }

            // 3. ¿Toca algún Bunker?
            if (!impactada) {
                for (Bunker b : bunkers) {
                    if (!b.estaDestruido() &&
                        bala[0] >= b.getPosicionX() - 10 && bala[0] <= b.getPosicionX() + 50 &&
                        bala[1] <= b.getPosicionY() + 40 && bala[1] >= b.getPosicionY() - 96) {
                        
                        b.recibirImpacto();
                        ObservadorJuego.notificarBunkerActualizado(idPartida, b.getId(), b.getVida());
                        impactada = true;
                        break;
                    }
                }
            }

            if (impactada) {
                ObservadorJuego.notificarBalaJugadorDestruida(idPartida, bala[0], bala[1]);
                aEliminar.add(bala);
            }
        });

        for (int[] b : aEliminar) balasJugador.eliminar(b);
    }

    /**
     * @brief Pausa las rutinas de ejecución asíncrona del juego.
     */
    public synchronized void pausarJuego() {
        if (!gameOver) {
            detenerLoops();
            System.out.println("INFO [Juego]: Sala " + idPartida + " — Juego en espera (sin clientes activos).");
        }
    }

    /**
     * @brief Reactiva las rutinas de ejecución si existen las condiciones necesarias.
     */
    public synchronized void reanudarJuego() {
        // Solo lo reanudamos si no hay Game Over y si los hilos están apagados
        if (!gameOver && (scheduler == null || scheduler.isShutdown())) {
            arrancarLoops();
            System.out.println("INFO [Juego]: Sala " + idPartida + " — Juego reactivado.");
        }
    }
}
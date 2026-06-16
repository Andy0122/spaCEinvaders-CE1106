package modelo;

import java.util.Random;

/**
 * @class Ovni
 * @brief Entidad especial no predecible que cruza la pantalla horizontalmente.
 * Otorga bonificaciones de puntaje dinámicas al ser destruido por el jugador.
 */
public class Ovni {
    private static int siguienteId = 1;

    private final int id;
    private int x;
    private int y;
    private int velocidad;
    private int direccion;
    private int puntosExtra;
    private Random random;
    
    private static final int PUNTOS_MIN = 50;
    private static final int PUNTOS_MAX = 500;

    public Ovni(int x, int y, int velocidad, int puntosExtra) {
        this(x, y, velocidad, puntosExtra, 1);
    }

    public Ovni(int x, int y, int velocidad, int puntosExtra, int direccion) {
        this.id = siguienteId++;
        this.x = x;
        this.y = y;
        this.velocidad = velocidad;
        this.direccion = Math.signum(direccion) == 0 ? 1 : direccion;
        this.random = new Random();
        this.puntosExtra = puntosExtra;
    }

    @SuppressWarnings("unused")
    private int generarPuntosAleatorios() {
        return PUNTOS_MIN + random.nextInt(PUNTOS_MAX - PUNTOS_MIN + 1);
    }

    /**
     * @brief Actualiza la posición horizontal de la entidad en función de su velocidad y dirección.
     */
    public void mover() {
        x += velocidad * direccion;
    }

    public int getDireccion() { return direccion; }
    public void setDireccion(int direccion) { this.direccion = Math.signum(direccion) == 0 ? 1 : direccion; }
    public int getX() { return x; }
    public void setX(int x) { this.x = x; }
    public int getY() { return y; }
    public void setY(int y) { this.y = y; }
    public int getVelocidad() { return velocidad; }
    public void setVelocidad(int velocidad) { this.velocidad = velocidad; }
    public int getPuntosExtra() { return puntosExtra; }
    public void setPuntosExtra(int puntosExtra) { this.puntosExtra = puntosExtra; }
    public int getId() { return id; }
}
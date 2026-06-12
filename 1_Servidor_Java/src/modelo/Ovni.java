
/* 
    * Clase que representa a un ovni en el juego Space Invaders. 
    * El ovni tiene una posición, velocidad y puntos extra.
    * El ovni se mueve horizontalmente a través de la pantalla y otorga puntos extra al jugador si es destruido.
*/
import java.util.Random;

public class Ovni {
    private int x;
    private int y;
    private int velocidad;
    private int puntosExtra;
    private Random random;
    private static final int PUNTOS_MIN = 50;
    private static final int PUNTOS_MAX = 500;

    public Ovni(int x, int y, int velocidad) {
        this.x = x;
        this.y = y;
        this.velocidad = velocidad;
        this.random = new Random();
        this.puntosExtra = generarPuntosAleatorios();
    }

    private int generarPuntosAleatorios() {
        return PUNTOS_MIN + random.nextInt(PUNTOS_MAX - PUNTOS_MIN + 1);
    }

    public void mover() {
        x += velocidad;
    }

    public int getX() {
        return x;
    }

    public void setX(int x) {
        this.x = x;
    }

    public int getY() {
        return y;
    }

    public void setY(int y) {
        this.y = y;
    }

    public int getVelocidad() {
        return velocidad;
    }

    public void setVelocidad(int velocidad) {
        this.velocidad = velocidad;
    }

    public int getPuntosExtra() {
        return puntosExtra;
    }

    public void setPuntosExtra(int puntosExtra) {
        this.puntosExtra = puntosExtra;
    }
}

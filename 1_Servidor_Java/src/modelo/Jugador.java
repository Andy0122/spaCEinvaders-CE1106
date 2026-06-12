
/**
 * Representa al jugador (cañón) del juego.
 * Contiene atributos de posición, vidas y velocidad, así como métodos para
 * mover el cañón, disparar balas y recibir impactos. La clase también incluye
 */
public class Jugador {
    private int x; // posición horizontal
    private int y; // posición vertical
    private int vidas;
    private int velocidad;

    public Jugador(int inicioX, int inicioY) {
        this.x = inicioX;
        this.y = inicioY;
        this.vidas = 3; // inicia con 3 vidas
        this.velocidad = 5;
    }

    public int getX() { return x; }
    public int getY() { return y; }
    public int getVidas() { return vidas; }

    public void setVelocidad(int v) { this.velocidad = v; }

    // Mueve el cañón a la izquierda
    public void moverIzquierda() {
        x -= velocidad;
        if (x < 0) x = 0;
    }

    // Mueve el cañón a la derecha (se puede ajustar el límite externo desde quien lo use)
    public void moverDerecha(int limiteDerecho) {
        x += velocidad;
        if (x > limiteDerecho) x = limiteDerecho;
    }

    // Dispara una bala desde la posición actual. Se retorna una instancia interna Bala.
    public Bala disparar() {
        return new Bala(x, y - 10); // la bala aparece por encima del cañón
    }

    // El jugador recibe impacto: pierde una vida
    public void recibirImpacto() {
        if (vidas > 0) vidas--;
    }

    public boolean estaVivo() { return vidas > 0; }

    // Clase interna simple para representar una bala del jugador
    public static class Bala {
        private int x;
        private int y;
        private int velocidad = 10; // sentido hacia arriba

        public Bala(int x, int y) {
            this.x = x;
            this.y = y;
        }

        public int getX() { return x; }
        public int getY() { return y; }

        // Avanza la bala hacia arriba
        public void actualizar() {
            y -= velocidad;
        }
    }
}

package modelo;

/**
 * @class Jugador
 * @brief Entidad que representa al usuario dentro del entorno de juego.
 * Administra el estado espacial, contador de vidas y generación de proyectiles.
 */
public class Jugador {
    private int x; 
    private int y; 
    private int vidas;
    private int velocidad;

    /**
     * @brief Constructor del jugador.
     * @param inicioX Coordenada horizontal inicial.
     * @param inicioY Coordenada vertical inicial.
     */
    public Jugador(int inicioX, int inicioY) {
        this.x = inicioX;
        this.y = inicioY;
        this.vidas = 3; 
        this.velocidad = 5;
    }

    public int getX() { return x; }
    public int getY() { return y; }
    public int getVidas() { return vidas; }

    public void setVelocidad(int v) { this.velocidad = v; }

    /**
     * @brief Desplaza la entidad hacia la izquierda respetando los márgenes lógicos.
     */
    public void moverIzquierda() {
        x -= velocidad;
        if (x < 0) x = 0;
    }

    /**
     * @brief Desplaza la entidad hacia la derecha respetando un límite establecido.
     * @param limiteDerecho Límite máximo en el eje X.
     */
    public void moverDerecha(int limiteDerecho) {
        x += velocidad;
        if (x > limiteDerecho) x = limiteDerecho;
    }

    /**
     * @brief Genera una instancia de proyectil instanciada desde la posición actual del cañón.
     * @return Objeto Bala inicializado.
     */
    public Bala disparar() {
        return new Bala(x, y - 10);
    }

    /**
     * @brief Reduce el contador de vidas tras una colisión enemiga.
     */
    public void recibirImpacto() {
        if (vidas > 0) vidas--;
    }

    public boolean estaVivo() { return vidas > 0; }

    public void agregarVida(int cantidad) {
        this.vidas += cantidad;
        if (this.vidas < 0) {
            this.vidas = 0;
        }
    }

    /**
     * @class Bala
     * @brief Estructura interna para el manejo espacial de los proyectiles aliados.
     */
    public static class Bala {
        private int x;
        private int y;
        private int velocidad = 10; 

        public Bala(int x, int y) {
            this.x = x;
            this.y = y;
        }

        public int getX() { return x; }
        public int getY() { return y; }

        public void actualizar() {
            y -= velocidad;
        }
    }
}
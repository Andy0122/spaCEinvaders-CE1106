package modelo;

/**
 * @class Extraterrestre
 * @brief Entidad hostil estándar del juego.
 * Implementa la interfaz Enemigo y define el comportamiento para los tipos:
 * 1 (Calamar), 2 (Cangrejo) y 3 (Pulpo).
 */
public class Extraterrestre implements Enemigo {
    private static int siguienteId = 1;

    private final int id;
    private int x;
    private int y;
    private int tipo;
    private int vida;
    private int puntos;
    private int velocidad;

    /**
     * @brief Inicializa una nueva instancia de un extraterrestre.
     * @param tipo Clasificación del alienígena (1, 2 o 3).
     * @param x Posición horizontal inicial.
     * @param y Posición vertical inicial.
     */
    public Extraterrestre(int tipo, int x, int y) {
        this.id = siguienteId++;
        this.tipo = tipo;
        this.vida = calcularVida(tipo);
        this.puntos = calcularPuntos(tipo);
        this.velocidad = calcularVelocidad(tipo);
        this.x = x;
        this.y = y;
    }

    private int calcularVida(int tipo) {
        switch (tipo) {
            case 1: return 10;
            case 2: return 20;
            case 3: return 30;
            default: return 10;
        }
    }

    private int calcularPuntos(int tipo) {
        switch (tipo) {
            case 1: return 10; // Calamar
            case 2: return 20; // Cangrejo
            case 3: return 40; // Pulpo
            default: return 10;
        }
    }

    private int calcularVelocidad(int tipo) {
        switch (tipo) {
            case 1: return 2;
            case 2: return 3;
            case 3: return 4;
            default: return 2;
        }
    }

    @Override public int getId()        { return id; }
    @Override public int getTipo()      { return tipo; }
    @Override public int getVida()      { return vida; }
    @Override public int getPuntos()    { return puntos; }
    @Override public int getVelocidad() { return velocidad; }
    @Override public int getPosicionX() { return x; }
    @Override public int getPosicionY() { return y; }

    @Override public void setVelocidad(int velocidad)      { this.velocidad = velocidad; }
    @Override public void setPosicion(int x, int y)        { this.x = x; this.y = y; }
    @Override public void mover(int deltaX, int deltaY)    { this.x += deltaX; this.y += deltaY; }
    @Override public boolean estaDestruido()               { return vida <= 0; }

    /**
     * @brief Procesa el daño recibido tras una colisión con un proyectil.
     * @return true si el impacto resultó en la destrucción de la entidad, false en caso contrario.
     */
    public boolean recibirImpacto() {
        if (vida > 0) {
            vida -= 10;
            if (vida < 0) vida = 0;
        }
        return estaDestruido();
    }
}
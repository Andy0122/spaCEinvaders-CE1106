/* 
 * Clase que representa a un extraterrestre en el juego Space Invaders. 
 * Tipos:
 *   1 = Calamar  → 10 pts
 *   2 = Cangrejo → 20 pts
 *   3 = Pulpo    → 40 pts
 */
package modelo;

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
     * Constructor único. Recibe el tipo (1, 2 o 3) y la posición inicial.
     * FabricaEnemigos es responsable de deducir el tipo antes de llamar aquí.
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
            case 1: return 10;   // Calamar
            case 2: return 20;   // Cangrejo
            case 3: return 40;   // Pulpo
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

    public boolean recibirImpacto() {
        if (vida > 0) {
            vida -= 10;
            if (vida < 0) vida = 0;
        }
        return estaDestruido();
    }
}

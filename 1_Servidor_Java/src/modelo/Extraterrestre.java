/* 
 * Clase que representa a un extraterrestre en el juego Space Invaders. 
 * Cada extraterrestre tiene un tipo, posición, vida, puntos y velocidad.
 * El tipo determina la cantidad de vida y puntos que otorga al ser destruido.
 * La clase incluye métodos para recibir impactos, verificar si el extraterrestre está destruido y
    
*/
public class Extraterrestre {
    private int x;
    private int y;
    private int tipo;
    private int vida;
    private int puntos;
    private int velocidad;
    
    public Extraterrestre(int tipo, int x, int y) {
        this.tipo = tipo;
        this.vida = calcularVida(tipo);
        this.puntos = calcularPuntos(tipo);
        this.x = x;
        this.y = y;
    }

    private int calcularVida(int tipo) {
     switch (tipo) {
            case 1:
                return 10; // Vida base para tipo 1
            case 2:
                return 20; // Vida base para tipo 2
            case 3:
                return 30; // Vida base para tipo 3
        }
     return tipo;
    }

    private int calcularPuntos(int tipo) {
        switch (tipo) {

            case 1:
                return 10;
            case 2:
                return 20;
            case 3:
                return 40;
            default:
                return 0;
        }
    }

    public int getTipo() {
        return tipo;
    }

    public int getVida() {
        return vida;
    }

    public int getPuntos() {
        return puntos;
    }

    public boolean estaDestruido() {
        return vida == 0;
    }

    public boolean recibirImpacto() {
        if (vida > 0) {
            vida -= 10; // Cada impacto reduce la vida en 10 puntos
            if (vida < 0) {
                vida = 0; 
            }
        }
        return estaDestruido();
    }

    public int getPosicionX() {
        return x;
    }

    public int getPosicionY() {
        return y;
    }
    public void setPosicion(int x, int y) {
        this.x = x;
        this.y = y;
    }
}
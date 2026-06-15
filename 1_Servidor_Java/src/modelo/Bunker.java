/*
    * Clase que representa el bunker en el juego Space Invaders. El bunker tiene una cantidad de vida que se reduce cada vez que es impactado por un disparo de los extraterrestres. Cuando la vida del bunker llega a cero, se considera destruido.
    * El bunker actúa como una barrera de protección para el jugador, permitiendo que los disparos de los extraterrestres sean absorbidos por el bunker en lugar de impactar directamente al jugador. Sin embargo, el bunker no es indestructible y puede ser destruido si recibe suficientes impactos.
    * Esta clase incluye métodos para recibir impactos, verificar si el bunker está destruido y obtener la vida actual del bunker.
*/
package modelo;

public class Bunker {
    private int id;
    private int vida;
    private int posicionX;
    private int posicionY;

    public Bunker(int id, int posicionX, int posicionY) {
        this.id = id;
        this.vida = 100;
        this.posicionX = posicionX;
        this.posicionY = posicionY;
    }

    public int getId() {
        return id;
    }

    public int getVida() {
        return vida;
    }

    public int getPosicionX() {
        return posicionX;
    }

    public int getPosicionY() {
        return posicionY;
    }
    
    public void recibirImpacto() {
        if (vida > 0) {
            vida -= 20;
            if (vida < 0) {
                vida = 0; 
            }
        }
    }

    public void setVida(int vida) {
        this.vida = Math.max(0, vida);
    }

    public boolean estaDestruido() {
        return vida == 0;
    }
}
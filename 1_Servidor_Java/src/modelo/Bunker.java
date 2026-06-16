package modelo;

/**
 * @class Bunker
 * @brief Representa una barrera de defensa estática en el juego.
 * Proporciona protección al jugador absorbiendo los impactos de los proyectiles enemigos
 * hasta que su integridad estructural se reduce a cero.
 */
public class Bunker {
    private int id;
    private int vida;
    private int posicionX;
    private int posicionY;

    /**
     * @brief Constructor de la entidad Bunker.
     * @param id Identificador único de la defensa.
     * @param posicionX Coordenada horizontal en la cuadrícula.
     * @param posicionY Coordenada vertical en la cuadrícula.
     */
    public Bunker(int id, int posicionX, int posicionY) {
        this.id = id;
        this.vida = 100;
        this.posicionX = posicionX;
        this.posicionY = posicionY;
    }

    public int getId() { return id; }
    public int getVida() { return vida; }
    public int getPosicionX() { return posicionX; }
    public int getPosicionY() { return posicionY; }

    /**
     * @brief Reduce la integridad estructural del bunker tras recibir un impacto.
     */
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

    /**
     * @brief Evalúa si el bunker ha sido completamente destruido.
     * @return true si la vida es 0, false en caso contrario.
     */
    public boolean estaDestruido() {
        return vida == 0;
    }
}
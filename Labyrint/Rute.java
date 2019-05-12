public abstract class Rute{

    // Initialiserer instansvariable
    protected int rad, kolonne;
    private Labyrint lab;
    protected Rute nord,syd,vest,oest;
    protected static Liste<String> losninger;

    // Konstruktor
    public Rute(int kolonne, int rad, Labyrint lab){
        this.kolonne = kolonne;
        this.rad = rad;
        this.lab = lab;
    }


    // Setter naboer for ruten, bruker referense til labyrinten den er del av
    public void settNaboer(){
        nord = lab.hentRute(kolonne,rad-1);
        syd = lab.hentRute(kolonne,rad+1);
        vest = lab.hentRute(kolonne-1,rad);
        oest = lab.hentRute(kolonne+1,rad);
    }

    // Bruker den rekursive metoden gaa() for aa finne utvei fra ruten
    public Liste<String> finnUtvei(){
        losninger = new Stabel<String>();
        gaa("");
        return losninger;
    }

    // Rekursiv metode, kaller gaa() paa alle naboer
    public void gaa(String sti){
      
    }

    // Hjelpemetode for aa avgjore om en rute er besokt
    public boolean besoktRute(String sti, Rute r){
        String rute = "(" + r.kolonne + "," + r.rad + ")";
        return sti.contains(rute);
    }

    abstract char tilTegn();

    @Override
    public String toString(){
        String rep = "k: " + kolonne + " r: " + rad;
        return rep;
    }

}

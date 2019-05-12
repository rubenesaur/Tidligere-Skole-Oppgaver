public class Aapning extends HvitRute{
    public Aapning(int kolonne, int rad, Labyrint lab){
        super(kolonne,rad,lab);
    }

    @Override
    public void gaa(String sti){
        sti +=  "(" + kolonne + "," + rad + ") ";
        losninger.settInn(sti);
    }

}

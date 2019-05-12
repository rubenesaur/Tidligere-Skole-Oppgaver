public class HvitRute extends Rute{
    public HvitRute(int kolonne, int rad, Labyrint lab){
        super(kolonne,rad,lab);
    }

    public char tilTegn(){
        return '.';
    }

    public void gaa(String sti){
        sti +=  "(" + kolonne + "," + rad + ") " + "--> ";
        if(nord != null && besoktRute(sti,nord) != true){
            nord.gaa(sti);
        }

        if(syd != null && besoktRute(sti,syd) != true){
            syd.gaa(sti);
        }
        if(vest != null && besoktRute(sti,vest) != true){
            vest.gaa(sti);
        }

        if(oest != null && besoktRute(sti,oest) != true){
            oest.gaa(sti);
        }
    }

}

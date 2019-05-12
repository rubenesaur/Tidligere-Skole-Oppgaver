import java.util.Scanner;
import java.io.File;
import java.io.FileNotFoundException;

public class Labyrint{

    // Initialiserer instansvariable
    private Rute[][] ruter;
    private int rader;
    private int kolonner;

    // Privat konstruktor som kun kalles fra den statiske metoden i denne klassen
    private Labyrint(int rader, int kolonner, char[][] ruteChars){
        this.rader = rader;
        this.kolonner = kolonner;

        ruter = new Rute[rader][kolonner];
        for(int x = 0; x < rader; x++){
            for(int y = 0; y < kolonner; y++){
                int kolonne = y+1;
                int rad = x+1;
                if(ruteChars[x][y] == '#'){
                    ruter[x][y] = new SortRute(kolonne,rad,this);
                }else if(erAapning(kolonne,rad)){
                    ruter[x][y] = new Aapning(kolonne,rad,this);
                }else{
                    ruter[x][y] = new HvitRute(kolonne,rad,this);
                }
            }
        }
    }

    // Sjekker om rute paa kolonne og rad er en aapning
    private boolean erAapning(int kolonne, int rad){
        boolean radB = (rad > 1 && rad < rader);
        boolean kolB = (kolonne > 1 && kolonne < kolonner);
        return !(radB && kolB);
    }


    public void settMinimalUtskrift(){

    }

    // Setter opp labyrinten fra fil, og returnerer det oppsatte Labyrint objektet
    public static Labyrint lesFraFil(File fil) throws FileNotFoundException{
        Scanner leser = new Scanner(fil);
        int antallRader = Integer.parseInt(leser.next());
        int antallKolonner = Integer.parseInt(leser.next());
        leser.nextLine();
        char[][] ruteChars = new char[antallRader][antallKolonner];

        for(int i = 0; i < antallRader; i++){
            char[] linjen = leser.nextLine().toCharArray();
            ruteChars[i] = linjen;
        }

        Labyrint nyLabyrint = new Labyrint(antallRader,antallKolonner,ruteChars);
        nyLabyrint.settNaboer();
        return nyLabyrint;
    }

    // Henter rute paa gitt kolonne og rad
    public Rute hentRute(int kolonne, int rad){
        if(kolonne >= 1 && kolonne <= kolonner && rad >= 1 && rad <= rader){
            return ruter[rad-1][kolonne-1];
        }
        return null;
    }

    // Setter naboer for alle ruter i labyrinten
    public void settNaboer(){
        for(int x = 0; x < rader; x++){
            for(int y = 0; y < kolonner; y++){
                ruter[x][y].settNaboer();
            }
        }
    }

    // Bruker rekursjon for aa finne utvei fra en rute, returnerer Liste med utveiene
    public Liste<String> finnUtveiFra(int kolonne, int rad){
        Rute r = hentRute(kolonne,rad);
        return r.finnUtvei();
    }

    // returnerer String representasjon av labyrinten
    @Override
    public String toString(){
        String rep = "";
        for(int x = 0; x < rader; x++){
            for(int y = 0; y < kolonner; y++){
                rep = rep + ruter[x][y].tilTegn();
            }
            rep = rep + "\n";
        }
        return rep;
    }
}

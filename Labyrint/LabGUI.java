import javafx.application.Application;
import javafx.scene.Scene;
import javafx.stage.Stage;
import javafx.scene.layout.Pane;
import javafx.scene.layout.VBox;
import javafx.scene.layout.HBox;
import javafx.stage.FileChooser;
import javafx.scene.control.TextField;
import javafx.scene.control.Button;
import javafx.event.EventHandler;
import javafx.event.ActionEvent;
import javafx.scene.layout.GridPane;
import javafx.scene.layout.BorderStroke;
import javafx.scene.layout.BorderStrokeStyle;
import javafx.scene.layout.BorderWidths;
import javafx.scene.layout.Border;
import javafx.scene.paint.Color;
import javafx.scene.layout.Background;
import javafx.scene.layout.BackgroundFill;
import javafx.scene.input.MouseEvent;
import javafx.scene.layout.BorderPane;
import javafx.scene.control.Label;

import java.util.Scanner;
import java.io.File;
import java.io.FileNotFoundException;


public class LabGUI extends Application{

    private static final int SKJERM_BREDDE = 720;
    private static final int SKJERM_HOYDE = 600 ;

    private Labyrint laben;
    private BorderPane rot;
    private int rader;
    private int kolonner;
    private Liste<String> losninger;
    private GUIRute[][] ruter;
    private TextField antallInt;
    private boolean[][] tabell = null;

    @Override
    public void start(Stage vindu) throws Exception{
        rot = new BorderPane();
        rot.setTop(toppHBoks());
        Scene scene = new Scene(rot, SKJERM_BREDDE,SKJERM_HOYDE);
        Label antallTittel = new Label("Antall losninger:");
        antallInt = new TextField();
        VBox antall = new VBox(antallTittel,antallInt);
        rot.setRight(antall);

        vindu.setScene(scene);

        vindu.show();
        vindu.setTitle("Labyrint med GUI");
    }

    private HBox toppHBoks(){

        // Pathen til labyrint-filen
        TextField filPath = new TextField();

        // Aapner filvelger dialog
        Button velgFilKnapp = new Button("Velg fil...");
        velgFilKnapp.setOnAction(new EventHandler<ActionEvent>(){
                @Override
                public void handle(ActionEvent event){
                    FileChooser fileChooser = new FileChooser();
                    fileChooser.setTitle("Velg labyrintfil");
                    File selectedFile  = fileChooser.showOpenDialog(null);
                    if(selectedFile != null){

                        filPath.setText(selectedFile.getPath());
                    }
                }
            });

        // Lager labyrinten ut fra filen i tekstfeltet
        Button lastInnLab = new Button("Last inn");
        lastInnLab.setOnAction(new EventHandler<ActionEvent>(){
                @Override
                public void handle(ActionEvent event){
                    try{
                        tabell = null;
                        antallInt.setText("");
                        File filen = new File(filPath.getText());
                        laben = Labyrint.lesFraFil(filen);
                        Scanner sc = new Scanner(filen);
                        rader = Integer.parseInt(sc.next());
                        kolonner = Integer.parseInt(sc.next());
                        rot.setLeft(lagLabyrinten());

                    }catch(FileNotFoundException fe){
                        System.out.println("Fant ikke filen");
                    }
                }
            });

        return new HBox(100,velgFilKnapp,filPath,lastInnLab);

    }

    // Setter opp labyrinten med ruter, og returnerer "rutenettet"
    private GridPane lagLabyrinten(){
        ruter = new GUIRute[rader][kolonner];
        GridPane rutenett = new GridPane();
        for(int rad = 0; rad < rader; rad++){
            for(int kol = 0;kol < kolonner; kol++){
                Rute ruten = laben.hentRute(kol+1,rad+1);
                int storrelse = 7 + (SKJERM_BREDDE+SKJERM_HOYDE)/(rader*kolonner);
                if(ruten.tilTegn() == '.'){
                    ruter[rad][kol] = new GUIRute(storrelse,true,this,kol+1,rad+1);
                }else{
                    ruter[rad][kol] = new GUIRute(storrelse,false,this,kol+1,rad+1);
                }
                rutenett.add(ruter[rad][kol],kol,rad);
            }
        }
        return rutenett;
    }

    // Konverter losningString til en boolean tabell
    public static boolean[][] losningStringTilTabell(String losningString, int bredde, int hoyde){
        if(losningString != null){
            boolean[][] losning = new boolean[hoyde][bredde];
            java.util.regex.Pattern p = java.util.regex.Pattern.compile("\\(([0-9]+),([0-9]+)\\)");
            java.util.regex.Matcher m = p.matcher(losningString.replaceAll("\\s",""));
            while(m.find()) {
                int x = Integer.parseInt(m.group(1))-1;
                int y = Integer.parseInt(m.group(2))-1;
                losning[y][x] = true;
            }
            return losning;
        }
        return null;
    }


    // Klasse for rutene i labyrinten
    private class GUIRute extends Pane{
        boolean erHvit;
        LabGUI denne;
        int kolonne, rad;

        public GUIRute(int storrelse, boolean erHvit,LabGUI denne,int kolonne, int rad){
            this.erHvit = erHvit;
            this.denne = denne;
            this.kolonne = kolonne;
            this.rad = rad;
            setBorder(new Border(new BorderStroke(Color.BLACK,BorderStrokeStyle.SOLID,null,new BorderWidths(1))));
            setMinWidth(storrelse);
            setMinHeight(storrelse);
            if(erHvit){
                setBackground(new Background(new BackgroundFill(Color.WHITE,null,null)));
            }else{
                setBackground(new Background(new BackgroundFill(Color.BLACK,null,null)));
            }

            addEventHandler(MouseEvent.MOUSE_CLICKED, new EventHandler<MouseEvent>(){
                    public void handle(MouseEvent event){
                        if(erHvit){

                            endreFarge(Color.WHITE);

                            denne.losninger = denne.laben.finnUtveiFra(kolonne,rad);
                            denne.antallInt.setText("" + denne.losninger.storrelse());
                            denne.tabell = LabGUI.losningStringTilTabell(denne.losninger.fjern(),denne.kolonner,denne.rader);
                            endreFarge(Color.RED);

                        }
                    }
                });
        }

        public void endreFarge(Color c){
            if(denne.tabell != null){
                for(int rad = 0; rad < denne.rader; rad++){
                    for(int kol = 0; kol < denne.kolonner; kol++){
                        if(denne.tabell[rad][kol]){
                            denne.ruter[rad][kol].setBackground(new Background(new BackgroundFill(c,null,null)));
                        }
                    }
                }
            }

        }


    }


    public static void main(String[] args) {
        launch(args);
    }
}

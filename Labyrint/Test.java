import java.io.File;
import java.io.FileNotFoundException;

public class Test{
    public static void main(String[] args) throws FileNotFoundException{
        Labyrint test = Labyrint.lesFraFil(new File("5.in"));
        System.out.print(test);
        Liste<String> losn = test.finnUtveiFra(2,2);
        System.out.println(losn.storrelse());
        /*     for(String s : losn){
            System.out.println(s);
            }*/
    }
}

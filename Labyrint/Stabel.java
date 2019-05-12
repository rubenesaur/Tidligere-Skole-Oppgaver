import java.util.Iterator;

public class Stabel<T> implements Liste<T> {
    protected int elementer;

    // hode og node er Node pekere, for aa holde styr paa forste og siste node
    protected Node hode;
    protected Node hale;

    // Konstruktor setter opp alle linkene og lager "dummy node"
    public Stabel(){
        elementer = 0;
        hode = new Node(null);
        hale = hode;
        hode.neste = hale;
        hale.forrige = hode;
    }

    // Kjorer gjennom alle Noder i lenken og teller
    public int storrelse(){
        return elementer;
    }

    // Sjekker om listen er tom, altsaa om hode sin neste er hale
    public boolean erTom(){
        return hode.neste == hale;
    }

    // Lager en ny Node, og setter inn som forste ved aa oppdatere linkene
    public void settInn(T element){
        Node temp = new Node(element);
        temp.neste = hode.neste;
        temp.neste.forrige = temp;
        temp.forrige = hode;
        hode.neste = temp;
        elementer++;


    }

    // Fjerner den forste Noden og returnerer elementet
    public T fjern(){
        Node temp = hode.neste;
        temp.neste.forrige = hode;
        hode.neste = temp.neste;
        elementer--;
        return temp.element;
    }

    // Returnerer Iterator objektet
    public Iterator<T> iterator() {
        return new StabelIterator();
    }

    // En intern klasse, protected for aa ha tilgang til variablene i subklasser
    protected class Node{
        Node neste = null;
        Node forrige = null;
        T element;

        public Node(T element){
            this.element = element;
        }
    }

    // Iterator klassen, beskriver hvordan listen skal itereres gjennom
    private class StabelIterator implements Iterator{

        private Node temp;

        public StabelIterator(){
            temp = hode;
        }

        public boolean hasNext(){
            return temp.neste != hale;
        }

        public T next(){
            if(hasNext()){
                temp = temp.neste;
                return temp.element;
            }
            return null;
        }

        public void remove(){

        }
    }
}

import java.util.concurrent.*;
import java.util.Arrays;

public class Oblig4{
    int[] allMax;
    int globalMax;
    int n;
    int seed;
    int k;
    int[] a;
    int[] b;
    CyclicBarrier bar1,bar2,stepB,stepC,stepD;
    int globalCount[][];
    double globalTid;
    private static final int USE_BITS = 7;
    private static final int KJORINGER = 7;


    public static void main(String[] args) {
        if(args.length != 3){
            System.out.println("Wrong arguments, run with: java Oblig4 <n> <seed> <k> ");
            return;
        }
        int n = Integer.parseInt(args[0]);
        int seed = Integer.parseInt(args[1]);
        int k = Integer.parseInt(args[2]);
        int[] sourceArr = Oblig4Precode.generateArray(n,seed);
        int[] seqSortArr = null;
        int[] parSortArr = null;
        double[] seqTider = new double[KJORINGER];
        double[] parTider = new double[KJORINGER];

        Oblig4 seqKjoring,parKjoring;
        System.out.println("n: " + n + " seed: " + seed + " kjerner: " + k);
        for(int i = 0; i < KJORINGER; i++){
            seqKjoring = new Oblig4(n,seed,k);
            parKjoring = new Oblig4(n,seed,k);
            
            double startTid = System.nanoTime();
            seqSortArr = seqKjoring.seqSort(sourceArr.clone());
            seqTider[i] = (System.nanoTime()-startTid)/1000000.0;
            checkSort(seqSortArr);

            startTid = System.nanoTime();
            parSortArr = parKjoring.paraSort(sourceArr.clone());
            parTider[i] = (System.nanoTime()-startTid)/1000000.0;
            checkSort(parSortArr);

            compareArr(seqSortArr,parSortArr);
        }

        Arrays.sort(seqTider);
        Arrays.sort(parTider);
        
        System.out.println("Tid seq: " + seqTider[KJORINGER/2] +"ms" + " Tid par: " + parTider[KJORINGER/2] +"ms" + " Speedup: "+ String.format("%.2f",seqTider[KJORINGER/2]/parTider[KJORINGER/2]));

        Oblig4Precode.saveResults(Oblig4Precode.Algorithm.SEQ, seed,seqSortArr);
        Oblig4Precode.saveResults(Oblig4Precode.Algorithm.PARA, seed,parSortArr);

    }

    public Oblig4(int n, int seed, int k){
        this.n = n;
        this.seed = seed;
        this.k = k;
    }

    public static void compareArr(int[] a, int[] b){
        if(a.length == b.length){
            for(int i = 0; i < a.length; i++){
                if(a[i] != b[i]){
                    System.out.println("Not alike i:" + i + " a:"+a[i] + " b:" + b[i]);
                }
            }
        }else{
            System.out.println("Different length of arrays");
        }
    }

    public static void checkSort(int[] a){
        for(int i = 0; i < a.length-1; i++){
            if(a[i] > a[i+1]){
                System.out.println("Not sorted: a[" + i + "]: " + a[i] + " a[" + i+1 + "]: " + a[i+1]);
                return;
            }
        }
    }

    public int[] seqSort(int[] unsorted){
        a = unsorted;
        // Step A
        int max = 0;
        for(int i = 0; i < a.length; i++){
            if(a[i] > max){
                max = a[i];
            }
        }
        int numBits = 1;
        while(max >= (1L << numBits)){
            numBits++;
        }
        int numDigits = Math.max(1,numBits/USE_BITS);
        int[] bit = new int[numDigits];
        int rest = numBits % numDigits;
        for(int i = 0; i < bit.length; i++){
            bit[i] = numBits/numDigits;
            if(rest-- > 0){
                bit[i]++;
            }
        }
        b = new int[a.length];
        int shift = 0;
        for(int i = 0; i < bit.length; i++){
            seqRadix(bit[i],shift);
            shift += bit[i];

            int[] tmp = a;
            a = b;
            b = tmp;
        }
        return a;
    }

    private void seqRadix(int maskLen, int shift){
        int mask = (1 << maskLen) - 1;
        int[] count = new int[mask+1];

        // Step B
        for(int i = 0; i < a.length; i++){
            count[(a[i] >>> shift) & mask]++;
        }

        // Step C

        int accumulated = 0;
        int[] pointers = new int[count.length];
        for(int i = 0; i< count.length; i++){
            pointers[i] = accumulated;
            accumulated += count[i];
        }

        // Step D
        for(int i = 0; i < a.length; i++){
            b[pointers[(a[i] >>> shift) & mask]++] = a[i];
        }
    }

    public int[] paraSort(int[] unsorted){
        a = unsorted;

        // Step A
        int max = finnMax(a);
        int numBits = 1;
        while(max >= (1L << numBits)){
            numBits++;
        }
        int numDigits = Math.max(1,numBits/USE_BITS);
        int[] bit = new int[numDigits];
        int rest = numBits % numDigits;
        for(int i = 0; i < bit.length; i++){
            bit[i] = numBits/numDigits;
            if(rest-- > 0){
                bit[i]++;
            }
        }
        b = new int[a.length];
        int shift = 0;
        for(int i = 0; i < bit.length; i++){
            paraRadix(bit[i],shift);
            shift += bit[i];
            int[] tmp = a;
            a = b;
            b = tmp;
        }
        return a;
    }

    private void paraRadix(int maskLen, int shift){
        int mask = (1<<maskLen) - 1;
        globalCount = new int[k][];
        int partSize = n/k;
        stepB = new CyclicBarrier(k);
        stepC = new CyclicBarrier(k);
        stepD = new CyclicBarrier(k+1);
        globalTid = System.nanoTime();
        for(int i = 0; i < k; i++){
            int start = i*partSize;
            int stop = start + partSize;
            if(i == k-1){
                stop = n;
            }
            new Thread(new paraRadWorker(start,stop,shift,mask,i)).start();
        }
        try{
            stepD.await();
        }catch(Exception e){return;}
    }

    private int finnMax(int[] a){
        int partSize = n / k;
        allMax = new int[k];

        bar1 = new CyclicBarrier(k);
        bar2 = new CyclicBarrier(2);
        for(int i = 0; i < k; i++){
            int start = i*partSize;
            int stop = start + partSize;
            if(i == k-1){
                stop = n;
            }
            new Thread(new finnPara(a,start,stop,i)).start();
        }
        try{
            bar2.await();
        }catch(Exception e){return 0;}
        return globalMax;
    }

    class finnPara implements Runnable{
        int start;
        int stop;
        int[] a;
        int currentThread;
        int max;
        finnPara(int[] a, int start, int stop, int currentThread){
            this.start = start;
            this.stop = stop;
            this.a = a;
            this.currentThread = currentThread;
            max = 0;
        }

        public void run(){
            for(int i = start; i < stop; i++){
                if(a[i] > max){
                    max = a[i];
                }
            }
            allMax[currentThread] = max;
            try{
                bar1.await();
            }catch(Exception e){return;}
            if(currentThread == 0){
                globalMax = max;
                for(int i = 0; i < allMax.length; i++){
                    if(allMax[i] > globalMax){
                        globalMax = allMax[i];
                    }
                }

                try{
                    bar2.await();
                }catch(Exception e){return;}
            }
        }
    }

    class paraRadWorker implements Runnable{
        int shift, mask, start, stop;
        int[] localCount;
        int currThread;


        public paraRadWorker(int start, int stop,int shift, int mask, int currThread){
            this.start = start;
            this.stop = stop;
            this.shift = shift;
            this.mask = mask;
            localCount = new int[mask+1];
            this.currThread = currThread;
        }
        public void run(){
            // Step B
            for(int i = start; i < stop; i++){
                localCount[(a[i] >>> shift)&mask]++;
            }
            globalCount[currThread] = localCount;
            try{
                stepB.await();
            }catch(Exception e){return;}

            // Step C
            localCount = new int[mask+1];
            for(int i = 0; i < localCount.length; i++){
                int sum = 0;
                for(int r = 0; r < k; r++){
                    int s = 0;
                    while(s < i){
                        sum += globalCount[r][s];
                        s++;
                    }
                    if(s == i && r < currThread){
                        sum += globalCount[r][i];
                    }
                }
                localCount[i] = sum;
            }

            // Step D
            for(int i = start; i < stop; i++){
                b[localCount[(a[i] >>> shift) & mask]++] = a[i];
            }
            try{
                stepD.await();
            }catch(Exception e){return;}
        }

    }


}

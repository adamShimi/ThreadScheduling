import java.lang.Thread;

public class ThreadScheduling extends Thread {

    static {
        System.loadLibrary("pthreadAffinity");
    }

    //CPU number used for sched_affinity in native code
    public int affinity;
    //Actual "doing stuff" thread
    public Slave thread;

    public ThreadScheduling(int affinity, Slave thread) {
        this.affinity = affinity;
        this.thread = thread;
    }

    public static native int getCPUNumber();
    public native void setAffinity();

    public void run() {
        this.setAffinity();
    }

    public static void main(String[] args) {
        int nb = 0;
        if(args.length < 1) {
            System.out.println("Not enough arguments");
            System.exit(0);
        }
        try {
            nb = Integer.parseInt(args[0]);
        } catch (NumberFormatException e) {
            System.out.println("The argument needs to be an int");
            System.exit(0);
        }
        ThreadScheduling[] threadSched = new ThreadScheduling[nb];
        int nbCpu = ThreadScheduling.getCPUNumber();

        for(int i = 0; i < nb; i++) {
            threadSched[i] = new ThreadScheduling(i % nbCpu, new Slave(i));
            threadSched[i].start();
        }

        for(int i = 0; i < nb; i++) {
            try {
                threadSched[i].join();
            } catch (InterruptedException e) {
                Thread.currentThread().interrupt();
            }
        }
    }
}


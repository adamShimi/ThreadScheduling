import java.lang.Thread;

public class Slave extends Thread {

    public int number;

    public Slave(int number) {
        this.number = number;
    }

    public void run() {
        System.out.println(
                        "Thread number "
                        + this.number
                        + " is now running");
        try {
            Thread.sleep(5000);
        } catch (InterruptedException e) {
            Thread.currentThread().interrupt();
        }
        System.out.println(
                        "Thread number "
                        + this.number
                        + " is now closing");
    }

}

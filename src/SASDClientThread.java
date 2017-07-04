import java.net.Socket;

public class SASDClientThread extends Socket implements Runnable {
    private SASDGenericServer server;

    @Override
    public void run() {
        server.removeClient(this);
    }

    /**
     * Start a new thread with this server-side
     * client handler
     */
    public void start(SASDGenericServer server) {
        this.server = server;
        new Thread(this).start();
    }
}

import java.io.IOException;
import java.net.Socket;

public class SASDClientThread extends SASDPacketBay implements Runnable {
    private SASDGenericServer server;

    SASDClientThread(Socket socket) throws IOException {
        super(socket);
    }

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

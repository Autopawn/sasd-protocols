import java.io.IOException;
import java.net.ServerSocket;
import java.net.Socket;
import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;
import java.util.logging.Level;
import java.util.logging.Logger;

public class SASDGenericServer extends ServerSocket {
    // Set a logger
    private static final Logger LOGGER = Logger.getLogger(SASDGenericServer.class.getName());

    private boolean running = true;
    private List<SASDClientThread> clients = new ArrayList<>();

    public SASDGenericServer(int port) throws IOException {
        super(port);
        LOGGER.log(Level.INFO, "Server bound at port {0}", port);
    }

    public synchronized void removeClient(SASDClientThread client) {
        clients.remove(client);
    }

    public void run() {
        while (running) {
            try {
                SASDClientThread client = (SASDClientThread) accept();
                client.start(this);

            } catch (IOException e) {
                e.printStackTrace();
            }
        }
    }

    @Override
    protected void finalize() throws Throwable {
        // Close Server socket on object destruction
        close();
    }
}

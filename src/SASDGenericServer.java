import java.io.IOException;
import java.net.ServerSocket;
import java.util.ArrayList;
import java.util.Collections;
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

    public void removeClient(SASDClientThread client) {
        synchronized (clients) {
            clients.remove(client);
        }
    }

    public void broadcastPacket(SASDPacket packet, List<SASDClientThread> ignoreList) throws IOException {
        for (SASDClientThread client : clients) {
            if (!ignoreList.contains(client)) {
                client.sendPacket(packet);
            }
        }
    }

    public void broadcastPacket(SASDPacket packet) throws IOException {
        broadcastPacket(packet, Collections.emptyList());
    }

    public void run() {
        while (running) {
            try {
                SASDClientThread client = new SASDClientThread(accept());
                clients.add(client);
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

    public boolean isRunning() {
        return running;
    }
}

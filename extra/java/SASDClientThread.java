import java.io.IOException;
import java.net.Socket;
import java.util.ArrayList;
import java.util.Collections;

public class SASDClientThread extends SASDPacketBay implements Runnable {
    private SASDGenericServer server;

    SASDClientThread(Socket socket) throws IOException {
        super(socket);
    }

    @Override
    public void run() {
        while (server.isRunning()) {
            try {
                SASDPacket packet = recvPacket();
                server.broadcastPacket(packet,
                        Collections.singletonList(this));

            } catch (IOException | ClassNotFoundException e) {
                e.printStackTrace();
            }
        }

        //server.removeClient(this);
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

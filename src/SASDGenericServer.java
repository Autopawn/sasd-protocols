import java.io.IOException;
import java.net.ServerSocket;
import java.net.Socket;
import java.util.logging.Level;
import java.util.logging.Logger;

public class SASDGenericServer extends ServerSocket {
    // Set a logger
    private static final Logger LOGGER = Logger.getLogger(SASDGenericServer.class.getName());

    private boolean running = true;

    public SASDGenericServer(int port) throws IOException {
        super(port);
        LOGGER.log(Level.INFO, "Server bound at port {0}", port);
    }

    public void run() {
        while (running) {
            try {
                Socket client = accept();

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

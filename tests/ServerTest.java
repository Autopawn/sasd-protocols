import java.io.IOException;
import java.net.ServerSocket;
import java.net.Socket;

public class ServerTest extends ServerSocket {
    public ServerTest() throws IOException {
        super(5000);
    }

    public static void main(String[] args) {
        try {
            // Listen on port 5000
            ServerTest server = new ServerTest();

            // Accept client
            Socket client = server.accept();

            // Wrap socket in packet bay
            SASDPacketBay bay = new SASDPacketBay(client);

            // Send greetings packet
            SASDPacket greetings = new TypeAPacket(0, 1);
            bay.sendPacket(greetings);

            // Wait for response from client
            SASDPacket response = bay.recvPacket();

            // Print response
            System.out.println(response);

            // Send closing packet
            bay.sendPacket(new TypeAPacket(response.getId() + 1, 2));

            // Close connections
            bay.close();
            server.close();

        } catch (IOException | ClassNotFoundException e) {
            e.printStackTrace();
        }
    }
}

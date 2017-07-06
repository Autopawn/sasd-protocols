import java.io.IOException;
import java.net.Socket;

public class ClientTest extends SASDPacketBay {

    ClientTest() throws IOException {
        super(new Socket("localhost", 5000));
    }

    public static void main(String[] args) {
        try {
            // Connect to server on port 5000
            ClientTest client = new ClientTest();

            // Receive greetings packet
            SASDPacket greetings = client.recvPacket();

            // Print greetings
            System.out.println(greetings);

            // Send packet to server
            SASDPacket packet = new TypeBPacket(greetings.getId() + 1, 1, 42);
            client.sendPacket(packet);

            // Receive and print closing packet
            System.out.println(client.recvPacket());

            // Close socket
            client.close();

        } catch (IOException | ClassNotFoundException e) {
            e.printStackTrace();
        }

    }
}

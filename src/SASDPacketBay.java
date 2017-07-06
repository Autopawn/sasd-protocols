import java.io.IOException;
import java.io.ObjectInputStream;
import java.io.ObjectOutputStream;
import java.net.Socket;

public class SASDPacketBay {
    private final Socket s;

    SASDPacketBay(Socket socket) {
        s = socket;
    }

    SASDPacket recvPacket() throws IOException, ClassNotFoundException {
        return (SASDPacket) new ObjectInputStream(s.getInputStream()).readObject();
    }

    void sendPacket(SASDPacket packet) throws IOException {
        new ObjectOutputStream(s.getOutputStream()).writeObject(packet);
        s.getOutputStream().flush();
    }

    void close() throws IOException {
        s.close();
    }

    @Override
    protected void finalize() throws Throwable {
        s.close();
    }
}

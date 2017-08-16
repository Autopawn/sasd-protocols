import java.io.IOException;
import java.io.ObjectInputStream;
import java.io.ObjectOutputStream;
import java.net.Socket;

public class SASDPacketBay {
    private final Socket s;

    SASDPacketBay(Socket socket) {
        s = socket;
    }

    synchronized SASDPacket recvPacket() throws IOException, ClassNotFoundException {
        return (SASDPacket) new ObjectInputStream(s.getInputStream()).readObject();
    }

    synchronized void sendPacket(SASDPacket packet) throws IOException {
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

    @Override
    public boolean equals(Object o) {
        if (this == o) return true;
        if (o == null || getClass() != o.getClass()) return false;

        SASDPacketBay that = (SASDPacketBay) o;

        return s != null ? s.equals(that.s) : that.s == null;
    }

    @Override
    public int hashCode() {
        return s != null ? s.hashCode() : 0;
    }
}

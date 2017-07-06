import java.io.Serializable;

public class TypeAPacket extends SASDPacket implements Serializable {
    private int data;

    public TypeAPacket(int id, int data) {
        super(id);
        this.data = data;
    }

    @Override
    public String toString() {
        return "TypeAPacket{" +
                "data=" + data +
                '}';
    }
}
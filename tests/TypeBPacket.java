import java.io.Serializable;

public class TypeBPacket extends SASDPacket implements Serializable {
    private int data;
    private transient int thisIsNotIncluded;

    public TypeBPacket(int id, int data, int skip) {
        super(id);
        this.data = data;
        thisIsNotIncluded = skip;
    }

    public TypeBPacket(int id, int data) {
        this(id, data, 0);
    }

    @Override
    public String toString() {
        return "TypeBPacket{" +
                "data=" + data +
                ", thisIsNotIncluded=" + thisIsNotIncluded +
                '}';
    }
}

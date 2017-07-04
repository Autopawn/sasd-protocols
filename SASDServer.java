import java.net.ServerSocket;
import java.net.Socket;
import java.io.ObjectInputStream;
import java.io.ObjectOutputStream;

public class SASDServer{
    // Collects a serie of sockets, sends them to each one preriodically, detects when one disconnects.
    public static void main(String[] args) throws Exception{

        LinkedList<Socket> sockets = new LinkedList<Socket>();
        ServerSocket listener = new ServerSocket(5555);

        Socket new_socket = null;

        while(true){
            try{
                new_socket = null;
                // Receive incoming connection:
                new_socket = listener.accept();
                System.out.println("New connection: "+new_socket.getInetAddress());
                sockets.add(new_socket);
            }catch(Exception ex){
                // Close socket and print error.
                if(new_socket!=null) new_socket.close();
                ex.printStackTrace();
                System.out.println("Connection failed!");
            }
        }
    }

    private static void serve_clients(LinkedList<Socket> sockets){
        // Serves the clients sending whatever data the protocol indicates and the InetAddresses of
        // all the other connected clients.
        // TODO: implement.
    }
}

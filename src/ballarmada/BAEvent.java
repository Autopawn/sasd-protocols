public class BAEvent{
    int player;
    int button;
    // -1 = Creation of the player.
    // -2 = Destruction of the player.
    // 0 = Right.
    // 1 = Top.
    // 2 = Left.
    // 3 = Down.
    boolean state;
    // 0 = Released.
    // 1 = Pressed.
    int time;
}

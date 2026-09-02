world 8 5;

robot R at 1 2;
obstacle at 3 2;

behavior R {
    every tick {
        if obstacle_ahead() {
            turn_right();
        } else {
            move_forward();
        }
    }
}

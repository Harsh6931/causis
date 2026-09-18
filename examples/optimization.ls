world 10 10;

robot R at 1 1;
target T at 8 8;

behavior R {
    every tick {
        x = 2 + 3;

        if false {
            move_left();
        } else {
            move_right();
        }

        turn_left();
        turn_right();

        stop();

        move_up();
    }
}
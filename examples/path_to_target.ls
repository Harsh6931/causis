world 20 20;

robot R at 2 2;
target T at 17 17;

obstacle at 8 5;
obstacle at 8 6;
obstacle at 8 7;
obstacle at 8 8;

behavior R {
    every tick {
        if distance_to(T) == 0 {
            stop();
        }

        if obstacle_ahead() {
            turn_right();
            move_forward();
        } else {
            move_toward(T);
            if collision() {
                turn_right();
            }
        }
    }
}

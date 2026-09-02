world 10 10;

robot R at 1 1;
target T at 8 8;

behavior R {
    every tick {
        move_toward(T);
    }
}

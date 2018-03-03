

void init_clock(){
    for(int i = 0; i < MAX_MS; i++)
        vector_clock[i] = 0;
}

void increment_clock(){
    aquire_lock(CLOCK);

    vector_clock[atoi(me.id)]++;
    printf("[");
    int i;
    for(i = 0; i < MAX_MS-1; i++)
        printf("%u, ", vector_clock[i]);
    printf("%u]\n", vector_clock[i]);

    release_lock(CLOCK);
}

void update_and_increment_clock(unsigned int* vc){
    aquire_lock(CLOCK);

    int i;
    for(i = 0; i < MAX_MS; i++){
        if(vc[i] > vector_clock[i])
            vector_clock[i] = vc[i];
    }
    vector_clock[atoi(me.id)]++;
    printf("[");
    for(i = 0; i < MAX_MS-1; i++)
        printf("%u, ", vector_clock[i]);
    printf("%u]\n", vector_clock[i]);

    release_lock(CLOCK);
}

void client_update_and_increment_clock(){
    update_and_increment_clock((unsigned int*)&client_sock.rec_clock);
}

void server_update_and_increment_clock(){
    update_and_increment_clock((unsigned int*)&server_sock.rec_clock);
}

void freeze_clock(){
    aquire_lock(CLOCK);

    vector_clock[atoi(me.id)]++;
    for(int i = 0; i < MAX_MS; i++)
        frozen_clock[i] = vector_clock[i];

    release_lock(CLOCK);
}

void print_clock(){
    aquire_lock(CLOCK);
    int i;
    printf("[");
    for(i = 0; i < MAX_MS-1; i++)
        printf("%u, ", vector_clock[i]);
    printf("%u]\n", vector_clock[i]);

    release_lock(CLOCK);
}

void print_frozen_clock(){
    int i;
    printf("[");
    for(i = 0; i < MAX_MS-1; i++)
        printf("%u, ", frozen_clock[i]);
    printf("%u]\n", frozen_clock[i]);
}


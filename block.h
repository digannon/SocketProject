void print_block(struct block b){
    int i;
    printf("Timestamp:\t[");
    for(i = 0; i < MAX_MS-1; i++)
        printf("%u, ", b.timestamp[i]);
    printf("%u]\n", b.timestamp[i]);
    printf("Sender ID:\t%s\n", b.s_id);
    printf("Sender IP:\t%s\n", b.s_ip);
    printf("Sender Port:\t%s\n", b.s_port);
    printf("Initial Coins:\t%s\n", b.s_init_coins);
    printf("Ending Coins:\t%s\n", b.s_end_coins);
    printf("Reciever ID:\t%s\n", b.r_id);
    printf("Reciever IP:\t%s\n", b.r_ip);
    printf("Reciever Port:\t%s\n", b.r_port);
    printf("Initial Coins:\t%s\n", b.r_init_coins);
    printf("Ending Coins:\t%s\n", b.r_end_coins);
    printf("Block ID:\t%i\n", b.bid);
}

void print_blocks(){
    printf("-------------------\n");
    printf("--  Blockcahain  --\n");
    struct block* b = head_block;
    while(b != NULL){
        print_block(b[0]);
        printf("\n");
        b = b[0].next;
    }
    printf("-------------------\n");
    print_menu();
}

void add_block_fields(struct block* b, char* fields){
    int fidx = 0;
    int midx;
    char buffer[8];
    // Timestamp
    for(int i = 0; i < MAX_MS; i++){
        midx = 0;
        while(fields[fidx] != '$'){
            buffer[midx] = fields[fidx];
            fidx++;
            midx++;
        }
        buffer[midx] = '\0';
        b[0].timestamp[i] = atoi(buffer);
        fidx++;
    }
    // s_id
    midx = 0;
    while(fields[fidx] != '$'){
        b[0].s_id[midx] = fields[fidx];
        fidx++;
        midx++;
    }
    b[0].s_id[midx] = '\0';
    fidx++;
    // s_ip
    midx = 0;
    while(fields[fidx] != '$'){
        b[0].s_ip[midx] = fields[fidx];
        fidx++;
        midx++;
    }
    b[0].s_ip[midx] = '\0';
    fidx++;
    // s_port
    midx = 0;
    while(fields[fidx] != '$'){
        b[0].s_port[midx] = fields[fidx];
        fidx++;
        midx++;
    }
    b[0].s_port[midx] = '\0';
    fidx++;
    // s_init_coins
    midx = 0;
    while(fields[fidx] != '$'){
        b[0].s_init_coins[midx] = fields[fidx];
        fidx++;
        midx++;
    }
    b[0].s_init_coins[midx] = '\0';
    fidx++;
    // s_end_coins
    midx = 0;
    while(fields[fidx] != '$'){
        b[0].s_end_coins[midx] = fields[fidx];
        fidx++;
        midx++;
    }
    b[0].s_end_coins[midx] = '\0';
    fidx++;
    // r_id
    midx = 0;
    while(fields[fidx] != '$'){
        b[0].r_id[midx] = fields[fidx];
        fidx++;
        midx++;
    }
    b[0].r_id[midx] = '\0';
    fidx++;
    // r_ip
    midx = 0;
    while(fields[fidx] != '$'){
        b[0].r_ip[midx] = fields[fidx];
        fidx++;
        midx++;
    }
    b[0].r_ip[midx] = '\0';
    fidx++;
    // r_port
    midx = 0;
    while(fields[fidx] != '$'){
        b[0].r_port[midx] = fields[fidx];
        fidx++;
        midx++;
    }
    b[0].r_port[midx] = '\0';
    fidx++;
    // r_init_coins
    midx = 0;
    while(fields[fidx] != '$'){
        b[0].r_init_coins[midx] = fields[fidx];
        fidx++;
        midx++;
    }
    b[0].r_init_coins[midx] = '\0';
    fidx++;
    // r_end_coins
    midx = 0;
    while(fields[fidx] != '$'){
        b[0].r_end_coins[midx] = fields[fidx];
        fidx++;
        midx++;
    }
    b[0].r_end_coins[midx] = '\0';
    fidx++;
    // bid
    midx = 0;
    while(fields[fidx] != '$'){
        buffer[midx] = fields[fidx];
        fidx++;
        midx++;
    }
    buffer[midx] = '\0';
    b[0].bid = atoi(buffer);
}

void add_block(char* fields, int need_lock){
    if(need_lock > 0)
        aquire_lock(BLOCK);
    if(head_block == NULL){
        head_block = malloc(sizeof(struct block));
        add_block_fields(head_block, fields);
        tail_block = head_block;
    }
    else{
        struct block* b = malloc(sizeof(struct block));
        add_block_fields(b, fields);
        tail_block[0].next = b;
        tail_block = b;
    }
    num_blocks++;
    if(need_lock > 0)
        release_lock(BLOCK);
}

void add_blocks(char* full_block_mstr){
    aquire_lock(BLOCK);
    int fidx = 0;
    int midx;
    int count;
    char* fields;
    while(full_block_mstr[fidx] != '\0'){
        fields = malloc(256);
        count = 0; 
        midx = 0;
        while(count < MAX_MS+10){
            fields[midx] = full_block_mstr[fidx];
            midx++;
            fidx++;
            if(full_block_mstr[fidx] == '$')
                count++;
        }
        fields[midx] = full_block_mstr[fidx];
        midx++;
        fidx++;
        fields[midx] = '\0';
        add_block(fields, 0);
        free(fields);
    }
    release_lock(BLOCK);
}

void copy_fields(struct block* b1, struct block b2){
    int i = 0;
    for(i = 0; i < MAX_MS; i++)
        b1[0].timestamp[i] = b2.timestamp[i];
    sprintf(b1[0].s_id, "%i", atoi(b2.s_id));
    sprintf(b1[0].s_port, "%i", atoi(b2.s_port));
    sprintf(b1[0].s_init_coins, "%i", atoi(b2.s_init_coins));
    sprintf(b1[0].s_end_coins, "%i", atoi(b2.s_end_coins));
    sprintf(b1[0].r_id, "%i", atoi(b2.r_id));
    sprintf(b1[0].r_port, "%i", atoi(b2.r_port));
    sprintf(b1[0].r_init_coins, "%i", atoi(b2.r_init_coins));
    sprintf(b1[0].r_end_coins, "%i", atoi(b2.r_end_coins));
    b1[0].bid = b2.bid;
    i = 0;
    while(b2.s_ip[i] != '\0'){
        b1[0].s_ip[i] = b2.s_ip[i];
        i++;
    }
    b1[0].s_ip[i] = '\0';
    i = 0;
    while(b2.r_ip[i] != '\0'){
        b1[0].r_ip[i] = b2.r_ip[i];
        i++;
    }
    b1[0].r_ip[i] = '\0';
}

void copy_new_block(struct block b){
    aquire_lock(BLOCK);
    if(head_block == NULL){
        head_block = malloc(sizeof(struct block));
        copy_fields(head_block, b);
        tail_block = head_block;
    }
    else{
        struct block* b1 = malloc(sizeof(struct block));
        copy_fields(b1, b);
        tail_block[0].next = b1;
        tail_block = b1;
    }
    num_blocks++;
    release_lock(BLOCK);
}

void load_block(struct sock* s, struct block b){
    int i;
    char* obuf = (char*)&s[0].out_buf;
    int* oidx = (int*)&s[0].out_idx;
    char* buf = (char*)&s[0].buffer;
    // Timestamp
    for(i = 0; i < MAX_MS; i++){
        sprintf(s[0].buffer, "%u", b.timestamp[i]);
        buf_to_out(s);
    }
    // s_id
    i = 0;
    while(b.s_id[i] != '\0'){
        obuf[oidx[0]] = b.s_id[i];
        oidx[0]++;
        i++;
    }
    obuf[oidx[0]] = '$';
    oidx[0]++;
    // s_ip
    i = 0;
    while(b.s_ip[i] != '\0'){
        obuf[oidx[0]] = b.s_ip[i];
        oidx[0]++;
        i++;
    }
    obuf[oidx[0]] = '$';
    oidx[0]++;
    // s_port
    i = 0;
    while(b.s_port[i] != '\0'){
        obuf[oidx[0]] = b.s_port[i];
        oidx[0]++;
        i++;
    }
    obuf[oidx[0]] = '$';
    oidx[0]++;
    // s_init_coins
    i = 0;
    while(b.s_init_coins[i] != '\0'){
        obuf[oidx[0]] = b.s_init_coins[i];
        oidx[0]++;
        i++;
    }
    obuf[oidx[0]] = '$';
    oidx[0]++;
    // s_end_coins
    i = 0;
    while(b.s_end_coins[i] != '\0'){
        obuf[oidx[0]] = b.s_end_coins[i];
        oidx[0]++;
        i++;
    }
    obuf[oidx[0]] = '$';
    oidx[0]++;
    // r_id
    i = 0;
    while(b.r_id[i] != '\0'){
        obuf[oidx[0]] = b.r_id[i];
        oidx[0]++;
        i++;
    }
    obuf[oidx[0]] = '$';
    oidx[0]++;
    // r_ip
    i = 0;
    while(b.r_ip[i] != '\0'){
        obuf[oidx[0]] = b.r_ip[i];
        oidx[0]++;
        i++;
    }
    obuf[oidx[0]] = '$';
    oidx[0]++;
    // r_port
    i = 0;
    while(b.r_port[i] != '\0'){
        obuf[oidx[0]] = b.r_port[i];
        oidx[0]++;
        i++;
    }
    obuf[oidx[0]] = '$';
    oidx[0]++;
    // r_init_coins
    i = 0;
    while(b.r_init_coins[i] != '\0'){
        obuf[oidx[0]] = b.r_init_coins[i];
        oidx[0]++;
        i++;
    }
    obuf[oidx[0]] = '$';
    oidx[0]++;
    // r_end_coins
    i = 0;
    while(b.r_end_coins[i] != '\0'){
        obuf[oidx[0]] = b.r_end_coins[i];
        oidx[0]++;
        i++;
    }
    obuf[oidx[0]] = '$';
    oidx[0]++;

    sprintf(buf, "%i", b.bid);
    buf_to_out(s);

    obuf = NULL;
    oidx = NULL;
    buf = NULL;
}

void load_blocks(struct sock* s){
    struct block* b = head_block;
    while(b != NULL){
        load_block(s, b[0]);
        b = b[0].next;
    }
}

void client_load_block(struct block b){
    struct sock* s = (struct sock*) &client_sock;
    load_block(s, b);
    s = NULL;
}

void server_load_block(struct block b){
    struct sock* s = (struct sock*) &server_sock;
    load_block(s, b);
    s = NULL;
}

void client_load_blocks(){
    struct sock* s = (struct sock*) &client_sock;
    load_blocks(s);
    s = NULL;
}

void server_load_blocks(){
    struct sock* s = (struct sock*) &server_sock;
    load_blocks(s);
    s = NULL;
}
void delete_blocks(){
    struct block* b = head_block;
    while(b != NULL){
        b = b[0].next;
        free(head_block);
        head_block = b;
    }
    num_blocks = 0;
}

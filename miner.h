void print_miner(struct miner m){
    printf("ID:\t%s\n", m.id);
    printf("IP:\t%s\n", m.ip);
    printf("Port:\t%s\n", m.port);
    printf("Coins:\t%s\n", m.coins);
}
void print_miners(){
    printf("-------------------\n");
    printf("-----  Peers  -----\n");
    struct miner* m = head;
    while(m != NULL){
        print_miner(m[0]);
        printf("\n");
        m = m[0].next;
    }
    printf("-------------------\n");
}
void add_fields(struct miner* m, char* fields){
    int fidx = 0;
    // ID
    int midx = 0;
    while(fields[fidx] != '$'){
        m[0].id[midx] = fields[fidx];
        fidx++;
        midx++;
    }
    m[0].id[midx] = '\0';
    fidx++;
    // IP
    midx = 0;
    while(fields[fidx] != '$'){
        m[0].ip[midx] = fields[fidx];
        fidx++;
        midx++;
    }
    m[0].ip[midx] = '\0';
    fidx++;
    // Port
    midx = 0;
    while(fields[fidx] != '$'){
        m[0].port[midx] = fields[fidx];
        fidx++;
        midx++;
    }
    m[0].port[midx] = '\0';
    fidx++;
    // Coins
    midx = 0;
    while(fields[fidx] != '$'){
        m[0].coins[midx] = fields[fidx];
        fidx++;
        midx++;
    }
    m[0].coins[midx] = '\0';
}

void add_miner(char* fields){
    if( head == NULL ){
        head = malloc(sizeof(struct miner));
        add_fields(head, fields);
    }
    else{
        struct miner* m = malloc(sizeof(struct miner));
        add_fields(m, fields);
        head[0].last = m;
        m[0].next = head;
        head = m;
    }
    num_miners++;
}

void add_miners(char* full_mstr){
    int fidx = 0;
    int midx;
    int count;
    char* fields;
    while(full_mstr[fidx] != '\0'){
        fields = malloc(64);
        count = 0;
        midx = 0;
        while(count < 4){
            fields[midx] = full_mstr[fidx];
            midx++;
            fidx++;
            if(full_mstr[fidx] == '$')
                count++;
        }
        fields[midx] = full_mstr[fidx];
        midx++;
        fidx++;
        fields[midx] = '\0';
        add_miner(fields);
        free(fields);
    }
}


void load_miner(struct sock* s, struct miner m){
    int i;
    char* obuf = (char*)&s[0].out_buf;
    int* oidx = (int*)&s[0].out_idx;
    // ID
    i = 0;
    while(m.id[i] != '\0'){
        obuf[oidx[0]] = m.id[i];
        oidx[0]++;
        i++;
    }
    obuf[oidx[0]] = '$';
    oidx[0]++;

    // IP
    i = 0;
    while(m.ip[i] != '\0'){
        obuf[oidx[0]] = m.ip[i];
        oidx[0]++;
        i++;
    }
    obuf[oidx[0]] = '$';
    oidx[0]++;

    // Port
    i = 0;
    while(m.port[i] != '\0'){
        obuf[oidx[0]] = m.port[i];
        oidx[0]++;
        i++;
    }
    obuf[oidx[0]] = '$';
    oidx[0]++;

    // Coins
    i = 0;
    while(m.coins[i] != '\0'){
        obuf[oidx[0]] = m.coins[i];
        oidx[0]++;
        i++;
    }
    obuf[oidx[0]] = '$';
    oidx[0]++;

    obuf = NULL;
    oidx = NULL;
}

int deregister(){
    struct miner* m = head;
    while(m != NULL && atoi(m[0].id) != atoi(server_sock.m[0].id))
        m = m[0].next;
    if(m != NULL){
        if(m[0].last == NULL && m[0].next == NULL)
            head = NULL;
        else if(m[0].last == NULL && m[0].next != NULL){
            head = m[0].next;
            head[0].last = NULL;
        }
        else if(m[0].last != NULL && m[0].next == NULL)
            m[0].last[0].next = NULL;
        else{
            m[0].last[0].next = m[0].next;
            m[0].next[0].last = m[0].last;
        }
        free(m);
        num_miners--;
        //save();
        return 1; //SUCCESS
    }
    else
        return 0; //FAILURE
}

void client_load_miner(struct miner m){
    struct sock* s = (struct sock*) &client_sock;
    load_miner(s, m);
    s = NULL;
}

void server_load_miner(struct miner m){
    struct sock* s = (struct sock*) &server_sock;
    load_miner(s, m);
    s = NULL;
}

void load_miners(struct sock* s){
    struct miner* m = head;
    while(m != NULL){
        load_miner(s, m[0]);
        m = m[0].next;
    }
    m = NULL;
}

void client_load_miners(){
    struct sock* s = (struct sock*) &client_sock;
    load_miners(s);
    s = NULL;
}

void server_load_miners(){
    struct sock* s = (struct sock*) &server_sock;
    load_miners(s);
    s = NULL;
}

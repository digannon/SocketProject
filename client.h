char* server_ip;
unsigned int server_port;

void establish(){
    client_sock.sock = socket(AF_INET, SOCK_STREAM, 0);
    bzero(&client_sock.other_address, sizeof(client_sock.other_address));
    client_sock.other_address.sin_family = AF_INET;
    client_sock.other_address.sin_port = htons(server_port);
    inet_pton(AF_INET, server_ip, &client_sock.other_address.sin_addr);
    int c = connect(client_sock.sock, (struct sockaddr *) &client_sock.other_address, sizeof(client_sock.other_address));
    printf("Established connection with %i\n", atoi(client_sock.m[0].id));
}

void set_manager_ip_port(){
    printf("Enter manager IP: ");
    scanf("%s", manager.ip);
    printf("Enter manager Port: ");
    scanf("%s", manager.port);
}

void set_server_address(struct miner* m){
    if(m == NULL){
        client_sock.m = (struct miner*) &manager;
        server_ip = manager.ip;
        server_port = atoi(manager.port);
    }
    else{
        client_sock.m = m;
        server_ip = m[0].ip;
        server_port = atoi(m[0].port);
    }
}

void client_send(){
    printf("  Sending message to %u.\n", atoi(client_sock.m[0].id));
    client_sock.out_buf[client_sock.out_idx] = '\0';
    client_sock.out_idx++;
    write(client_sock.sock, client_sock.out_buf, client_sock.out_idx);
}

void client_read(){
    read(client_sock.sock, client_sock.in_buf, BUF_SIZE);
    client_parse_header();
    client_update_and_increment_clock();
    printf("  Message recieved from %u.\n", atoi(client_sock.m[0].id));
}

void client_close(){
    printf("Ending connection with %u\n", atoi(client_sock.m[0].id));
    close(client_sock.sock);
    print_menu();
}

void client_write_char(char c){
    client_sock.out_buf[client_sock.out_idx] = c;
    client_sock.out_idx++;
}

void manager_connect(int op){
    set_server_address(NULL);
    establish();
    client_header_out();
    if(op == 0){// Request ID from Manager
        client_write_char('r');
        client_load_miner(me);
        client_send();
        read(client_sock.sock, client_sock.in_buf, BUF_SIZE);
        client_parse_header();
        printf("  Message recieved from %u.\n", atoi(client_sock.m[0].id));
        client_buffer_payload(1);
        sprintf(me.id, "%i", atoi(client_sock.buffer));
        if(atoi(me.id) > 0){
            printf("  Request Successful! ID: %s\n", me.id);
            client_update_and_increment_clock();
        }
        else
            printf("  Requesnt Unseccessful :(\n");
        client_close();
    }
    else if(op == 1){ // Query Manager for other peers
        client_write_char('q');
        client_send();
        client_read();
        client_buffer_payload(0);
        add_miners(client_sock.buffer);
        if(num_miners > 0)
            printf("  Success! Number of Peers: %u\n", num_miners);
        else
            printf("  Query unsuccessful :(\n");
        client_close();
    }
}

void request_blocks(){
    aquire_lock(BLOCK);
    client_header_out();
    client_write_char('r');
    client_send();
    while(1 == 1){
        client_read();
        client_buffer_payload(0);
        if(client_sock.buffer[0] == '!')
            break;
        add_block(client_sock.buffer, 0);
        client_header_out();
        client_send();
    }
    release_lock(BLOCK);
}

void broadcast_arrival(){
    if(num_miners > 1){
        struct miner* m = head;
        freeze_clock();
        while(m != NULL){
            if(atoi(m[0].id) != atoi(me.id)){
                set_server_address(m);
                print_frozen_clock();
                establish();
                client_frozen_header_out();
                    client_write_char('r');
                    client_load_miner(me);
                    client_send();
                    client_read();
                    // Get # coins
                    client_in_to_buf();
                    sprintf(m[0].coins, "%i", atoi(client_sock.buffer));
                    // Get # blocks
                    client_in_to_buf();
                    if(atoi(client_sock.buffer) > num_blocks){
                        printf("  Peer has %i blocks.\n", atoi(client_sock.buffer));
                        request_blocks();
                    }
                    else{
                        printf("  Peer has few blocks, not updating blockchain.\n");
                        client_header_out();
                        client_write_char('n');
                        client_send();
                    }
                
                client_close();
            }
            m = m[0].next;
        }
    }
}

void broadcast_block(struct block* b){
    if(num_miners > 1){
        struct miner* m = head;
        freeze_clock();
        while(m != NULL){
            if(atoi(m[0].id) != atoi(me.id)){
                set_server_address(m);
                print_frozen_clock();
                establish();
                client_frozen_header_out();

                client_write_char('b');
                client_load_block(b[0]);
                client_send();

                client_close();
            }
            m = m[0].next;
        }
    }
}


void broadcast_confirmed(int bid){
    if(num_miners > 1){
        struct miner* m = head;
        freeze_clock();
        while(m != NULL){
            if(atoi(m[0].id) != atoi(me.id)){
                set_server_address(m);
                print_frozen_clock();
                establish();
                client_frozen_header_out();

                client_write_char('c');
                sprintf(client_sock.buffer, "%i", bid);
                client_buf_to_out();
                client_send();

                client_close();
            }
            m = m[0].next;
        }
    }
}

void dereg(){
    set_server_address(NULL);
    establish();
    client_header_out();
    client_write_char('d');
    client_load_miner(me);
    client_send();
    client_close();

    if(num_miners > 1){
        struct miner* m = head;
        freeze_clock();
        while(m != NULL){
            if(atoi(m[0].id) != atoi(me.id)){
                set_server_address(m);
                print_frozen_clock();
                establish();
                client_frozen_header_out();

                client_write_char('d');
                client_load_miner(me);
                client_send();

                client_close();
            }
            m = m[0].next;
        }
    }
    exit(0);
}

void client_block(struct block* b){
    aquire_lock(CLOCK);
    for(int i = 0; i < MAX_MS; i++)
        b[0].timestamp[i] = vector_clock[i];
    release_lock(CLOCK);

    
    copy(b[0].s_id, me.id);
    copy(b[0].s_ip, me.ip);
    copy(b[0].s_port, me.port);
    copy(b[0].s_init_coins, me.coins);
    sprintf(b[0].s_end_coins, "%i", atoi(me.coins)+pending_amount);

    copy(b[0].r_id, client_sock.m[0].id);
    copy(b[0].r_ip, client_sock.m[0].ip);
    copy(b[0].r_port, client_sock.m[0].port);
    copy(b[0].r_init_coins, client_sock.m[0].coins);
    sprintf(b[0].r_end_coins, "%i", atoi(client_sock.m[0].coins)-pending_amount);

    b[0].bid = proof_id+atoi(me.id)*100000;
    proof_id++;
}

void make_offer(int id, int offer){
    if(offer > atoi(me.coins)){
        printf("Not enough coins :(");
        return;
    }
    if(pending > 0){
        printf("Already in pending deal :(");
        return;
    }
    pending = 1;
    struct miner* m = get_miner(id);
    set_server_address(m);
    establish();
    client_header_out();
    client_write_char('o');
    sprintf(client_sock.buffer, "%i", offer);
    client_buf_to_out();
    client_send();
    client_read();
    if(client_sock.in_buf[client_sock.in_idx] == 'y'){
        pending_amount = offer*-1;
        printf("  Offer Accepted.\n");
        client_close();
        printf("Creating Pending Block\n");
        int widx = get_worker();
        client_block((struct block*)&pd[widx].p_block);
        broadcast_block((struct block*)&pd[widx].p_block);
        set(widx);
    }
    else{
        pending = 0;
        printf("  Offer Rejected.");
        client_close();
    }
}

void offer_in(){// Offer Coins
    if(pending > 0){
        printf("Already in pending deal :(");
        return;
    }

    char input_id[3];
    char input_offer[10];
    printf("Enter Reciever ID: ");
    scanf("%s", input_id);
    printf("Enter Offer: ");
    scanf("%s", input_offer);
    make_offer(atoi(input_id), atoi(input_offer));
}


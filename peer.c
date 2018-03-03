#include "locks.h"
#include "control.h"
#include "clock.h"
#include "miner.h"
#include "block.h"
#include "worker.h"
#include "server.h"
#include "client.h"

void init_me(char* argv[]){
    char fields[64];
    fields[0] = '0';
    fields[1] = '$';
    int fidx = 2;
    int aidx;
    for(int i = 1; i < 4; i++){
        aidx = 0;
        while(argv[i][aidx] != '\0'){ 
            fields[fidx] = argv[i][aidx];
            fidx++;
            aidx++;
        }
        fields[fidx] = '$';
        fidx++;
    }
    fields[fidx] = '\0';
    fidx++;
    add_fields(&me, fields);

    sprintf(manager.id, "%i", 0);
}

void *menu(){
    char menu[1];
    int choice;
    manager_connect(0);
    manager_connect(1);
    broadcast_arrival();
    while(1 == 1){
        scanf("%s", menu);
        choice = atoi(menu);
        if(choice == 0){
            printf("------------------\n");
            printf("------  Me  ------\n");
            print_miner(me);
            printf("------------------\n");
            print_menu();
        }
        else if(choice == 1){
            print_miners();
            print_menu();
        }
        else if(choice == 2)
            print_blocks();
        else if(choice == 3)
            offer_in();
        else if(choice == 4)
            dereg();
    }
}

void *worker_watcher(){
    while(1 == 1){
        for(int i = WORKER; i < WORKER+MAX_MS/2; i++){
            if(locks[i] > 0 && pd[i-WORKER].result > 0){
                if(pd[i-WORKER].already_confirmed == 0){
                    pd[i-WORKER].already_confirmed = 1;
                    copy_new_block(pd[i-WORKER].p_block);
                    int s_id = atoi(pd[i-WORKER].p_block.s_id);
                    int r_id = atoi(pd[i-WORKER].p_block.r_id);
                    struct miner* m = get_miner(s_id);
                    sprintf(m[0].coins, "%i", atoi(pd[i-WORKER].p_block.s_end_coins));
                    m = get_miner(r_id);
                    sprintf(m[0].coins, "%i", atoi(pd[i-WORKER].p_block.r_end_coins));
                    m = NULL;
                    if(r_id == atoi(me.id)){
                        sprintf(me.coins, "%i", atoi(pd[i-WORKER].p_block.r_end_coins));
                        pending = 0;
                    }
                    broadcast_confirmed(pd[i-WORKER].p_block.bid);
                }
                release_worker(i-WORKER);
            }
        }
    }
}

void send_blocks(){
    aquire_lock(BLOCK);
    printf("  Sending Blocks.\n");
    struct block* b = head_block;
    while(b != NULL){
        increment_clock();
        server_header_out();
        server_load_block(b[0]);
        server_send();
        printf("  Block Sent.\n");
        read(connfd, server_sock.in_buf, BUF_SIZE);
        b = b[0].next;
    }
    increment_clock();
    server_header_out();
    server_write_char('!');
    server_send();
    printf("  End Message Sent.\n");
    release_lock(BLOCK);
}

void registr(){
    printf("  New Peer Registerd.\n");
    server_buffer_payload(0);
    add_miner(server_sock.buffer);
    server_sock.m = head;
    printf("  Peer added.\n");
    increment_clock();
    server_header_out();
    sprintf(server_sock.buffer, "%i", atoi(me.coins));
    server_buf_to_out();
    sprintf(server_sock.buffer, "%i", num_blocks);
    server_buf_to_out();
    server_send();
    printf("  Response Sent.\n");

    read(connfd, server_sock.in_buf, BUF_SIZE);
    server_parse_header();
    server_update_and_increment_clock();
    printf("  Recieved message from: %u\n", atoi(server_sock.m[0].id));

    if(server_sock.in_buf[server_sock.in_idx] == 'r'){
        server_sock.in_idx++;
        send_blocks();
    }
    else
        printf("  Doesn't want my blocks.\n");
}

void offer(){
    printf("  Offer Recieved.\n");
    increment_clock();
    server_header_out();
    if(pending > 0){
        server_sock.out_buf[server_sock.out_idx] = 'n';
        server_sock.out_idx++;
        server_send();
        printf("  Rejection Send.\n");
    }
    else{
        pending = 1;
        server_buffer_payload(0);
        pending_amount = atoi(server_sock.buffer);
        //printf("out_idx %i\n", server_sock.out_idx);
        server_sock.out_buf[server_sock.out_idx] = 'y';
        server_sock.out_idx++;
        server_send();
        printf("  Acceptance Sent.\n");
    }
}

void pend_block(){
    printf("  Recieved Pending Block.\n");
    int widx = get_worker();
    server_buffer_payload(0);
    add_block_fields((struct block*)&pd[widx].p_block, server_sock.buffer);
    start_worker(widx);
}

void confirm(){
    printf("  Recieved Block Confirmation.\n");
    server_buffer_payload(1);
    int bid = atoi(server_sock.buffer);
    // check workers
    int i;
    for(i = 0; i < MAX_MS/2; i++){
        if(pd[i].p_block.bid == bid)
            break;
    }
    if(i < MAX_MS/2 && pd[i].already_confirmed == 0 ){
        pd[i].already_confirmed = 1;
        int s_id = atoi(pd[i].p_block.s_id);
        int r_id = atoi(pd[i].p_block.r_id);
        struct miner* m = get_miner(s_id);
        sprintf(m[0].coins, "%i", atoi(pd[i].p_block.s_end_coins));
        m = get_miner(r_id);
        sprintf(m[0].coins, "%i", atoi(pd[i].p_block.r_end_coins));
        m = NULL;
        if(s_id == atoi(me.id)){
            sprintf(me.coins, "%i", atoi(pd[i].p_block.s_end_coins));
            pending = 0;
        }
        else if(r_id == atoi(me.id)){
            sprintf(me.coins, "%i", atoi(pd[i].p_block.r_end_coins));
            pending = 0;
        }
        copy_new_block(pd[i].p_block);
        printf("  Block added to blockchain.\n");
    }
    else
        printf("  Block previously confirmed, ignoring confirmation.\n");
}

void respond(){
    read(connfd, server_sock.in_buf, BUF_SIZE);
    server_parse_header();
    server_update_and_increment_clock();
    printf("Recieved message from: %u\n", atoi(server_sock.m[0].id));

    if(server_sock.in_buf[server_sock.in_idx] == 'r'){
        server_sock.in_idx++;
        registr();
    }
    else if(server_sock.in_buf[server_sock.in_idx] == 'o'){
        server_sock.in_idx++;
        offer();
    }
    else if(server_sock.in_buf[server_sock.in_idx] == 'b'){
        server_sock.in_idx++;
        pend_block();
    }
    else if(server_sock.in_buf[server_sock.in_idx] == 'c'){
        server_sock.in_idx++;
        confirm();
    }
    else if(server_sock.in_buf[server_sock.in_idx] == 'd'){
        server_sock.in_idx++;
        deregister();
    }
    printf("Closing Connection with client: %u\n", atoi(server_sock.m[0].id));
    print_menu();
}

void *server_thread_action(){
    while(1 == 1){
        set_other_end();
        respond();
        setsockopt(connfd,SOL_SOCKET,SO_REUSEADDR,&true,sizeof(int));
    }
}

int main(int argc, char* argv[]){
    if(argc != 4){
        printf("Need \'IP  Port  Coins\'.");
        exit(1);
    }
    init_clock();
    init_workers();
    init_me(argv);
    set_me_server_address(atoi(argv[2]));
    make_socket();
    set_manager_ip_port();

    pthread_t watcher_thread;
    pthread_attr_t watcher_attr;
    pthread_attr_init(&watcher_attr);
    pthread_create(&watcher_thread, &watcher_attr, worker_watcher, NULL);

    pthread_t server_thread;
    pthread_attr_t server_attr;
    pthread_attr_init(&server_attr);
    pthread_create(&server_thread, &server_attr, server_thread_action, NULL);

    pthread_t          client_thread;
    pthread_attr_t     client_attr;
    pthread_attr_init(&client_attr);
    pthread_create(&client_thread, &client_attr, menu, NULL);


    pthread_join(watcher_thread, NULL);
    pthread_join(server_thread, NULL);
    pthread_join(client_thread, NULL);

}

#include "locks.h"
#include "control.h"
#include "clock.h"
#include "miner.h"
#include "server.h"

int next_id = 1;

char* file_name;

int get_next_id(){
    struct miner* m;
    while(1 == 1){
        m = head;
        while(m != NULL){
            if( atoi(m[0].id) == next_id )
                break;
            else
                m = m[0].next;
        }
        if(m == NULL)
            break;
        else{
            next_id++;
            if(next_id == MAX_MS)
                next_id = 1;
        }
    }
}

void registr(){
    printf("  Register Request\n");
    if(num_miners == MAX_MS-1){
        printf("  Network Full, Rejecting Request.\n");
        increment_clock();
        server_header_out();
        sprintf(server_sock.buffer, "%u", 0);
    }
    else{
        server_buffer_payload(0);
        get_next_id();
        add_miner(server_sock.buffer);
        sprintf(head[0].id, "%u", next_id);
        printf("  Accepted, Sending ID: %u\n", next_id);
        increment_clock();
        server_header_out();
        sprintf(server_sock.buffer, "%u", next_id);
    }
    server_buf_to_out();
    server_send();
    printf("   Response Sent.\n");
}

void query(){
    printf("  Query Request\n");
    increment_clock();
    server_header_out();
    server_load_miners();
    server_send();
    printf("  Peers Sent.\n");
}

void save(){
    printf("Saving to %s\n", file_name);
    FILE * fp; 
    fp = fopen(file_name, "w");
    char num[10];
    sprintf(num, "%u", num_miners);
    fputs(num, fp);
    fputs("\n", fp);
    struct miner* m = head;
    while(m != NULL){
        fputs(m[0].id, fp);
        fputs("\t", fp);
        fputs(m[0].ip, fp);
        fputs("\t", fp);
        fputs(m[0].port, fp);
        fputs("\n", fp);

        m = m[0].next;
    }

    fclose(fp);
}

void respond(){
    read(connfd, server_sock.in_buf, BUF_SIZE);
    server_parse_header();
    server_update_and_increment_clock();
    printf("Recieved message from: %u\n", atoi(server_sock.m[0].id));
    
    if(server_sock.in_buf[server_sock.in_idx] == 'r'){
        server_sock.in_idx++;
        registr();
        print_miners();
        save();
    }
    else if(server_sock.in_buf[server_sock.in_idx] == 'q'){
        server_sock.in_idx++;
        query();
    }
    else if(server_sock.in_buf[server_sock.in_idx] == 'd'){
        server_sock.in_idx++;
        deregister();
        print_miners();
        save();
    }
    printf("Closing Connection with client: %u\n", atoi(server_sock.m[0].id));
}

int main(int argc, char* argv[])
{
    if(argc < 2 || argc > 3){
        printf("Need \'Port\' \'(filename)\'.\n");
        exit(1);
    }
    sprintf(me.id, "%u", 0);
    init_clock();
    set_me_server_address(atoi(argv[1]));
    make_socket();

    if(argc == 3)
        file_name = argv[2];
    else{
        file_name = malloc(12);
        file_name[0] = 'd';file_name[1] = 'a';file_name[2] = 't';file_name[3] = 'a';
        file_name[4] = 'b';file_name[5] = 'a';file_name[6] = 's';file_name[7] = 'e';
        file_name[8] = '.';file_name[9] = 't';file_name[10] = 'x';file_name[11] = 't';
        file_name[12] = '\0';
    }

    while(1 == 1){
        set_other_end();
        respond();
        setsockopt(connfd,SOL_SOCKET,SO_REUSEADDR,&true,sizeof(int));
    }
}

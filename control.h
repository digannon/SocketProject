
#define BUF_SIZE 512
#define BACKLOG	128

unsigned int vector_clock[MAX_MS];
unsigned int frozen_clock[MAX_MS];

struct sock client_sock;
struct sock server_sock;


struct sockaddr_in me_address;
unsigned int other_address_size;
int connfd;
ssize_t n;
int true = 1;

int pending = 0;
int pending_amount = 0;

struct miner me;
struct miner manager;
struct miner* head;
unsigned int num_miners = 0;

struct miner{
    char id[3];
    char ip[16];
    char port[6];
    char coins[10];

    struct miner* next;
    struct miner* last;
};

struct miner* get_miner(int id){
    if(id == 0)
        return (struct miner*)&manager;
    else{
        struct miner* m = head;
        while(m != NULL){
            if(atoi(m[0].id) == id)
                return m;
            m = m[0].next;
        }
        return (struct miner*)&manager;
    }
}

struct sock{
    char in_buf[BUF_SIZE];
    char out_buf[BUF_SIZE];
    char buffer[BUF_SIZE];
    
    int in_idx;
    int out_idx;
    int buf_idx;

    struct miner* m;
    unsigned int rec_clock[MAX_MS];
    struct sockaddr_in other_address;
    int sock;
};

struct block{
    unsigned int timestamp[MAX_MS];

    char s_id[3]; //s is for sender
    char s_ip[16];
    char s_port[6];
    char s_init_coins[10];
    char s_end_coins[10];

    char r_id[3]; //r is for reciever
    char r_ip[16];
    char r_port[6];
    char r_init_coins[10];
    char r_end_coins[10];

    int bid; // block ID

    struct block* next;
};
struct block* head_block;
struct block* tail_block;
unsigned int num_blocks = 0;

void in_to_buf(struct sock* s){
    char* ibuf = (char*)&s[0].in_buf;
    char* buf = (char*)&s[0].buffer;
    int* iidx = (int*)&s[0].in_idx;
    int* bidx = (int*)&s[0].buf_idx;
    bidx[0] = 0;
    while(ibuf[iidx[0]] != '\0' && ibuf[iidx[0]] != '$'){
        buf[bidx[0]] = ibuf[iidx[0]];
        iidx[0]++;
        bidx[0]++;
    }
    buf[bidx[0]] = '\0';
    iidx[0]++;
    bidx[0]++;

    ibuf = NULL;
    buf = NULL;
    iidx = NULL;
    bidx = NULL;
}

void buf_to_out(struct sock* s){
    char* obuf = (char*)&s[0].out_buf;
    char* buf = (char*)&s[0].buffer;
    int* oidx = (int*)&s[0].out_idx;
    int* bidx = (int*)&s[0].buf_idx;
    bidx[0] = 0;
    while(buf[bidx[0]] != '\0'){
        obuf[oidx[0]] = buf[bidx[0]];
        oidx[0]++;
        bidx[0]++;
    }
    obuf[oidx[0]] = '$';
    oidx[0]++;

    obuf = NULL;
    buf = NULL;
    oidx = NULL;
    bidx = NULL;
}

void header_out(struct sock* s){
    s[0].out_idx = 0;
    char* obuf = (char*)&s[0].out_buf;
    char* buf = (char*)&s[0].buffer;
    int* oidx = (int*)&s[0].out_idx;
    obuf[0] = 0;
    int i;
    //Timestamp
    for(i = 0; i < MAX_MS; i++){
        sprintf(buf, "%u", vector_clock[i]);
        buf_to_out(s);
    }
    // ID
    i = 0;
    while(me.id[i] != '\0'){
        obuf[oidx[0]] = me.id[i];
        oidx[0]++;
        i++;
    }
    obuf[oidx[0]] = '$';
    oidx[0]++;

    obuf = NULL;
    buf = NULL;
    oidx = NULL;
}

void frozen_header_out(struct sock* s){
    s[0].out_idx = 0;
    char* obuf = (char*)&s[0].out_buf;
    char* buf = (char*)&s[0].buffer;
    int* oidx = (int*)&s[0].out_idx;
    obuf[0] = 0;
    int i;
    //Timestamp
    for(i = 0; i < MAX_MS; i++){
        sprintf(buf, "%u", frozen_clock[i]);
        buf_to_out(s);
    }
    // ID
    i = 0;
    while(me.id[i] != '\0'){
        obuf[oidx[0]] = me.id[i];
        oidx[0]++;
        i++;
    }
    obuf[oidx[0]] = '$';
    oidx[0]++;

    obuf = NULL;
    buf = NULL;
    oidx = NULL;
}

void parse_header(struct sock* s){
    s[0].in_idx = 0;
    int i;
    char* ibuf = (char*)&s[0].in_buf;
    char* buf = (char*)&s[0].buffer;

    unsigned int* rc = (unsigned *)&s[0].rec_clock;
    for(i = 0; i < MAX_MS; i++){
        in_to_buf(s);
        rc[i] = atoi(buf);
    }
    in_to_buf(s);
    s[0].m = get_miner(atoi(buf));

    ibuf = NULL;
    buf = NULL;
}

void copy(char* in, char* out){
    int i = 0;
    while(out[i] != '\0'){
        in[i] = out[i];
        i++;
    }
    in[i] = '\0';
}

void buffer_payload(struct sock* s, int back){
    char* ibuf = (char*)&s[0].in_buf;
    char* buf = (char*)&s[0].buffer;
    int* iidx = (int*)&s[0].in_idx;
    int* bidx = (int*)&s[0].buf_idx;
    bidx[0] = 0;
    while(ibuf[iidx[0]] != '\0'){
        buf[bidx[0]] = ibuf[iidx[0]];
        iidx[0]++;
        bidx[0]++;
    }
    bidx[0] -= back;
    buf[bidx[0]] = '\0';

    ibuf = NULL;
    buf = NULL;
    iidx = NULL;
    bidx = NULL;
}


void server_in_to_buf(){
    struct sock* s = (struct sock*) &server_sock;
    in_to_buf(s);
    s = NULL;
}

void server_buf_to_out(){
    struct sock* s = (struct sock*) &server_sock;
    buf_to_out(s);
    s = NULL;
}

void client_in_to_buf(){
    struct sock* s = (struct sock*) &client_sock;
    in_to_buf(s);
    s = NULL;
}

void client_buf_to_out(){
    struct sock* s = (struct sock*) &client_sock;
    buf_to_out(s);
    s = NULL;
}

void client_header_out(){
    struct sock* s = (struct sock*) &client_sock;
    header_out(s);
    s = NULL;
}

void server_header_out(){
    struct sock* s = (struct sock*) &server_sock;
    header_out(s);
    s = NULL;
}
void client_frozen_header_out(){
    struct sock* s = (struct sock*) &client_sock;
    frozen_header_out(s);
    s = NULL;
}

void server_frozen_header_out(){
    struct sock* s = (struct sock*) &server_sock;
    frozen_header_out(s);
    s = NULL;
}

void client_parse_header(){
    struct sock* s = (struct sock*) &client_sock;
    parse_header(s);
    s = NULL;
}

void server_parse_header(){
    struct sock* s = (struct sock*) &server_sock;
    parse_header(s);
    s = NULL;
}


void client_buffer_payload(int back){
    struct sock* s = (struct sock*) &client_sock;
    buffer_payload(s, back);
    s = NULL;
}

void server_buffer_payload(int back){
    struct sock* s = (struct sock*) &server_sock;
    buffer_payload(s, back);
    s = NULL;
}

void print_menu(){
    printf("-----------------------------------\n");
    printf("        0: Print My Info\n");
    printf("        1: Print Peers\n");
    printf("        2: Print Blockchain\n");
    printf("        3: Offer Coins\n");
    printf("        4: Deregister\n");
    printf("-----------------------------------\n");
}

struct proof_data{
    struct block p_block;
    int already_confirmed;
    int result;
};

int idx;

pthread_t worker[MAX_MS/2];
pthread_attr_t worker_attr[MAX_MS/2];
struct proof_data pd[MAX_MS/2];
int proof_id = 0;

int init_workers(){
    for(int i = 0; i < MAX_MS/2; i++){
        pd[i].already_confirmed = 0;
        pd[i].result = 0;
    }
}

int get_worker(){
    int lidx;
    while(1 == 1){
        lidx = WORKER;
        while(lidx < WORKER + MAX_MS/2){
            if(locks[lidx] == 0){
                aquire_lock(lidx);
                return lidx-WORKER;
            }
            lidx++;
        }
    }
}

void release_worker(int widx){
    pd[widx].already_confirmed = 0;
    pd[widx].result = 0;
    pthread_join( worker[widx], NULL );
    release_lock(widx+WORKER);
}
void *proof(){
    int ix = idx;
    release_lock(IDX);
    printf("Starting proof %i\n", ix);
    int p = 15485863;
    int d;
    for(int i = 0; i < 50; i++){
        for(d = 3; d <= p/3; d++){
            if(p % d == 0)
                break;
            d++;
        }
    }
    pd[idx].result = 1;
    printf("Proof %i done\n", ix);
}

void set(int widx){
    pd[widx].already_confirmed = 0;
    pd[widx].result = 0;
}

void start_worker(int widx){
    pd[widx].already_confirmed = 0;
    pd[widx].result = 0;
    pthread_attr_init(&(worker_attr[widx]));
    aquire_lock(IDX);
    idx = widx;
    printf("Starting worker %i\n", widx);
    pthread_create( &(worker[widx]), &(worker_attr[widx]), proof, NULL );
}

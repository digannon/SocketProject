

void set_me_server_address(unsigned short port_num){
    /* Construct local address structure */
    memset(&me_address, 0, sizeof(me_address));   /* Zero out structure */
    me_address.sin_family = AF_INET;                /* Internet address family */
    me_address.sin_addr.s_addr = htonl(INADDR_ANY); /* Any incoming interface */
    me_address.sin_port = htons(port_num);      /* Local port */
}

void make_socket(){
    server_sock.sock = socket(AF_INET, SOCK_STREAM, 0);
    bind(server_sock.sock, (struct sockaddr *) &me_address, sizeof(me_address));
    listen(server_sock.sock, BACKLOG);
}

void set_other_end(){
    other_address_size = sizeof(server_sock.other_address);
    connfd = accept( server_sock.sock, (struct sockaddr *) &server_sock.other_address, &other_address_size );
    printf("Connection Request Recieved\n");
}

void server_send(){
    server_sock.out_buf[server_sock.out_idx] = '\0';
    server_sock.out_idx++;
    write(connfd, server_sock.out_buf, server_sock.out_idx);
}


void server_write_char(char c){
    server_sock.out_buf[server_sock.out_idx] = c;
    server_sock.out_idx++;
}

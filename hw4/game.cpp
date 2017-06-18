#include <iostream>
#include <cstdlib>
#include <cstdio>
#include <unistd.h> // getopt
#include <string>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/errno.h>
#include <netdb.h>
#include <fcntl.h>

#include "othello.h"

static int width;
static int height;
static int cx = 3;
static int cy = 3;

using namespace std;

int play_game(int sockfd, bool first){

    bool your_turn = first;

    initscr();			// start curses mode
	getmaxyx(stdscr, height, width);// get screen size

	cbreak();			// disable buffering
					// - use raw() to disable Ctrl-Z and Ctrl-C as well,
	halfdelay(1);			// non-blocking getch after n * 1/10 seconds
	noecho();			// disable echo
	keypad(stdscr, TRUE);		// enable function keys and arrow keys
	curs_set(0);			// hide the cursor

	init_colors();

 restart:
	clear();
	cx = cy = 3;
	init_board();
	draw_board();
	draw_cursor(cx, cy, 1);
	draw_score();
	refresh();

	attron(A_BOLD);
	move(height-1, 0);	printw("Arrow keys: move; Space: put GREEN; Return: put PURPLE; R: reset; Q: quit");
	attroff(A_BOLD);

	while(true) {			// main loop
        bool turn = false;
        int ch = getch();
        if( your_turn ){ // yes your turn, eat from keyboard and send it through
            char buffer[5];
            sprintf(buffer, "%d", ch);
            write(sockfd, buffer, sizeof(buffer)-1);
            if( read(sockfd, buffer, sizeof(buffer)-1) != -1){ // someone want to say something
                ch = buffer[0];
            }
        }
        else{ // not your turn
            char buffer[5];
            if( ch == 'r' || ch == 'R' || ch == 'q' || ch == 'Q'){ // not your turn but you pressed 'r' or 'q'
                buffer[0] = ch;
                buffer[1] = '\0';
                write(sockfd, buffer, sizeof(buffer)-1);
            }
            else if(read(sockfd, buffer, sizeof(buffer)-1) == -1){ // nothing to read
                ch = '\0';
            }
            else{ // reading succeed
                ch = atoi(buffer);
            }
        }
		int moved = 0;

		switch(ch) {
    		case ' ':
    			board[cy][cx] = PLAYER1;
    			draw_cursor(cx, cy, 1);
    			draw_score();
                turn = true;
    			break;
    		case 0x0d:
    		case 0x0a:
    		case KEY_ENTER:
    			board[cy][cx] = PLAYER2;
    			draw_cursor(cx, cy, 1);
    			draw_score();
                turn = true;
    			break;
    		case 'q':
    		case 'Q':
    			goto quit;
    			break;
    		case 'r':
    		case 'R':
    			goto restart;
    			break;
    		case 'k':
    		case KEY_UP:
    			draw_cursor(cx, cy, 0);
    			cy = (cy-1+BOARDSZ) % BOARDSZ;
    			draw_cursor(cx, cy, 1);
    			moved++;
    			break;
    		case 'j':
    		case KEY_DOWN:
    			draw_cursor(cx, cy, 0);
    			cy = (cy+1) % BOARDSZ;
    			draw_cursor(cx, cy, 1);
    			moved++;
    			break;
    		case 'h':
    		case KEY_LEFT:
    			draw_cursor(cx, cy, 0);
    			cx = (cx-1+BOARDSZ) % BOARDSZ;
    			draw_cursor(cx, cy, 1);
    			moved++;
    			break;
    		case 'l':
    		case KEY_RIGHT:
    			draw_cursor(cx, cy, 0);
    			cx = (cx+1) % BOARDSZ;
    			draw_cursor(cx, cy, 1);
    			moved++;
    			break;
		}
        if(turn)
            your_turn = !your_turn;
		if(moved) {
			refresh();
			moved = 0;
		}
		napms(1);		// sleep for 1ms
	}

 quit:
	endwin();			// end curses mode

	return 0;
}

int hostname_to_ip(const char *hostname , char *ip){
    struct hostent *he;
    struct in_addr **addr_list;
    int i;

    if ( (he = gethostbyname( hostname ) ) == NULL)
    {
        // get the host info
        herror("gethostbyname");
        return 1;
    }

    addr_list = (struct in_addr **) he->h_addr_list;

    for(i = 0; addr_list[i] != NULL; i++)
    {
        //Return the first one;
        strcpy(ip , inet_ntoa(*addr_list[i]) );
        return 0;
    }
    return 1;
}

int server(int port){
    int sockfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    //fcntl(sockfd, F_SETFL, O_NONBLOCK); // non blocking io
    struct sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr)); // clear it
    server_addr.sin_family = AF_INET; // ipv4
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(port);
    if( bind(sockfd, (struct sockaddr*)&server_addr, sizeof(server_addr)) == -1 ){
        perror( strerror(errno) );
        return -1;
    }

    // listen for a request
    listen(sockfd, 20);

    // accept a request
    while(1){
        cout << "Waiting for a client on port " << port << "..." <<endl;

        struct sockaddr_in clnt_addr;
        socklen_t clnt_addr_size = sizeof(clnt_addr);
        int clnt_sock = accept(sockfd, (struct sockaddr*)&clnt_addr, &clnt_addr_size);
        fcntl(clnt_sock, F_SETFL, O_NONBLOCK); // non blocking io
        cout << "accept!" <<endl;

        // send something to the client
        play_game(clnt_sock, true);
        close(clnt_sock);
    }

    // close everything
    close(sockfd);

    return 0;
}

int client(string ip, int port){
    // create socket
    int sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    fcntl(sock, F_SETFL, O_NONBLOCK); // non blocking io

    // send request to the server
    // char ip_cstr[50]; strcpy(ip_cstr, ip.c_str());
    char ip_cstr[50];
    if( hostname_to_ip(ip.c_str(), ip_cstr) != 0) strcpy(ip_cstr, ip.c_str()); // resolve the hostname
    struct sockaddr_in serv_addr;
    memset(&serv_addr, 0, sizeof(serv_addr));  // clear it
    serv_addr.sin_family = AF_INET;  // ipv4
    serv_addr.sin_addr.s_addr = inet_addr(ip_cstr); // ip address
    serv_addr.sin_port = htons(port);  // port
    while( connect(sock, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) != 0 ) usleep(100 * 1000); // 100ms

    // read from server
    play_game(sock, false);

    // close everything
    close(sock);

    return 0;
}

int main(int argc, char *argv[]){
    // get args for distinguishing the app is for server or client
    bool is_server = false;
    int server_port = -1;
    string ip_with_port;

    if( argc < 2 ){
        cout << "Too few arguments!" <<endl;
        return -1;
    }
    char command = '\0';
    while( (command=getopt(argc, argv, "s:c:")) != -1 ){
        switch (command) {
            case 's': // server
                is_server = true;
                server_port = atoi(optarg);
                break;
            case 'c': // client
                is_server = false;
                ip_with_port = string(optarg);
                break;
        }
    }
    if(optind != argc){
        cout << "Argument error!" <<endl;
        return -1;
    }

    // deal with server
    if(is_server){
        server(server_port);
    }
    else{// deal with client
        // separate ip with port
        size_t sm_pos = ip_with_port.find(":"); // semicolon position
        if( sm_pos == string::npos ){ // cannnot find it
            cout << "No port provided!" <<endl;
            return -1;
        }
        string ip = ip_with_port.substr(0, sm_pos);
        int port = stoi(ip_with_port.substr(sm_pos+1));
        client(ip, port);
    }

    return 0;
}

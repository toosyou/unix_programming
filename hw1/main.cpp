#include <iostream>
#include <cstdio>
#include <vector>
#include <arpa/inet.h>
#include <cstdlib>
#include <fstream>
#include <cstring>
#include <map>
#include <sys/types.h>
#include <dirent.h>
#include <unistd.h>
#include <getopt.h>
#include <regex>

using namespace std;

#define ADDRESS_TCP "/proc/net/tcp"
#define ADDRESS_TCP6 "/proc/net/tcp6"
#define ADDRESS_UDP "/proc/net/udp"
#define ADDRESS_UDP6 "/proc/net/udp6"

struct proc_net{
    bool ipv4;
    bool tcp;
    char local_ip[200];
    int local_port;
    char remote_ip[200];
    int remote_port;
    int uid;
    int inode;

    proc_net(bool is_ipv4 = true, bool is_tcp=true){
        this->ipv4 = is_ipv4;
        this->tcp = is_tcp;
    }

    char* _read_ip(char* ip_str, char* output_str, bool is_ipv4 = true){
        if(is_ipv4){
            int tmp_ip[4];
            for(int i=0;i<4;++i){
                char* dontcare = NULL;
                char hex_str[3] = {0};
                hex_str[0] = ip_str[i*2];
                hex_str[1] = ip_str[i*2+1];
                hex_str[2] = '\0';
                tmp_ip[3-i] = strtol(hex_str, &dontcare, 16);
            }
            sprintf(output_str, "%d.%d.%d.%d", tmp_ip[0], tmp_ip[1], tmp_ip[2], tmp_ip[3]);
        }
        else{
            struct in6_addr tmp_ip;

            if (sscanf(ip_str,
                "%2hhx%2hhx%2hhx%2hhx%2hhx%2hhx%2hhx%2hhx%2hhx%2hhx%2hhx%2hhx%2hhx%2hhx%2hhx%2hhx",
                &tmp_ip.s6_addr[3], &tmp_ip.s6_addr[2], &tmp_ip.s6_addr[1], &tmp_ip.s6_addr[0],
                &tmp_ip.s6_addr[7], &tmp_ip.s6_addr[6], &tmp_ip.s6_addr[5], &tmp_ip.s6_addr[4],
                &tmp_ip.s6_addr[11], &tmp_ip.s6_addr[10], &tmp_ip.s6_addr[9], &tmp_ip.s6_addr[8],
                &tmp_ip.s6_addr[15], &tmp_ip.s6_addr[14], &tmp_ip.s6_addr[13], &tmp_ip.s6_addr[12]) == 16){

                inet_ntop(AF_INET6, &tmp_ip, output_str, 200);
            }
        }
        return output_str;
    }

    void read_remote_ip(char* ip_str){
        this->_read_ip(ip_str, this->remote_ip, this->ipv4);
        return;
    }

    void read_local_ip(char* ip_str){
        this->_read_ip(ip_str, this->local_ip, this->ipv4);
        return;
    }

    void read_local_ip_port(char* ip_port_str){
        char *dontcare = NULL;
        char *ip_str = strtok(ip_port_str, ":");
        char *port_str = strtok(NULL, ":");
        this->read_local_ip(ip_str);
        this->local_port = strtol(port_str, &dontcare, 16);
    }

    void read_remote_ip_port(char* ip_port_str){
        char *dontcare = NULL;
        char *ip_str = strtok(ip_port_str, ":");
        char *port_str = strtok(NULL, ":");
        this->read_remote_ip(ip_str);
        this->remote_port = strtol(port_str, &dontcare, 16);
    }

};

struct program{
    int PID;
    char name[200];
};

int number_of_digits(int input){
    int number_of_digits = 0;

    do {
        ++number_of_digits;
        input /= 10;
    } while (input);

    return number_of_digits;
}

proc_net read_one_net(fstream &fin, bool ipv4=true, bool tcp=true){
    proc_net rtn(ipv4, tcp);

    char buffer[200] = {0};
    int number_end_dontcare = tcp ? 7 : 3;

    fin >> buffer; //local_address:port
    rtn.read_local_ip_port(buffer);
    fin >> buffer; //rem_address:port
    rtn.read_remote_ip_port(buffer);
    for(int i=0;i<4;++i)
        fin >> buffer; // dontcare
    fin >> rtn.uid;
    fin >> buffer; //timeout
    fin >> rtn.inode;
    for(int i=0;i<number_end_dontcare;++i)
        fin >> buffer; //dontcare

    return rtn;
}

vector<proc_net> read_net(const char* address_net, bool ipv4, bool tcp){
    vector<proc_net> rtn;

    char buffer[200] = {0};
    int number_title = tcp ? 12 : 15;
    fstream fin(address_net, fstream::in);
    if(fin.is_open() == false)
        return rtn;

    //headline, dontcare
    for(int i=0;i<number_title;++i){
        fin >> buffer;
    }
    while(fin >> buffer){ // {sl}:
        rtn.push_back( read_one_net(fin, ipv4, tcp) );
    }
    fin.close();

    return rtn;
}

vector<proc_net> read_net(const char* address_net, const char* type){
    bool ipv4 = true;
    bool tcp = true;
    if(strcmp(type, "tcp") == 0){
        ipv4 = true;
        tcp = true;
    }
    else if(strcmp(type, "tcp6") == 0){
        ipv4 = false;
        tcp = true;
    }
    else if(strcmp(type, "udp") == 0){
        ipv4 = true;
        tcp = false;
    }
    else if(strcmp(type, "udp6") == 0){
        ipv4 = true;
        tcp = false;
    }
    return read_net(address_net, ipv4, tcp);
}

bool is_number(char* str){
    for(int i=0;str[i]!='\0';++i){
        if(isdigit(str[i]) == false)
            return false;
    }
    return true;
}

char* get_program_name(char* full_path){
    char* rtn = NULL;
    for(int i=0;full_path[i]!='\0';++i){
        if(full_path[i] == '/')
            rtn = full_path + i;
    }
    return rtn+1;
}

int get_inode_from_fd_link(char* fd_link){
    if(strncmp(fd_link, "socket:[", 8) != 0)
        return -1;
    return atoi(fd_link+8);
}

map<int, program> build_inode_program_map(void){
    map<int, program> inode_program;

    // read through /proc/[PID]s
    DIR* proc_dir = opendir("/proc/");
    struct dirent *pid_dir_ptr;
    while((pid_dir_ptr = readdir(proc_dir))!=NULL){
        if(is_number(pid_dir_ptr->d_name)){
            // get pid and name for program structure
            char exe_path[200] = {0};
            char full_link_exe[200] = {0};
            program this_program;
            this_program.PID = atoi(pid_dir_ptr->d_name);
            sprintf(exe_path, "/proc/%s/exe", pid_dir_ptr->d_name);
            if(readlink(exe_path, full_link_exe, 200) == -1)
                continue;
            strcpy(this_program.name, get_program_name(full_link_exe));

            // read through /proc/[PID]/fd/[fd_number]s
            struct dirent *fd_dir_ptr;
            char fd_dir_address[200] = {0};
            DIR* fd_dir;
            sprintf(fd_dir_address, "/proc/%s/fd", pid_dir_ptr->d_name);
            fd_dir = opendir(fd_dir_address);
            while((fd_dir_ptr = readdir(fd_dir)) != NULL){
                if(is_number(fd_dir_ptr->d_name)){
                    char fd_path[200] = {0};
                    char link_path[200] = {0};
                    int inode = 0;
                    sprintf(fd_path, "/proc/%s/fd/%s", pid_dir_ptr->d_name, fd_dir_ptr->d_name);
                    if(readlink(fd_path, link_path, 200) == -1)
                        continue;
                    if((inode = get_inode_from_fd_link(link_path)) != -1){
                        inode_program[inode] = this_program;
                    }
                }
            }
        }
    }

    return inode_program;
}

void print_title(void){
    cout << "Proto Local Address"<< "\t\t" << "Foreign Address" << "\t\t" << "PID/Program name and arguments" <<endl;
    return;
}

void print_tcp_udp(proc_net &net){
    if(net.tcp)
        cout << "tcp";
    else
        cout << "udp";
    if(net.ipv4 == false)
        cout << "6";
    else
        cout << " ";
    cout << "  ";
    return;
}

void print_local_ip_port(proc_net &net){
    cout << net.local_ip << ":" << net.local_port << "\t";
    // ip:port is too short
    if(strlen(net.local_ip) + number_of_digits(net.local_port) < 8)
        cout << "\t";
    if(strlen(net.local_ip) + number_of_digits(net.local_port) < 16)
        cout << "\t";
    return;
}

void print_remote_ip_port(proc_net &net){
    cout << net.remote_ip << ":";
    if(net.remote_port != 0)
        cout << net.remote_port << "\t";
    else
        cout << "*" << "\t";
    // ip:port is too short
    if(strlen(net.remote_ip) + number_of_digits(net.remote_port) < 8)
        cout << "\t";
    if(strlen(net.remote_ip) + number_of_digits(net.remote_port) < 16)
        cout << "\t";
    return;
}

void print_net(vector<proc_net> &nets, map<int, program> &inode_program, bool use_filter, char* filter){
    for(int i=0;i<nets.size();++i){
        if(use_filter){
            if(inode_program.find(nets[i].inode) != inode_program.end()){ // find
                program &this_program = inode_program[ nets[i].inode ];
                try{
                    regex regex_filter(filter);
                    if(regex_search(std::string(this_program.name), regex_filter) == false)
                        continue;
                } catch(regex_error &e){
                    continue;
                }
            }
            else // not find
                continue;
        }
        print_tcp_udp(nets[i]);
        print_local_ip_port(nets[i]);
        print_remote_ip_port(nets[i]);

        //print pid and program name
        if(inode_program.find(nets[i].inode) != inode_program.end()){
            program &this_program = inode_program[ nets[i].inode ];
            cout << this_program.PID << "/" << this_program.name;
        }

        cout <<endl;
    }
    return;
}

int main(int argc, char *argv[]){

    const char* short_options = "tu";
    struct option long_options[] = {
        { "tcp", 0, NULL, 't' },
        { "udp", 0, NULL, 'u' }
    };

    bool tcp_only = false;
    bool udp_only = false;
    bool use_filter = false;
    char *filter = NULL;
    int option;
    while((option = getopt_long(argc, argv, short_options, long_options, NULL)) != -1){
        switch (option) {
            case 't':
                if(udp_only == false)
                    tcp_only = true;
                break;
            case 'u':
                if(tcp_only == false)
                    udp_only = true;
                break;
        }
    }
    use_filter = optind < argc;
    filter = argv[optind];

    map<int, program> inode_program = build_inode_program_map();

    vector<proc_net> tcps = read_net(ADDRESS_TCP, "tcp");
    vector<proc_net> tcp6s = read_net(ADDRESS_TCP6, "tcp6");
    vector<proc_net> udps = read_net(ADDRESS_UDP, "udp");
    vector<proc_net> udp6s = read_net(ADDRESS_UDP6, "udp6");

    if(udp_only == false){
        cout << "List of TCP connections:" <<endl;
        print_title();
        print_net(tcps, inode_program, use_filter, filter);
        print_net(tcp6s, inode_program, use_filter, filter);
        cout << endl;
    }
    if(tcp_only == false){
        cout << "List of UDP connections:" <<endl;
        print_title();
        print_net(udps, inode_program, use_filter, filter);
        print_net(udp6s, inode_program, use_filter, filter);
    }

    return 0;
}

#include <iostream>
#include <cstdio>
#include <vector>
#include <arpa/inet.h>
#include <cstdlib>

using namespace std;

struct proc_net{
    bool ipv4;
    char local_ip[200];
    int local_port;
    char remote_ip[200];
    int remote_port;

    proc_net(bool is_ipv4 = true){
        this->ipv4 = is_ipv4;
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


};

int main(int argc, char *argv[]){

    proc_net tcp6(false);
    tcp6.read_local_ip(argv[1]);

    printf("%s\n", tcp6.local_ip);

    return 0;
}

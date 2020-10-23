#define EXTERN

/////////////////////////
// RT Kernel includes
//
#ifdef WATCOM
	#include "FreeRTOS.h"
	#include "task.h"
#else
	#include "./../RTLinux/RTLinux.h"
#endif



#include "net/ethernet.h"

#include <linux/ip.h>
#include <linux/tcp.h>
#include <linux/udp.h>

#include <linux/if_packet.h>
#include <linux/if_ether.h>
#include <linux/if_arp.h>





    // For Proper memory allocation in the structure
    #pragma push
    #pragma pack(1)

    typedef struct arp_packet {
        // ETH Header 
        uint8_t dest_mac[6];
        uint8_t src_mac[6];
        int16_t ether_type;
        // ARP Header
        int16_t hw_type;
        int16_t proto_type;
        int8_t hw_size;
        int8_t proto_size;
        int16_t arp_opcode;
        int8_t sender_mac[6];
        int32_t sender_ip;
        int8_t target_mac[6];
        int32_t target_ip;
        // Paddign
        char padding[18];
        } ARP_PKT;

    #pragma pop
    
        
        
    int print_pkt(char *, int len);

   

    // 1 = OK
// 0 = niente di fatto
// -1 = errore
int get_mac_addr_from_ip (IpAddr_t ip, EthAddr_t *mac_address, int Mode ) {
    int send_socket = 0, local_socket = 0,retVal = 0;
    struct sockaddr_in *sin;
    struct sockaddr_ll sa;
    struct ifreq ifr;
    ARP_PKT pkt;
    int32_t myipAddr;
    char host[256], gateway[256], str[256];

   
    
    snprintf(host, sizeof(host), "%d.%d.%d.%d", ip[0], ip[1], ip[2], ip[3]);
    snprintf(gateway, sizeof(gateway), "%d.%d.%d.%d", 192, 1681, 1, 1);

    

    // Open socket for accessing the IPv4 address of specified Interface
    local_socket = socket(AF_INET, SOCK_STREAM, 0); 
    if( local_socket < 0 ) {
        perror("IF Socket");
        return -1;
    }

    // provide interface name to ifreq structure
    memcpy(ifr.ifr_ifrn.ifrn_name, App.ETHInterfaceName, IF_NAMESIZE);

    // IOCTL to get ip address
    retVal = ioctl(local_socket, SIOCGIFADDR, &ifr, sizeof(ifr));
    if( retVal < 0 ) {
        perror("IOCTL");
        // close(local_socket);
        // return -1;
    }

    // Simple typecasting for easy access to ip address
    sin = (struct sockaddr_in *)&ifr.ifr_ifru.ifru_addr;
    myipAddr = sin->sin_addr.s_addr;



    if( ioctl(local_socket, SIOCGIFHWADDR, &ifr, sizeof(ifr)) < 0 ){
        perror("IOCTL");
        // close(local_socket);
        // return -1;
    }

    // printf("IF Name: %s IP Address: %s ", App.ETHInterfaceName, inet_ntoa(sin->sin_addr)); 
    // printf("IP = 0x%lx\n", ipAddr);
    // printf("MAC address: %02x:%02x:%02x:%02x:%02x:%02x \n",  ifr.ifr_hwaddr.sa_data[0]&0xFF,  ifr.ifr_hwaddr.sa_data[1]&0xFF,  ifr.ifr_hwaddr.sa_data[2]&0xFF,  ifr.ifr_hwaddr.sa_data[3]&0xFF,  ifr.ifr_hwaddr.sa_data[4]&0xFF,  ifr.ifr_hwaddr.sa_data[5]&0xFF);



    // Socket to send ARP packet 
    send_socket = socket(PF_PACKET, SOCK_RAW, htons(/*ETH_P_ALL*/ETH_P_ARP));
    if( send_socket == -1 ) {
        snprintf(str, sizeof(str), "%sARP Socket...(have root access by \"sudo ./lcu\" ?)...%s", (char*)ANSI_COLOR_RED, (char*)ANSI_COLOR_RESET);
        perror(str);
        if (local_socket>0) close(local_socket);
        return -1;
    }


    // dest : broadcast
    memset(pkt.dest_mac, -1, (6 * sizeof(int8_t)));

    // src : my mac addr
    memset(pkt.src_mac+0, (ifr.ifr_hwaddr.sa_data[0]&0xFF), sizeof(int8_t));
    memset(pkt.src_mac+1, (ifr.ifr_hwaddr.sa_data[1]&0xFF), sizeof(int8_t));
    memset(pkt.src_mac+2, (ifr.ifr_hwaddr.sa_data[2]&0xFF), sizeof(int8_t));
    memset(pkt.src_mac+3, (ifr.ifr_hwaddr.sa_data[3]&0xFF), sizeof(int8_t));
    memset(pkt.src_mac+4, (ifr.ifr_hwaddr.sa_data[4]&0xFF), sizeof(int8_t));
    memset(pkt.src_mac+5, (ifr.ifr_hwaddr.sa_data[5]&0xFF), sizeof(int8_t));

    pkt.ether_type = htons(ETH_P_ARP); 

    // ARP Header
    pkt.hw_type = htons(1);
    pkt.proto_type = htons(0x0800);
    pkt.hw_size = 6;
    pkt.proto_size = 4;
    pkt.arp_opcode = htons(1);
    memcpy(pkt.sender_mac, pkt.src_mac, (6 * sizeof(int8_t)));
    pkt.sender_ip = myipAddr;
    memset(pkt.target_mac, 0 , (6 * sizeof(int8_t)));
    pkt.target_ip = inet_addr(host); // inet_addr(gateway);

    // Padding
    memset(pkt.padding, 0 , 18 * sizeof(int8_t)); 


    // For sending the packet We need it!
    retVal = ioctl(local_socket, SIOCGIFINDEX, &ifr, sizeof(ifr));
    if( retVal < 0 ) {
        perror("IOCTL");
        close(local_socket);
        return -1;
    }

    shutdown(local_socket, SHUT_RDWR);
    local_socket = 0;
    
    
    sa.sll_family = AF_PACKET;
    sa.sll_ifindex = ifr.ifr_ifindex;
    sa.sll_protocol = htons(ETH_P_ARP);
    sa.sll_addr[0] = ifr.ifr_hwaddr.sa_data[0]&0xFF;
    sa.sll_addr[1] = ifr.ifr_hwaddr.sa_data[1]&0xFF;
    sa.sll_addr[2] = ifr.ifr_hwaddr.sa_data[2]&0xFF;
    sa.sll_addr[3] = ifr.ifr_hwaddr.sa_data[3]&0xFF;
    sa.sll_addr[4] = ifr.ifr_hwaddr.sa_data[4]&0xFF;
    sa.sll_addr[5] = ifr.ifr_hwaddr.sa_data[5]&0xFF;
    /*MAC - end*/
    sa.sll_addr[6]  = 0x00;/*not used*/
    sa.sll_addr[7]  = 0x00;/*not used*/


    /*address length*/
    sa.sll_halen = ETH_ALEN;

    /*ARP hardware identifier is ethernet*/
    sa.sll_hatype = ARPHRD_ETHER;

    /*target is another host*/
    sa.sll_pkttype = PACKET_OTHERHOST; // PACKET_OUTGOING; // PACKET_BROADCAST; // PACKET_OTHERHOST;



    // print_pkt((char*)&pkt, sizeof(pkt));


    if( sendto(send_socket, &pkt, sizeof(pkt), 0,(struct sockaddr *)&sa, sizeof(sa)) < 0 ) {
        perror("sendto");
        if (send_socket>0) close(send_socket);
        if (local_socket>0) close(local_socket);
        return -1;
    }


    /*
    pkt.target_ip = inet_addr(gateway);
    if( sendto(send_socket, &pkt, sizeof(pkt), 0,(struct sockaddr *)&sa, sizeof(sa)) < 0 ) {
        perror("sendto2");
        if (send_socket>0) close(send_socket);
        if (local_socket>0) close(local_socket);
        return -1;
    }    
    */



    struct sockaddr_ll sll;
    // struct ifreq ifr;

    sll.sll_family = AF_PACKET;
    sll.sll_ifindex = ifr.ifr_ifru.ifru_ivalue;
    sll.sll_protocol = htons(ETH_P_ARP);

    sll.sll_addr[0] = ifr.ifr_hwaddr.sa_data[0]&0xFF;
    sll.sll_addr[1] = ifr.ifr_hwaddr.sa_data[1]&0xFF;
    sll.sll_addr[2] = ifr.ifr_hwaddr.sa_data[2]&0xFF;
    sll.sll_addr[3] = ifr.ifr_hwaddr.sa_data[3]&0xFF;
    sll.sll_addr[4] = ifr.ifr_hwaddr.sa_data[4]&0xFF;
    sll.sll_addr[5] = ifr.ifr_hwaddr.sa_data[5]&0xFF;
    /*MAC - end*/
    sll.sll_addr[6]  = 0x00;/*not used*/
    sll.sll_addr[7]  = 0x00;/*not used*/


    if (bind( send_socket, (struct sockaddr*)&sll, sizeof( sll)) < 0) {
        perror("bind");
        if (send_socket>0) close(send_socket);
        if (local_socket>0) close(local_socket);
        return -1;
    }



    char packet[4096] = {0}, source[256], dest[256];
    struct ether_header *eth = (struct ether_header *) packet;




    if (Mode & 1)
        printf("[Searching on %s", host);
    else 
        printf("[Reading on %s", host);

    fflush(stdout);
    
    

    struct timeval tv;
    uint32_t time_limit_ms, t0 = xTaskGetTickCount();

    if (Mode & 1) {
        tv.tv_sec = 1;
        tv.tv_usec = 0*1000;
        time_limit_ms = 3 * 1000;
    } else {
        tv.tv_sec = 0;
        tv.tv_usec = 250*1000;
        time_limit_ms = 1 * 1000;
    }

    setsockopt(send_socket, SOL_SOCKET, SO_RCVTIMEO, (const char*)&tv, sizeof(struct timeval));

    int k;
    
            
    while(1) {

        k = recv( send_socket, packet, sizeof(packet), 0);

        if (k>0) {


            // skip to the next frame if it's not an ARP packet
            // if ((((buf[12]) << 8) + buf[13]) != ETH_P_ARP) {


            // skip to the next frame if it's not an ARP REPLY
            // if (ntohs (arp_frame->arp_op) != ARPOP_REPLY) continue;



            // fprintf(stdout, "eth-> ether_type:%d", (int)eth-> ether_type); fflush(stdout);

            if (ntohs( eth-> ether_type) == ETH_P_IP) {  // IP

                print_pkt(packet, k);

                // printf("...IP...");
                ARP_PKT *arp_pkt = (ARP_PKT*)packet;
                struct iphdr *ip_pkt = (struct iphdr *) (packet + sizeof(struct ether_header));


                inet_ntop( AF_INET, &ip_pkt->saddr, source, 16);
                inet_ntop( AF_INET, &ip_pkt->daddr, dest, 16);


                /*
                if (Mode & 1) {
                    printf( "...IP...%s%02X:%02X:%02X:%02X:%02X:%02X%s", (char*)ANSI_COLOR_BLUE, arp_pkt->dest_mac[0], arp_pkt->dest_mac[1], arp_pkt->dest_mac[2], arp_pkt->dest_mac[3], arp_pkt->dest_mac[4], arp_pkt->dest_mac[5], (char*)ANSI_COLOR_RESET );
                    printf( "-%s%02X:%02X:%02X:%02X:%02X:%02X%s", (char*)ANSI_COLOR_BLUE, arp_pkt->src_mac[0], arp_pkt->src_mac[1], arp_pkt->src_mac[2], arp_pkt->src_mac[3], arp_pkt->src_mac[4], arp_pkt->src_mac[5], (char*)ANSI_COLOR_RESET );
                }
                */

                if (ip_pkt->daddr == myipAddr) {
                    printf( "...%s...%s...%sOK%s\n", source, dest, (char*)ANSI_COLOR_GREEN, (char*)ANSI_COLOR_RESET);
                    mac_address[0][0] = arp_pkt->src_mac[0];
                    mac_address[0][1] = arp_pkt->src_mac[1];
                    mac_address[0][2] = arp_pkt->src_mac[2];
                    mac_address[0][3] = arp_pkt->src_mac[3];
                    mac_address[0][4] = arp_pkt->src_mac[4];
                    mac_address[0][5] = arp_pkt->src_mac[5];
                    retVal = 1;
                    break;
                } else {
                    printf( "...(%s -> %s)\n", source, dest);
                }


                switch (ip_pkt->protocol) {
                    case 6: { //TCP
                        // struct tcphdr *tcp_pkt = (struct tcphdr*) (packet+sizeof( struct ether_header)+sizeof( struct iphdr));
                        // printf( "TCP: %s:%d -> %s:%d\n", source, ntohs( tcp_pkt->source), dest, ntohs( tcp_pkt->dest));
                        break;
                    }
                    case 17: { //UDP
                        // struct udphdr *udp_pkt = (struct udphdr*) (packet+sizeof( struct ether_header)+sizeof( struct iphdr));
                        // printf( "UDP: %s:%d -> %s:%d\n", source, ntohs( udp_pkt->source), dest, ntohs( udp_pkt->dest));
                        break;
                    }
                    default:
                        printf( "[?IPprot:%d]\n", (int)ip_pkt->protocol);
                        break;
                }


            } else if( ntohs( eth-> ether_type) == ETH_P_ARP) {  // ARP
                ARP_PKT *arp_pkt = (ARP_PKT*)packet;


                // print_pkt(packet, k);

                if (arp_pkt->arp_opcode != ARPOP_REPLY && arp_pkt->arp_opcode != 512 && arp_pkt->arp_opcode != 256) {

                    printf( "!ARP reply %d..", (int)arp_pkt->arp_opcode);

                } else {

                    char *addr_id = (char *)&arp_pkt->sender_ip;
                    if ((unsigned char)addr_id[0] == 192 &&
                        (unsigned char)addr_id[1] == 168 &&
                        (unsigned char)addr_id[2] == 1 &&
                        (unsigned char)addr_id[3] == 1 ) {
                        if (Mode & 1) {
                            printf( "GATEWAY ARP:%s%02X:%02X:%02X:%02X:%02X:%02X%s", (char*)ANSI_COLOR_BLUE, (unsigned int)arp_pkt->src_mac[0], arp_pkt->src_mac[1], arp_pkt->src_mac[2], arp_pkt->src_mac[3], arp_pkt->src_mac[4], arp_pkt->src_mac[5], (char*)ANSI_COLOR_RESET );
                        }

                    } else if ((unsigned char)addr_id[0] == ip[0] &&
                        (unsigned char)addr_id[1] == ip[1] &&
                        (unsigned char)addr_id[2] == ip[2] &&
                        (unsigned char)addr_id[3] == ip[3]) {


                        if (Mode & 1) {
                            printf( "ARP:%s%02X:%02X:%02X:%02X:%02X:%02X%s", (char*)ANSI_COLOR_BLUE, (unsigned int)arp_pkt->src_mac[0], arp_pkt->src_mac[1], arp_pkt->src_mac[2], arp_pkt->src_mac[3], arp_pkt->src_mac[4], arp_pkt->src_mac[5], (char*)ANSI_COLOR_RESET );
                        }

                        printf( "...%sOK%s\n", (char*)ANSI_COLOR_GREEN, (char*)ANSI_COLOR_RESET);

                        mac_address[0][0] = arp_pkt->src_mac[0];
                        mac_address[0][1] = arp_pkt->src_mac[1];
                        mac_address[0][2] = arp_pkt->src_mac[2];
                        mac_address[0][3] = arp_pkt->src_mac[3];
                        mac_address[0][4] = arp_pkt->src_mac[4];
                        mac_address[0][5] = arp_pkt->src_mac[5];

                        retVal = 1;
                        break;

                        // 

                    } else {
                        printf( "%sNOT OUT ARP Reply (%d.%d.%d.%d)...%s\n", (char*)ANSI_COLOR_RED, (unsigned char)addr_id[0], (unsigned char)addr_id[1], (unsigned char)addr_id[2], (unsigned char)addr_id[3], (char*)ANSI_COLOR_RESET);
                    }
                }

            } else {
                printf( "%s", ANSI_COLOR_RED);
                print_pkt(packet, k);                
                printf( "%s", ANSI_COLOR_RESET);
                if (Mode & 1) 
                    printf( "%sUNK PROTOCOLO:%d%s\n", ANSI_COLOR_RED, ntohs( eth-> ether_type), ANSI_COLOR_RESET);
                break;
            }

        } else {
            // perror("recv:");
            // break;
        }

        if (xTaskGetTickCount() - t0 > time_limit_ms) {

            printf("]\n");
            break;

        } else {
            if (Mode & 1) {
                fprintf(stdout, ".");
                fflush(stdout);
            }
        }
    }



    if (send_socket>0) shutdown(send_socket, SHUT_RDWR);
    if (local_socket>0) shutdown(local_socket, SHUT_RDWR);
        

        // close(send_socket);
        // close(if_fd);
    
    // printf("\n=========PACKET=========\n");
    // print_pkt((void *)&pkt, sizeof(pkt));

    // _get_MAC(ip, 7373);
            
        
    return retVal;
}






int print_pkt(char *buf, int len) {
    int j = 0;

    printf("\n--------------------------------\n");
    for(j = 0; j < len; j++ ) {
        if((j%16) == 0 && j != 0 )
            printf("\n");
        if (j==6) printf("-");
        if (j==12) printf("[");
        if (j==14) printf("]");
            printf("%02X ", (unsigned int)(*(buf+j)& 0xFF) );
    } 

    printf("[%d bytes]\n", len);
    return 0;
}

    
void udpHandler( IpAddr_t ip, uint16_t sport, uint16_t dport, int8_t *buf, uint16_t buf_size) {

    printf("UDP response from host %hhu.%hhu.%hhu.%hhu\n", ip[0],ip[1], ip[2],ip[3]);
    
    if (buf) {
        if (buf_size) {
            fprintf(stderr, "%s", (char*)buf);
        }
    }
}






int get_MAC_addr (IpAddr_t ip, uint16_t port, EthAddr_t *mac_address, int Mode ) {

    int res = 0, retVal = 0, index1B = xrt_connect_udp( ip, port, port );
    uint32_t requestTimeoutMs = 1500;
    
    
    if (index1B) {
        int res = 0;
        struct timespec t, t0;
        struct ifreq ifr;
        char buf[32];
        
        if (xrt_send_udp( ip, port, port, "?", 1, 0 ) < 0) {
        }        
        while ( (res = xrt_recive_udp( ip, port, port, buf, (uint16_t)sizeof(buf), 0 )) > 0) {
            usleep(10*1000);
            printf("Cleaning mac addres queue: %02x:%02x:%02x:%02x:%02x:%02x \n", (int)buf[0]&0xFF, (int)buf[1]&0xFF, (int)buf[2]&0xFF, (int)buf[3]&0xFF, (int)buf[4]&0xFF, (int)buf[5]&0xFF );
        }
        
        usleep(10*1000);
        
        if (xrt_send_udp( ip, port, port, "?", 1, 0 ) < 0) {
            perror("SEND");
            return -1;
        }
        


        // clock_gettime(CLOCK_MONOTONIC, &t0);
        TickType_t xLastWakeTime = xTaskGetTickCount();

        
        while (1) {

            usleep(25*1000);

            res = xrt_recive_udp( ip, port, port, buf, (uint16_t)sizeof(buf), 0 );
        
            if (res < 0) {
                
                perror("RECV"); 
                return -1;

            } else if (res > 0) {

                /*
                memcpy(ifr.ifr_ifrn.ifrn_name, App.ETHInterfaceName, IF_NAMESIZE);
                if( ioctl(GLUdpHost[index1B-1].sock, SIOCGIFDSTADDR, &ifr, sizeof(ifr)) < 0 ){
                    perror("IOCTL");
                }
                printf("MAC address: %02x:%02x:%02x:%02x:%02x:%02x \n",  ifr.ifr_hwaddr.sa_data[0]&0xFF,  ifr.ifr_hwaddr.sa_data[1]&0xFF,  ifr.ifr_hwaddr.sa_data[2]&0xFF,  ifr.ifr_hwaddr.sa_data[3]&0xFF,  ifr.ifr_hwaddr.sa_data[4]&0xFF,  ifr.ifr_hwaddr.sa_data[5]&0xFF);
                */
                if (res > 6) {                    
                    // printf("ARP response size:%d \n", res);
                }
                
                if (res >= 6) {
                    
                    mac_address[0][0] = buf[0];
                    mac_address[0][1] = buf[1];
                    mac_address[0][2] = buf[2];
                    mac_address[0][3] = buf[3];
                    mac_address[0][4] = buf[4];
                    mac_address[0][5] = buf[5];

                    // printf("REMOTE MAC address: %02x:%02x:%02x:%02x:%02x:%02x \n", (int)buf[0]&0xFF, (int)buf[1]&0xFF, (int)buf[2]&0xFF, (int)buf[3]&0xFF, (int)buf[4]&0xFF, (int)buf[5]&0xFF );
                    
                    retVal = 1;                    
                    break;

                } else {
                    retVal = -1;
                }
            }
            
            // clock_gettime(CLOCK_MONOTONIC, &t);
            // if (t.tv_sec - t0.tv_sec > 1) break;
            if (xTaskGetTickCount() - xLastWakeTime > requestTimeoutMs) 
                break;
        }

        
        shutdown(GLUdpHost[index1B-1].sock, SHUT_RDWR);
        GLUdpHost[index1B-1].sock = 0;
        
        GLUdpHost[index1B-1].host[0] = 0;
        GLUdpHost[index1B-1].host[1] = 0;
        GLUdpHost[index1B-1].host[2] = 0;
        GLUdpHost[index1B-1].host[3] = 0;
    }
        
return retVal;    
}





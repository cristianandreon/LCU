#ifndef LINUX_ARP_H_

    #ifdef EXTERN
    	#ifdef __cplusplus
        	#define LINUX_ARP_H_ extern "C"
    	#else
        	#define LINUX_ARP_H_ extern
    	#endif
    #else
        #define LINUX_ARP_H_
    #endif


    LINUX_ARP_H_ int get_mac_addr_from_ip (IpAddr_t ip, EthAddr_t *mac_address, int Mode);
    LINUX_ARP_H_ int get_MAC_addr (IpAddr_t ip, uint16_t port, EthAddr_t *mac_address, int Mode);
    

    typedef struct _UdpHeader {

        // All of these need to be in network byte order.
        uint16_t src;
        uint16_t dst;
        uint16_t len;
        uint16_t chksum;

    } UdpHeader;

    
    
    
    typedef struct _TcpHeader {

        uint16_t src;
        uint16_t dst;
        uint32_t seqnum;
        uint32_t acknum;
        uint8_t  hlenBits;  // Use the methods!
        uint8_t  codeBits;
        uint16_t window;
        uint16_t checksum;
        uint16_t urgent;

    } TcpHeader;





    typedef struct _IpHeader {

        uint8_t  versHlen;       // vers:4, Hlen:4
        uint8_t  service_type;
        uint16_t total_length;

        // Fragmentation support
        //   flags 0 to 15
        //   0: always 0
        //   1: 0=May Fragment, 1=Don't Fragment
        //   2: 0=Last Fragment, 1=More Fragments
        //   3 to 15: Fragment offset in units of 8 bytes

        uint16_t ident;
        uint16_t flags;          // flags:3, frag_offset:13

        uint8_t  ttl;
        uint8_t  protocol;
        uint16_t chksum;

        uint8_t ip_src[4];
        uint8_t ip_dest[4];


        // uint16_t IpIdent;
        
    } IpHeader;


#endif

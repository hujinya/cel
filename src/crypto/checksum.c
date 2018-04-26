#include "cel/crypto/checksum.h"
#include "cel/error.h"
#include "cel/log.h"

/* Debug defines */
#define Debug(args)  /* cel_log_debug args */
#define Warning(args) CEL_SETERRSTR(args) /* cel_log_warning args */
#define Err(args)   CEL_SETERRSTR(args) /* cel_log_err args */

/*
 * The checksum is the 16-bit one's complement of the one's complement
 * sum of the entire VRRP message starting with the version field.  For
 * computing the checksum, the checksum field is set to zero. 
 * See RFC1071 for more detail.
 */
U16 cel_checksum(U16 *buf, size_t len)
{
    int sum;
    
    for (sum = 0; len > 1; len -= 2)
        sum += *buf++;
    /* Mop up an odd byte, if necessary */
    if (len == 1)
        sum += (*(BYTE *)buf << 8);

    sum = (sum >> 16) + (sum & 0xFFFF);     /* add hi 16 to low 16 */
    sum += (sum >> 16);                     /* add carry */    

    return  (U16)(~sum);  
}

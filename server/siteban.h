/*
                          Site Ban List
 */

#ifndef SITEBAN_H
#define SITEBAN_H

#include <sys/types.h>

typedef union {

	u_int32_t whole;
	u_int8_t part_u8[4];

} siteban_ip_union;


/*
 *	Site ban entry structure
 */
typedef struct {

	siteban_ip_union ip;
	int restrict;		/* Restriction level. */

} siteban_struct;

extern siteban_struct **siteban;
extern int total_sitebans;


extern int SiteBanIsAllocated(int n);
extern int SiteBanIsBanned(siteban_ip_union *ip, int restrict);
extern int SiteBanAdd(
        siteban_ip_union *ip,
        int restrict            /* Just 0 for now. */
);
extern int SiteBanRemoveIP(siteban_ip_union *ip);

extern void SiteBanDelete(int n);
extern void SiteBanDeleteAll();



#endif	/* SITEBAN_H */

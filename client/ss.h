/*
                                 Sound Schemes
 */

#ifndef SS_H
#define SS_H

/*
 *      Sound scheme item structure:
 */
typedef struct {
        
        char *path;
                        
} ss_item_struct;
extern ss_item_struct **ss_item;
extern int total_ss_items;


/* In ss.c */
extern int SSIsAllocated(int n);
extern ss_item_struct *SSGetPtr(int n);
        
extern int SSAllocate(char *path);
extern int SSAllocateExplicit(int n);
extern void SSDelete(int n);
extern void SSDeleteAll(void);


#endif	/* SS_H */

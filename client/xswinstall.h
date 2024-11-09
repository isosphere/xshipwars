#ifndef XSWINSTALL_H
#define XSWINSTALL_H

/*
 *	Install object options flags:
 */
#define XSW_INST_OBJ_FLAG_DIR			(1 << 1)
#define XSW_INST_OBJ_FLAG_RECURSIVE		(1 << 2)



extern int XSWDoInstallObject(
        char *target,   /* Target. */
        char *source,   /* Source. */
        mode_t m,       /* Permissions for newly installed object. */
        unsigned long options,  /* One of XSW_INST_OBJ_FLAG_*. */
        void *data      /* Data argument. */
);


#endif	/* XSWINSTALL_H */

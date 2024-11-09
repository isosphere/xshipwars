/*
               Image library (Imlib) operating system wrapper

 */

#ifdef USE_IMLIB

#include <Imlib.h>
#include "../include/osw-x.h"

typedef struct {

	char is_init;
	ImlibData *imlib_data;

} imlib_resource_struct;


extern int IMLIBOSWInit();
extern int IMLIBOSWLoadFileToImage(char *filename, image_t **image);
extern void IMLIBOSWShutdown();



#endif /* USE_IMLIB */


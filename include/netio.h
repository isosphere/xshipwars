/*
                    Network IO Functions

 */

#ifndef NETIO_H
#define NETIO_H

extern int NetIsSocketWritable(int s, int *error_level);
extern int NetIsSocketReadable(int s, int *error_level);

#endif	/* NETIO_H */

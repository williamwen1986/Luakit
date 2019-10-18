#ifndef tenum_h
#define tenum_h


typedef enum {
	FIRST,
	SECOND
} Order;

typedef enum {
	FALSE,
	TRUE
} Status;

Status checkenum (Order o);

#endif
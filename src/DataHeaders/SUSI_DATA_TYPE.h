#ifndef SUSI_DATA_TYPE_h
#define SUSI_DATA_TYPE_h

#include <stdint.h>


/* Identificativi della Direzione ricevuta dal Master */
#define	SUSI_DIRECTION		uint8_t
#define	SUSI_DIR_REV		0
#define	SUSI_DIR_FWD		1


/* Identificativi dei Gruppi Funzioni */
#define	SUSI_FN_GROUP		uint8_t
#define	SUSI_FN_0_4			0
#define	SUSI_FN_5_12		1
#define	SUSI_FN_13_20		2
#define	SUSI_FN_21_28		3
#define	SUSI_FN_29_36		4
#define	SUSI_FN_37_44		5
#define	SUSI_FN_45_52		6
#define	SUSI_FN_53_60		7
#define	SUSI_FN_61_68		8


/* Identificativi dei Gruppi AUXs */
#define	SUSI_AUX_GROUP		uint8_t
#define	SUSI_AUX_1_8		0
#define	SUSI_AUX_9_16		1
#define	SUSI_AUX_17_24		2
#define	SUSI_AUX_25_32		3


/* Identificativi dei Gruppi Funzioni Analogiche */
#define	SUSI_AN_GROUP		uint8_t
#define	SUSI_AN_FN_1		0
#define	SUSI_AN_FN_2		1
#define	SUSI_AN_FN_3		2
#define	SUSI_AN_FN_4		3
#define	SUSI_AN_FN_5		4
#define	SUSI_AN_FN_6		5
#define	SUSI_AN_FN_7		6
#define	SUSI_AN_FN_8		7


#endif

#ifndef WPLIST_H
#define WPLIST_H
#include <stdio.h>
#include <stdlib.h>

#define SCREEN_WIDTH 80
/* Number of chars supported by GPS device for wp-name */
#define SUPPORTED_NAME_LENGTH 12

/* verbosity print command */
#define v_printf(v, s, ...)\
	if(verbosity>=v) fprintf(stderr, s, ## __VA_ARGS__);
#define dprintf(s)\
	 { { fprintf(stderr, "++Error in file %s,  line %d:\n  ", __FILE__, __LINE__);\
							  fprintf(stderr, s); }}

/* list declaration */
struct pnode{
	char *name;
	double lat;
	double lon;
	int x;
	int y;
	struct pnode *next;
};

typedef struct pnode Point_node;


/* list operations */
int list_append(Point_node *head, Point_node *p);
int list_free(Point_node *head);
int list_length(Point_node *head);
Point_node* list_init_node(Point_node *n, double lat, double lon, int x, int y);
Point_node* list_get_next(Point_node *n);
void list_print(Point_node *list);
#endif

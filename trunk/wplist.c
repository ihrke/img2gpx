#include "wplist.h"

/* ---- LIST ---- */
Point_node* list_init_node(Point_node *n, double lat, double lon, int x, int y){
	if(!(n=(Point_node*)malloc(sizeof(Point_node)))) return NULL;
	n->next=NULL;
	n->name=(char*)malloc(SUPPORTED_NAME_LENGTH*sizeof(char));
	n->name[0]='\0';
	n->lat = lat;
	n->lon = lon;
	n->x   = x;
	n->y   = y;
	return n;
}
int list_length(Point_node *head){
	if(head->next==NULL) return 0;
	else return 1+list_length(head->next);	
}

int list_append(Point_node *head, Point_node *p){
	if((head->next)!=NULL) list_append(head->next, p);
	else head->next=p;
}
int list_free(Point_node *head){
	if(head->next) list_free(head->next);
	else{ 
		free(head->name);
		free(head);	
	}
}

Point_node* list_get_next(Point_node *cur){
	return cur->next;
}
void list_print(Point_node *l){
	char *n="Unknown\0";
	if(l->name[0]!='\0') n=l->name;	  
	fprintf(stderr, "--P(%s, %d, %d, %f, %f)", n,l->x, l->y, l->lat, l->lon);
	if(list_get_next(l)) list_print(list_get_next(l));
}

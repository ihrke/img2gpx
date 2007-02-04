#include "parser.h"
#include <string.h>
#include <time.h>

int parse_reference_points(FILE *cfg, Point_node *head){
	int c;	  
	double lat, lon;
	int num_refs;
	Point_node *tmp;
	char *section;

	section = (char*)malloc(20*sizeof(char));
	num_refs=0;

	rewind(cfg);
	for (;;){
		while((c=fgetc(cfg))!='[') if(c==EOF) return -1; 
		fscanf(cfg, "%10s", section);
		if(!strncmp((const char*)section, "reference]", 10)){
			v_printf(99, "Found Reference section in config file...\n");
			while(c=fgetc(cfg)){ 
				if(c=='#'){ /* comment */
					while((c=fgetc(cfg))!='\n') ;
				} else if (c=='\n') { 
					continue;
				} else if (c=='['){  /* new section */
					fseek(cfg, -1, SEEK_CUR); 
					break;
				} else if (c>='0' && c<='9') {
					fseek(cfg, -1, SEEK_CUR); 
					fscanf(cfg, "%lf", &lat);
					fseek(cfg, 1, SEEK_CUR); 
					fscanf(cfg, "%lf", &lon);
					num_refs++;
					tmp = list_init_node(tmp, lat, lon, 0,0);
					list_append(head, tmp);
					v_printf(90, "Reference point: %f, %f\n", lat, lon);
				} else if (c==EOF) return 0;
			}
			
			break;
	 	} else { continue; }	
		break;
	}
	free(section);
 	return num_refs;
}

int parse_waypoint_names(FILE *cfg, Point_node *wp_head){
	int c, num_names;
	char *section;

	section = (char*)malloc(20*sizeof(char));
	num_names=0;	
	rewind(cfg);
	for (;;){
		while(c=fgetc(cfg)) {
			if(c==EOF) return -1; 
			else if(c=='[') break;
			else if(c=='#') while((c=fgetc(cfg))!='\n') ;
		}
		
		fscanf(cfg, "%10s", section);
		if(!strncmp((const char*)section, "waypoints]", 10)){
			v_printf(99, "Found Waypoint section in config file...\n");
			while(c=fgetc(cfg)){ 
				if(c=='#'){ /* comment */
					while((c=fgetc(cfg))!='\n') ;
				} else if (c=='\n') { 
					continue;
				} else if (c=='['){  /* new section */
					fseek(cfg, -1, SEEK_CUR); 
					break;
				} else if (c>=33 && c<=126) {
					printf("Char = %c\n", c);
					fseek(cfg, -1, SEEK_CUR); 
					fscanf(cfg, "%10s", (wp_head->name));
					c = fgetc(cfg);
					if(c != '\n'){ 
						while((c=fgetc(cfg))!='\n') ;
					}
					v_printf(90, "Found Waypoint name: %s\n", wp_head->name);
					num_names++;
					wp_head = list_get_next(wp_head);
				} else if (c==EOF) return 0;
			}
			break;
	 	} else { continue; }	
		break;
	}
	return num_names;
}


int parse_config_section(FILE *cfg, int *verbosity, png_color *signal){
	return 1;
}

void parse_wplist_to_gpx(const Point_node *head, FILE *file){
	char strtime[255];
	time_t t;
	struct tm *tmp;
	t = time(NULL);
	tmp = localtime(&t);
	strftime(strtime, 255, "%Y-%m-%dT%H:%M:%SZ", tmp);

	/* write header */
	fprintf(file, "<?xml version=\"1.0\"?>\n");
	fprintf(file, "<gpx version=\"1.0\" creator=\"Img2Gpx\" xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\" xmlns=\"http://www.topografix.com/GPX/1/0\" xsi:schemaLocation=\"http://www.topografix.com/GPX/1/0 http://www.topografix.com/GPX/1/0/gpx.xsd\">\n"); 
	fprintf(file, "<time>%s</time>\n", &strtime);
	while((head=list_get_next(head))!=NULL){
		fprintf(file, "<wpt lat=\"%f\" lon=\"%f\">\n", head->lat, head->lon);
		fprintf(file, "<name>%s</name>\n", head->name);
		fprintf(file, "<sym>Waypoint</sym>\n");
		fprintf(file, "</wpt>\n");
	}
	fprintf(file, "</gpx>\n</xml>\n");
}

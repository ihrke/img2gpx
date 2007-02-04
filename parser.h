#ifndef PARSER_H
#define PARSER_H

#include "wplist.h"
#include<stdio.h>
#include<png.h>

extern int verbosity;

int parse_reference_points(FILE *cfg, Point_node *head);
int parse_waypoint_names(FILE *cfg, Point_node *wp_head);
int parse_config_section(FILE *cfg, int *verbosity, png_color *signal);
void parse_wplist_to_gpx(const Point_node *head, FILE *file);

#endif

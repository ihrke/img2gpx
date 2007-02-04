#include <png.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include "parser.h"
#include "wplist.h"
#include "linalg.h"

int verbosity; /* [0, 100] */
png_color signal; /* color used for signalling */

void setup_png(FILE *f, png_structpp img, png_infopp info);
void draw_progress_bar(float p);
int cmp_color(png_byte *color_ptr, png_color *color);

int main(int argc, char **argv){
	int width, height;
   png_bytep row[3]; /* three rows of a png file */
	png_bytep tmp; 	/* temporary pointer for swapping */
	FILE *fpin, *config;
	int i,j;
	int num_refs;
	png_structpp img_ptr;
	png_infopp img_info_ptr;
	Point_node *ref_head, *ref_cur;
	Point_node *wp_head, *wp_cur;
	matrix A, p, *a, M;

	if(argc<3){ 
		printf("Usage: %s <png file> <config file>\n", argv[0]);
		return 0;
	}

	if(!(config = fopen(argv[2], "r"))){
		dprintf("Opening config file failed\n");	  
		return -1;
	}

	/* setting defaults */
	verbosity=90;
	signal.red=255; 
	signal.green=0;
	signal.blue=0;
	/* override with entries from config file */
	parse_config_section(config, &verbosity, &signal);
	v_printf(99, "Setting verbosity to %i\n", verbosity);
	v_printf(99, "Setting signal colour to (%i, %i, %i)\n", signal.red, signal.green, signal.blue);


	/* creating list head for reference and waypoint list */
	wp_head = list_init_node(wp_head, 0, 0, 0, 0);
	ref_head = list_init_node(ref_head, 0, 0, 0, 0);

	num_refs = parse_reference_points(config, ref_head);
	v_printf(50, "Found %d reference points in config file '%s'\n", num_refs, argv[2]);
	
	img_ptr = (png_structpp)malloc(sizeof(png_structp));
	img_info_ptr = (png_infopp)malloc(sizeof(png_infop));
	
	fpin = fopen(argv[1], "rb");
	if(!fpin) {
		dprintf("Opening image file failed\n");	  
		return -1;
	}

	/* makes necessary transformations to read the data (see libpng for details)*/
	setup_png(fpin, img_ptr, img_info_ptr);

   width = png_get_image_width(*img_ptr, *img_info_ptr);
   height = png_get_image_height(*img_ptr, *img_info_ptr);
	for(i=0; i<3; i++) 
		row[i] = (png_bytep)malloc(3*width*sizeof(png_byte));

	/* -- processing here -- */
	ref_cur = ref_head;
	for(i=0; i<height; i++){ /* traverse rows */
		draw_progress_bar((float)i/(float)height);
		/*if((i*SCREEN_WIDTH)%height==0) putchar('*');	  */
   	png_read_row(*img_ptr, row[0], NULL);	
		/* swapping */
		tmp=row[0]; row[0]=row[1];
		row[1]=row[2]; row[2]=tmp;

		/* traverse middle row */
		for(j=0; j<3*width; j+=3){
			if(cmp_color(row[1]+j*sizeof(png_byte), &signal)){ 
				v_printf(99, "Pixel (%d, %d) matches signal color\n", i, j/3);	  
				/* checking for
				 *  +-+
				 *  -+-
				 *  +-+	  
				 */
				if(!cmp_color(row[0]+j*sizeof(png_byte), &signal) &&
						!cmp_color(row[2]+j*sizeof(png_byte), &signal) &&
						!cmp_color(row[1]+j*sizeof(png_byte)-3, &signal) &&
						!cmp_color(row[1]+j*sizeof(png_byte)+3, &signal) &&
						cmp_color(row[0]+j*sizeof(png_byte)-3, &signal) &&
						cmp_color(row[0]+j*sizeof(png_byte)+3, &signal) &&
						cmp_color(row[2]+j*sizeof(png_byte)+3, &signal) &&
						cmp_color(row[2]+j*sizeof(png_byte)-3, &signal) ){
					/* it is a reference point */
					v_printf(60, "Pixel (%d, %d) is a Reference point\n", i, j/3);	  
					if(!(ref_cur = list_get_next(ref_cur))){
						dprintf("Something's wrong here: not enough reference points in config file...");	  
						return -1;
					}
					ref_cur->x=i;
					ref_cur->y=j/3;
				}
				/* checking for
				 *  -+-
				 *  +-+
				 *  -+-	  
				 */
				else if(cmp_color(row[0]+j*sizeof(png_byte), &signal) &&
						cmp_color(row[2]+j*sizeof(png_byte), &signal) &&
						cmp_color(row[1]+j*sizeof(png_byte)-3, &signal) &&
						cmp_color(row[1]+j*sizeof(png_byte)+3, &signal) &&
						!cmp_color(row[0]+j*sizeof(png_byte)-3, &signal) &&
						!cmp_color(row[0]+j*sizeof(png_byte)+3, &signal) &&
						!cmp_color(row[2]+j*sizeof(png_byte)-3, &signal) &&
						!cmp_color(row[2]+j*sizeof(png_byte)+3, &signal) ){
					/* it is a Waypoint */
					v_printf(60, "Pixel (%d, %d) is a Waypoint\n", i, j/3);	  
					wp_cur = list_init_node(wp_cur, 0, 0, i, j/3);	  
					list_append(wp_head, wp_cur);
				}

			}
		}
	}
	putchar('\n');
	 /* -- -- */

	i=parse_waypoint_names(config, list_get_next(wp_head));
	v_printf(99, "Found %i names in config file\n", i);
	
	/* georeference waypoints using a 2 dimensional conformal affine transformation
	 * lat = ax + by + c    ->  v1 = [a b c]
	 * lon = dx + ey + f    ->  v2 = [d e f]
	 */
	matrix_init(&A, 2*list_length(ref_head), 6);
	matrix_init(&p, 2*list_length(ref_head), 1);

	if(list_length(ref_head)<3) {
		dprintf("Not enough reference points for georeferencing...\n");
		return -1;
	} 

	ref_cur=list_get_next(ref_head);
	for(i=1; i<=2*list_length(ref_head); i+=2){
		matrix_set_row(&A, i, (double)ref_cur->x, (double)ref_cur->y, 1.0,0.0,0.0,0.0);
		matrix_set_row(&A, i+1, 0.0,0.0,0.0,(double)ref_cur->x, (double)ref_cur->y, 1.0);
		matrix_set_row(&p, i, (double)ref_cur->lat);
		matrix_set_row(&p, i+1, (double)ref_cur->lon);
		ref_cur=list_get_next(ref_cur);
	}
	/* solve Ax=p via A'Ay=A'p, where y is the least square solution for x */
	a = matrix_solve_least_squares(&A, &p);
	matrix_init(&M, 2, 3);
	matrix_set(&M, 6, a->data[0][0], a->data[1][0], a->data[2][0],
						   a->data[3][0], a->data[4][0], a->data[5][0]);
	matrix_print("A", &A);
	matrix_print("p", &p);
	matrix_print("a", a);
	matrix_print("M", &M);
	matrix_destroy(&A);
	matrix_destroy(&p);
	matrix_destroy(a);

	matrix_init(&p, 3, 1);  /* image point coordinates */
	wp_cur=wp_head;
	while((wp_cur=list_get_next(wp_cur))!=NULL){
		matrix_set(&p, 3, (double)wp_cur->x, (double)wp_cur->y, 1.0);
		a = matrix_multiply(&M, &p);
		wp_cur->lat = a->data[0][0];
		wp_cur->lon = a->data[1][0];
	}
	matrix_destroy(a);
	matrix_destroy(&M);
	matrix_destroy(&p);

	parse_wplist_to_gpx(wp_head, stdout);

	v_printf(50, "Reference Points:\n-----------------\n");
	if(verbosity>=50) list_print(ref_head);
	v_printf(50, "\n");
	v_printf(50, "Found  Waypoints:\n-----------------\n");
	if(verbosity>=50) list_print(wp_head);
	v_printf(50, "\n");
	
	png_read_end(*img_ptr, NULL);

   png_destroy_read_struct(img_ptr, img_info_ptr, (png_infopp)NULL);
	free(img_ptr); free(img_info_ptr);
	list_free(ref_head);
	list_free(wp_head);
	fclose(fpin);
	fclose(config);
	return 0;
}

/*----------- ---------------------*/

void setup_png(FILE *fp, png_structpp png_ptr, png_infopp info_ptr){
	int width, height, bit_depth, color_type;
	png_byte header[8];	  

	/* check the first bytes of file */
	fread(header, 1, 8, fp);
	if(!png_check_sig(header, 8)){ 
		dprintf("PNG header corrupt\n");	  
		return; 
	}

	/* INITIALIZE read struct */
   *png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
   if (!(*png_ptr)) {
		dprintf("creation of PNG read-struct failed\n");	  
		return;
	}

	*info_ptr = png_create_info_struct(*png_ptr);
  	if (!(*info_ptr)) {
  		png_destroy_read_struct(png_ptr, (png_infopp)NULL, (png_infopp)NULL);
		dprintf("creation of PNG info-struct failed\n");
      return;
   }

	png_init_io(*png_ptr, fp);/* initialization for fread */
	png_set_sig_bytes(*png_ptr, 8); /* read first 8 bytes already (checking) */

	png_read_info(*png_ptr, *info_ptr); /* read information from file*/
	
   width = png_get_image_width(*png_ptr, *info_ptr);
   height = png_get_image_height(*png_ptr, *info_ptr);
   bit_depth = png_get_bit_depth(*png_ptr, *info_ptr);
   color_type = png_get_color_type(*png_ptr, *info_ptr);	
	v_printf(50, "Read image (%dx%d), Bit-Depth: %d, Colour: %d\n", width, height, bit_depth, color_type);

	/*if palleted image, convert it to RGB*/
   if (color_type == PNG_COLOR_TYPE_PALETTE) png_set_palette_to_rgb(*png_ptr);

	/*strip alpha channel*/
   if (color_type & PNG_COLOR_MASK_ALPHA) png_set_strip_alpha(*png_ptr);

	/*convert grayscale image to "RGB"*/
   if (color_type == PNG_COLOR_TYPE_GRAY || color_type == PNG_COLOR_TYPE_GRAY_ALPHA) 
				png_set_gray_to_rgb(*png_ptr);
	
	/*strip 16 bit to 8*/
	if (bit_depth == 16) png_set_strip_16(*png_ptr);

	/*expand optimized images to bit-depth of 8*/
	if (bit_depth < 8) png_set_packing(*png_ptr);

	/*interlacing not supported*/
	if(png_get_interlace_type(*png_ptr, *info_ptr)!=PNG_INTERLACE_NONE) {
		dprintf("Interlacing not supported");	  
		return;
	}

	/*updating the info-struct for that image (in case transformations have been applied)*/
	png_read_update_info(*png_ptr, *info_ptr);
}

void draw_progress_bar(float p){
	int i;	  
	for(i=0; i<SCREEN_WIDTH*2; i++) fputc('\b', stderr);	
	fprintf(stderr, "[%3i%% ", (int)(p*100));
	for(i=0; i<(int)(SCREEN_WIDTH-2)*p; i++) fputc('*', stderr);	
	for(; i<SCREEN_WIDTH-1; i++) fputc(' ', stderr);	
	fputc(']', stderr);
}

int cmp_color(png_byte *color_ptr, png_color *color){
	if(color_ptr[0]==color->red &&
		color_ptr[1]==color->green &&
		color_ptr[2]==color->blue) return 1;
	else return 0;
}


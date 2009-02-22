#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <SDL.h>
#include <SDL_image.h>

#define HEIGHT 600
#define WIDTH  800
#define TORUS 1

// Fuer einen 24-Bit-Modus unabhaengig von der Bytereihenfolge.
// Wird von also DrawPixel() benoetigt
#include "SDL_endian.h"

unsigned int height = HEIGHT;
unsigned int width = WIDTH;
unsigned int feld_size;

char feld[WIDTH][HEIGHT],feld_alt[WIDTH][HEIGHT];
//char **feld, **feld_alt;
SDL_Surface *display;


void prepare()
{
	    // init video stuff
    if ( SDL_Init( SDL_INIT_VIDEO) < 0 )
    {
       fprintf(stderr, "SDL konnte nicht initialisiert werden:  %s\n", SDL_GetError());
       exit(1);
     }

    atexit(SDL_Quit);

    // init screen
    display = SDL_SetVideoMode( WIDTH, HEIGHT, 8, SDL_SWSURFACE);
    if ( display == NULL )
    {
      fprintf(stderr, "Konnte kein Fenster %dx%dpx oeffnen: %s\n", WIDTH,HEIGHT,SDL_GetError());
      exit(1);
    }

	feld_size = height*width;

	/*
	feld = (char **)malloc(height * sizeof(char *));
	for(int i = 0; i < height; i++) {
		feld[i] = (char *)malloc(width * sizeof(char));
		if(NULL == feld[i]) {
			printf("Out of memory!\n");
			exit(1);
		}
	}

	feld_alt = (char **)malloc(height * sizeof(char *));
	for(int i = 0; i < height; i++) {
		feld_alt[i] = (char *)malloc(width * sizeof(int));
		if(NULL == feld_alt[i]) {
			printf("Out of memory!\n");
			exit(1);
		}
	}
	*/

	memset(feld, 0, feld_size);
	memset(feld_alt, 0, feld_size);
	SDL_FillRect(display, NULL, 255);
	SDL_Flip(display);
}

void DrawPixel(SDL_Surface *screen, int x, int y,Uint8 R, Uint8 G,Uint8 B)
{
    Uint32 color = SDL_MapRGB(screen->format, R, G, B);

    if ( SDL_MUSTLOCK(screen) )
    {
        if ( SDL_LockSurface(screen) < 0 ) {
            return;
        }
    }

    switch (screen->format->BytesPerPixel) {
        case 1: { /* vermutlich 8 Bit */
            Uint8 *bufp;

            bufp = (Uint8 *)screen->pixels + y*screen->pitch + x;
            *bufp = color;
        }
        break;

        case 2: { /* vermutlich 15 Bit oder 16 Bit */
            Uint16 *bufp;

            bufp = (Uint16 *)screen->pixels + y*screen->pitch/2 + x;
            *bufp = color;
        }
        break;

        case 3: { /* langsamer 24-Bit-Modus, selten verwendet */
            Uint8 *bufp;

            bufp = (Uint8 *)screen->pixels + y*screen->pitch + x * 3;
            if(SDL_BYTEORDER == SDL_LIL_ENDIAN) {
                bufp[0] = color;
                bufp[1] = color >> 8;
                bufp[2] = color >> 16;
            } else {
                bufp[2] = color;
                bufp[1] = color >> 8;
                bufp[0] = color >> 16;
            }
        }
        break;

        case 4: { /* vermutlich 32 Bit */
            Uint32 *bufp;

            bufp = (Uint32 *)screen->pixels + y*screen->pitch/4 + x;
            *bufp = color;
        }
        break;
    }

    if ( SDL_MUSTLOCK(screen) )
    {
        SDL_UnlockSurface(screen);
    }

}

void handle_events()
{
	SDL_Event event;
	while(SDL_PollEvent(&event) == 1)
	{
		if(event.type == SDL_QUIT)
		{
			exit(0);
		}
	}
}

int save_field_to_file(char *path)
{
	FILE *fd;
	fd = fopen(path, "w+");
	if(NULL == fd)
	{
		fprintf(stderr,"Fehler: Konnte Datei %s nicht öffnen oder erstellen!",path);
		exit(1);
	}
	fprintf(fd, "x:%d y:%d\n",WIDTH,HEIGHT);
	for(int y=0;y<HEIGHT;y++)
	{
		for(int x=0;x<HEIGHT;x++)
		{
			if(feld[x][y] == 1)
			{
				fputc('0',fd);
			}
			else
			{
				fputc(' ',fd);
			}
		}
		fputc('\n',fd);
	}
	fclose(fd);
}

int get_pixel(int x, int y)
{
#ifdef TORUS
	x %= WIDTH;
	if(x < 0)
		x+=WIDTH;

	y %= HEIGHT;
	if(y < 0)
		y+=HEIGHT;
#else
	if(y<0 || x < 0 || y > HEIGHT-1 || x > WIDTH-1)
		return 0;
#endif
	return feld_alt[x][y];
}

int test_life(int x, int y)
{
	unsigned char summe;

	//Zelle lebt schon
	if(feld_alt[x][y]) {
		summe  = get_pixel(x-1, y-1);
		summe += get_pixel(x, y-1);
		summe += get_pixel(x+1, y-1);

		summe += get_pixel(x-1, y  );
		summe += get_pixel(x+1, y  );

		summe += get_pixel(x-1, y+1);
		summe += get_pixel(x  , y+1);
		summe += get_pixel(x+1, y+1);
		if(summe == 2 || summe == 3)
		{
			return 1;
		}
	} else
	{
		summe  = get_pixel(x-1, y-1);
		summe += get_pixel(x,   y-1);
		summe += get_pixel(x+1, y-1);

		summe += get_pixel(x-1, y  );
		summe += get_pixel(x+1, y  );

		summe += get_pixel(x-1, y+1);
		summe += get_pixel(x  , y+1);
		summe += get_pixel(x+1, y+1);
		if(summe == 3)
		{
			return 1;
		}
	}

	return 0;
}

int round()
{
	unsigned int counter = 0;
	memcpy(feld_alt, feld, feld_size);
	memset(feld, 0, feld_size);
	SDL_FillRect(display, NULL, 255);
	for(int y=0;y<HEIGHT;y++)
	{
		for(int x=0;x<WIDTH;x++)
		{
			if(test_life(x,y))
			{
				feld[x][y] = 1;
				DrawPixel( display, x, y, 0,0,0);
				counter++;
			}
		}
	}
	/*char *field ins display zeichnen*/
	//if(counter%100==1)
	SDL_Flip(display);
	return counter;
}

void gol()
{
	unsigned int counter = 0,counter_old = 0,rounds = 0;
	char c,c2;
	char path[32];
	handle_events();/////////////////////////////////
	SDL_Delay(1000);
	for(int y=0;y<HEIGHT;y++)
	{
		handle_events();/////////////////////////////////
		for(int x=0;x<WIDTH;x++)
		{
			if(feld[x][y])
			{
				DrawPixel( display, x, y, 0,0,0);
				counter++;
			}
		}
	}
	SDL_Flip(display);
	#ifndef DEBUG
	//fprintf(stdout,"%8d:%8d %c%c\n",rounds,counter,c,c2);
	fprintf(stdout,"%8d:%8d\n",rounds,counter);
	//sprintf(path,"haha/feld%04d.gol",rounds);
	//save_field_to_file(path);
	#endif
	SDL_Delay(1000);
	while(1)
	{
		handle_events();/////////////////////////////////
		counter_old = counter;
		if((counter = round()) == 0)
		{
			SDL_Delay(1000);
			exit(0);
		}
		rounds++;

		if(counter < counter_old)
		{
			c = '<';
			c2= ' ';
		}
		else if(counter > counter_old)
		{
			c = ' ';
			c2= '>';
		}
		else
		{
			c = '-';
			c2= '-';
		}
		#ifndef DEBUG
		//fprintf(stdout,"%8d:%8d %c%c\n",rounds,counter,c,c2);
		fprintf(stdout,"%8d:%8d\n",rounds,counter);
		//sprintf(path,"haha/feld%04d.gol",rounds);
		//save_field_to_file(path);
		#endif
	}
}

void randm(int perc)
{
	srand(time(NULL));
	for(int y=0;y<HEIGHT;y++)
	{
		for(int x=0;x<WIDTH;x++)
		{
			if(rand()%perc == 1)
			{
				feld[x][y] = 1;
			}
		}
	}
}

int main()
{
	char path[] = "test.gol";
	prepare();
	randm(2);


	gol();
}

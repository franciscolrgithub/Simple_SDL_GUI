#include <stdio.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <time.h>

#include "ssg_button.h"
#include "ssg_text.h"

/*
   is_ssg_button_pressed()
   This functions returns 1 if the ssg_button pointed to by button
   has been pressed (at the coordinates x,y)
 */
int is_ssg_button_pressed(struct ssg_button* button, int x, int y)
{
    return x>=button->x && x<=button->x+button->w
        && y>=button->y && y<=button->y+button->h;
}

/*
   print_ssg_button()
   if the button doesn't have an image:
   This function displays the ssg_button pointed to by button in the
   SDL_Renderer pointed to by renderer, and its tag too (with the
   font "font", which is an SDL_Surface* loaded from the image
   font1.png
   if the button has an image (button->image != NULL) then the function
   copies the whole texture to the renderer, specifically at the
   button's coordinates and with the same width and height (without
   the border of the button)
 */
void print_ssg_button(SDL_Renderer* renderer,
                      struct ssg_button* button,
                      SDL_Surface* font)
{
    if(button->image == NULL)
    {
        //we save the current render drawing color to not lose it
        SDL_Color rgb;
        SDL_GetRenderDrawColor(renderer, &rgb.r, &rgb.g, &rgb.b,
                               &rgb.a);
        SDL_SetRenderDrawColor(renderer, button->red2, button->green2,
                               button->blue2, 0xFF);
        SDL_RenderDrawLine(renderer, button->x, button->y,
                           button->x+button->w, button->y);
        SDL_RenderDrawLine(renderer, button->x, button->y+button->h,
                           button->x+button->w, button->y+button->h);
        SDL_RenderDrawLine(renderer, button->x, button->y,
                           button->x, button->y+button->h);
        SDL_RenderDrawLine(renderer, button->x+button->w, button->y,
                           button->x+button->w, button->y+button->h);

        // filling the rectangle with the primary colour
        SDL_SetRenderDrawColor(renderer, button->visible_red,
                               button->visible_green,
                               button->visible_blue, 0xFF);

        for (int i = 0; i < button->h-1; i++)
        {
            SDL_RenderDrawLine(renderer, button->x+1, button->y+1+i,
                               button->x+button->w-1, button->y+1+i);
        }
        SDL_SetRenderDrawColor(renderer, rgb.r, rgb.g, rgb.b, rgb.a);

        // writing the tag of the button
        print_ssg_text(renderer, button->tag, font);
    }
    else
    {
        SDL_Color rgb;
        SDL_GetRenderDrawColor(renderer, &rgb.r, &rgb.g, &rgb.b,
                               &rgb.a);
        SDL_SetRenderDrawColor(renderer, button->red2, button->green2,
                               button->blue2, 0xFF);
        SDL_RenderDrawLine(renderer, button->x, button->y,
                           button->x+button->w, button->y);
        SDL_RenderDrawLine(renderer, button->x, button->y+button->h,
                           button->x+button->w, button->y+button->h);
        SDL_RenderDrawLine(renderer, button->x, button->y,
                           button->x, button->y+button->h);
        SDL_RenderDrawLine(renderer, button->x+button->w, button->y,
                           button->x+button->w, button->y+button->h);
        // we substract 2 to w and h because we want to display the
        // button's border in all cases
        SDL_Rect r = {button->x+1, button->y+1,
            button->w-1, button->h-1};
        SDL_RenderCopy(renderer, button->image, NULL, &r);
    }
}

/*
   button_press_feedback()
   This function changes the colour of the ssg_button pointed to by
   button_v suddenly, then goes back to its original colour slowly,
   so as to give some feedback to the user when pressing a button
 */
void * button_press_feedback(void* button_v)
{
    struct ssg_button* button = (struct ssg_button*)button_v;

    if(button->red>191)
        button->visible_red = 255;
    else
        button->visible_red = button->red + 64;

    if(button->green>191)
        button->visible_green = 255;
    else
        button->visible_green = button->green + 64;

    if(button->blue>191)
        button->visible_blue = 255;
    else
        button->visible_blue = button->blue + 64;

    struct timespec tme = { 0, 5000000};

    for (size_t i = 0; i<64; i++)
    {
        if(button->visible_red > button->red)
            button->visible_red -= 1;
        if(button->visible_green > button->green)
            button->visible_green -= 1;
        if(button->visible_blue > button->blue)
            button->visible_blue -= 1;
        nanosleep(&tme, NULL);
     }

    return NULL;
}

/*
   clrcpy()
   This function only copies the button's colour to its visible colour
 */
void clrcpy(struct ssg_button* button)
{
    button->visible_red = button->red;
    button->visible_green = button->green;
    button->visible_blue = button->blue;
}

/*
   free_button()
   This function frees the memory region pointed to by button, so as
   the ssg_text tag
 */
void free_button(struct ssg_button* button)
{
    free_text(button->tag);
    if (button->image)
    {
        SDL_DestroyTexture(button->image);
    }

    if (button->image_surface)
    {
        SDL_FreeSurface(button->image_surface);
    }
    free(button);
}

/*
   new_button_std()
   This function returns a pointer to a new heap allocated ssg_button.
   Instead of giving the programmer the possibility to fully customize
   the button, this function helps them making buttons fast, as some
   of the fields are filled automatically by the funtion
 */
struct ssg_button* new_button_std(int x, int y, char nombre[],
                       int red, int green, int blue, char* tag)
{
    struct ssg_button* new_button = malloc(sizeof(struct ssg_button));
    new_button->x = x;
    new_button->y = y;
    new_button->w = 80;
    new_button->h = 50;
    strncpy(new_button->nombre, nombre, BUTTON_NAME_LIMIT);
    new_button->nombre[BUTTON_NAME_LIMIT - 1] = 0;

    new_button->red = red;
    new_button->green = green;
    new_button->blue = blue;

    new_button->image = NULL;

    clrcpy(new_button);

    /* chosing the button's border color: white if the button's color
       is dark, dark grey otherwise
     */
    int average_color = (red + green + blue) / 3;

    if (average_color < 128)
    {
        new_button->red2 = 0xFF;
        new_button->green2 = 0xFF;
        new_button->blue2 = 0xFF;
    }
    else
    {
        new_button->red2 = 0x44;
        new_button->green2 = 0x44;
        new_button->blue2 = 0x44;
    }

    new_button->tag = new_ssg_text(x+2, y+2, 80-4, 50-4,
                                   new_button->red2,
                                   new_button->green2,
                                   new_button->blue2, tag, "dfjdkfj");
    new_button->image = NULL;
    new_button->image_surface = NULL;
    return new_button;
}

struct ssg_button* new_button_with_image(int x, int y,
                                         char name[], int red2,
                                         int green2, int blue2,
                                         char* tag,
                                         char *image_path,
                                         struct ssg_gui *gui)
                                         //SDL_Texture* image)
{
    struct ssg_button* new_button = malloc(sizeof(struct ssg_button));
    new_button->x = x;
    new_button->y = y;
    new_button->w = 80;
    new_button->h = 50;
    strncpy(new_button->nombre, name, BUTTON_NAME_LIMIT);
    new_button->nombre[BUTTON_NAME_LIMIT - 1] = 0;

    new_button->red2 = red2;
    new_button->green2 = green2;
    new_button->blue2 = blue2;

    // loading image
    SDL_Surface* image_button_surface = IMG_Load(image_path);
    if (!image_button_surface)
    {
        fprintf(stderr, "ERROR: Couldn't IMG_Load() the button's image\n");
        free(new_button);
        return NULL;
    }

    new_button->image_surface = image_button_surface;

    SDL_Texture* image_button_texture =
        SDL_CreateTextureFromSurface(gui->renderer, image_button_surface);

    new_button->image = image_button_texture;

    new_button->tag = new_ssg_text(x+2, y+2, 80-4, 50-4, new_button->red2,
                                   new_button->green2, new_button->blue2, tag, "dkfjkdjfk");
    return new_button;
}

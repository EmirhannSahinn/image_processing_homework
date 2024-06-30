#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>
#pragma pack(1)
#define MAX_PATH_LENGHT 200

typedef unsigned char BYTE; //1 bytes of memory
typedef unsigned short WORD; //2 bytes of memory
typedef unsigned int DWORD; //4 bytes of memory
typedef int LONG; //4 bytes of memory

typedef struct _BMPFH //takes 14 bytes of memory
{
	BYTE bftype1; //1 byte and must be 'B'
	BYTE bftype2; //1 byte and must be 'M'
	DWORD bfsize; //4 bytes gives us the all size of bmp file (including headers,palette (if it has) data)
	WORD bfreserved1; // 2 btyes of memory could be set as 0
	WORD bfreserved2; // 2 btyes of memory could be set as 0
	DWORD bfOffbits; //4 bytes of memeory gives the position of data starts in the bmp file
} BMPFH;

typedef struct _BMPIH //40 bytes for windows bitmap file
{
	DWORD bisize; //4 bytes and it gives the size of info header
	LONG  biw; //4 bytes and it is the width of image
	LONG bih; //4 bytes and it is the height of iimage
	WORD biplane; //2 bytes which is not important for us
	WORD bibitcount; //2 bytes it is about the type of bitmap file if it is 1 2 color, 4 (16 colors) ..
	DWORD biComp; //4 bytes not important
	DWORD bisizeimage; //4 bytes and it gives the size of data in the image 
	LONG bix; //4 bytes not importatnt
	LONG biy; //4 bytes not important
	DWORD biclused; //4 bytes not important
 	DWORD biclimp; //4 byets not importatnt for us
} BMPIH;

typedef struct _PALET//in palette it describes colors (what is the color number)
{
	BYTE rgbblue;//blue commponent
	BYTE rgbgreen;//green component
	BYTE rgbred;//red component
	BYTE rgbreserved;//reserved byte the user can use this for therir aims
} PALET;
typedef struct _IMAGE
{
	BMPFH   bmpfh;
	BMPIH   bmpih;
	PALET   *palet;
	BYTE    *data;
} IMAGE;

void ImageWrite(IMAGE *image, char *filename);
IMAGE *ImageRead(IMAGE *image, char *filename);

IMAGE *ImageRead(IMAGE *image, char *filename)
{
    BMPFH bmpfh;
    BMPIH bmpih;
    FILE *fp;
    DWORD r, rowsize, size;

    fp = fopen(filename, "rb"); //tries to open the filename
    
    if (fp == NULL)
    {
        printf("File was not found...");
        exit(1);
    }
    
    fread(&bmpfh, sizeof(BMPFH), 1, fp); //reads bitmap info header from the file and set to bmpih
    
    if (bmpfh.bftype1 != 'B' || bmpfh.bftype2 != 'M') //check the file it is bmp file or not. Hint: .bmp files start with "BM"
    {
        printf("This is not a bitmap file.");
        exit(1);
    }

    fread(&bmpih, sizeof(BMPIH), 1, fp); //reads bitmap file header from the file and set to bmph
    image = (IMAGE *) malloc(bmpfh.bfsize); //allocates memory for image
    
    if (image == NULL)
    {
        printf("There is no enough memory for this operation");
        exit(1);
    }
    
    image->bmpfh = bmpfh; //sets bmpfh to image 
    image->bmpih = bmpih; //sets bmpih to image
    
    r = 0; //r is set to 0 in case 24 bits per pixel or more (this kind of images does not have color palette)
    if (bmpih.bibitcount == 1) //if the image 1 bit per pixel (the number of clor is 2)
        r = 2;
    if (bmpih.bibitcount == 4) //if the image 4 bits per pixel (the number of clor is 16)
        r = 16;
    if (bmpih.bibitcount == 8) //if the image 8 bits per pixel (the number of clor is 256)
        r = 256;
    if (r != 0)
    {
        image->palet = (PALET *)malloc(4 * r); //allocate memory for color palette
        fread(image->palet, sizeof(BYTE), 4 * r, fp); //read color palette from image to the memory
    }

    rowsize = (image->bmpih.biw * image->bmpih.bibitcount + 31) / 32 * 4; //calculates 1 row of image size 
    size = rowsize * image->bmpih.bih; //all size of image is calculated and set to size 
    image->data = (BYTE *) malloc(size); //allocates enough memory for iamage data
    fread(image->data, size, 1, fp); //reads image data from file and sets to image->data
    fclose(fp); //closes the file
    return image; //returns the adress of image on the memory
    
}
void ImageWrite(IMAGE *image, char *filename)
{
    FILE *fp;
    int r, rowsize, size;

    fp = fopen(filename, "wb"); //opens the file
    
    if (fp == NULL)
    {
        printf("Fie opening error!");
        exit(1);
    }
    
    image->bmpfh.bftype2 = 'M';
    fwrite(&image->bmpfh, sizeof(BMPFH), 1, fp); //writes the bitmap file header to the file
    fwrite(&image->bmpih, sizeof(BMPIH), 1, fp); //writes he bitmep info header to the file
    r = 0;
    if (image->bmpih.bibitcount == 1) //if the image has 2 colors
        r = 2;
    if (image->bmpih.bibitcount == 4) //if the image has 16 colors
        r = 16;
    if (image->bmpih.bibitcount == 8) //if the image has 256 colors
        r = 256;
    if (r != 0) //if the image has color palette the palette is written to the file
        fwrite(image->palet, 4 * r, 1, fp);
    rowsize = (image->bmpih.biw * image->bmpih.bibitcount + 31) / 32 * 4; //a row size of image is calculated 
    size = rowsize * image->bmpih.bih; //all size of image is calculated
    fwrite(image->data, size, 1, fp); //write image data to the file
    fclose(fp); //closes the file 
}

void highPassFilter(IMAGE *image, int filter[3][3])
{
    int h, w, rowsize, i, j, k, n, m;
    BYTE *data;
    int sum;
    w = image->bmpih.biw;
    h = image->bmpih.bih;
    rowsize = (image->bmpih.bibitcount * w + 31) / 32 * 4;
    data = (BYTE *)malloc(h * rowsize * sizeof(BYTE));
    memcpy(data, image->data, h * rowsize);

    for (i = 1; i < h - 1; i++)
    {
        for (j = 1; j < rowsize - 1; j++)
        {
            sum = 0;
            for (n = -1; n <= 1; n++)
            {
                for (m = -1; m <= 1; m++)
                {
                    sum += filter[n + 1][m + 1] * data[(i + n) * rowsize + j + m];
                }
            }
            if (sum < 0)
                sum = 0;
            if (sum > 255)
                sum = 255;
            image->data[i * rowsize + j] = sum;
        }
    }
    free(data);
    return;
}

void gradientOfGreyScaleImage(IMAGE *image)
{
    int x, y;
	int h = image->bmpih.bih;
    int w = image->bmpih.biw;
    int rowsize = (image->bmpih.bibitcount * w + 31) / 32 * 4;
    BYTE *result = (BYTE *)malloc(h * rowsize * sizeof(BYTE));

    for (y = 1; y < h - 1; y++)
    {
        for (x = 1; x < w - 1; x++)
        {
            float dx = image->data[y * w + (x + 1)] - image->data[y * w + (x - 1)];
            float dy = image->data[(y + 1) * w + x] - image->data[(y - 1) * w + x];
            result[y * w + x] = (BYTE)sqrt(dx * dx + dy * dy);
        }
    }

    memcpy(image->data, result, h * rowsize);
    free(result);
}

int menu()
{
	int choice;
    while(1)
    {
        printf("1- Gradient of Grey Scale\n");
        printf("2- Simple Laplacian Filter\n");
        printf("3- Sobel Full Filter\n");
        printf("4- Quit the program\n");
        printf("Enter your choice: ");
        scanf("%d", &choice);

        if(choice >= 1 && choice <= 4)
        {
            return choice;
        }
        else
        {
            printf("Invalid input! Please enter a number between 1-4.\n\n");
            continue;
        }

    }

    return choice;
}

int main()
{
	
	IMAGE *image = (IMAGE *)malloc(sizeof(IMAGE));
	char path[MAX_PATH_LENGHT], tempPath[MAX_PATH_LENGHT];
	
	while(1)
	{
		
		strcpy(path, "C:/Users/Emirhan-Sahin/Desktop/image_processing_homework/images/"); //update the image folder path
		strcpy(tempPath, path);
		image = ImageRead(image, strcat(tempPath,"kelebek.bmp"));
		
		int choice = menu();
		
		if(choice == 4)
		{
			free(image);
			return 0;
		}
		if(choice == 1)
		{
			gradientOfGreyScaleImage(image);
        	strcpy(tempPath, path);
			ImageWrite(image, strcat(tempPath,"gradient_grey.bmp"));
            strcpy(path, "\0");
            continue;
		}
		else if(choice == 2)
		{
			int filter[3][3] = {{0, 1, 0}, {1, -4, 1}, {0, 1, 0}};
            highPassFilter(image, filter);
            strcpy(tempPath, path);
			ImageWrite(image, strcat(tempPath, "simple_laplacian.bmp"));
            continue;
		}
		else if(choice == 3)
		{
			int filter[3][3] = {{-1, -2, -1}, {0, 0, 0}, {1, 2, 1}};
            highPassFilter(image, filter);
            strcpy(tempPath, path);
            ImageWrite(image, strcat(tempPath, "sobel_full.bmp"));
            continue;	
		}
	}
}

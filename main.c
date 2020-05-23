/*
** Adam Patyk
** ECE 6680
** Lab 1: Image Display (Windows)
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <windows.h>

#define MAX_FILENAME_CHARS 320

unsigned char *read_image(char **, int, int *, int *, int *, int *);

LRESULT CALLBACK EventProcessor (HWND hWnd,UINT uMsg,WPARAM wParam,LPARAM lParam) {
    return(DefWindowProc(hWnd,uMsg,wParam,lParam));
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance,
                   LPSTR     lpCmdLine, int nCmdShow) {
    WNDCLASS	         	wc;
    HWND			        WindowHandle;
    int			            i, bytes, type, argc;
	int						ROWS, COLS;
    unsigned char     		*dispdata, *filedata, *dispimg, red, green, blue;
    BITMAPINFO				*bm_info;
    HDC			            hDC;
	RECT					wr;

    wc.style=CS_HREDRAW | CS_VREDRAW;
    wc.lpfnWndProc=(WNDPROC)EventProcessor;
    wc.cbClsExtra=wc.cbWndExtra=0;
    wc.hInstance=hInstance;
    wc.hIcon=wc.lpszMenuName=NULL;
    wc.hCursor=LoadCursor(NULL,IDC_ARROW);
    wc.hbrBackground=(HBRUSH)(COLOR_WINDOW+1);
    wc.lpszClassName=L"Image Window Class";
    if (RegisterClass(&wc) == 0)
        exit(0);

	// read in image
    filedata = read_image(__argv, 1, &ROWS, &COLS, &bytes, &type);

    // initialize array to hold data for image display
    dispdata = (unsigned char *)calloc(ROWS * COLS * 2, sizeof(unsigned char *));

    // manipulate file data to align correctly for display (16-bit)
    // *little-endian*
    for (i = 0; i < ROWS * COLS; i++) {
      if (type == 0) { // grayscale
        red = green = blue = filedata[i];
      } else if (type == 1) { // RGB color
        red = filedata[3 * i];
        green = filedata[3 * i + 1];
        blue = filedata[3 * i + 2];
      }
	  // dummy bit/don't care at MSb of LSB i.e. dRGB
      dispdata[2 * i + 1] = ((red & 0xF8) >> 1) | (green >> 6);
      dispdata[2 * i] = ((green & 0xF8) << 2) | ((blue & 0xF8) >> 3);
    }

	// create rectangle to accomodate for window size
	wr.right = COLS;
	wr.left = 0;
	wr.bottom = ROWS;
	wr.top = 0;
	AdjustWindowRect(&wr, WS_OVERLAPPEDWINDOW, FALSE);

    WindowHandle=CreateWindowExA(0, "Image Window Class","ECE 6680 Lab 1",
								WS_OVERLAPPEDWINDOW,
								0, 0,
								wr.right - wr.left,    // width of the window
								wr.bottom - wr.top,    // height of the window
								NULL, NULL, hInstance, NULL);
    if (WindowHandle == NULL) {
        MessageBoxA(NULL,"No window","Try again",MB_OK | MB_APPLMODAL);
        exit(0);
    }
    ShowWindow (WindowHandle, SW_SHOWNORMAL);

    bm_info=(BITMAPINFO *)calloc(1,sizeof(BITMAPINFO) + 256 * sizeof(RGBQUAD));
    hDC=GetDC(WindowHandle);

    // set up bmiHeader field of bm_info
	bm_info->bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
	bm_info->bmiHeader.biWidth = COLS;
	bm_info->bmiHeader.biHeight = -ROWS;
	bm_info->bmiHeader.biPlanes = 1;
	bm_info->bmiHeader.biBitCount = 16;
	bm_info->bmiHeader.biCompression = BI_RGB;
	bm_info->bmiHeader.biSizeImage = ROWS * COLS;

    for (i=0; i<256; i++) {	/* colormap */
        bm_info->bmiColors[i].rgbBlue=bm_info->bmiColors[i].rgbGreen=bm_info->bmiColors[i].rgbRed=i;
        bm_info->bmiColors[i].rgbReserved=0;
    }
    SetDIBitsToDevice(hDC,0,0,COLS,ROWS,0,0,
                      0, /* first scan line */
                      ROWS, /* number of scan lines */
					  dispdata,bm_info,DIB_RGB_COLORS);
    ReleaseDC(WindowHandle,hDC);
    free(bm_info);
    MessageBoxA(NULL,"Press OK to continue","",MB_OK | MB_APPLMODAL);
}

unsigned char *read_image(char **argv, int arg, int *rows, int *cols, int *bytes, int *type) {
    FILE *fpt;
	char header[MAX_FILENAME_CHARS];
    unsigned char		*img;

	if ((fpt = fopen(argv[arg], "rb")) == NULL) {
		MessageBoxA(NULL,"Unable to open file for reading","",MB_OK | MB_APPLMODAL);
        exit(0);
    }

    fscanf(fpt, "%s %d %d %d", header, cols, rows, bytes);

    if (strcmp(header,"P5") == 0) { // grayscale
        img = (unsigned char *)calloc(*rows * *cols, sizeof(unsigned char));
        header[0] = fgetc(fpt);	// read white-space char that separates header
        fread(img, 1, *cols * *rows, fpt);
        *type = 0;
    } else if (strcmp(header,"P6") == 0) { // RGB color
        img = (unsigned char *)calloc(3 * *rows * *cols, sizeof(unsigned char));
        header[0] = fgetc(fpt);	// read white-space char that separates header
        fread(img, 3, *cols * *rows, fpt);
        *type = 1;
    } else {
		MessageBoxA(NULL,"PPM image could not be read","",MB_OK | MB_APPLMODAL);
        exit(0);
    }
    fclose(fpt);

    return img;
}

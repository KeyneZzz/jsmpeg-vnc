#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <stdio.h>
#include <stdlib.h>

#include "grabber.h"

grabber_t *grabber_create(HWND window, grabber_crop_area_t crop) {
	grabber_t *self = (grabber_t *)malloc(sizeof(grabber_t));
	memset(self, 0, sizeof(grabber_t));
	
	RECT rect;
	GetClientRect(window, &rect);
	
	self->window = window;
	
	self->width = rect.right-rect.left;
	self->height = rect.bottom-rect.top;

	self->crop = crop;
	if( crop.width == 0 || crop.height == 0 ) {
		self->crop.width = self->width - crop.x;
		self->crop.height = self->height - crop.y; 
	}

	self->width = self->crop.width;
	self->height = self->crop.height;
	
	self->windowDC = GetDC(window);
	self->memoryDC = CreateCompatibleDC(self->windowDC);
	self->bitmap = CreateCompatibleBitmap(self->windowDC, self->width, self->height);
	
	self->bitmapInfo.biSize = sizeof(BITMAPINFOHEADER);
	self->bitmapInfo.biPlanes = 1;
	self->bitmapInfo.biBitCount = 32;
	self->bitmapInfo.biWidth = self->width;
	self->bitmapInfo.biHeight = -self->height;
	self->bitmapInfo.biCompression = BI_RGB;
	self->bitmapInfo.biSizeImage = 0;
	
	self->pixels = malloc(self->width * self->height * 4);

	return self;
}

void grabber_destroy(grabber_t *self) {
	if( self == NULL ) { return; }

	ReleaseDC(self->window, self->windowDC);
    DeleteDC(self->memoryDC);
    DeleteObject(self->bitmap);
	
	free(self->pixels);
	free(self);
}

void *grabber_grab(grabber_t *self) {
	SelectObject(self->memoryDC, self->bitmap);
	BitBlt(self->memoryDC, 0, 0, self->width, self->height, self->windowDC, self->crop.x, self->crop.y, SRCCOPY|CAPTUREBLT);
	CURSORINFO cursor = { sizeof(cursor) };
    	GetCursorInfo(&cursor);

        RECT rect;
        GetWindowRect(self->window, &rect);
        ICONINFO info = { sizeof(info) };
        GetIconInfo(cursor.hCursor, &info);
        const int x = cursor.ptScreenPos.x - rect.left - rect.left - info.xHotspot;
        const int y = cursor.ptScreenPos.y - rect.top - rect.top - info.yHotspot;
        BITMAP bmpCursor = { 0 };
        GetObject(info.hbmColor, sizeof(bmpCursor), &bmpCursor);
        DrawIconEx(self->memoryDC, x, y, cursor.hCursor, bmpCursor.bmWidth, bmpCursor.bmHeight,
            0, NULL, DI_NORMAL);

	GetDIBits(self->memoryDC, self->bitmap, 0, self->height, self->pixels, (BITMAPINFO*)&(self->bitmapInfo), DIB_RGB_COLORS);
	
	return self->pixels;
}


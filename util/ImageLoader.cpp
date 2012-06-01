#include "ImageLoader.h"
#include "../base/Log.h"

#include <png.h>
#include <assert.h>
#include <stdlib.h>

static void read_from_buffer(png_structp png_ptr, png_bytep outBytes, png_size_t byteCountToRead);

struct FileBufferOffset {
	FileBuffer file;
	int offset;
};
ImageDesc ImageLoader::loadPng(const std::string& context, const FileBuffer& file) {
	uint8_t PNG_header[8];

	ImageDesc result;
	result.datas = 0;
	result.type = ImageDesc::RAW;

	memcpy(PNG_header, file.data, 8);
	if (png_sig_cmp(PNG_header, 0, 8) != 0) {
		LOGW("%s is not a PNG\n", context.c_str());
		return result;
	}

	png_structp PNG_reader = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
	if (PNG_reader == NULL)
	{
		LOGW("Can't start reading %s.\n", context.c_str());
		return result;
	}

	png_infop PNG_info = png_create_info_struct(PNG_reader);
	if (PNG_info == NULL)
	{
		LOGW("ERROR: Can't get info for %s\n", context.c_str());
		png_destroy_read_struct(&PNG_reader, NULL, NULL);
		return result;
	}

png_infop PNG_end_info = png_create_info_struct(PNG_reader);

	if (setjmp(png_jmpbuf(PNG_reader)))
	{
		LOGW("ERROR: Can't load %s\n", context.c_str());
		png_destroy_read_struct(&PNG_reader, &PNG_info, &PNG_info);
		return result;
	}

	FileBufferOffset fb;
	fb.file = file;
	fb.offset = 8;
	png_set_read_fn(PNG_reader, &fb, &read_from_buffer);
	// png_init_io(PNG_reader, PNG_file);
	png_set_sig_bytes(PNG_reader, 8);

	png_read_info(PNG_reader, PNG_info);

	result.width = png_get_image_width(PNG_reader, PNG_info);
	result.height = png_get_image_height(PNG_reader, PNG_info);

	png_uint_32 bit_depth, color_type;
	bit_depth = png_get_bit_depth(PNG_reader, PNG_info);
	color_type = png_get_color_type(PNG_reader, PNG_info);

	result.channels = 0;
	if (color_type == PNG_COLOR_TYPE_GRAY)
	{
		if (bit_depth < 8) {
			png_set_expand_gray_1_2_4_to_8(PNG_reader);
		}
		result.channels = 1;
	} else if (color_type == PNG_COLOR_TYPE_GRAY_ALPHA) {
		result.channels = 2;
	} else if (color_type == PNG_COLOR_TYPE_RGB) {
		result.channels = 3;
	} else if (color_type == PNG_COLOR_TYPE_RGBA) {
		result.channels = 4;
	} else {
		LOGW("%s INVALID color type: %u", context.c_str(), color_type);
		assert(false);
	}

	if (png_get_valid(PNG_reader, PNG_info, PNG_INFO_tRNS))
	{
		png_set_tRNS_to_alpha(PNG_reader);
	}
	else
	{
		png_set_filler(PNG_reader, 0xff, PNG_FILLER_AFTER);
	}

	if (bit_depth == 16)
	{
		png_set_strip_16(PNG_reader);
	}

	png_read_update_info(PNG_reader, PNG_info);

	int rowbytes = png_get_rowbytes(PNG_reader, PNG_info);
	png_byte* PNG_image_buffer = (png_byte*) malloc(rowbytes * result.height);

	png_byte** PNG_rows = (png_byte**)malloc(result.height * sizeof(png_byte*));
	
	int row;
	for (row = 0; row < result.height; ++row) {
		PNG_rows[result.height - 1 - row] = PNG_image_buffer + (row * rowbytes);
	}


	png_read_image(PNG_reader, PNG_rows);
	free(PNG_rows);

	png_destroy_read_struct(&PNG_reader, &PNG_info, &PNG_end_info);

	result.datas = (char*)PNG_image_buffer;
	return result;
}
		
ImageDesc ImageLoader::loadEct1(const std::string& context, const FileBuffer& file) {
	ImageDesc result;
	return result;
}

static void read_from_buffer(png_structp png_ptr, png_bytep outBytes, png_size_t byteCountToRead) {
   if(png_get_io_ptr(png_ptr) == NULL)
      return;   // add custom error handling here
   FileBufferOffset* buffer = (FileBufferOffset*)png_get_io_ptr(png_ptr);
   memcpy(outBytes, &buffer->file.data[buffer->offset], byteCountToRead);
   buffer->offset += byteCountToRead;
}

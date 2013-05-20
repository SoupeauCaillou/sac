#include "TexturePacker.h"
#include <png.h>
#include <iostream>

#include <base/Log.h>

static char* loadPng(const char* assetName, int* width, int* height);

int main(int argc, char** argv) {
	if (argc <= 1) {
		LOGE( "Usage: texture_packer file1.png file2.png ... fileN.png" );
		return -1;
	}

	TEXTURE_PACKER::TexturePacker *tp = TEXTURE_PACKER::createTexturePacker();
	tp->setTextureCount(argc - 1);
	for (int i=1; i<argc; i++) {
		int width, height;
		char* img = loadPng(argv[i], &width, &height);
		if (!img) {
			LOGE ("Unable to load '" << argv[i] << "'" );
			return -1;
		}
		tp->addTexture(width, height);
	}

	int finalW, finalH;
	#if 0
	int unused_area =
	#endif
	tp->packTextures(finalW, finalH,true,true);

	// GAUTIER BORDEL, LAISSE LES std::cout :p
	std::cout << "Atlas size:" << finalW << "," << finalH << std::endl;

	for (int i=1; i<argc; i++) {
		int x, y, wid, hit;
		bool rotated = tp->getTextureLocation(i-1, x, y, wid, hit);
		// MEME REMARQUE :p
		std::cout << argv[i] << "," << x << "," << y << "," << wid << "," << hit << "," << rotated << std::endl;
	}

	TEXTURE_PACKER::releaseTexturePacker(tp);
	return 0;
}

static char* loadPng(const char* assetName, int* width, int* height)
{
	png_byte* PNG_image_buffer;
	FILE *PNG_file = fopen(assetName, "rb");
	if (PNG_file == NULL) {
		std::cerr << assetName << " not found" << std::endl;
		return 0;
	}

	unsigned char PNG_header[8];

	fread(PNG_header, 1, 8, PNG_file);
	if (png_sig_cmp(PNG_header, 0, 8) != 0) {
		std::cerr << assetName << " is not a PNG." << std::endl;
		return 0;
	}

	png_structp PNG_reader = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
	if (PNG_reader == NULL)
	{
		std::cerr << "Can't start reading %s." << assetName << std::endl;
		fclose(PNG_file);
		return 0;
	}

	png_infop PNG_info = png_create_info_struct(PNG_reader);
	if (PNG_info == NULL)
	{
		std::cerr << "Can't get info for " << assetName << std::endl;
		png_destroy_read_struct(&PNG_reader, NULL, NULL);
		fclose(PNG_file);
		return 0;
	}

	png_infop PNG_end_info = png_create_info_struct(PNG_reader);
	if (PNG_end_info == NULL)
	{
		std::cerr << "Can't get end info for " << assetName << std::endl;
		png_destroy_read_struct(&PNG_reader, &PNG_info, NULL);
		fclose(PNG_file);
		return 0;
	}

	if (setjmp(png_jmpbuf(PNG_reader)))
	{
		std::cerr << "Can't load " << assetName << std::endl;
		png_destroy_read_struct(&PNG_reader, &PNG_info, &PNG_end_info);
		fclose(PNG_file);
		return 0;
	}

	png_init_io(PNG_reader, PNG_file);
	png_set_sig_bytes(PNG_reader, 8);

	png_read_info(PNG_reader, PNG_info);

	*width = png_get_image_width(PNG_reader, PNG_info);
	*height = png_get_image_height(PNG_reader, PNG_info);

	png_uint_32 bit_depth, color_type;
	bit_depth = png_get_bit_depth(PNG_reader, PNG_info);
	color_type = png_get_color_type(PNG_reader, PNG_info);

	if (color_type == PNG_COLOR_TYPE_PALETTE)
	{
		png_set_palette_to_rgb(PNG_reader);
	}

	if (color_type == PNG_COLOR_TYPE_GRAY && bit_depth < 8)
	{
		png_set_expand_gray_1_2_4_to_8(PNG_reader);
	}

	if (color_type == PNG_COLOR_TYPE_GRAY ||
		color_type == PNG_COLOR_TYPE_GRAY_ALPHA)
	{
		png_set_gray_to_rgb(PNG_reader);
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

	PNG_image_buffer = new png_byte[4 * (*width) * (*height)];
	png_byte** PNG_rows = new png_byte*[*height * sizeof(png_byte*)];

	unsigned int row;
	for (row = 0; row < (unsigned)*height; ++row) {
		PNG_rows[*height - 1 - row] = PNG_image_buffer + (row * 4 * *width);
	}

	png_read_image(PNG_reader, PNG_rows);

	delete[] PNG_rows;

	png_destroy_read_struct(&PNG_reader, &PNG_info, &PNG_end_info);
	fclose(PNG_file);

	return (char*)PNG_image_buffer;
}

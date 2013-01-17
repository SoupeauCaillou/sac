/*!
 * \file ImageLoader.h
 * \brief A tool to load images
 * \author Pierre-Eric Pelloux-Prayer
 * \author Gautier Pelloux-Prayer
 */
#pragma once

#include <string>
#include "../api/AssetAPI.h"

/*!
 * \struct ImageDesc
 * \brief Structure which contains image description
 */
struct ImageDesc {
	char* datas; /*!< Value of each pixels */
	int width, height; /*!< Size of the image */
    int channels, mipmap; /*!< Number of channel */
    /*! \enum type
     * \brief Type of image
     */
    enum {
		RAW, /*!< RAW */
		ECT1, /*!< ECT1 */
		PVR /*!< PVR */
	} type;
};

/*!
 * \class ImageLoader
 * \brief Description of object which load images
 */
class ImageLoader {
	public:
        /*! \brief Load an image in png format
         *  \param context : ?
         *  \param file : image file
         *  \return An image description structure
         */
		static ImageDesc loadPng(const std::string& context, const FileBuffer& file);

        /*! \brief Load an image in Ect1 format
         *  \param context : ?
         *  \param file : image file
         *  \return An image description structure
         */
        static ImageDesc loadEct1(const std::string& context, const FileBuffer& file);

        /*! \brief Load an image in raw format
         *  \param context : ?
         *  \param file : image file
         *  \return An image description structure
         */
		static ImageDesc loadPvr(const std::string& context, const FileBuffer& file);
};

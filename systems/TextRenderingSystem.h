/*!
 * \file TextRenderingSystem.h
 * \brief 
 * \author Pierre-Eric Pelloux-Prayer
 * \author Gautier Pelloux-Prayer
 */
#pragma once

#if defined(ANDROID) || defined(EMSCRIPTEN)
#include <GLES2/gl2.h>
#else
#include <GL/glew.h>
#endif

#include "System.h"

#include <list>

/*! \struct TextRenderingComponent
 *  \brief
 */
struct TextRenderingComponent {
	const static float LEFT;
	const static float CENTER;
	const static float RIGHT;

	const static int IsANumberBit = 1 << 0;
	const static int AdjustHeightToFillWidthBit = 1 << 1;
	const static int MultiLineBit = 1 << 2;

	TextRenderingComponent() : text(""), fontName("typo"),
	positioning(CENTER), hide(true), flags(0), cameraBitMask(~0U) {
		caret.show = false;
		caret.speed = caret.dt = 0;
		blink.offDuration =
		blink.onDuration =
		blink.accum = 0;
	}

	std::string text; //!< Text to show
	Color color; //!< Color of text to show

	//if its localizable
	std::string localizableID; //!< Id of text language (if it's localizable)

	union {
		float charHeight; //!< Size of character of string
		float maxCharHeight; //!< Size max of character of string
	};

	std::string fontName; //!< Font name to use
	float positioning; //!< Position of text
	bool hide; //!< State of text (hide or not)
	int flags; //!< ?
    /*! \struct caret
     *  \brief ? */
    struct {
		bool show; //!< ?
		float speed; //!< ?
		float dt; //!< ?
	} caret;
    /*! \struct blink
     *  \brief Used to add an effect of blink to text*/
	struct {
		float offDuration; //!< Time while text is hide (in ms)
		float onDuration;  //!< Time while text is show (in ms)
		float accum; //!< ?
	} blink;
	unsigned cameraBitMask; //!< ?

	// managed by systems
	std::vector<Entity> drawing; //!< ?
};

#define theTextRenderingSystem TextRenderingSystem::GetInstance()
#define TEXT_RENDERING(e) theTextRenderingSystem.Get(e)

/*! \class TextRendering
 *  \brief
 */
UPDATABLE_SYSTEM(TextRendering)

public :
    /*! \brief creates a new entity
     *  \return returns an entity */
	Entity CreateEntity();
    /*! \brief delete an entity
     *  \param e : entity to delete */
	void DeleteEntity(Entity e);

    /*! \brief add a font
     *  \param fontName : name of new font
     *  \param charH2Wratio : ratio between height and width for each character of font */
	void registerFont(const std::string& fontName, const std::map<unsigned char, float>& charH2Wratio) {
		fontRegistry[fontName] = charH2Wratio;
	}

    /*! \brief computes the width of textrendering component
     *  \param [out] trc :
     *  \return Return the computed value */
	float computeTextRenderingComponentWidth(TextRenderingComponent* trc);

private:
    std::map<Entity, unsigned int> cache; //!< ?
	std::list<Entity> renderingEntitiesPool; //!< ?
	std::map<std::string, std::map<unsigned char, float> > fontRegistry;//!< ?
};

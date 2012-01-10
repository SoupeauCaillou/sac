#include "RenderingSystem.h"
#include "../base/MathUtil.h"
#include <GL/gl.h>
#include <GL/glu.h>
#include <cstdio>
#include <cstdlib>

#include "TransformationSystem.h"

INSTANCE_IMPL(RenderingSystem);
	
RenderingSystem::RenderingSystem() : ComponentSystem<RenderingComponent>("rendering") { 
	nextValidRef = 1;
}

void RenderingSystem::setWindowSize(int w, int h) {

	float ratio = w / (float)h;
	/* setup camera */
	glMatrixMode(GL_PROJECTION); 
	glLoadIdentity(); 
	glOrtho(-1, 1, -1.0 / ratio, 1.0 / ratio, 0, 1); 
}

TextureRef RenderingSystem::loadTextureFile(const std::string& assetName) {
	if (!decompressPNG)
		return 0;

	int w, h;
	char* data = (*decompressPNG)(assetName.c_str(), &w, &h);

	if (!data)
		return 0;

	/* create GL texture */
	glEnable(GL_TEXTURE_2D);
	GLuint texture;
	glGenTextures(1, &texture);
	glBindTexture(GL_TEXTURE_2D, texture);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w,
                h, 0, GL_RGBA, GL_UNSIGNED_BYTE,
                data);

	textures[nextValidRef] = texture;

	free(data);

	return nextValidRef++;
}

void RenderingSystem::DoUpdate(float dt) {
	glMatrixMode(GL_MODELVIEW); 
	glLoadIdentity();

	/* render */
	glBegin(GL_QUADS);
	for(ComponentIt it=components.begin(); it!=components.end(); ++it) {
		Entity a = (*it).first;			
		RenderingComponent* rc = (*it).second;
		const TransformationComponent* tc = TRANSFORM(a);

		if (rc->texture > 0)
		{
			glEnable(GL_TEXTURE_2D);
			glBindTexture(GL_TEXTURE_2D, textures[rc->texture]);
		}
		else {
			glDisable(GL_TEXTURE_2D);
		}

		// glColor4f(1.0f, 0.5f, 0.0f, 1.0f);

		glPushMatrix();
		glTranslatef(tc->worldPosition.X, tc->worldPosition.Y, 0.0f);
		glRotatef(tc->worldRotation, 0, 0, 1);
		glScalef(rc->size.X, rc->size.Y, 1.0f);
	
		glTexCoord2f(1, 1); glVertex2f( 0.5f,  0.5f);
		glTexCoord2f(0, 1); glVertex2f(-0.5f,  0.5f);
		glTexCoord2f(0, 0); glVertex2f(-0.5f, -0.5f);
		glTexCoord2f(1, 0); glVertex2f( 0.5f, -0.5f);

		glPopMatrix();
	}
	glEnd();
}


attribute vec2 aWindowPosition;
attribute vec2 aTexCoord;
attribute vec4 aColor;

uniform mat4 uMvp;
uniform vec2 windowSize;
varying vec2 uvVarying;
varying vec4 colorVarying;

void main()
{
    gl_Position = uMvp * vec4(aWindowPosition, -0.5, 1.0);
    colorVarying = aColor;
    uvVarying = aTexCoord;
}


#ifdef GL_ES
precision lowp float;
#endif
uniform sampler2D tex0;
uniform sampler2D tex1;
uniform vec4 vColor;

varying vec2 uvVarying;

void main()
{
    gl_FragColor.rgb = (vColor.rgb) * vColor.a;
    gl_FragColor.a = vColor.a;
}


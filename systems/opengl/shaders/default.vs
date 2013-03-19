attribute vec3 aPosition;
attribute vec2 aTexCoord; 

uniform float uMvp[6];
uniform vec3 uCamera;

varying vec2 uvVarying;

void main()
{
	mat4 worldToCamRot = mat4(
		vec4(cos(uCamera.z), -sin(uCamera.z), 0.0, 0.0),
		vec4(sin(uCamera.z), cos(uCamera.z), 0.0, 0.0),
		vec4(0.0, 0.0, 1, 0.0),
		vec4(0, 0, 0, 1.0));
    mat4 worldToCamTrans = mat4(
        vec4(1.0, 0.0, 0.0, 0.0),
        vec4(0.0, 1.0, 0.0, 0.0),
        vec4(0.0, 0.0, 1.0, 0.0),
        vec4(-uCamera.xy, 0.0, 1.0));
	mat4 mvp = mat4(
		vec4(uMvp[0], 0.0, 0.0, 0.0),
		vec4(0.0, uMvp[1], 0.0, 0.0),
		vec4(0.0, 0.0, uMvp[2], 0.0),
		vec4(uMvp[3], uMvp[4], uMvp[5], 1.0));
	gl_Position = mvp * worldToCamRot * worldToCamTrans * vec4(aPosition, 1.0);
	uvVarying = aTexCoord;
}


#pragma once
#ifndef _LIBMATH_H_
#define _LIBMATH_H_
#endif

struct float2 {
	float x, y; float2(float _x, float _y) { x = _x; y = _y; }
};
struct float3 {
	float x, y, z; 
	float3(float _x, float _y, float _z) { x = _x; y = _y; z = _z; }
};
struct float4 {
	float x, y, z, w; float4(float _x, float _y, float _z, float _w) { x = _x; y = _y; z = _z; w = _w; }
};
struct float2x2{
	float m[2][2];
	float2x2()
	{
		for (int i = 0; i < 2; i++) for (int j = 0; j < 2; j++) m[i][j] = 0.0f;
	}
	float2x2(
		float _m00, float _m01,
		float _m10, float _m11
	)
	{
		m[0][0] = _m00; m[0][1] = _m01;
		m[1][0] = _m10; m[1][1] = _m11;
	}

	float operator()(int _row, int _column)
	{
		return m[_row][_column];
	}

	void set(int _row, int _column, float _val)
	{
		m[_row][_column] = _val;
	}

	float2 row(int _row)
	{
		return float2(m[_row][0], m[_row][1]);
	}

	float2 column(int _column)
	{
		return float2(m[0][_column], m[1][_column]);
	}
};
struct float3x3 {
	float m[3][3];
	float3x3()
	{
		for (int i = 0; i < 3; i++) for (int j = 0; j < 3; j++) m[i][j] = 0.0f;
	}
	float3x3(
		float _m00, float _m01, float _m02,
		float _m10, float _m11, float _m12,
		float _m20, float _m21, float _m22
	)
	{
		m[0][0] = _m00; m[0][1] = _m01; m[0][2] = _m02;
		m[1][0] = _m10; m[1][1] = _m11; m[1][2] = _m12;
		m[2][0] = _m20; m[2][1] = _m21; m[2][2] = _m22;
	}

	float operator()(int _row, int _column)
	{
		return m[_row][_column];
	}

	void set(int _row, int _column, float _val)
	{
		m[_row][_column] = _val;
	}

	float3 row(int _row)
	{
		return float3(m[_row][0], m[_row][1], m[_row][2]);
	}

	float3 column(int _column)
	{
		return float3(m[0][_column], m[1][_column], m[2][_column]);
	}
};
struct float4x4 {
	float m[4][4];
	float4x4()
	{
		for (int i = 0; i < 4; i++) for(int j=0;j<4;j++) m[i][j] = 0.0f;
	}
	float4x4(
		float _m00, float _m01, float _m02, float _m03,
		float _m10, float _m11, float _m12, float _m13,
		float _m20, float _m21, float _m22, float _m23,
		float _m30, float _m31, float _m32, float _m33
	)
	{
		m[0][0] = _m00; m[0][1] = _m01; m[0][2] = _m02; m[0][3] = _m03;
		m[1][0] = _m10; m[1][1] = _m11; m[1][2] = _m12; m[1][3] = _m13;
		m[2][0] = _m20; m[2][1] = _m21; m[2][2] = _m22; m[2][3] = _m23;
		m[3][0] = _m30; m[3][1] = _m31; m[3][2] = _m32; m[3][3] = _m33;
	}

	float operator()(int _row, int _column)
	{
		return m[_row][_column];
	}

	void set(int _row, int _column, float _val)
	{
		m[_row][_column] = _val;
	}

	float4 row(int _row)
	{
		return float4(m[_row][0], m[_row][1], m[_row][2], m[_row][3]);
	}

	float4 column(int _column)
	{
		return float4(m[0][_column], m[1][_column], m[2][_column], m[3][_column]);
	}
};

float2 operator+(float2& v1, float2& v2) { return float2(v1.x + v2.x, v1.y + v2.y); }
float2 operator-(float2& v1, float2& v2) { return float2(v1.x - v2.x, v1.y - v2.y); }
float2 operator*(float2& v, float f) { return float2(v.x * f, v.y * f); }
float2 operator*(float f, float2& v) { return float2(v.x * f, v.y * f); }
float2 operator/(float2& v, float f) { return float2(v.x / f, v.y / f); }
float2 operator/(float f, float2& v) { return float2(f / v.x, f / v.y); }
float3 operator+(float3& v1, float3& v2) { return float3(v1.x + v2.x, v1.y + v2.y, v1.z + v2.z); }
float3 operator-(float3& v1, float3& v2) { return float3(v1.x - v2.x, v1.y - v2.y, v1.z - v2.z); }
float3 operator*(float3& v, float f) { return float3(v.x * f, v.y*f,v.z*f); }
float3 operator*(float f, float3& v) { return float3(v.x * f, v.y*f,v.z*f); }
float3 operator/(float3& v, float f) { return float3(v.x / f, v.y / f, v.z / f); }
float3 operator/(float f, float3& v) { return float3(f/v.x, f/v.y, f/v.z); }
float4 operator+(float4& v1, float4& v2) { return float4(v1.x + v2.x, v1.y + v2.y, v1.z+v2.z, v1.w + v2.w); }
float4 operator-(float4& v1, float4& v2) { return float4(v1.x - v2.x, v1.y - v2.y, v1.z - v2.z, v1.w - v2.w); }
float4 operator*(float4& v, float f) { return float4(v.x * f, v.y * f, v.z * f, v.w * f); }
float4 operator*(float f, float4& v) { return float4(v.x * f, v.y * f, v.z * f, v.w * f); }
float4 operator/(float4& v, float f) { return float4(v.x / f, v.y / f, v.z / f, v.w / f); }
float4 operator/(float f, float4& v) { return float4(f / v.x, f / v.y, f / v.z, f / v.w); }


float dot(float2& v1, float2& v2) { return v1.x * v2.x + v1.y * v2.y; }
float dot(float3& v1, float3& v2) { return v1.x * v2.x + v1.y * v2.y + v1.z*v2.z; }
float dot(float4& v1, float4& v2) { return v1.x * v2.x + v1.y * v2.y + v1.z*v2.z + v1.w*v2.w; }
float3 cross(float3& v1, float3& v2) { return float3(v1.y * v2.z - v1.z * v2.y, v1.z * v2.x - v1.x * v2.z, v1.x * v2.y - v1.y * v2.x); }
float length(float2& v) { return sqrtf(dot(v, v)); }
float length(float3& v) { return sqrtf(dot(v, v)); }
float length(float4& v) { return sqrtf(dot(v, v)); }
float2 normalize(float2& v) { return v/length(v); }
float3 normalize(float3& v) { return v/length(v); }
float4 normalize(float4& v) { return v/length(v); }

float2 mul(float2x2& m, float2& v) { return float2(dot(m.row(0), v), dot(m.row(1), v));}
float3 mul(float3x3& m, float3& v) { return float3(dot(m.row(0), v), dot(m.row(1), v), dot(m.row(2), v)); }
float4 mul(float4x4& m, float4& v) { return float4(dot(m.row(0), v), dot(m.row(1), v), dot(m.row(2), v), dot(m.row(3), v)); }
float2x2 mul(float2x2& m1, float2x2& m2) { float2x2 m = float2x2();  for (int i = 0; i < 2; i++) for (int j = 0; j < 2; j++) m.set(i, j, dot(m1.row(i), m2.column(j))); return m; }
float3x3 mul(float3x3& m1, float3x3& m2) { float3x3 m = float3x3();  for (int i = 0; i < 3; i++) for (int j = 0; j < 3; j++) m.set(i, j, dot(m1.row(i), m2.column(j))); return m; }
float4x4 mul(float4x4& m1, float4x4& m2) { float4x4 m = float4x4();  for (int i = 0; i < 4; i++) for (int j = 0; j < 4; j++) m.set(i, j, dot(m1.row(i), m2.column(j))); return m; }

float4x4 compute_view_matrix(float3 _position_camera, float3 _view_point) {

	float3 up = { 0.0f,1.0f,0.0f };
	float3 forward = _view_point - _position_camera;
	forward = normalize(forward);
	float3 right = normalize(cross(up, forward));
	float3 newUp = normalize(cross(forward, right));

	float aa = -dot(right, _position_camera);
	float bb = -dot(newUp, _position_camera);
	float cc = -dot(forward, _position_camera);

	float4x4 view_matrix = {
			right.x,    newUp.x,    forward.x,    0.0f,
			right.y,    newUp.y,    forward.y,    0.0f,
			right.z,    newUp.z,    forward.z,    0.0f,
			-dot(right, _position_camera),   -dot(newUp, _position_camera),  -dot(forward, _position_camera),   1.0f
	};

	return view_matrix;
}

float4x4 compute_orthographic_matrix(float _width, float _height, float _znear, float _zfar) {
	float4x4 orthographic_matrix = float4x4(
										2.0f/_width,	0.0f,	0.0f,	0.0f,
										0.0f,	2.0f/_height,	0.0f,	0.0f,
										0.0f,	0.0f,	1.0f/(_zfar-_znear),	0.0f,
										0.0f,	0.0f,	-_znear/(_zfar-_znear),	1.0f
										);
	return orthographic_matrix;
}

float4x4 compute_perspective_matrix(float _fov, float _aspect_ratio, float _znear, float _zfar)
{
	float h = 1 / tanf(_fov * 0.5);
	float w = h / _aspect_ratio;
	float a = _zfar / (_zfar - _znear);
	float b = (-_znear * _zfar) / (_zfar - _znear);

	float4x4 projection_matrix = float4x4(
		w, 0.0f, 0.0f, 0.0f,
		0.0f, h, 0.0f, 0.0f,
		0.0f, 0.0f, a, 1.0f,
		0.0f, 0.0f, b, 0.0f
	);

	return projection_matrix;
}

float4x4 compute_translation_matrix(float x, float y, float z)
{
	float4x4 translation_matrix =
	{
		1.0f, 0.0f, 0.0f, 0.0,
		0.0f, 1.0f, 0.0f, 0.0,
		0.0f, 0.0f, 1.0f, 0.0,
		x, y, z, 1.0f
	};
	return translation_matrix;
}

float4x4 compute_rotation_matrix(float angle, float3 axis)

{
	float cosA = cosf(angle);
	float sinA = sinf(angle);
	float oneMinusCosA = 1.0f - cosA;

	float x = axis.x;
	float y = axis.y;
	float z = axis.z;

	float4x4 rotation_matrix =
	{
		cosA + oneMinusCosA * x * x, oneMinusCosA* x* y + sinA * z, oneMinusCosA* x* z - sinA * y,	0.0f,
		oneMinusCosA* x* y - sinA * z,	cosA + oneMinusCosA * y * y, oneMinusCosA* y* z + sinA * x, 0.0f,
		oneMinusCosA* x* z + sinA * y, oneMinusCosA* y* z - sinA * x, cosA + oneMinusCosA * z * z, 0.0f,
		0.0f, 0.0f, 0.0f, 1.0f
	};

	return rotation_matrix;
}

float4x4 compute_scale_matrix(float x, float y, float z)
{
	float4x4 scale_matrix =
	{
		x, 0.0f, 0.0f, 0.0f,
		0.0f, y, 0.0f, 0.0f,
		0.0f, 0.0f, z, 0.0f,
		0.0f, 0.0f, 0.0f, 1.0f
	};
	return scale_matrix;
}


float4 extract_rotation_info(float4x4 rotation_matrix)
{
	float trace = rotation_matrix.m[0][0] + rotation_matrix.m[1][1] + rotation_matrix.m[2][2];
	float cos_angle = (trace - 1.0f) * 0.5f;
	float angle = acosf(cos_angle);

	float4 rotation_info = float4(0.0,0.0,0.0,0.0);
	rotation_info.w = angle;

	if (angle > 0.0f)
	{
		rotation_info.x = rotation_matrix.m[2][1] - rotation_matrix.m[1][2];
		rotation_info.y = rotation_matrix.m[0][2] - rotation_matrix.m[2][0];
		rotation_info.z = rotation_matrix.m[1][0] - rotation_matrix.m[0][1];

		float inv_twice_sin_angle = -1.0f / (2.0f * sinf(angle));
		rotation_info.x *= inv_twice_sin_angle;
		rotation_info.y *= inv_twice_sin_angle;
		rotation_info.z *= inv_twice_sin_angle;
	}
	else
	{
		// No hay rotación, devuelve un vector nulo
		rotation_info.x = 0.0f;
		rotation_info.y = 0.0f;
		rotation_info.z = 0.0f;
	}

	return rotation_info;
}


float smoothstep(float a, float b, float val)
{
	if (val < a) return 0.0f;
	if (val > b) return 1.0f;
	float t = (val - a) / (b - a);
	val = 3.0 * t * t - 2.0 * t * t * t;
	return val;
}

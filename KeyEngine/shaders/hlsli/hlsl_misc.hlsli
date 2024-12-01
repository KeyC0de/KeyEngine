// HLSL built-in
//
//float dot( float3 A, float3 B )
//{
//	return A.x * B.x + A.y * B.y + A.z * B.z;
//}
//
//// return the Euclidean length of a vector.
//float length( float3 v )
//{
//	return sqrt( dot(v, v) );
//}
//
//float2 normalize( float2 v )
//{
//	return v / length( v );
//}
//
///// \brief	every vector component divided by the vector's magnitude/length
//float3 normalize( float3 v )
//{
//	return v / length( v );
//}
//
//float4 normalize( float4 v )
//{
//	return v / length( v );
//}
//

#define IDENTITY_MATRIX matrix( 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1 )


// 4x4 ordered dithering matrix
static const float ditherOrdered4x4[4][4] =
{
	{0,			8/16.f,		1/16.f,		9/16.f},
	{12/16.f,	4/16.f,		13/16.f,	5/16.f},
	{3/16.f,	11/16.f,	2/16.f,		10/16.f},
	{15/16.f,	7/16.f,		14/16.f,	6/16.f}
};

// outputs random number \in [0, 1) ie. between 0.0 and 0.999999 inclusive
float rand( float2 uv )
{
	float2 k1 = float2(23.14069263277926, 2.665144142690225); // e^pi (Gelfond's constant), 2^sqrt(2) (Gelfond's Schneider constant)
	return frac( cos( dot(uv, k1) ) * 12345.6789 );
}

float lengthSquared( float2 v )
{
	return dot( v, v );
}

float lengthSquared( float3 v )
{
	return dot( v, v );
}

float lengthSquared( float4 v )
{
	return dot( v, v );
}

float3 getTriangleNormal( float3 a,
	float3 b,
	float3 c )
{
	float3 edge1 = b - a;
	float3 edge2 = c - a;
	float3 normal = cross( edge1, edge2 );
	normal = normalize( normal );
	return normal;
}

float2x2 extract2x2( matrix mat )
{
	return float2x2( mat._11_12, mat._21_22 );
	// float2x2 will consist of:
	// [mat._11, mat._12]
	// [mat._21, mat._22]
}

float2x2 extract2x2From3x3( float3x3 mat )
{
	return float2x2( mat._11_12, mat._21_22 );
}

float3x3 extract3x3( matrix mat )
{
	return float3x3( mat._11_12_13, mat._21_22_23, mat._31_32_33 );
}

float4 fresnelLerp( float3 vec,
	float3 pos,
	float4 colorA,
	float4 colorB,
	float alpha = 1.0f,
	float reflectivity = 50.0f,
	float rZero = 1.0f )
{
	float4 fresnel = rZero + (1.0f - rZero) * pow( abs( 1.0f - dot( vec, pos ) ), 5.0 );
	float4 result = 5 * colorA + lerp( colorA, colorB, fresnel );
	result.a = alpha;
	return result;
}

float3x3 getRotatorAroundAxisAngle( float angle,
	float3 axis )
{
	float s;
	float c;
	sincos( angle, s, c );

	float x = axis.x;
	float y = axis.y;
	float z = axis.z;
	float t = 1 - c;

	return float3x3(
		t * x * x + c,      t * x * y - s * z,  t * x * z + s * y,
		t * x * y + s * z,  t * y * y + c,      t * y * z - s * x,
		t * x * z - s * y,  t * y * z + s * x,  t * z * z + c
	);
}

matrix inverseOfMatrix( matrix m )
{
	float n11 = m[0][0]; float n12 = m[1][0]; float n13 = m[2][0]; float n14 = m[3][0];
	float n21 = m[0][1]; float n22 = m[1][1]; float n23 = m[2][1]; float n24 = m[3][1];
	float n31 = m[0][2]; float n32 = m[1][2]; float n33 = m[2][2]; float n34 = m[3][2];
	float n41 = m[0][3]; float n42 = m[1][3]; float n43 = m[2][3]; float n44 = m[3][3];

	float t11 = n23 * n34 * n42 - n24 * n33 * n42 + n24 * n32 * n43 - n22 * n34 * n43 - n23 * n32 * n44 + n22 * n33 * n44;
	float t12 = n14 * n33 * n42 - n13 * n34 * n42 - n14 * n32 * n43 + n12 * n34 * n43 + n13 * n32 * n44 - n12 * n33 * n44;
	float t13 = n13 * n24 * n42 - n14 * n23 * n42 + n14 * n22 * n43 - n12 * n24 * n43 - n13 * n22 * n44 + n12 * n23 * n44;
	float t14 = n14 * n23 * n32 - n13 * n24 * n32 - n14 * n22 * n33 + n12 * n24 * n33 + n13 * n22 * n34 - n12 * n23 * n34;

	float det = n11 * t11 + n21 * t12 + n31 * t13 + n41 * t14;
	float idet = 1.0f / det;

	matrix ret;

	ret[0][0] = t11 * idet;
	ret[0][1] = ( n24 * n33 * n41 - n23 * n34 * n41 - n24 * n31 * n43 + n21 * n34 * n43 + n23 * n31 * n44 - n21 * n33 * n44 ) * idet;
	ret[0][2] = ( n22 * n34 * n41 - n24 * n32 * n41 + n24 * n31 * n42 - n21 * n34 * n42 - n22 * n31 * n44 + n21 * n32 * n44 ) * idet;
	ret[0][3] = ( n23 * n32 * n41 - n22 * n33 * n41 - n23 * n31 * n42 + n21 * n33 * n42 + n22 * n31 * n43 - n21 * n32 * n43 ) * idet;

	ret[1][0] = t12 * idet;
	ret[1][1] = ( n13 * n34 * n41 - n14 * n33 * n41 + n14 * n31 * n43 - n11 * n34 * n43 - n13 * n31 * n44 + n11 * n33 * n44 ) * idet;
	ret[1][2] = ( n14 * n32 * n41 - n12 * n34 * n41 - n14 * n31 * n42 + n11 * n34 * n42 + n12 * n31 * n44 - n11 * n32 * n44 ) * idet;
	ret[1][3] = ( n12 * n33 * n41 - n13 * n32 * n41 + n13 * n31 * n42 - n11 * n33 * n42 - n12 * n31 * n43 + n11 * n32 * n43 ) * idet;

	ret[2][0] = t13 * idet;
	ret[2][1] = ( n14 * n23 * n41 - n13 * n24 * n41 - n14 * n21 * n43 + n11 * n24 * n43 + n13 * n21 * n44 - n11 * n23 * n44 ) * idet;
	ret[2][2] = ( n12 * n24 * n41 - n14 * n22 * n41 + n14 * n21 * n42 - n11 * n24 * n42 - n12 * n21 * n44 + n11 * n22 * n44 ) * idet;
	ret[2][3] = ( n13 * n22 * n41 - n12 * n23 * n41 - n13 * n21 * n42 + n11 * n23 * n42 + n12 * n21 * n43 - n11 * n22 * n43 ) * idet;

	ret[3][0] = t14 * idet;
	ret[3][1] = ( n13 * n24 * n31 - n14 * n23 * n31 + n14 * n21 * n33 - n11 * n24 * n33 - n13 * n21 * n34 + n11 * n23 * n34 ) * idet;
	ret[3][2] = ( n14 * n22 * n31 - n12 * n24 * n31 - n14 * n21 * n32 + n11 * n24 * n32 + n12 * n21 * n34 - n11 * n22 * n34 ) * idet;
	ret[3][3] = ( n12 * n23 * n31 - n13 * n22 * n31 + n13 * n21 * n32 - n11 * n23 * n32 - n12 * n21 * n33 + n11 * n22 * n33 ) * idet;

	return ret;
}

// http://www.euclideanspace.com/maths/geometry/rotations/conversions/matrixToQuaternion/
// assumes m is orthogonal
float4 matrixToQuaternion( matrix m )
{
	float tr = m[0][0] + m[1][1] + m[2][2];
	float4 q = float4(0, 0, 0, 0);

	if ( tr > 0 )
	{
		float s = sqrt( tr + 1.0 ) * 2; // S=4*qw
		q.w = 0.25 * s;
		q.x = (m[2][1] - m[1][2]) / s;
		q.y = (m[0][2] - m[2][0]) / s;
		q.z = (m[1][0] - m[0][1]) / s;
	}
	else if ( (m[0][0] > m[1][1]) && (m[0][0] > m[2][2]) )
	{
		float s = sqrt( 1.0 + m[0][0] - m[1][1] - m[2][2] ) * 2; // S=4*qx
		q.w = (m[2][1] - m[1][2]) / s;
		q.x = 0.25 * s;
		q.y = (m[0][1] + m[1][0]) / s;
		q.z = (m[0][2] + m[2][0]) / s;
	}
	else if ( m[1][1] > m[2][2] )
	{
		float s = sqrt( 1.0 + m[1][1] - m[0][0] - m[2][2] ) * 2; // S=4*qy
		q.w = (m[0][2] - m[2][0]) / s;
		q.x = (m[0][1] + m[1][0]) / s;
		q.y = 0.25 * s;
		q.z = (m[1][2] + m[2][1]) / s;
	}
	else
	{
		float s = sqrt( 1.0 + m[2][2] - m[0][0] - m[1][1] ) * 2; // S=4*qz
		q.w = (m[1][0] - m[0][1]) / s;
		q.x = (m[0][2] + m[2][0]) / s;
		q.y = (m[1][2] + m[2][1]) / s;
		q.z = 0.25 * s;
	}

	return q;
}

matrix getAxisMatrix( float3 right,
	float3 up,
	float3 forward )
{
	float3 xAxis = right;
	float3 yAxis = up;
	float3 zAxis = forward;
	return matrix(
		xAxis.x, yAxis.x, zAxis.x, 0,
		xAxis.y, yAxis.y, zAxis.y, 0,
		xAxis.z, yAxis.z, zAxis.z, 0,
		0, 0, 0, 1 );
}

// http://stackoverflow.com/questions/349050/calculating-a-lookat-matrix
matrix getLookAtMatrix( float3 forward,
	float3 up )
{
	float3 xAxis = normalize( cross( forward, up ) );
	return getAxisMatrix( xAxis, up, forward );
}

matrix getLookAtMatrix( float3 target,
	float3 eye,
	float3 up )
{
	float3 zAxis = normalize( target - eye );
	float3 xAxis = normalize( cross( up, zAxis ) );
	float3 yAxis = cross( zAxis, xAxis );
	return getAxisMatrix( xAxis, yAxis, zAxis );
}

matrix extractRotationMatrix( matrix m )
{
	float scaleX = length( float3(m[0][0], m[0][1], m[0][2]) );
	float scaleY = length( float3(m[1][0], m[1][1], m[1][2]) );
	float scaleZ = length( float3(m[2][0], m[2][1], m[2][2]) );

	// if determinant is negative, we need to invert the scale
	float det = determinant( m );
	if ( det < 0 )
	{
		scaleX = -scaleX;
	}

	float invScaleX = 1.0 / scaleX;
	float invScaleY = 1.0 / scaleY;
	float invScaleZ = 1.0 / scaleZ;

	m[0][0] *= invScaleX;
	m[0][1] *= invScaleX;
	m[0][2] *= invScaleX;
	m[0][3] = 0;

	m[1][0] *= invScaleY;
	m[1][1] *= invScaleY;
	m[1][2] *= invScaleY;
	m[1][3] = 0;

	m[2][0] *= invScaleZ;
	m[2][1] *= invScaleZ;
	m[2][2] *= invScaleZ;
	m[2][3] = 0;

	m[3][0] = 0;
	m[3][1] = 0;
	m[3][2] = 0;
	m[3][3] = 1;

	return m;
}

float drawCircle( const in float2 uv,
	const in float radius )
{
	return step( radius, length( uv ) );
}

float drawRectangle( const in float2 uv,
	const in float2 scale )
{
	scale = float2( 0.5 ) - scale * 0.5;
	float2 rect = float2( step( scale.x, uv.x ), step( scale.y, uv.y ) )
				* float2( step( scale.x, 1.0 - uv.x ), step( scale.y, 1.0 - uv.y ) );
	return rect.x * rect.y;
}

float2x2 rotate2d( float angle )
{
	return float2x2( cos( angle ), sin( angle ), -sin( angle ), cos( angle ) );
}

float3 hsvToRgb( float3 hsv )
{
	float3 RGB = hsv.z;

	float h = (hsv.x + 180) / 60.0; //get a hue in the 0..5 interval from the -180 180 intv
	float s = hsv.y;
	float v = hsv.z;

	float i = floor( h );
	float f = h - i;

	float p = (1.0 - s);
	float q = (1.0 - s * f);
	float t = (1.0 - s * (1 - f));

	if ( i == 0 )
	{
		RGB = float3(1, t, p);
	}
	else if ( i == 1 )
	{
		RGB = float3(q, 1, p);
	}
	else if ( i == 2 )
	{
		RGB = float3(p, 1, t);
	}
	else if ( i == 3 )
	{
		RGB = float3(p, q, 1);
	}
	else if ( i == 4 )
	{
		RGB = float3(t, p, 1);
	}
	else // i == -1
	{
		RGB = float3(1, p, q);
	}

	RGB *= v;

	return RGB;
}
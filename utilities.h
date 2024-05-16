// utilities.h
#ifndef UTILITIES_H
#define UTILITIES_H

typedef struct
{
  float3 x, y, z;
} float3x3;

__DEVICE__ float3 ACES_RRT(float3 v, float a, float b, float c, float d, float e, float f, float g, float h, int per_channel)
{
  if (per_channel == 1)
  {
    // Apply the tone mapping curve
    float x = ((v.x * (a * v.x + b) + c) * v.x + d) / ((v.x * (e * v.x + f) + g) * v.x + h);
    float y = ((v.y * (a * v.y + b) + c) * v.y + d) / ((v.y * (e * v.y + f) + g) * v.y + h);
    float z = ((v.z * (a * v.z + b) + c) * v.z + d) / ((v.z * (e * v.z + f) + g) * v.z + h);
    float3 result = make_float3(x, y, z);
    return result;
  }
  else
  {
    // Apply the tone mapping function to the luminance
    float mappedY = ((v.y * (a * v.y + b) + c) * v.y + d) / ((v.y * (e * v.y + f) + g) * v.y + h);
    // Calculate the scaling factor
    float scale = (v.y != 0.0f) ? (mappedY / v.y) : 0.0f;
    // Scale the X, Y, and Z channels
    float3 result = make_float3(v.x * scale, mappedY, v.z * scale);
    return result;
  }
}

__DEVICE__ float3 drago(float3 value, float bias, float L_max)
{
  // Extract the Y channel (luminance)
  float Y = value.y;
  // Apply the Drago tone mapping function to the luminance
  float mappedY = log(1.0 + Y * bias) / log(1.0 + L_max * bias);
  // Calculate the scaling factor
  float scale = (Y != 0.0f) ? (mappedY / Y) : 0.0f;
  // Scale the X, Y, and Z channels
  float3 result = make_float3(value.x * scale, mappedY, value.z * scale);
  return result;
}

__DEVICE__ float3 filmic(float3 value, float A, float B, float C, float D, float E, float F)
{
  float L = value.y;
  float mappedL = ((L * (A * L + C * B) + D * E) / (L * (A * L + B) + D * F)) - (E / F);

  float scale = mappedL / L;
  float x = value.x * scale;
  float y = mappedL; // Directly use the tone-mapped luminance
  float z = value.z * scale;

  return make_float3(x, y, z);
}

__DEVICE__ inline float3 hable(float3 v, float A, float B, float C, float D, float E, float F, int per_channel)
{
  float L = v.y;
  // Apply the tone mapping function to the luminance
  float mappedY = ((L * (A * L + C * B) + D * E) / (L * (A * L + B) + D * F));
  // Calculate the scaling factor
  float scale = (L != 0.0f) ? (mappedY / L) : 0.0f;
  // Scale the X, Y, and Z channels
  if (per_channel == 1)
  {
    float mappedX = ((v.x * (A * v.x + C * B) + D * E) / (v.x * (A * v.x + B) + D * F));
    float mappedZ = ((v.z * (A * v.z + C * B) + D * E) / (v.z * (A * v.z + B) + D * F));
    float3 result = make_float3(mappedX, mappedY, mappedZ);
    return result;
  }
  else
  {
    float3 result = make_float3(v.x * scale, mappedY, v.z * scale);
  }
  float3 result = make_float3(v.x * scale, mappedY, v.z * scale);
  return result;
}

// Function to convert Lab to XYZ
__DEVICE__ float3 labToXyz(float3 lab)
{
  float y = (lab.x + 16.0) / 116.0;
  float x = lab.y / 500.0 + y;
  float z = y - lab.z / 200.0;

  float3 xyz;
  xyz.x = 0.95047 * ((x * x * x > 0.008856) ? (x * x * x) : ((x - 16.0 / 116.0) / 7.787));
  xyz.y = (y * y * y > 0.008856) ? (y * y * y) : ((y - 16.0 / 116.0) / 7.787);
  xyz.z = 1.08883 * ((z * z * z > 0.008856) ? (z * z * z) : ((z - 16.0 / 116.0) / 7.787));
  return xyz;
}

__DEVICE__ inline float3 reinhard(float3 value, int method)
{
  if (method == 0)
  {
    return value;
  }
  // Apply Reinhard tone mapping to the Y channel for luminance
  else if (method == 1)
  {
    float L = value.y;
    float mappedL = L / (L + 1.0f);
    // Adjust the X, Y, and Z channels by the same factor
    float scale = mappedL / L;
    float x = value.x * scale;
    float y = mappedL; // Directly use the tone-mapped luminance
    float z = value.z * scale;
    float3 result = make_float3(x, y, z);
    return result;
  }
  else if (method == 2)
  {
    float L = 0.2126 * value.x + 0.7152 * value.y + 0.0722 * value.z;
    float mappedL = L / (L + 1.0f);
    float scale = mappedL / L;
    float x = value.x * scale;
    float y = value.y * scale;
    float z = value.z * scale;
    float3 result = make_float3(x, y, z);
    return result;
  }
  else if (method == 3)
  {
    float x = value.x / (value.x + 1.0);
    float y = value.y / (value.y + 1.0);
    float z = value.z / (value.z + 1.0);
    float3 result = make_float3(x, y, z);
    return result;
  }
  else if (method == 4)
  {
    float L = value.y;
    float3 result;
    if (L > 0.5)
    {
      float mappedL = L / (L + 1.0);
      float scale = mappedL / L;
      result = make_float3(value.x * scale, mappedL, value.z * scale);
    }
    else
    {
      result = value; // Keep the original value if below threshold
    }
    return result;
  }
  else if (method == 5)
  {
    float L = _fmaxf(_fmaxf(value.x, value.y), value.z);
    float mappedL = L / (L + 1.0);
    float scale = mappedL / L;
    float x = value.x * scale;
    float y = value.y * scale;
    float z = value.z * scale;
    float3 result = make_float3(x, y, z);
    return result;
  }
}

__DEVICE__ inline float3 maxf3(float b, float3 a)
{
  // For each component of float3 a, return max of component and float b
  return make_float3(_fmaxf(a.x, b), _fmaxf(a.y, b), _fmaxf(a.z, b));
}

// Function to convert XYZ to Lab
__DEVICE__ float3 xyzToLab(float3 xyz)
{
  float3 lab;
  xyz.x = xyz.x / 0.95047;
  xyz.z = xyz.z / 1.08883;

  xyz.x = (xyz.x > 0.008856) ? pow(xyz.x, 1.0 / 3.0) : (7.787 * xyz.x) + (16.0 / 116.0);
  xyz.y = (xyz.y > 0.008856) ? pow(xyz.y, 1.0 / 3.0) : (7.787 * xyz.y) + (16.0 / 116.0);
  xyz.z = (xyz.z > 0.008856) ? pow(xyz.z, 1.0 / 3.0) : (7.787 * xyz.z) + (16.0 / 116.0);

  lab.x = (116.0 * xyz.y) - 16.0;
  lab.y = 500.0 * (xyz.x - xyz.y);
  lab.z = 200.0 * (xyz.y - xyz.z);
  return lab;
}

#endif
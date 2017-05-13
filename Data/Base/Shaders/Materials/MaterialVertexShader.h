#pragma once

#define USE_WORLDPOS

#include <Shaders/Common/GlobalConstants.h>
#include <Shaders/Common/ObjectConstants.h>
#include <Shaders/Materials/MaterialInterpolator.h>

#if defined(USE_OBJECT_POSITION_OFFSET)
  float3 GetObjectPositionOffset(VS_IN Input, ezPerInstanceData data);
#endif

#if defined(USE_WORLD_POSITION_OFFSET)
  float3 GetWorldPositionOffset(VS_IN Input, ezPerInstanceData data);
#endif


ezPerInstanceData GetInstanceData(VS_IN Input)
{
  #if INSTANCING
    return perInstanceData[Input.InstanceID];
  #else
    return perInstanceData;
  #endif
}

VS_OUT FillVertexData(VS_IN Input)
{
  ezPerInstanceData data = GetInstanceData(Input);
  float4x4 objectToWorld = TransformToMatrix(data.ObjectToWorld);
  float3x3 objectToWorldNormal = TransformToRotation(data.ObjectToWorldNormal);

  float3 objectPosition = Input.Position;
  #if defined(USE_OBJECT_POSITION_OFFSET)
    objectPosition += GetObjectPositionOffset(Input, data);
  #endif

  VS_OUT Output;
  Output.WorldPosition = mul(objectToWorld, float4(objectPosition, 1.0)).xyz;
  #if defined(USE_WORLD_POSITION_OFFSET)
    Output.WorldPosition += GetWorldPositionOffset(Input, data);
  #endif

  Output.Position = mul(WorldToScreenMatrix, float4(Output.WorldPosition, 1.0));

  #if defined(USE_NORMAL)
    Output.Normal = normalize(mul(objectToWorldNormal, Input.Normal));
  #endif

  #if defined(USE_TANGENT)
    float handednessCorrection = 2.0f - dot(Input.Tangent, Input.Tangent);
    float3 biTangent = cross(Input.Normal, Input.Tangent) * handednessCorrection;

    Output.Tangent = normalize(mul(objectToWorldNormal, Input.Tangent));
    Output.BiTangent = normalize(mul(objectToWorldNormal, biTangent));
  #endif

  #if defined(USE_TEXCOORD0)
    #if USE_TEXCOORD1
      Output.TexCoords = float4(Input.TexCoord0, Input.TexCoord1);
    #else
      Output.TexCoords = Input.TexCoord0;
    #endif
  #endif

  #if defined(USE_COLOR)
    Output.Color = Input.Color;
  #endif

  #if INSTANCING
    Output.InstanceID = Input.InstanceID;
  #endif

  return Output;
}

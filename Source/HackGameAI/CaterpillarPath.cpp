// Copyright Epic Games, Inc. All Rights Reserved.

#include "CaterpillarPath.h"
#include "Components/SceneComponent.h"

ACaterpillarPath::ACaterpillarPath()
{
	PrimaryActorTick.bCanEverTick = false;

	Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	RootComponent = Root;
}

float ACaterpillarPath::GetPerimeter() const
{
	const float Width = MaxCorner.X - MinCorner.X;
	const float Height = MaxCorner.Y - MinCorner.Y;
	return 2.0f * Width + 2.0f * Height;
}

FVector ACaterpillarPath::GetLocationAtDistance(float Distance) const
{
	const float Width = MaxCorner.X - MinCorner.X;
	const float Height = MaxCorner.Y - MinCorner.Y;
	const float Perimeter = 2.0f * Width + 2.0f * Height;

	if (Perimeter <= 0.0f)
	{
		return FVector(MinCorner.X, MinCorner.Y, PathZ);
	}

	// Proper positive modulo: FMath::Fmod can return a negative result for negative input
	// (e.g. Fmod(-50, 400) == -50), which would not be a valid wrapped distance. Add the
	// perimeter and re-mod so the result always lands in [0, Perimeter).
	float WrappedDistance = FMath::Fmod(Distance, Perimeter);
	if (WrappedDistance < 0.0f)
	{
		WrappedDistance += Perimeter;
	}

	// Segment 1: (MaxX, MinY) -> (MaxX, MaxY), length = Height.
	const FVector2D Seg1Start(MaxCorner.X, MinCorner.Y);
	const FVector2D Seg1End(MaxCorner.X, MaxCorner.Y);
	if (WrappedDistance < Height)
	{
		const FVector2D Point = FMath::Lerp(Seg1Start, Seg1End, Height > 0.0f ? WrappedDistance / Height : 0.0f);
		return FVector(Point.X, Point.Y, PathZ);
	}
	WrappedDistance -= Height;

	// Segment 2: (MaxX, MaxY) -> (MinX, MaxY), length = Width.
	const FVector2D Seg2Start(MaxCorner.X, MaxCorner.Y);
	const FVector2D Seg2End(MinCorner.X, MaxCorner.Y);
	if (WrappedDistance < Width)
	{
		const FVector2D Point = FMath::Lerp(Seg2Start, Seg2End, Width > 0.0f ? WrappedDistance / Width : 0.0f);
		return FVector(Point.X, Point.Y, PathZ);
	}
	WrappedDistance -= Width;

	// Segment 3: (MinX, MaxY) -> (MinX, MinY), length = Height.
	const FVector2D Seg3Start(MinCorner.X, MaxCorner.Y);
	const FVector2D Seg3End(MinCorner.X, MinCorner.Y);
	if (WrappedDistance < Height)
	{
		const FVector2D Point = FMath::Lerp(Seg3Start, Seg3End, Height > 0.0f ? WrappedDistance / Height : 0.0f);
		return FVector(Point.X, Point.Y, PathZ);
	}
	WrappedDistance -= Height;

	// Segment 4: (MinX, MinY) -> (MaxX, MinY), length = Width. Remainder handles float error at
	// the wrap boundary, so clamp the alpha into [0, 1].
	const FVector2D Seg4Start(MinCorner.X, MinCorner.Y);
	const FVector2D Seg4End(MaxCorner.X, MinCorner.Y);
	const float Alpha = Width > 0.0f ? FMath::Clamp(WrappedDistance / Width, 0.0f, 1.0f) : 0.0f;
	const FVector2D Point = FMath::Lerp(Seg4Start, Seg4End, Alpha);
	return FVector(Point.X, Point.Y, PathZ);
}

// Fill out your copyright notice in the Description page of Project Settings.


#include "LineTrace.h"

// Sets default values for this component's properties
ULineTrace::ULineTrace()
{

}


// Called when the game starts
void ULineTrace::BeginPlay()
{
	Super::BeginPlay();

	// ...
	
}

FHitResult ULineTrace::LineTraceSingle(FVector Start, FVector End, bool ShowDebugLine)
{
	FHitResult HitResult;
	FCollisionObjectQueryParams CollisionObjectQueryParams;
	FCollisionQueryParams CollisionQueryParams;
	CollisionQueryParams.AddIgnoredActor(GetOwner());

	if(GetWorld()->LineTraceSingleByObjectType(OUT HitResult, Start, End, CollisionObjectQueryParams, CollisionQueryParams))
	{
		if (ShowDebugLine)
		{
			DrawDebugLine(GetWorld(), Start, HitResult.TraceEnd, FColor::Red, false, 3.0f, 0, 5.0f);
		}
		return HitResult;
	}
	else
	{
		if (ShowDebugLine)
		{
			DrawDebugLine(GetWorld(), Start, HitResult.TraceEnd, FColor::Red, false, 3.0f, 0, 5.0f);
		}
		return HitResult;
	}
}

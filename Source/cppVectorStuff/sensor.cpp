// Fill out your copyright notice in the Description page of Project Settings.


#include "sensor.h"

// Sets default values
Asensor::Asensor()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void Asensor::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void Asensor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}


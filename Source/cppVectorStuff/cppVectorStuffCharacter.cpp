// Copyright Epic Games, Inc. All Rights Reserved.

#include "cppVectorStuffCharacter.h"
#include "cppVectorStuffProjectile.h"
#include "Animation/AnimInstance.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/InputComponent.h"
#include "GameFramework/InputSettings.h"
#include "HeadMountedDisplayFunctionLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "MotionControllerComponent.h"
#include "XRMotionControllerBase.h" // for FXRMotionControllerBase::RightHandSourceId
#include <Math/Color.h>
#include <DrawDebugHelpers.h>
#include "Math/Vector.h"

//change test for perforce version contol

DEFINE_LOG_CATEGORY_STATIC(LogFPChar, Warning, All);

//////////////////////////////////////////////////////////////////////////
// AcppVectorStuffCharacter

AcppVectorStuffCharacter::AcppVectorStuffCharacter()
{
	// Set size for collision capsule
	GetCapsuleComponent()->InitCapsuleSize(55.f, 96.0f);

	// set our turn rates for input
	BaseTurnRate = 45.f;
	BaseLookUpRate = 45.f;

	// Create a CameraComponent	
	FirstPersonCameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("FirstPersonCamera"));
	FirstPersonCameraComponent->SetupAttachment(GetCapsuleComponent());
	FirstPersonCameraComponent->SetRelativeLocation(FVector(-39.56f, 1.75f, 64.f)); // Position the camera
	FirstPersonCameraComponent->bUsePawnControlRotation = true;

	// Create a mesh component that will be used when being viewed from a '1st person' view (when controlling this pawn)
	Mesh1P = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("CharacterMesh1P"));
	Mesh1P->SetOnlyOwnerSee(true);
	Mesh1P->SetupAttachment(FirstPersonCameraComponent);
	Mesh1P->bCastDynamicShadow = false;
	Mesh1P->CastShadow = false;
	Mesh1P->SetRelativeRotation(FRotator(1.9f, -19.19f, 5.2f));
	Mesh1P->SetRelativeLocation(FVector(-0.5f, -4.4f, -155.7f));

	// Create a gun mesh component
	FP_Gun = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("FP_Gun"));
	FP_Gun->SetOnlyOwnerSee(false);			// otherwise won't be visible in the multiplayer
	FP_Gun->bCastDynamicShadow = false;
	FP_Gun->CastShadow = false;
	// FP_Gun->SetupAttachment(Mesh1P, TEXT("GripPoint"));
	FP_Gun->SetupAttachment(RootComponent);

	FP_MuzzleLocation = CreateDefaultSubobject<USceneComponent>(TEXT("MuzzleLocation"));
	FP_MuzzleLocation->SetupAttachment(FP_Gun);
	FP_MuzzleLocation->SetRelativeLocation(FVector(0.2f, 48.4f, -10.6f));
	// Default offset from the character location for projectiles to spawn
	GunOffset = FVector(100.0f, 0.0f, 10.0f);

	// Note: The ProjectileClass and the skeletal mesh/anim blueprints for Mesh1P, FP_Gun, and VR_Gun 
	// are set in the derived blueprint asset named MyCharacter to avoid direct content references in C++.

	// Create VR Controllers.
	R_MotionController = CreateDefaultSubobject<UMotionControllerComponent>(TEXT("R_MotionController"));
	R_MotionController->MotionSource = FXRMotionControllerBase::RightHandSourceId;
	R_MotionController->SetupAttachment(RootComponent);
	L_MotionController = CreateDefaultSubobject<UMotionControllerComponent>(TEXT("L_MotionController"));
	L_MotionController->SetupAttachment(RootComponent);

	// Create a gun and attach it to the right-hand VR controller.
	// Create a gun mesh component
	VR_Gun = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("VR_Gun"));
	VR_Gun->SetOnlyOwnerSee(false);			// otherwise won't be visible in the multiplayer
	VR_Gun->bCastDynamicShadow = false;
	VR_Gun->CastShadow = false;
	VR_Gun->SetupAttachment(R_MotionController);
	VR_Gun->SetRelativeRotation(FRotator(0.0f, -90.0f, 0.0f));

	VR_MuzzleLocation = CreateDefaultSubobject<USceneComponent>(TEXT("VR_MuzzleLocation"));
	VR_MuzzleLocation->SetupAttachment(VR_Gun);
	VR_MuzzleLocation->SetRelativeLocation(FVector(0.000004, 53.999992, 10.000000));
	VR_MuzzleLocation->SetRelativeRotation(FRotator(0.0f, 90.0f, 0.0f));		// Counteract the rotation of the VR gun model.

	// Uncomment the following line to turn motion controllers on by default:
	//bUsingMotionControllers = true;
}

void AcppVectorStuffCharacter::BeginPlay()
{
	// Call the base class  
	Super::BeginPlay();

	//GEngine->AddOnScreenDebugMessage(-1, 5, FColor::Cyan, "hello world doo");

	//Attach gun mesh component to Skeleton, doing it here because the skeleton is not yet created in the constructor
	FP_Gun->AttachToComponent(Mesh1P, FAttachmentTransformRules(EAttachmentRule::SnapToTarget, true), TEXT("GripPoint"));

	// Show or hide the two versions of the gun based on whether or not we're using motion controllers.
	if (bUsingMotionControllers)
	{
		VR_Gun->SetHiddenInGame(false, true);
		Mesh1P->SetHiddenInGame(true, true);
	}
	else
	{
		VR_Gun->SetHiddenInGame(true, true);
		Mesh1P->SetHiddenInGame(false, true);
	}
	//TArray<AActor*> foundActor;
	//UGameplayStatics::GetAllActorsOfClass(GetWorld(),Asensor::, foundActor);//some more stuff to do, but usable with kinsmet include
}
//
//APlayerCameraManager *camManager = GetWorld()->GetFirstPlayerController()->PlayerCameraManager;
//FVector camLocation = camManager->GetCameraLocation();
//FVector camForward  = camManager->GetCameraRotation().Vector();
void AcppVectorStuffCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);//on tick event
	//SendTrace();only for debugging

	//
	//////compare camera vector to target using dot products/////////////////
	//FVector PlayerStart = FirstPersonCameraComponent->GetComponentLocation();
	//FVector PlayerEnd = FirstPersonCameraComponent->GetComponentLocation() + (FirstPersonCameraComponent->GetForwardVector() * 100);
	////DrawDebugLine(GetWorld(), PlayerStart, PlayerEnd, FColor::Blue);//detach from player to see, otherwise it's a single unit, less than a pixel.


	//FVector TargetV = sensor->GetActorLocation() - FirstPersonCameraComponent->GetComponentLocation();//makes vector towards target, based on player location and sensor location.
	//FVector PlayerV = FirstPersonCameraComponent->GetForwardVector();//get the forward FVector of the camera component. where ya looking.
	////DrawDebugLine(GetWorld(), FirstPersonCameraComponent->GetComponentLocation(), sensor->GetActorLocation(), FColor::Orange);// track where the sensor is
	//TargetV.Normalize();//normalize vectors for comparison.returns bool when used inline, so cant use during set

	//float VecSimilar = PlayerV | TargetV;
	////float VecSimilar = FVector::DotProduct(PlayerV, TargetV);

	//if (GEngine)
	//{		
	//	FColor PrintAligned = (VecSimilar > 0.9991f) ? FColor::Green : FColor::Red ;
	//	//0.9991 or higher is on target
	//	// IF close enough, then change the print color to green

	//	GEngine->AddOnScreenDebugMessage(-1, 0.1f, PrintAligned, FString::Printf(TEXT("Similarity to target: %f"), VecSimilar));
	//	//the %f allows the specified float to be shown in the text object.
	//}////////////////////////////////////////////////////////////


	

}
/*as used in unreal engine math function./Engine/Source/Runtime/Core/Private/Math/UnrealMath.cpp
FVector FMath::GetReflectionVector(const FVector& Direction, const FVector& SurfaceNormal)
{
	FVector SafeNormal(SurfaceNormal.GetSafeNormal());

	return Direction - 2 * (Direction | SafeNormal) * SafeNormal;
}
*/

//////////////////////////////////////////////////////////////////////////
// Input

void AcppVectorStuffCharacter::SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent)
{
	// set up gameplay key bindings
	check(PlayerInputComponent);

	// Bind jump events
	PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &ACharacter::Jump);
	PlayerInputComponent->BindAction("Jump", IE_Released, this, &ACharacter::StopJumping);

	// Bind fire event
	PlayerInputComponent->BindAction("Fire", IE_Pressed, this, &AcppVectorStuffCharacter::OnFire);

	// Enable touchscreen input
	EnableTouchscreenMovement(PlayerInputComponent);

	PlayerInputComponent->BindAction("ResetVR", IE_Pressed, this, &AcppVectorStuffCharacter::OnResetVR);

	// Bind movement events
	PlayerInputComponent->BindAxis("MoveForward", this, &AcppVectorStuffCharacter::MoveForward);
	PlayerInputComponent->BindAxis("MoveRight", this, &AcppVectorStuffCharacter::MoveRight);

	// We have 2 versions of the rotation bindings to handle different kinds of devices differently
	// "turn" handles devices that provide an absolute delta, such as a mouse.
	// "turnrate" is for devices that we choose to treat as a rate of change, such as an analog joystick
	PlayerInputComponent->BindAxis("Turn", this, &APawn::AddControllerYawInput);
	PlayerInputComponent->BindAxis("TurnRate", this, &AcppVectorStuffCharacter::TurnAtRate);
	PlayerInputComponent->BindAxis("LookUp", this, &APawn::AddControllerPitchInput);
	PlayerInputComponent->BindAxis("LookUpRate", this, &AcppVectorStuffCharacter::LookUpAtRate);
}

void ReportHit(FHitResult HitIn, int Index, float time)
{
	GEngine->AddOnScreenDebugMessage(-1, time, FColor::Emerald, FString::Printf(TEXT("Hit %i stats:"), Index));

	//report impact if hit
	GEngine->AddOnScreenDebugMessage(-1, time, FColor::Emerald, FString::Printf(TEXT("hit %i X value %f"), Index, HitIn.ImpactPoint.X));
	GEngine->AddOnScreenDebugMessage(-1, time, FColor::Emerald, FString::Printf(TEXT("hit %i Y value %f"), Index, HitIn.ImpactPoint.Y));
	GEngine->AddOnScreenDebugMessage(-1, time, FColor::Emerald, FString::Printf(TEXT("hit %i Z value %f"), Index, HitIn.ImpactPoint.Z));
}//local use only

FVector ReflectMe(FVector StartPoint, FVector ImpactPoint, FVector ImpactSurfaceNormal)
{//or as the blueprint asks: direction vector, and surface normal
	/////EQUASION///
	//OriginPoint + Vector = DestinationPoint
	//Vector = DestinationPoint - OriginPoint
	//translated to linetraces: OriginVector = (ImpactPoint - StartPoint)
	// 
	// to reflect a vector across a surface normal // http://www.sunshine2k.de/articles/coding/vectorreflection/vectorreflection.html
	//v - 2 * (v | n) * n //incoming vector = v // surface normal = n// | is dot product FVector operator
	//
	//OldVector = (HitResult.ImpactPoint - HitResult.StartPoint)
	//NewVector = OldVector - ((OldVector | HitResult.ImpactNormal) * 2 * HitResult.ImpactNormal);//reflected to form new vector

	//don't need to waste memory for a one off calculation just to make it look half it's length
	return (ImpactPoint - StartPoint) - (((ImpactPoint - StartPoint) | ImpactSurfaceNormal) * 2 * ImpactSurfaceNormal);
}//local use only

FVector ReflectedEndPointCalc(FHitResult LineTraceHitIn, float DistanceAway)
{
	//explanation
	//destination(the end of your road trip) 
	// = origin(place on the map your starting at)
	//	 + Vector(think the Direction of how to get to a destination) 
	//	   * distance(how far from the starting point to go, while following the instructions)
	// 
	////visualization
	//imagine your a blind autopilot. so you start driving from the origin. the vector gives you instructions on how to travel(a simple direction like head east), and when those instructions are combined with a distance parameter:
	// you will travel to a destination, from a beginning point, by following the instructions for a certain distance, therefore arriving at the destination
	// 
	// //mathy version
	//FVector EndPoint = (neworigin(impact) + getReflectVector(LineTraceHitIn.TraceStart, LineTraceHitIn.ImpactPoint, LineTraceHitIn.ImpactNormal) * DistanceToExtend);
	return LineTraceHitIn.ImpactPoint + ReflectMe(LineTraceHitIn.TraceStart, LineTraceHitIn.ImpactPoint, LineTraceHitIn.ImpactNormal) * DistanceAway;
}

void AcppVectorStuffCharacter::SendTrace()//callable in blueprint
{
	//GEngine->AddOnScreenDebugMessage(-1, 1.5f, FColor::Emerald, FString::Printf(TEXT("now Tracing")));//check if call works properly

	//notes
	//FHitResult *NullifyHit = &HitResult; //pointer = memory address of this thing//used to nullify if needed.
	//keep in mind, update on define, not on get(variable) call. update on call is a function feature that needs built

	//constants
	float Foward = 10000;
	FColor RayColor = FColor::Blue;
	FCollisionQueryParams CollisionParams;//uses default settings https://docs.unrealengine.com/5.3/en-US/API/Runtime/Engine/FCollisionQueryParams/
	int length = 4;//can make editor editable varaible later//reflections
	float DebugMessageTime = 10.0f;
	
	//hit storage
	FHitResult HitResult;//hit result storage https://docs.unrealengine.com/4.26/en-US/API/Runtime/Engine/Engine/FHitResult/

	//update on iteration using hits
	//inital starting values hopefully based on gun, but if not then based on camera.
	FVector StartPoint = (IsValid(FP_Gun)) ? FP_Gun->GetComponentLocation() : FirstPersonCameraComponent->GetComponentLocation();

	//inital vector to trace along. start + vectorDirection * distanceTraveled
	FVector EndPoint = StartPoint + FirstPersonCameraComponent->GetForwardVector() * Foward;


	for (size_t i = 0; i < length; i++)
	{
		DrawDebugLine(GetWorld(), StartPoint, EndPoint, RayColor, false, 3.0f);//draw something visible
		if (GetWorld()->LineTraceSingleByChannel(HitResult, StartPoint, EndPoint, ECC_Visibility, CollisionParams))
		{
			GEngine->AddOnScreenDebugMessage(-1, DebugMessageTime, FColor::Emerald, FString::Printf(TEXT("trace %i is a HIt."), i));
			ReportHit(HitResult, i, DebugMessageTime);

			//update using hit details.
			StartPoint = HitResult.ImpactPoint;
			EndPoint = ReflectedEndPointCalc(HitResult, Foward);
		}
		else { GEngine->AddOnScreenDebugMessage(-1, DebugMessageTime, FColor::Emerald, FString::Printf(TEXT("trace %i is a dud."), i)); }//if trace fails
	}
	
	//still some engine based bugs, not all surfaces will trigger on visibility layer it seems. otherwise the math works fine.
}

void AcppVectorStuffCharacter::OnFire()
{
	//SendTrace();
	// try and fire a projectile
	if (ProjectileClass != nullptr)
	{
		UWorld* const World = GetWorld();
		if (World != nullptr)
		{
			if (bUsingMotionControllers)
			{
				const FRotator SpawnRotation = VR_MuzzleLocation->GetComponentRotation();
				const FVector SpawnLocation = VR_MuzzleLocation->GetComponentLocation();
				World->SpawnActor<AcppVectorStuffProjectile>(ProjectileClass, SpawnLocation, SpawnRotation);
			}
			else
			{
				const FRotator SpawnRotation = GetControlRotation();
				// MuzzleOffset is in camera space, so transform it to world space before offsetting from the character location to find the final muzzle position
				const FVector SpawnLocation = ((FP_MuzzleLocation != nullptr) ? FP_MuzzleLocation->GetComponentLocation() : GetActorLocation()) + SpawnRotation.RotateVector(GunOffset);

				//Set Spawn Collision Handling Override
				FActorSpawnParameters ActorSpawnParams;
				ActorSpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButDontSpawnIfColliding;

				// spawn the projectile at the muzzle
				World->SpawnActor<AcppVectorStuffProjectile>(ProjectileClass, SpawnLocation, SpawnRotation, ActorSpawnParams);
			}
		}
	}

	// try and play the sound if specified
	if (FireSound != nullptr)
	{
		UGameplayStatics::PlaySoundAtLocation(this, FireSound, GetActorLocation());
	}

	// try and play a firing animation if specified
	if (FireAnimation != nullptr)
	{
		// Get the animation object for the arms mesh
		UAnimInstance* AnimInstance = Mesh1P->GetAnimInstance();
		if (AnimInstance != nullptr)
		{
			AnimInstance->Montage_Play(FireAnimation, 1.f);
		}
	}
}

void AcppVectorStuffCharacter::OnResetVR()
{
	UHeadMountedDisplayFunctionLibrary::ResetOrientationAndPosition();
}

void AcppVectorStuffCharacter::BeginTouch(const ETouchIndex::Type FingerIndex, const FVector Location)
{
	if (TouchItem.bIsPressed == true)
	{
		return;
	}
	if ((FingerIndex == TouchItem.FingerIndex) && (TouchItem.bMoved == false))
	{
		OnFire();
	}
	TouchItem.bIsPressed = true;
	TouchItem.FingerIndex = FingerIndex;
	TouchItem.Location = Location;
	TouchItem.bMoved = false;
}

void AcppVectorStuffCharacter::EndTouch(const ETouchIndex::Type FingerIndex, const FVector Location)
{
	if (TouchItem.bIsPressed == false)
	{
		return;
	}
	TouchItem.bIsPressed = false;
}

//Commenting this section out to be consistent with FPS BP template.
//This allows the user to turn without using the right virtual joystick

//void AcppVectorStuffCharacter::TouchUpdate(const ETouchIndex::Type FingerIndex, const FVector Location)
//{
//	if ((TouchItem.bIsPressed == true) && (TouchItem.FingerIndex == FingerIndex))
//	{
//		if (TouchItem.bIsPressed)
//		{
//			if (GetWorld() != nullptr)
//			{
//				UGameViewportClient* ViewportClient = GetWorld()->GetGameViewport();
//				if (ViewportClient != nullptr)
//				{
//					FVector MoveDelta = Location - TouchItem.Location;
//					FVector2D ScreenSize;
//					ViewportClient->GetViewportSize(ScreenSize);
//					FVector2D ScaledDelta = FVector2D(MoveDelta.X, MoveDelta.Y) / ScreenSize;
//					if (FMath::Abs(ScaledDelta.X) >= 4.0 / ScreenSize.X)
//					{
//						TouchItem.bMoved = true;
//						float Value = ScaledDelta.X * BaseTurnRate;
//						AddControllerYawInput(Value);
//					}
//					if (FMath::Abs(ScaledDelta.Y) >= 4.0 / ScreenSize.Y)
//					{
//						TouchItem.bMoved = true;
//						float Value = ScaledDelta.Y * BaseTurnRate;
//						AddControllerPitchInput(Value);
//					}
//					TouchItem.Location = Location;
//				}
//				TouchItem.Location = Location;
//			}
//		}
//	}
//}

void AcppVectorStuffCharacter::MoveForward(float Value)
{
	if (Value != 0.0f)
	{
		//return;
		// add movement in that direction
		AddMovementInput(GetActorForwardVector(), Value);
	}
}

void AcppVectorStuffCharacter::MoveRight(float Value)
{
	if (Value != 0.0f)
	{
		// add movement in that direction
		AddMovementInput(GetActorRightVector(), Value);
	}
}

void AcppVectorStuffCharacter::TurnAtRate(float Rate)
{
	// calculate delta for this frame from the rate information
	AddControllerYawInput(Rate * turnSpeed * BaseTurnRate * GetWorld()->GetDeltaSeconds());
}

void AcppVectorStuffCharacter::LookUpAtRate(float Rate)
{
	// calculate delta for this frame from the rate information
	AddControllerPitchInput(Rate * BaseLookUpRate * GetWorld()->GetDeltaSeconds());
}

bool AcppVectorStuffCharacter::EnableTouchscreenMovement(class UInputComponent* PlayerInputComponent)
{
	if (FPlatformMisc::SupportsTouchInput() || GetDefault<UInputSettings>()->bUseMouseForTouch)
	{
		PlayerInputComponent->BindTouch(EInputEvent::IE_Pressed, this, &AcppVectorStuffCharacter::BeginTouch);
		PlayerInputComponent->BindTouch(EInputEvent::IE_Released, this, &AcppVectorStuffCharacter::EndTouch);

		//Commenting this out to be more consistent with FPS BP template.
		//PlayerInputComponent->BindTouch(EInputEvent::IE_Repeat, this, &AcppVectorStuffCharacter::TouchUpdate);
		return true;
	}
	
	return false;
}

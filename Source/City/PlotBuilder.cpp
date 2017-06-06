// Fill out your copyright notice in the Description page of Project Settings.

#include "City.h"
#include "simplexnoise.h"
#include "PlotBuilder.h"



// Sets default values
APlotBuilder::APlotBuilder()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;
}

// Called when the game starts or when spawned
void APlotBuilder::BeginPlay()
{
	Super::BeginPlay();
	
}






TArray<FMetaPolygon> APlotBuilder::sanityCheck(TArray<FMetaPolygon> plots, TArray<FPolygon> others) {
	TArray<FMetaPolygon> added;
	for (FMetaPolygon p : plots) {
		bool shouldAdd = true;
		//for (FPolygon o : others) {
		//	if (testCollision(p, o, 2000)) {
		//		shouldAdd = false;
		//		break;
		//	}
		//}
		if (shouldAdd) {
			for (FMetaPolygon a : added) {
				if (testCollision(p, a, -1000)) {
					shouldAdd = false;
					break;
				}
			}
		}
		if (shouldAdd)
			added.Add(p);
	}
	return added;
}



FPlotInfo APlotBuilder::generateHousePolygons(FPlotPolygon p, int maxFloors, int minFloors, float noiseScale) {
	FPlotInfo info;
	TArray<FHousePolygon> housePolygons;

	float maxArea = 4500.0f;
	float minArea = 1200.0f;

	if (!p.open) {
		FHousePolygon original;
		original.points = p.points;
//		original.checkOrientation();
		//if (!p.buildLeft)
		//	original.reverse();
		original.buildLeft = true;
		original.open = false;
		original.population = p.population;
		original.type = p.type;
		for (int32 i = 1; i < original.points.Num(); i++) {
			original.entrances.Add(i);
			original.windows.Add(i);
		}
		FVector center = p.getCenter();
		p.type = raw_noise_2d((center.X + 31000000)*noiseScale, (center.Y + 3000000)*noiseScale) < 0.5 ? RoomType::office : RoomType::apartment;


		TArray<FHousePolygon> refinedPolygons = original.refine(maxArea, 0, 0);
		for (FHousePolygon r : refinedPolygons) {
			r.height = randFloat() * (maxFloors - minFloors) + minFloors;
			if (raw_noise_2d((r.housePosition.X + 1000000.0)*noiseScale*10, (r.housePosition.Y+ 1000000.0)*noiseScale * 10) > 0.7) {
				r.height *= 2;
			}
			r.type = p.type;
			r.simplePlotType = r.type == RoomType::office ? SimplePlotType::asphalt : SimplePlotType::green;

			float area = r.getArea();
			UE_LOG(LogTemp, Log, TEXT("area of new house polygon: %f"), area);

			if (area < minArea || area > maxArea) {
				FSimplePlot fs;
				fs.pol = r;
				fs.pol.reverse();
				fs.pol.offset(FVector(0, 0, 30));
				fs.type = FMath::RandBool() ? SimplePlotType::green : SimplePlotType::asphalt;
				fs.decorate();
				info.leftovers.Add(fs);

			}
			else {
				r.checkOrientation();
				housePolygons.Add(r);
				//others.Add(r);
			}
		}

	}
//	else {
//		return info;
//		// wander along the line and place adjacent houses on the curve
//		info.houses = housePolygons;
//		return info;
//
//		float minLen = 3000;
//		float minWidth = 3000;
//
//		float maxLen = 4000;
//		float maxWidth = 4000;
//
//		float acceptableLen = 8000;
//
//		int next = 1;
//		FVector prevTan = FVector(0, 0, 0);
//		FVector prev1 = FVector(0, 0, 0);
//
//		FVector currPos = p.points[0];
//		while (next < p.points.Num()) {
//			if (FVector::Dist(p.points[next], currPos) < minLen) {
//				currPos = p.points[next];
//				next++;
//				prev1 = FVector(0.0f, 0.0f, 0.0f);
//				prevTan = FVector(0.0f, 0.0f, 0.0f);
//				continue;
//			}
//			FHousePolygon fh;
//			FVector tangent1 = p.points[next] - currPos;
//			tangent1.Normalize();
//			FPolygon pol;
//			float len = 0;
//
//			bool overridePlacementOk = false;
//			if (p.points.Num() > next + 2 && FVector::Dist(p.points[next + 2], currPos) < acceptableLen){//FVector::Dist(p.points[next + 1], currPos)) {
//				pol.points.Add(currPos);
//				pol.points.Add(p.points[next]);
//				pol.points.Add(p.points[next+1]);
//				FVector target = NearestPointOnLine(p.points[next + 2], p.points[next + 2] - p.points[next + 1], currPos);
//				pol.points.Add(target);
//				pol.points.Add(currPos);
//				next += 2;
//				prev1 = target;
//				prevTan = currPos - target;
//				currPos = target;
//				overridePlacementOk = true;
//
//			}
//			else if (p.buildLeft) {
//				FVector tangent2 = FRotator(0, 90, 0).RotateVector(tangent1);
//				if (FVector::Dist(p.points[next], currPos) < acceptableLen)
//					len = FVector::Dist(p.points[next], currPos);
//				else
//					len = std::min(FVector::Dist(p.points[next], currPos), randFloat()*(maxLen - minLen) + minLen);
//				float width = randFloat()*(maxWidth - minWidth) + minWidth;
//				if (prev1.X != 0.0f) {
//					pol.points.Add(prev1 + prevTan * width);
//					pol.points.Add(prev1);
//				}
//				else {
//					pol.points.Add(currPos + width*tangent2);
//					pol.points.Add(currPos);
//				}
//				pol.points.Add(currPos + len*tangent1);
//				pol.points.Add(currPos + len*tangent1 + width*tangent2);
//				FVector first = pol.points[0];
//				pol.points.Add(first);
//			}
//			else {
//				FVector tangent2 = FRotator(0, 270, 0).RotateVector(tangent1);
//				if (FVector::Dist(p.points[next], currPos) < acceptableLen)
//					len = FVector::Dist(p.points[next], currPos);
//				else
//					len = std::min(FVector::Dist(p.points[next], currPos), randFloat()*(maxLen - minLen) + minLen);
//
//				float width = randFloat()*(maxWidth - minWidth) + minWidth;
//
//				pol.points.Add(currPos + len*tangent1 + width*tangent2);
//				pol.points.Add(currPos + len*tangent1);
//				if (prev1.X != 0.0f) {
//					pol.points.Add(prev1);
//					pol.points.Add(prev1 + prevTan * width);
//				}
//				else {
//					pol.points.Add(currPos);
//					pol.points.Add(currPos + width*tangent2);
//				}
//
//				FVector first = pol.points[0];
//				pol.points.Add(first);
//			}
//			FPolygon tmp;
//			
// 			//if (overridePlacementOk || !testCollision(pol, others, 500, tmp)) {
//				fh.points = pol.points;
//				fh.checkOrientation();
//				fh.population = 1.0;
//				fh.height = randFloat() * (maxFloors - minFloors) + minFloors;
//				fh.housePosition = pol.getCenter();
//				fh.type = raw_noise_2d(fh.housePosition.X, fh.housePosition.Y) < 0 ? RoomType::office : RoomType::apartment;//randFloat() < 0.5 ? RoomType::office : RoomType::apartment;
//
//				fh.entrances.Add(2);
//				for (int i = 1; i < fh.points.Num(); i++) {
//					fh.windows.Add(i);
//				}
////				others.Add(fh);
//				housePolygons.Add(fh);
//				FVector tangent = pol.points[2] - pol.points[1];
//				tangent.Normalize();
//				prev1 = pol.points[2] + tangent;
//				prevTan = pol.points[3] - pol.points[2];
//				prevTan.Normalize();
//
//			//}
//
//
//			currPos += len*tangent1;
//
//		}
//
//	}
	info.houses = housePolygons;
	return info;

}

FPolygon APlotBuilder::generateSidewalkPolygon(FPlotPolygon p, float offsetSize) {
	FPolygon polygon;
	if (p.points.Num() > 2) {
		FVector center = p.getCenter();
		for (int i = 1; i < p.points.Num(); i++) {
			FVector tangent = p.points[i] - p.points[i - 1];
			tangent.Normalize();
			FVector offset = (p.buildLeft ? FRotator(0, 270, 0) : FRotator(0, 90, 0)).RotateVector(tangent * offsetSize);
			polygon.points.Add(p.points[i - 1] + offset);
			polygon.points.Add(p.points[i] + offset);
		}
		//if (FVector::Dist(p.points[p.points.Num() - 1], p.points[p.points.Num() - 2]) > 100.0f) {
		//	polygon.points.Add(p.points[p.points.Num() - 1]);
		//}

		if (!p.open) {
			//polygon.points.RemoveAt(polygon.points.Num() - 1);
			polygon.points.Add(FVector(polygon.points[0]));
			polygon.points.Add(FVector(polygon.points[1]));

			//polygon.points.Add(FVector(polygon.points[2]));


		}
		else {
			FVector last = p.points[p.points.Num() - 1];
			polygon.points.Add(last);
			//polygon.points.Add(last);

		}
	}
	return polygon;
}

FSidewalkInfo APlotBuilder::getSideWalkInfo(FPolygon sidewalk)
{
	FSidewalkInfo toReturn;
	float placeRatio = 0.001;
	if (FMath::FRand() < 0.1f) {
		for (int i = 1; i < sidewalk.points.Num(); i += 2) {
			int toPlace = placeRatio * (sidewalk.points[i] - sidewalk.points[i - 1]).Size();
			for (int j = 1; j < toPlace; j++) {
				FVector origin = sidewalk.points[i - 1];
				FVector target = sidewalk.points[i];
				FVector tan = target - origin;
				float len = tan.Size();
				tan.Normalize();
				toReturn.staticMeshes.Add(FMeshInfo{ "tree", FTransform(origin + j * tan * (len / toPlace)) });
			}
		}
	}


	return toReturn;
}

TArray<FMaterialPolygon> APlotBuilder::getSimplePlotPolygons(TArray<FSimplePlot> plots) {
	TArray<FMaterialPolygon> toReturn;
	PolygonType type;
	if (plots.Num() > 0)
		type = plots[0].type == SimplePlotType::asphalt ? PolygonType::concrete : PolygonType::green;
	for (FSimplePlot p : plots) {
		FMaterialPolygon newP;
		newP.points = p.pol.points;
		newP.type = type;// simplePolygonType;//p.type == SimplePlotType::asphalt ? PolygonType::concrete : PolygonType::green;;
		toReturn.Add(newP);

	}
	return toReturn;
}



// Called every frame
void APlotBuilder::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}


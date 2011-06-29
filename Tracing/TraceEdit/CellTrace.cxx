/*=========================================================================
Copyright 2009 Rensselaer Polytechnic Institute
Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License. 
=========================================================================*/

#include "TraceBit.h"
#include "TraceLine.h"
#include "CellTrace.h"
CellTrace::CellTrace()
{
	this->clearAll();
}
CellTrace::CellTrace(std::vector<TraceLine*> Segments)
{
	this->clearAll();
	this->setTraces(Segments);
}
void CellTrace::setTraces(std::vector<TraceLine*> Segments)
{
	this->segments = Segments;
	unsigned int i = 0;
	this->NumSegments = (int) this->segments.size();
	this->stems = (int) this->segments[0]->GetBranchPointer()->size();
	TraceBit rootBit = this->segments[0]->GetTraceBitsPointer()->front();
	//set the soma point as the intital bounds
	this->somaX = rootBit.x;
	this->somaY = rootBit.y;
	this->somaZ = rootBit.z;
	this->maxX = rootBit.x;
	this->maxY = rootBit.y;
	this->maxZ = rootBit.z;
	this->minX = rootBit.x;
	this->minY = rootBit.y;
	this->minZ = rootBit.z;
	this->somaSurface = this->segments[0]->GetSurfaceArea();
	this->somaVolume = this->segments[0]->GetVolume();
	this->SomaRadii = this->segments[0]->GetRadii();

	for(i = 0; i < this->segments.size(); i++)
	{
		this->IDs.insert(this->segments[i]->GetId());
		this->MaxMin(this->segments[i]->GetLength(), this->TotalPathLength, this->minPathLength, this->MaxPathLength);
		this->MaxMin(this->segments[i]->GetVolume(), this->TotalVolume, this->minSegmentVolume, this->maxSegmentVolume);
		this->MaxMin(this->segments[i]->GetEuclidianLength(),this->TotalEuclidianPath, this->MinEuclidianPath, this->MaxEuclidianPath);
		this->MaxMin(this->segments[i]->GetSurfaceArea(), this->surfaceAreaTotal, this->SurfaceAreaMin, this->SurfaceAreaMax);
		this->MaxMin(this->segments[i]->GetSectionArea(), this->sectionAreaTotal, this->SectionAreaMin, this->SectionAreaMax);
		this->MaxMin(this->segments[i]->GetSize(), this->TotalFragmentation, this->MinFragmentation, this->MaxFragmentation);
		this->MaxMin(this->segments[i]->GetBurkTaper(), this->TotalBurkTaper, this->MinBurkTaper, this->MaxBurkTaper);
		this->MaxMin(this->segments[i]->GetHillmanTaper(), this->TotalHillmanTaper, this->MinHillmanTaper, this->MaxHillmanTaper);

		double diam = 2*this->segments[i]->GetRadii();
		this->MaxMin(diam, this->TotalDiameter, this->MinDiameter, this->MaxDiameter);
		double diamPower = pow(diam,1.5);
		this->MaxMin(diamPower, this->TotalDiameterPower, this->MinDiameterPower, this->MaxDiameterPower);

		this->MaxMin(this->segments[i]->GetFragmentationSmoothness(), this->TotalContraction, this->MinContraction, this->MaxContraction);
		int tempLevel = this->segments[i]->GetLevel();
		if (this->segments[i]->isLeaf())
		{
			this->terminalTips++;

			TraceBit leafBit = this->segments[i]->GetTraceBitsPointer()->back();
			TraceBit leadBit = this->segments[i]->GetTraceBitsPointer()->front();
			this->MaxMin(this->segments[i]->GetSize(), this->TotalTerminalSegment, this->MinTerminalSegment, this->MaxTerminalSegment);
			if(!this->segments[i]->isRoot())
			{
				TraceBit parentBit = this->segments[i]->GetParent()->GetTraceBitsPointer()->back();
				double parentDiam = 2*parentBit.r;
				this->TotalLastParentDiam += parentDiam;
				if (this->LastParentDiamMax < parentDiam)
				{
					this->LastParentDiamMax = parentDiam;
				}else if(this->LastParentDiamMin > parentDiam)
				{
					this->LastParentDiamMin = parentDiam;
				}
			}//fails if non branching tree
			double DiamThreshold = 2*leadBit.r;
			this->DiamThresholdTotal += DiamThreshold;
			if (this->DiamThresholdMin > DiamThreshold)
			{
				this->DiamThresholdMin = DiamThreshold;
			}else if(this->DiamThresholdMax < DiamThreshold)
			{
				this->DiamThresholdMax = DiamThreshold;
			}
			float lx = leafBit.x;
			float ly = leafBit.y;
			float lz = leafBit.z;
			float total; //just so things work
			this->MaxMin(lx, total, this->minX, this->maxX);
			this->MaxMin(ly, total, this->minY, this->maxY);
			this->MaxMin(lz, total, this->minZ, this->maxZ);
			this->MaxMin(tempLevel, this->SumTerminalLevel, this->MinTerminalLevel, this->MaxTerminalLevel);
			this->MaxMin(this->segments[i]->GetPathLength(), this->TerminalPathLength, this->minTerminalPathLength, this->maxTerminalPathLength);
			
		}//end if leaf
		else //if(!this->segments[i]->isRoot())
		{
			this->branchPoints++;
			this->MaxMin(this->segments[i]->GetpartitionAsymmetry(), this->partitionAsymmetry, this->partitionAsymmetryMin, this->partitionAsymmetryMax);
			this->MaxMin(this->segments[i]->GetdaughterRatio(), this->daughterRatio, this->daughterRatioMin, this->daughterRatioMax);
			this->MaxMin(this->segments[i]->GetparentDaughterRatio(), this->parentDaughterRatio, this->parentDaughterRatioMin, this->parentDaughterRatioMax);
			this->MaxMin(this->segments[i]->GetRallPower(), this->rallPower, this->rallPowerMin, this->rallPowerMax);
			this->MaxMin(this->segments[i]->GetPk(), this->Pk, this->PkMin, this->PkMax);
			this->MaxMin(this->segments[i]->GetPk_2(), this->Pk_2, this->Pk_2Min, this->Pk_2Max);
			this->MaxMin(this->segments[i]->GetPk_classic(), this->Pk_classic, this->Pk_classicMin, this->Pk_classicMax);
			this->MaxMin(this->segments[i]->GetBifAmplLocal(), this->BifAmplLocal, this->BifAmplLocalMin, this->BifAmplLocalMax);
			this->MaxMin(this->segments[i]->GetBifAmpRemote(), this->BifAmpRemote, this->BifAmpRemoteMin, this->BifAmpRemoteMax);
			this->MaxMin(this->segments[i]->GetBranch1()->GetBifTiltLocal(), this->BifTiltLocal, this->BifTiltLocalMin, this->BifTiltLocalMax);
			this->MaxMin(this->segments[i]->GetBranch2()->GetBifTiltRemote(), this->BifTiltRemote, this->BifTiltRemoteMin, this->BifTiltRemoteMax);
			if (this->segments[i]->GetTerminalDegree() ==2)
			{
				this->terminalBifCount++;
				this->MaxMin(this->segments[i]->GetHillmanThreshold(), this->TotalHillmanThresh, this->MinHillmanThresh, this->MaxHillmanThresh);
			}
		}//end bifurcation features
	}//end for segment size
}
void CellTrace::setFileName(std::string newFileName)
{
	this->FileName = newFileName;
}
std::string CellTrace::GetFileName()
{
	std::string outputFileName;
	if (this->FileName != "file")
	{
		outputFileName = this->FileName;
	}
	else
	{
		std::stringstream newCellName; 
		newCellName<<"cell" << this->segments[0]->GetId();
		outputFileName = newCellName.str();
	}
	return outputFileName;
}
void CellTrace::getSomaCoord(double xyz[])
{
	xyz[0] = this->somaX;
	xyz[1] = this->somaY;
	xyz[2] = this->somaZ;
}
void CellTrace::getCellBounds(double bounds[])
{//min then max of x, y, z
	bounds[0] = this->minX;
	bounds[1] = this->maxX;
	bounds[2] = this->minY;
	bounds[3] = this->maxY;
	bounds[4] = this->minZ;
	bounds[5] = this->maxZ;
}
void CellTrace::clearAll()
{
	this->segments.clear();
	this->NumSegments = 0;
	this->stems = 0;
	this->branchPoints = 0;
	this->terminalTips = 0;

	this->MinTerminalLevel = 100; //something large for initial value
	this->MaxTerminalLevel = 0;
	this->SumTerminalLevel = 0;

	this->minTerminalPathLength = 100;
	this->maxTerminalPathLength=0;
	this->TerminalPathLength = 0;

	this->TotalFragmentation = 0;
	this->MinFragmentation = 100;
	this->MaxFragmentation = 0;

	this->TotalHillmanTaper = 0;
	this->MinHillmanTaper = 100;
	this->MaxHillmanTaper = 0;

	this->TotalBurkTaper = 0;
	this->MinBurkTaper = 100;
	this->MaxBurkTaper = 0;

	this->TotalHillmanThresh = 0;
	this->MinHillmanThresh = 100;
	this->MaxHillmanThresh = 0;
	this->terminalBifCount = 0;

	this->TotalContraction = 0;
	this->MinContraction = 100;
	this->MaxContraction = 0;

	this->TotalDiameter = 0;
	this->MinDiameter = 100;
	this->MaxDiameter = 0;

	this->TotalDiameterPower = 0;
	this->MinDiameterPower = 100;
	this->MaxDiameterPower = 0;

	this->TotalEuclidianPath = 0;
	this->MinEuclidianPath = 1000;
	this->MaxEuclidianPath = 0;

	this->TotalPathLength = 0;
	this->minPathLength = 1000;
	this->MaxPathLength = 0;

	this->TotalTerminalSegment = 0;
	this->MinTerminalSegment = 100;
	this->MaxTerminalSegment = 0;

	this->TotalVolume = 0;
	this->maxSegmentVolume = 0;
	this->minSegmentVolume = 1000;

	this->FileName = "file";

	this->somaX = 0;
	this->somaY = 0;
	this->somaZ = 0;

	this->maxX = 0;
	this->maxY = 0;
	this->maxZ = 0;

	this->minX = 100;
	this->minY = 100;
	this->minZ = 100;

	this->sectionAreaTotal = 0;
	this->SectionAreaMax = 0;
	this->SectionAreaMin = 100;

	this->surfaceAreaTotal = 0;
	this->SurfaceAreaMax = 0;
	this->SurfaceAreaMin = 100;

	this->DiamThresholdTotal = 0;
	this->DiamThresholdMax = 0;
	this->DiamThresholdMin = 100;

	this->TotalLastParentDiam = 0;
	this->LastParentDiamMax = 0;
	this->LastParentDiamMin = 100;

	this->somaVolume= 0;
	this->somaSurface = 0;
	this->SomaRadii = 0;

	this->daughterRatio = 0;
	this->daughterRatioMin = 100;
	this->daughterRatioMax = 0;

	this->parentDaughterRatio = 0;
	this->parentDaughterRatioMin = 100;
	this->parentDaughterRatioMax = 0;

	this->partitionAsymmetry = 0;
	this->partitionAsymmetryMin = 100;
	this->partitionAsymmetryMax = 0;

	this->rallPower = 0;
	this->rallPowerMin = 100;
	this->rallPowerMax = 0;

	this->Pk = 0;
	this->PkMin = 100;
	this->PkMax = 0;

	this->Pk_2 = 0;
	this->Pk_2Min = 100;
	this->Pk_2Max = 0;

	this->Pk_classic = 0;
	this->Pk_classicMin = 100;
	this->Pk_classicMax = 0;

	this->BifAmplLocal = 0;
	this->BifAmplLocalMin = 180;
	this->BifAmplLocalMax = 0;

	this->BifAmpRemote = 0;
	this->BifAmpRemoteMin = 180;
	this->BifAmpRemoteMax = 0;

	this->BifTiltLocal = 0;
	this->BifTiltLocalMin = 180;
	this->BifTiltLocalMax = 0;

	this->BifTiltRemote = 0;
	this->BifTiltRemoteMin = 180;
	this->BifTiltRemoteMax = 0;
}
void CellTrace::MaxMin(double NewValue, double &total, double &Min, double &Max)
{
	if (NewValue != -1)
	{
		total += NewValue;
		if (NewValue > Max)
		{
			Max = NewValue;
		}
		if (NewValue < Min)
		{
			Min = NewValue;
		}
	}
}
void CellTrace::MaxMin(float NewValue, float &total, float &Min, float &Max)
{
	if (NewValue != -1)
	{
		total += NewValue;
		if (NewValue > Max)
		{
			Max = NewValue;
		}
		if (NewValue < Min)
		{
			Min = NewValue;
		}
	}
}
void CellTrace::MaxMin(int NewValue, int &total, int &Min, int &Max)
{
	if (NewValue != -1)
	{
		total += NewValue;
		if (NewValue > Max)
		{
			Max = NewValue;
		}
		if (NewValue < Min)
		{
			Min = NewValue;
		}
	}
}
vtkSmartPointer<vtkVariantArray> CellTrace::DataRow()
{
	vtkSmartPointer<vtkVariantArray> CellData = vtkSmartPointer<vtkVariantArray>::New();
	CellData->InsertNextValue(this->segments[0]->GetId());
	CellData->InsertNextValue(this->NumSegments);
	CellData->InsertNextValue(this->stems);
	CellData->InsertNextValue(this->branchPoints);
	CellData->InsertNextValue(this->terminalTips);
	if (this->branchPoints == 0) 
	{
		this->branchPoints = 1;
	}//protect from divide by zero

	CellData->InsertNextValue(this->MinTerminalLevel);
	CellData->InsertNextValue(this->minTerminalPathLength);
	CellData->InsertNextValue(this->MaxTerminalLevel);
	CellData->InsertNextValue(this->maxTerminalPathLength);
	CellData->InsertNextValue(this->SumTerminalLevel /this->terminalTips);//average terminal level
	CellData->InsertNextValue(this->TerminalPathLength/this->terminalTips);//now average path to end
	CellData->InsertNextValue(this->TotalEuclidianPath);
	CellData->InsertNextValue(this->TotalEuclidianPath/this->NumSegments);//average segment euclidian length
	CellData->InsertNextValue(this->TotalPathLength);
	CellData->InsertNextValue(this->TotalPathLength/this->NumSegments);//average segment length

	CellData->InsertNextValue(this->TotalFragmentation);
	CellData->InsertNextValue(this->MinFragmentation);
	CellData->InsertNextValue(this->TotalFragmentation / this->NumSegments);
	CellData->InsertNextValue(this->MaxFragmentation);

	//CellData->InsertNextValue(this->TotalBurkTaper);
	CellData->InsertNextValue(this->MinBurkTaper);
	CellData->InsertNextValue(this->TotalBurkTaper / this->NumSegments);
	CellData->InsertNextValue(this->MaxBurkTaper);

	//CellData->InsertNextValue(this->TotalHillmanTaper);
	CellData->InsertNextValue(this->MinHillmanTaper);
	CellData->InsertNextValue(this->TotalHillmanTaper / this->NumSegments);
	CellData->InsertNextValue(this->MaxBurkTaper);

	CellData->InsertNextValue(this->MinContraction);
	CellData->InsertNextValue(this->TotalContraction / this->NumSegments);
	CellData->InsertNextValue(this->MaxContraction);

	CellData->InsertNextValue(this->MinDiameter);
	CellData->InsertNextValue(this->TotalDiameter / this->NumSegments);
	CellData->InsertNextValue(this->MaxDiameter);

	CellData->InsertNextValue(this->MinDiameterPower);
	CellData->InsertNextValue(this->TotalDiameterPower / this->NumSegments);
	CellData->InsertNextValue(this->MaxDiameterPower);

	CellData->InsertNextValue(this->DiamThresholdTotal/this->terminalTips);
	CellData->InsertNextValue(this->DiamThresholdMax);
	CellData->InsertNextValue(this->DiamThresholdMin);
	CellData->InsertNextValue(this->TotalLastParentDiam/this->terminalTips);
	CellData->InsertNextValue(this->LastParentDiamMax);
	CellData->InsertNextValue(this->LastParentDiamMin);

	CellData->InsertNextValue(this->TotalTerminalSegment);
	CellData->InsertNextValue(this->MinTerminalSegment);
	CellData->InsertNextValue(this->TotalTerminalSegment / this->terminalTips); //average
	CellData->InsertNextValue(this->MaxTerminalSegment);

	CellData->InsertNextValue(this->TotalVolume);
	CellData->InsertNextValue(this->TotalVolume/this->NumSegments);////average segment Volume
	CellData->InsertNextValue(this->minSegmentVolume);
	CellData->InsertNextValue(this->maxSegmentVolume);
	CellData->InsertNextValue(this->surfaceAreaTotal);
	CellData->InsertNextValue(this->SurfaceAreaMax);
	CellData->InsertNextValue(this->surfaceAreaTotal/this->NumSegments);
	CellData->InsertNextValue(this->SurfaceAreaMin);

	CellData->InsertNextValue(this->BifAmplLocal / this->branchPoints);
	CellData->InsertNextValue(this->BifAmplLocalMin);
	CellData->InsertNextValue(this->BifAmplLocalMax);
	/*CellData->InsertNextValue(this->BifTiltLocal/ this->branchPoints);
	CellData->InsertNextValue(this->BifTiltLocalMin);
	CellData->InsertNextValue(this->BifTiltLocalMax);*/

	CellData->InsertNextValue(this->BifAmpRemote / this->branchPoints);
	CellData->InsertNextValue(this->BifAmpRemoteMin);
	CellData->InsertNextValue(this->BifAmpRemoteMax);

	/*CellData->InsertNextValue(this->BifTiltRemote / this->branchPoints);
	CellData->InsertNextValue(this->BifTiltRemoteMin);
	CellData->InsertNextValue(this->BifTiltRemoteMax);*/

	CellData->InsertNextValue(this->daughterRatioMin);
	CellData->InsertNextValue(this->daughterRatio / this->branchPoints);
	CellData->InsertNextValue(this->daughterRatioMax);

	CellData->InsertNextValue(this->parentDaughterRatioMin);
	CellData->InsertNextValue(this->parentDaughterRatio/ this->branchPoints);
	CellData->InsertNextValue(this->parentDaughterRatioMax);

	CellData->InsertNextValue(this->partitionAsymmetryMin);
	CellData->InsertNextValue(this->partitionAsymmetry / this->branchPoints);
	CellData->InsertNextValue(this->partitionAsymmetryMax);

	CellData->InsertNextValue(this->MinHillmanThresh); 
	CellData->InsertNextValue(this->TotalHillmanThresh/ this->terminalBifCount);
	CellData->InsertNextValue(this->MaxHillmanThresh);

	CellData->InsertNextValue(this->rallPowerMin);
	CellData->InsertNextValue(this->rallPower / this->branchPoints);
	CellData->InsertNextValue(this->rallPowerMax);

	CellData->InsertNextValue(this->PkMin);
	CellData->InsertNextValue(this->Pk / this->branchPoints);
	CellData->InsertNextValue(this->PkMax);

	CellData->InsertNextValue(this->Pk_classicMin);
	CellData->InsertNextValue(this->Pk_classic / this->branchPoints);
	CellData->InsertNextValue(this->Pk_classicMax);

	CellData->InsertNextValue(this->Pk_2Min);
	CellData->InsertNextValue(this->Pk_2 / this->branchPoints);
	CellData->InsertNextValue(this->Pk_2Max);

	CellData->InsertNextValue(this->maxX - this->minX);//Width
	CellData->InsertNextValue(this->maxY - this->minY);//Length
	CellData->InsertNextValue(this->maxZ - this->minZ);//Height

	CellData->InsertNextValue(this->somaX);
	CellData->InsertNextValue(this->somaY);
	CellData->InsertNextValue(this->somaZ);
	CellData->InsertNextValue(this->SomaRadii);
	CellData->InsertNextValue(this->somaVolume);
	CellData->InsertNextValue(this->somaSurface);
	CellData->InsertNextValue(this->GetFileName().c_str());
	//std::cout << this->FileName << std::endl;
	return CellData;
}
vtkSmartPointer<vtkVariantArray> CellTrace::BoundsRow()
{
	// id, somaX somaY somaZ min/max xyz
	vtkSmartPointer<vtkVariantArray> CellBoundsRow = vtkSmartPointer<vtkVariantArray>::New();
	CellBoundsRow->InsertNextValue(this->segments[0]->GetId());
	CellBoundsRow->InsertNextValue(this->somaX);
	CellBoundsRow->InsertNextValue(this->somaY);
	CellBoundsRow->InsertNextValue(this->somaZ);
	CellBoundsRow->InsertNextValue(this->minX);
	CellBoundsRow->InsertNextValue(this->maxX);
	CellBoundsRow->InsertNextValue(this->minY);
	CellBoundsRow->InsertNextValue(this->maxY);
	CellBoundsRow->InsertNextValue(this->minZ);
	CellBoundsRow->InsertNextValue(this->maxZ);
	return CellBoundsRow;
}
std::string CellTrace::BasicFeatureString()
{
	std::stringstream features;
	features << this->GetFileName().c_str() << "\t";
	features << this->somaX << "\t";
	features << this->somaY << "\t";
	features << this->somaZ << "\t";
	return features.str();
}
std::set<long int> CellTrace::TraceIDsInCell()
{
	return this->IDs;
}
int CellTrace::rootID()
{
	return this->segments[0]->GetId();
}

TraceLine * CellTrace::getRootTrace()
{
	return this->segments.front();
}